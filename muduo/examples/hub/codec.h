#ifndef MUDUO_EXAMPLES_HUB_CODEC_H
#define MUDUO_EXAMPLES_HUB_CODEC_H

// internal header file

#include <muduo/base/Types.h>
#include <muduo/net/Buffer.h>

#include <boost/noncopyable.hpp>

namespace pubsub
{
	using muduo::string;

	enum ParseResult
	{
		kError,
		kSuccess,
		kContinue,
	};

	ParseResult parseMessage(muduo::net::Buffer* buf,
		string* cmd, // 命令，是吧！
		string* topic, // 话题
		string* content); // 内容
}

#endif  // MUDUO_EXAMPLES_HUB_CODEC_H

