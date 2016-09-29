// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include <muduo/base/ThreadPool.h>

#include <muduo/base/Exception.h>

#include <boost/bind.hpp>
#include <assert.h>
#include <stdio.h>

using namespace muduo;

ThreadPool::ThreadPool(const string& nameArg)
	: mutex_()
	, notEmpty_(mutex_)
	, notFull_(mutex_)
	, name_(nameArg)
	, maxQueueSize_(0)
	, running_(false)
{
}

ThreadPool::~ThreadPool()
{
	if (running_)
	{
		stop();
	}
}

void ThreadPool::start(int numThreads)
{
	assert(threads_.empty()); // 首先线程池为空
	running_ = true; // 表示现在正在运行啦
	threads_.reserve(numThreads);
	for (int i = 0; i < numThreads; ++i)
	{
		char id[32];
		snprintf(id, sizeof id, "%d", i + 1);
		threads_.push_back(new muduo::Thread(
		      boost::bind(&ThreadPool::runInThread, this), // 好吧，线程都调用runInThread函数
			name_+id));
		threads_[i].start(); // 好吧，我看到了，的确是开启线程
	}
	if (numThreads == 0 && threadInitCallback_)
	{
		threadInitCallback_();
	}
}

void ThreadPool::stop()
{
	{
		MutexLockGuard lock(mutex_);
		running_ = false;
		notEmpty_.notifyAll();
	}
	for_each(threads_.begin(),
		threads_.end(),
		boost::bind(&muduo::Thread::join, _1));
}

size_t ThreadPool::queueSize() const
{
	MutexLockGuard lock(mutex_);
	return queue_.size();
}

void ThreadPool::run(const Task& task)
{
	if (threads_.empty()) // 判断线程池是否为空
	{
		task();
	}
	else
	{
		MutexLockGuard lock(mutex_);
		while (isFull())
		{
			notFull_.wait();
		}
		assert(!isFull());

		queue_.push_back(task); // queue_里面是装任务的是吧！
		notEmpty_.notify();
	}
}

#ifdef __GXX_EXPERIMENTAL_CXX0X__
void ThreadPool::run(Task&& task)
{
	if (threads_.empty())
	{
		task();
	}
	else
	{
		MutexLockGuard lock(mutex_); // 加锁
		while (isFull())
		{
			notFull_.wait();
		}
		assert(!isFull());

		queue_.push_back(std::move(task));
		notEmpty_.notify();
	}
}
#endif

ThreadPool::Task ThreadPool::take()
{
	// 话说，take貌似是消费者进程应该调用的函数，是吧！
	MutexLockGuard lock(mutex_);
	// always use a while-loop, due to spurious wakeup
	while (queue_.empty() && running_)
	{
		notEmpty_.wait(); // 如果为空的话，就要等待
	}
	Task task;
	if (!queue_.empty()) // 如果任务queue_不为空
	{
		task = queue_.front();
		queue_.pop_front();
		if (maxQueueSize_ > 0)
		{
			notFull_.notify(); // 通知生产者，不满了,可以继续往queue_中放入task了
		}
	}
	return task;
}

bool ThreadPool::isFull() const
{
	mutex_.assertLocked();
	// 我这里要说一声的是,如果maxQueueSize_ == 0,也就是说对queue_的大小没有任何限制
	return maxQueueSize_ > 0 && queue_.size() >= maxQueueSize_;
}

void ThreadPool::runInThread() // 好吧，start函数中启动的线程就是在调用这个函数
{
	try
	{
		if (threadInitCallback_) // 这是一个指针吧
		{
			threadInitCallback_(); // 好吧，先运行这个init函数
		}
		while (running_)
		{
			Task task(take()); // 取出一个任务
			if (task)
			{
				task(); // 执行任务
			}
		}
	}
	catch (const Exception& ex)
	{
		fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
		fprintf(stderr, "reason: %s\n", ex.what());
		fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
		abort();
	}
	catch (const std::exception& ex)
	{
		fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
		fprintf(stderr, "reason: %s\n", ex.what());
		abort();
	}
	catch (...)
	{
		fprintf(stderr, "unknown exception caught in ThreadPool %s\n", name_.c_str());
		throw; // rethrow
	}
}

