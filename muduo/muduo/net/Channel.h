// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef MUDUO_NET_CHANNEL_H
#define MUDUO_NET_CHANNEL_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <muduo/base/Timestamp.h>

namespace muduo
{
	namespace net
	{

		class EventLoop; // 学一下这里的前向引用，避免包含头文件

		///
		/// A selectable I/O channel.
		///
		/// This class doesn't own the file descriptor.
		/// The file descriptor could be a socket,
		/// an eventfd, a timerfd, or a signalfd
		class Channel : boost::noncopyable
		{
		public:
			typedef boost::function<void()> EventCallback; // 好吧，这是一个回调函数
			typedef boost::function<void(Timestamp)> ReadEventCallback; // ReadEventCallback是另外一种形式的回调函数

			Channel(EventLoop* loop, int fd);
			~Channel();

			void handleEvent(Timestamp receiveTime); // 用于处理对应的事件
			void setReadCallback(const ReadEventCallback& cb)
			{ readCallback_ = cb; }
			void setWriteCallback(const EventCallback& cb)
			{ writeCallback_ = cb; }
			void setCloseCallback(const EventCallback& cb)
			{ closeCallback_ = cb; }
			void setErrorCallback(const EventCallback& cb)
			{ errorCallback_ = cb; }
#ifdef __GXX_EXPERIMENTAL_CXX0X__ // 下面的部分可以暂时不用看
			void setReadCallback(ReadEventCallback&& cb)
			{ readCallback_ = std::move(cb); }
			void setWriteCallback(EventCallback&& cb)
			{ writeCallback_ = std::move(cb); }
			void setCloseCallback(EventCallback&& cb)
			{ closeCallback_ = std::move(cb); }
			void setErrorCallback(EventCallback&& cb)
			{ errorCallback_ = std::move(cb); }
#endif

			  /// Tie this channel to the owner object managed by shared_ptr,
			  /// prevent the owner object being destroyed in handleEvent.
			void tie(const boost::shared_ptr<void>&);

			int fd() const { return fd_; } // fd表示的是文件描述符吗
			int events() const { return events_; } // 好吧，这里居然是一个events_
			void set_revents(int revt) { revents_ = revt; } // used by pollers
			// int revents() const { return revents_; }
			bool isNoneEvent() const { return events_ == kNoneEvent; }

			void enableReading() { events_ |= kReadEvent; update(); }
			void disableReading() { events_ &= ~kReadEvent; update(); }
			void enableWriting() { events_ |= kWriteEvent; update(); }
			void disableWriting() { events_ &= ~kWriteEvent; update(); }
			void disableAll() { events_ = kNoneEvent; update(); }
			bool isWriting() const { return events_ & kWriteEvent; } // 判断是否在writing
			bool isReading() const { return events_ & kReadEvent; } // 判断是否在reading

			  // for Poller
			int index() { return index_; } // index_是个啥玩意
			void set_index(int idx) { index_ = idx; }

			  // for debug
			string reventsToString() const;
			string eventsToString() const;

			void doNotLogHup() { logHup_ = false; }

			EventLoop* ownerLoop() { return loop_; } // loop_是EventLoop类型的，是吧！
			void remove();

		private:
			static string eventsToString(int fd, int ev); // fd表示文件描述符

			void update();
			void handleEventWithGuard(Timestamp receiveTime);

			static const int kNoneEvent;
			static const int kReadEvent;
			static const int kWriteEvent;

			EventLoop* loop_;
			const int  fd_;
			int        events_;
			int        revents_; // it's the received event types of epoll or poll
			int        index_; // used by Poller.
			bool       logHup_;

			boost::weak_ptr<void> tie_; // 只是一个weak_ptr，单纯用于判断对方是否还活着
			bool tied_;
			bool eventHandling_;
			bool addedToLoop_;
			ReadEventCallback readCallback_;
			EventCallback writeCallback_;
			EventCallback closeCallback_;
			EventCallback errorCallback_;
		};

	}
}
#endif  // MUDUO_NET_CHANNEL_H
