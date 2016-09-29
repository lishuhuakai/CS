// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef MUDUO_NET_TIMER_H
#define MUDUO_NET_TIMER_H

#include <boost/noncopyable.hpp>

#include <muduo/base/Atomic.h>
#include <muduo/base/Timestamp.h>
#include <muduo/net/Callbacks.h>

namespace muduo
{
	namespace net
	{
	///
	/// Internal class for timer event.
	///
		class Timer : boost::noncopyable
		{
		public:
			Timer(const TimerCallback& cb, Timestamp when, double interval)
				: callback_(cb) // 回调函数
				, expiration_(when) // 超时的时间
				, interval_(interval) // 大概是指每隔多少秒触发一次
				, repeat_(interval > 0.0) // 如果interval>0的话，那就是说一定要触发啦！
				, sequence_(s_numCreated_.incrementAndGet())
			{}

#ifdef __GXX_EXPERIMENTAL_CXX0X__
			Timer(TimerCallback&& cb, Timestamp when, double interval)
				: callback_(std::move(cb))
				, expiration_(when)
				, interval_(interval)
				, repeat_(interval > 0.0)
				, sequence_(s_numCreated_.incrementAndGet())
			{}
#endif

			void run() const
			{
				callback_(); // 运行对应的回调函数
			}

			Timestamp expiration() const  { return expiration_; }
			bool repeat() const { return repeat_; }
			int64_t sequence() const { return sequence_; }

			void restart(Timestamp now);

			static int64_t numCreated() { return s_numCreated_.get(); } // 用于得到一共创建了多少个timer

		private:
			const TimerCallback callback_;
			Timestamp expiration_;
			const double interval_;
			const bool repeat_;
			const int64_t sequence_;

			static AtomicInt64 s_numCreated_;
		};
	}
}
#endif  // MUDUO_NET_TIMER_H
