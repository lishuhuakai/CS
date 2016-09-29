#include <muduo/base/AsyncLogging.h>
#include <muduo/base/LogFile.h>
#include <muduo/base/Timestamp.h>

#include <stdio.h>

using namespace muduo;

AsyncLogging::AsyncLogging(const string& basename,
	size_t rollSize, // rollSize表示的是回滚的大小吗？
	int flushInterval)
	: flushInterval_(flushInterval)
	, running_(false)
	, basename_(basename)
	, rollSize_(rollSize)
	, thread_(boost::bind(&AsyncLogging::threadFunc, this), "Logging") // 好吧，貌似构建了一个线程用于记录
	, latch_(1)
	, mutex_()
	, cond_(mutex_)
	, currentBuffer_(new Buffer)
	, nextBuffer_(new Buffer) // 也就是说，一共有两个buffer
	, buffers_()
{
	currentBuffer_->bzero();
	nextBuffer_->bzero();
	buffers_.reserve(16); // buffers_是什么类型的变量，好吧，查了一下，居然是bufferVector类型的变量也就是存放bufferPtr类型指针的vector
}

void AsyncLogging::append(const char* logline, int len) // 往后面添加日志，是吧!
{
	muduo::MutexLockGuard lock(mutex_); // 先加锁
	if (currentBuffer_->avail() > len) // 如果当前的缓存块有足够的空间的话
	{
		currentBuffer_->append(logline, len);
	}
	else // 也就是说，当前的缓存块没有足够的空间
	{
		buffers_.push_back(currentBuffer_.release()); // buffers_用于存放写满了的buffer

		if (nextBuffer_) // 如果备用的缓存块可用
		{
			currentBuffer_ = boost::ptr_container::move(nextBuffer_); // 将其作为当前的缓存块
		}
		else // 实在没办法的话，分配新的缓存
		{
			currentBuffer_.reset(new Buffer); // Rarely happens
		}
		currentBuffer_->append(logline, len);
		cond_.notify(); // 通知后台写进程，可以写啦！
	}
}

void AsyncLogging::threadFunc() // 好吧，其实这个线程就是用来记录日志的
{
	// 其实相当于写者
	assert(running_ == true);
	latch_.countDown();
	LogFile output(basename_, rollSize_, false); // 其实就是构建一个文件啦
	BufferPtr newBuffer1(new Buffer); // 分别指向两个Buffer
	BufferPtr newBuffer2(new Buffer);
	newBuffer1->bzero(); // 清零
	newBuffer2->bzero();
	BufferVector buffersToWrite; // 一个新的BufferVector
	buffersToWrite.reserve(16);
	while (running_)
	{
		assert(newBuffer1 && newBuffer1->length() == 0);
		assert(newBuffer2 && newBuffer2->length() == 0);
		assert(buffersToWrite.empty());

		{
			muduo::MutexLockGuard lock(mutex_); // 首先是加锁
			if (buffers_.empty())  // unusual usage!
			{
				// 如果buffers_为空，注意buffers_是这个类的成员变量，实际上是一个vector类型
				cond_.waitForSeconds(flushInterval_); // 等待flushInterval_秒，也就是先等待前端先写
			}
			buffers_.push_back(currentBuffer_.release()); // 先将currentBuffer指向的buffer放入buffers_,一般来说运行到了这一步的话，currentBuffer指向的内存块
			// 已经被写入了数据
			currentBuffer_ = boost::ptr_container::move(newBuffer1); // currentBuffer指向备用的newBuffer1
			buffersToWrite.swap(buffers_); // 两个vector交换数据，这样的话，buffers就可以被别人使用啦
			if (!nextBuffer_)
			{
				nextBuffer_ = boost::ptr_container::move(newBuffer2); // nextBuffer指向新的buffer
			}
		} // 不得不说，这个锁加的恰如其分,一般来说，锁的范围是越小越好

		assert(!buffersToWrite.empty()); // 

		if (buffersToWrite.size() > 25) // 要写入的缓存块的个数太多，也就是说出现问题了
		{
			char buf[256];
			snprintf(buf,
				sizeof buf,
				"Dropped log messages at %s, %zd larger buffers\n",
				Timestamp::now().toFormattedString().c_str(),
				buffersToWrite.size()-2); // 也就是说，如果buffer过多，也就是说可能出现异常了
			fputs(buf, stderr); // 好吧，出错了
			output.append(buf, static_cast<int>(strlen(buf)));
			buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());
		}

		for (size_t i = 0; i < buffersToWrite.size(); ++i)
		{
		  // FIXME: use unbuffered stdio FILE ? or use ::writev ?
			output.append(buffersToWrite[i].data(), buffersToWrite[i].length());
		}

		if (buffersToWrite.size() > 2) // 因为前面已经写入文件完成了，所以这里的话，要释放内存
		{
		  // drop non-bzero-ed buffers, avoid trashing
			buffersToWrite.resize(2);
		}

		if (!newBuffer1)
		{
			assert(!buffersToWrite.empty());
			newBuffer1 = buffersToWrite.pop_back(); // 好吧，我知道这里是节约内存的想法
			newBuffer1->reset(); // 释放不必要的内存
		}

		if (!newBuffer2)
		{
			assert(!buffersToWrite.empty());
			newBuffer2 = buffersToWrite.pop_back();
			newBuffer2->reset();
		}

		buffersToWrite.clear();
		output.flush(); // 刷新，也就是写入
	}
	output.flush();
}

