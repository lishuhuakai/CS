// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include <muduo/net/EventLoop.h>
#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/Channel.h>
#include <muduo/net/Poller.h>
#include <muduo/net/SocketsOps.h>
#include <muduo/net/TimerQueue.h>

#include <boost/bind.hpp>

#include <signal.h>
#include <sys/eventfd.h>

using namespace muduo;
using namespace muduo::net;

namespace
{
	__thread EventLoop* t_loopInThisThread = 0; // 一个指针,难怪这个玩意在每个线程中都有一份呢。

	const int kPollTimeMs = 10000; // 一秒钟轮询10000次吗？

	int createEventfd()
	{
		int evtfd =:: eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC); // 非阻塞,我的印象里，evtfd只是一个计数器一样的东西
		if (evtfd < 0)
		{
			LOG_SYSERR << "Failed in eventfd";
			abort();
		}
		return evtfd;
	}

#pragma GCC diagnostic ignored "-Wold-style-cast"
	class IgnoreSigPipe // 貌似用于忽略pipe信号
	{
	public:
		IgnoreSigPipe()
		{
			::signal(SIGPIPE, SIG_IGN);
			// LOG_TRACE << "Ignore SIGPIPE";
		}
	};
#pragma GCC diagnostic error "-Wold-style-cast"

	IgnoreSigPipe initObj;
}

EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
	return t_loopInThisThread; // 每个线程都有一个eventloop
}

EventLoop::EventLoop()
	: looping_(false)
	, quit_(false)
	, eventHandling_(false)
	, callingPendingFunctors_(false)
	, iteration_(0)
	, threadId_(CurrentThread::tid()) // 初始化的时候记录了当前线程的id
	, poller_(Poller::newDefaultPoller(this))
	, timerQueue_(new TimerQueue(this)) // 好吧，要好好看一下timerQueue的实现代码啦！
	, wakeupFd_(createEventfd()) // 构建一个文件描述符,
	, wakeupChannel_(new Channel(this, wakeupFd_)) // 这里构建了一个新的channel,果然有一个wakeupChannel
	, currentActiveChannel_(NULL)
{
	LOG_DEBUG << "EventLoop created " << this << " in thread " << threadId_;
	if (t_loopInThisThread) // 如果t_loopInThisThread不为空
	{ /// 需要注意的是，这个t_loopInThisThread变量在每个线程中都会存在一份，也就是说，这个玩意不为空的话
	  /// 而唯一能够改变t_loopInThisThread变量值的，只有eventloop对象的构造函数
		LOG_FATAL << "Another EventLoop " << t_loopInThisThread
		          << " exists in this thread " << threadId_;
	}
	else
	{
		t_loopInThisThread = this;
	}
	wakeupChannel_->setReadCallback(
	    boost::bind(&EventLoop::handleRead, this)); // 也就是被唤醒之后，默认调用的是EventLoop的handleRead函数
	  // we are always reading the wakeupfd
	wakeupChannel_->enableReading(); // 这里调用enableReading()函数->Channel的update()函数->..->poll的updateChannel()函数
	// 从而将wakeupChannel加入了poll的pollers_数组里面。
}

EventLoop::~EventLoop()
{
	LOG_DEBUG << "EventLoop " << this << " of thread " << threadId_
	          << " destructs in thread " << CurrentThread::tid();
	wakeupChannel_->disableAll();  // 活着的channel
	wakeupChannel_->remove();
	::close(wakeupFd_); // 关闭文件描述符
	t_loopInThisThread = NULL;
}

void EventLoop::loop() // 表示开始循环，是吧！
{
	assert(!looping_);
	assertInLoopThread();
	looping_ = true; // 表示正在循环
	quit_ = false;  // FIXME: what if someone calls quit() before loop() ?
	LOG_TRACE << "EventLoop " << this << " start looping";

	while (!quit_)
	{
		activeChannels_.clear(); // activeChannels究竟是什么,表示可以操作的channel们
		pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_); // 好吧，居然让我发现了poll
		++iteration_; // 用于记录交互的次数
		if (Logger::logLevel() <= Logger::TRACE) 
		{
			printActiveChannels();
		}
		// TODO sort channel by priority
		eventHandling_ = true;
		for (ChannelList::iterator it = activeChannels_.begin();
		    it != activeChannels_.end(); ++it) // 迭代
		{
			currentActiveChannel_ = *it; // 当前活跃的channel
			currentActiveChannel_->handleEvent(pollReturnTime_);
		}
		currentActiveChannel_ = NULL;
		eventHandling_ = false;
		doPendingFunctors();
	}

	LOG_TRACE << "EventLoop " << this << " stop looping";
	looping_ = false;
}

void EventLoop::quit()
{
	quit_ = true; // 好吧，非常漂亮。
	// There is a chance that loop() just executes while(!quit_) and exits,
	// then EventLoop destructs, then we are accessing an invalid object.
	// Can be fixed using mutex_ in both places.
	if (!isInLoopThread())
	{
		wakeup();
	}
}

void EventLoop::runInLoop(const Functor& cb)
{
	if (isInLoopThread())
	{
		cb(); 
	}
	else
	{
		queueInLoop(cb);
	}
}

void EventLoop::queueInLoop(const Functor& cb)
{
	{
		MutexLockGuard lock(mutex_); // 加锁
		pendingFunctors_.push_back(cb);
	}

	if (!isInLoopThread() || callingPendingFunctors_)
	{
		wakeup();
	}
}

TimerId EventLoop::runAt(const Timestamp& time, const TimerCallback& cb)
{
	return timerQueue_->addTimer(cb, time, 0.0);
}

TimerId EventLoop::runAfter(double delay, const TimerCallback& cb)
{
	Timestamp time(addTime(Timestamp::now(), delay));
	return runAt(time, cb);
}

TimerId EventLoop::runEvery(double interval, const TimerCallback& cb)
{
	Timestamp time(addTime(Timestamp::now(), interval));
	return timerQueue_->addTimer(cb, time, interval);
}

#ifdef __GXX_EXPERIMENTAL_CXX0X__
// FIXME: remove duplication
void EventLoop::runInLoop(Functor&& cb)
{
	if (isInLoopThread())
	{
		cb();
	}
	else
	{
		queueInLoop(std::move(cb));
	}
}

void EventLoop::queueInLoop(Functor&& cb)
{
	{
		MutexLockGuard lock(mutex_);
		pendingFunctors_.push_back(std::move(cb));  // emplace_back
	}

	if (!isInLoopThread() || callingPendingFunctors_)
	{
		wakeup();
	}
}

TimerId EventLoop::runAt(const Timestamp& time, TimerCallback&& cb)
{
	return timerQueue_->addTimer(std::move(cb), time, 0.0);
}

TimerId EventLoop::runAfter(double delay, TimerCallback&& cb)
{
	Timestamp time(addTime(Timestamp::now(), delay));
	return runAt(time, std::move(cb));
}

TimerId EventLoop::runEvery(double interval, TimerCallback&& cb)
{
	Timestamp time(addTime(Timestamp::now(), interval));
	return timerQueue_->addTimer(std::move(cb), time, interval);
}
#endif

void EventLoop::cancel(TimerId timerId)
{
	return timerQueue_->cancel(timerId);
}

void EventLoop::updateChannel(Channel* channel) // 只有这里才调用了updateChannel
{
	assert(channel->ownerLoop() == this);
	assertInLoopThread();
	poller_->updateChannel(channel); // 这里的关系很复杂啊！
}

void EventLoop::removeChannel(Channel* channel)
{
	assert(channel->ownerLoop() == this);
	assertInLoopThread();
	if (eventHandling_)
	{
		assert(currentActiveChannel_ == channel ||
		    std::find(activeChannels_.begin(), activeChannels_.end(), channel) == activeChannels_.end());
	}
	poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel)
{
	assert(channel->ownerLoop() == this);
	assertInLoopThread();
	return poller_->hasChannel(channel);
}

void EventLoop::abortNotInLoopThread()
{
	LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
	          << " was created in threadId_ = " << threadId_
	          << ", current thread id = " <<  CurrentThread::tid();
}

void EventLoop::wakeup() // 我们来看一下wakeup函数究竟是如何操作的吧！
{
	uint64_t one = 1;
	ssize_t n = sockets::write(wakeupFd_, &one, sizeof one); // wakeupFd_主要是用于计数的吗？
	/// 具体而言，就是往wakeupFd_指向的文件里写入数据
	if (n != sizeof one)
	{
		LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
	}
}

void EventLoop::handleRead()
{
	uint64_t one = 1;
	ssize_t n = sockets::read(wakeupFd_, &one, sizeof one);
	if (n != sizeof one)
	{
		LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
	}
}

void EventLoop::doPendingFunctors()
{
	std::vector<Functor> functors;
	callingPendingFunctors_ = true;

	{
		MutexLockGuard lock(mutex_);
		functors.swap(pendingFunctors_);
	}

	for (size_t i = 0; i < functors.size(); ++i)
	{
		functors[i]();
	}
	callingPendingFunctors_ = false;
}

void EventLoop::printActiveChannels() const
{
	for (ChannelList::const_iterator it = activeChannels_.begin();
	    it != activeChannels_.end(); ++it)
	{
		const Channel* ch = *it;
		LOG_TRACE << "{" << ch->reventsToString() << "} ";
	}
}

