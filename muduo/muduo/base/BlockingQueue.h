// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_BLOCKINGQUEUE_H
#define MUDUO_BASE_BLOCKINGQUEUE_H

#include <muduo/base/Condition.h>
#include <muduo/base/Mutex.h>

#include <boost/noncopyable.hpp>
#include <deque>
#include <assert.h>

namespace muduo
{

	template<typename T>
	class BlockingQueue : boost::noncopyable // 阻塞队列
	{
	public:
		BlockingQueue()
			: mutex_(),
			notEmpty_(mutex_), // 初始化一个condition
			queue_()
		{
		}

		void put(const T& x)
		{
			MutexLockGuard lock(mutex_); // 首先加锁
			queue_.push_back(x); // 然后压栈
			notEmpty_.notify(); // wait morphing saves us,唤醒一个线程
			// http://www.domaigne.com/blog/computing/condvars-signal-with-mutex-locked-or-not/
		}

#ifdef __GXX_EXPERIMENTAL_CXX0X__
		void put(T&& x)
		{
			MutexLockGuard lock(mutex_);
			queue_.push_back(std::move(x));
			notEmpty_.notify();
		}
		// FIXME: emplace()
#endif

		T take() // 取出一个一个元素
		{
			MutexLockGuard lock(mutex_); // 首先加锁
			// always use a while-loop, due to spurious wakeup
			while (queue_.empty()) // 如果队列为空，就一直等待
			{
				notEmpty_.wait(); // 好吧，等待，一直到queue不为空
			}
			assert(!queue_.empty());
#ifdef __GXX_EXPERIMENTAL_CXX0X__
			T front(std::move(queue_.front()));
#else
			T front(queue_.front());
#endif
			queue_.pop_front();
			return front; // 返回
		}

		size_t size() const
		{
			MutexLockGuard lock(mutex_); // 首先加锁
			return queue_.size(); // 然后返回size
		}

	private:
		mutable MutexLock mutex_;
		Condition         notEmpty_; // notEmpty居然是一个条件变量
		std::deque<T>     queue_; // 双向队列
	};

}

#endif  // MUDUO_BASE_BLOCKINGQUEUE_H
