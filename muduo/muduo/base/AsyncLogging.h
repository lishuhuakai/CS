#ifndef MUDUO_BASE_ASYNCLOGGING_H
#define MUDUO_BASE_ASYNCLOGGING_H

#include <muduo/base/BlockingQueue.h>
#include <muduo/base/BoundedBlockingQueue.h>
#include <muduo/base/CountDownLatch.h>
#include <muduo/base/Mutex.h>
#include <muduo/base/Thread.h>

#include <muduo/base/LogStream.h>

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace muduo
{

	class AsyncLogging : boost::noncopyable // 好吧，异步的日志库
	{
	public:

		AsyncLogging(const string& basename,
			size_t rollSize,
			int flushInterval = 3);

		~AsyncLogging()
		{
			if (running_)
			{
				stop();
			}
		}

		void append(const char* logline, int len);

		void start()
		{
			running_ = true; // 表明开始运行啦！
			thread_.start(); 
			latch_.wait();
		}

		void stop()
		{
			running_ = false; 
			cond_.notify(); // 通知
			thread_.join(); // 等待线程退出么？
		}

	private:

	  // declare but not define, prevent compiler-synthesized functions
		AsyncLogging(const AsyncLogging&);  // ptr_container
		void operator=(const AsyncLogging&);  // ptr_container

		void threadFunc();

		typedef muduo::detail::FixedBuffer<muduo::detail::kLargeBuffer> Buffer; // 好吧，又看见Buffer了！
		typedef boost::ptr_vector<Buffer> BufferVector; // 好吧，日志还能串成一串
		typedef BufferVector::auto_type BufferPtr; // 这种指针类型还是挺不错的

		const int flushInterval_;
		bool running_;
		string basename_;
		size_t rollSize_;
		muduo::Thread thread_;
		muduo::CountDownLatch latch_;
		muduo::MutexLock mutex_;
		muduo::Condition cond_; // 各种各样的通信原语都具备了！
		BufferPtr currentBuffer_; // 指向当前的buffer
		BufferPtr nextBuffer_;
		BufferVector buffers_;
	};

}
#endif  // MUDUO_BASE_ASYNCLOGGING_H
