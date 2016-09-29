// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_THREADPOOL_H
#define MUDUO_BASE_THREADPOOL_H

#include <muduo/base/Condition.h>
#include <muduo/base/Mutex.h>
#include <muduo/base/Thread.h>
#include <muduo/base/Types.h>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <deque>

// 据说是用来表示线程池的,C++库的质量非常高。

namespace muduo
{

	class ThreadPool : boost::noncopyable
	{
	public:
		typedef boost::function<void()> Task; // 要执行的任务，其实就是一个函数吧！有了boost::function果然方便很多啊，都具有动态的特性了。

		explicit ThreadPool(const string& nameArg = string("ThreadPool"));
		~ThreadPool();

		  // Must be called before start().
		void setMaxQueueSize(int maxSize) { maxQueueSize_ = maxSize; }
		void setThreadInitCallback(const Task& cb) // 好吧，我看懂了，原来是设置回调函数
		{ threadInitCallback_ = cb; }

		void start(int numThreads);
		void stop();

		const string& name() const
		{ return name_; }

		size_t queueSize() const;

		  // Could block if maxQueueSize > 0
		void run(const Task& f);
#ifdef __GXX_EXPERIMENTAL_CXX0X__
		void run(Task&& f);
#endif

	private:
		bool isFull() const;
		void runInThread();
		Task take();

		mutable MutexLock mutex_;
		Condition notEmpty_;
		Condition notFull_;
		string name_; // pool的名字
		Task threadInitCallback_; // 应该是应该运行的任务吧！
		boost::ptr_vector<muduo::Thread> threads_;
		std::deque<Task> queue_; // 双向队列
		size_t maxQueueSize_;
		bool running_; // 一个全局的变量
	};

}

#endif
