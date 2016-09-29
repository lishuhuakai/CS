// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include <muduo/base/Logging.h>
#include <muduo/net/Channel.h>
#include <muduo/net/EventLoop.h>

#include <sstream>

#include <poll.h>

using namespace muduo;
using namespace muduo::net;
// 我需要解释！

const int Channel::kNoneEvent = 0; // 什么事件也不关心
const int Channel::kReadEvent = POLLIN | POLLPRI; // 表示读
const int Channel::kWriteEvent = POLLOUT; // 确实是写对应的位

Channel::Channel(EventLoop* loop, int fd__) // 这个fd__实际上代表的是eventfd类型
	: loop_(loop)
	, fd_(fd__)
	, events_(0)
	, revents_(0)
	, index_(-1)
	, logHup_(true)
	, tied_(false)
	, eventHandling_(false)
	, addedToLoop_(false)
{
}

Channel::~Channel()
{
	assert(!eventHandling_);
	assert(!addedToLoop_);
	if (loop_->isInLoopThread())
	{
		assert(!loop_->hasChannel(this));
	}
}

void Channel::tie(const boost::shared_ptr<void>& obj)
{
	tie_ = obj; // tie_是一个weak_ptr类型的指针吧！
	tied_ = true;
}

void Channel::update()
{
	addedToLoop_ = true; // addedToLoop表示什么意思？表示这个channel本身是否被添加进了loop (Eventloop)之中
	loop_->updateChannel(this);
}

void Channel::remove()
{
	assert(isNoneEvent());
	addedToLoop_ = false;
	loop_->removeChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime)
{
	boost::shared_ptr<void> guard;  // guard是个什么玩意？
	if (tied_)
	{
		guard = tie_.lock();
		if (guard) // 如果对方还没有析构掉
		{
			handleEventWithGuard(receiveTime);
		}
	}
	else
	{
		handleEventWithGuard(receiveTime);
	}
}

void Channel::handleEventWithGuard(Timestamp receiveTime)
{
	eventHandling_ = true; // 表示正在处理事件
	LOG_TRACE << reventsToString();
	/// POLLIN 普通或者优先级带数据可读
	/// POLLHUB 发生挂起
	/// POLLNVAL 描述符不是一个打开的文件
	/// POLLOUT 普通数据可写
	/// 
	if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) // 
	{
		if (logHup_)
		{
			LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLHUP";
		}
		if (closeCallback_) closeCallback_();
	}

	if (revents_ & POLLNVAL)
	{
		LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLNVAL";
	}

	if (revents_ & (POLLERR | POLLNVAL)) // POLLERR 发生错误
	{
		if (errorCallback_) errorCallback_();
	}
	if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) // 总之表示数据可读
	{
		if (readCallback_) readCallback_(receiveTime);
	}
	if (revents_ & POLLOUT) // 表示数据可写
	{
		if (writeCallback_) writeCallback_();
	}
	eventHandling_ = false;
}

string Channel::reventsToString() const
{
	return eventsToString(fd_, revents_);
}

string Channel::eventsToString() const
{
	return eventsToString(fd_, events_);
}

string Channel::eventsToString(int fd, int ev)
{
	std::ostringstream oss; // 好吧，又学了一个新的玩意，非常爽
	oss << fd << ": ";
	if (ev & POLLIN)
		oss << "IN ";
	if (ev & POLLPRI)
		oss << "PRI ";
	if (ev & POLLOUT)
		oss << "OUT ";
	if (ev & POLLHUP)
		oss << "HUP ";
	if (ev & POLLRDHUP) // POLLRDHUP表示对端套接字关闭
		oss << "RDHUP ";
	if (ev & POLLERR)
		oss << "ERR ";
	if (ev & POLLNVAL)
		oss << "NVAL ";

	return oss.str().c_str();
}
