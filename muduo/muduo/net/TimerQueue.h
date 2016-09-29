// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef MUDUO_NET_TIMERQUEUE_H
#define MUDUO_NET_TIMERQUEUE_H

// 这个玩意就是非常出名的TimeQueue啦！

#include <set>
#include <vector>

#include <boost/noncopyable.hpp>

#include <muduo/base/Mutex.h>
#include <muduo/base/Timestamp.h>
#include <muduo/net/Callbacks.h>
#include <muduo/net/Channel.h>

namespace muduo
{
	namespace net
	{

		class EventLoop; // 前向声明
		class Timer; // 记录计时器的东西
		class TimerId; // 用于记录计时器，也就是Timer的一些信息

		///
		/// A best efforts timer queue.
		/// No guarantee that the callback will be on time.
		///
		class TimerQueue : boost::noncopyable
		{
		public:
			TimerQueue(EventLoop* loop);
			~TimerQueue();

			  ///
			  /// Schedules the callback to be run at given time,
			  /// repeats if @c interval > 0.0.
			  ///
			  /// Must be thread safe. Usually be called from other threads.
			TimerId addTimer(const TimerCallback& cb,
				Timestamp when,
				double interval); // 用于添加一个定时器
#ifdef __GXX_EXPERIMENTAL_CXX0X__
			TimerId addTimer(TimerCallback&& cb,
				Timestamp when,
				double interval);
#endif

			void cancel(TimerId timerId);

		private:

		  // FIXME: use unique_ptr<Timer> instead of raw pointers.
			typedef std::pair<Timestamp, Timer*> Entry; // set中的element
			typedef std::set<Entry> TimerList;
			typedef std::pair<Timer*, int64_t> ActiveTimer;
			typedef std::set<ActiveTimer> ActiveTimerSet;

			void addTimerInLoop(Timer* timer);
			void cancelInLoop(TimerId timerId);
			// called when timerfd alarms
			void handleRead();
			// move out all expired timers
			std::vector<Entry> getExpired(Timestamp now);
			void reset(const std::vector<Entry>& expired, Timestamp now);

			bool insert(Timer* timer);

			EventLoop* loop_;
			const int timerfd_;
			Channel timerfdChannel_;
			// Timer list sorted by expiration
			TimerList timers_;

			  // for cancel()
			ActiveTimerSet activeTimers_;
			bool callingExpiredTimers_; /* atomic */
			ActiveTimerSet cancelingTimers_;
		};

	}
}
#endif  // MUDUO_NET_TIMERQUEUE_H
