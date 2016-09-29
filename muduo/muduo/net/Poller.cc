// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

// 我最怕的就是到处都是依赖
#include <muduo/net/Poller.h>

#include <muduo/net/Channel.h>

using namespace muduo;
using namespace muduo::net;

Poller::Poller(EventLoop* loop)
  : ownerLoop_(loop) // ownerLoop_是EventLoop类型的喽
{
}

Poller::~Poller()
{
}

bool Poller::hasChannel(Channel* channel) const
{ // Channel到底是个啥
  assertInLoopThread();
  ChannelMap::const_iterator it = channels_.find(channel->fd());
  return it != channels_.end() && it->second == channel;
}

