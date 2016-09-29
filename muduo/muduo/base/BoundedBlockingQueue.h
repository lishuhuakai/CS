// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_BOUNDEDBLOCKINGQUEUE_H
#define MUDUO_BASE_BOUNDEDBLOCKINGQUEUE_H

#include <muduo/base/Condition.h>
#include <muduo/base/Mutex.h>

#include <boost/circular_buffer.hpp>
#include <boost/noncopyable.hpp>
#include <assert.h>

namespace muduo
{

	template<typename T>
	class BoundedBlockingQueue : boost::noncopyable
	{
	public:
		explicit BoundedBlockingQueue(int maxSize)
			: mutex_(),
			notEmpty_(mutex_), // 不会notEmpty_和notFull_都是两个条件变量吧
			notFull_(mutex_),
			queue_(maxSize)
		{
		}

		void put(const T& x)
		{
			MutexLockGuard lock(mutex_);
			while (queue_.full()) // 如果满了，就要一直等待
			{
				notFull_.wait();
			}
			assert(!queue_.full());
			queue_.push_back(x);
			notEmpty_.notify(); // 唤醒消费者进程,表示已经不为空啦
		}

		T take()
		{
			MutexLockGuard lock(mutex_);
			while (queue_.empty()) // 如果为空，消费者就要一直等待
			{
				notEmpty_.wait();
			}
			assert(!queue_.empty());
			T front(queue_.front());
			queue_.pop_front();
			notFull_.notify(); // 唤醒生产者进程，表示已经已经不满了
			return front;
		}

		bool empty() const
		{
			MutexLockGuard lock(mutex_);
			return queue_.empty();
		}

		bool full() const
		{
			MutexLockGuard lock(mutex_);
			return queue_.full(); // 没想到居然还有这种函数
		}

		size_t size() const
		{
			MutexLockGuard lock(mutex_);
			return queue_.size();
		}

		size_t capacity() const // 返回容量
		{
			MutexLockGuard lock(mutex_);
			return queue_.capacity();
		}

	private:
		mutable MutexLock          mutex_;
		Condition                  notEmpty_;
		Condition                  notFull_;
		boost::circular_buffer<T>  queue_; // 循环队列吧！
	};

}

#endif  // MUDUO_BASE_BOUNDEDBLOCKINGQUEUE_H
