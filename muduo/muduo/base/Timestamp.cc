#include <muduo/base/Timestamp.h>

#include <sys/time.h>
#include <stdio.h>

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <inttypes.h>

#include <boost/static_assert.hpp>

using namespace muduo;

BOOST_STATIC_ASSERT(sizeof(Timestamp) == sizeof(int64_t));

string Timestamp::toString() const
{
	char buf[32] = { 0 };
	int64_t seconds = microSecondsSinceEpoch_ / kMicroSecondsPerSecond; // 秒数
	int64_t microseconds = microSecondsSinceEpoch_ % kMicroSecondsPerSecond; // 余下的微秒数
	snprintf(buf, sizeof(buf) - 1, "%" PRId64 ".%06" PRId64 "", seconds, microseconds);
	return buf;
}

string Timestamp::toFormattedString(bool showMicroseconds) const // 用于格式化输出
{
	char buf[32] = { 0 };
	time_t seconds = static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond); // 得到秒数
	// 一般来说time_t是一个长整型
	struct tm tm_time; // 这个结构用于直接存储年月日等信息
	gmtime_r(&seconds, &tm_time); // gmtime_r是线程安全版本的gmtime函数

	if (showMicroseconds) // 显示微秒，这应该是一个bool类型
	{
		int microseconds = static_cast<int>(microSecondsSinceEpoch_ % kMicroSecondsPerSecond);
		snprintf(buf,
			sizeof(buf),
			"%4d%02d%02d %02d:%02d:%02d.%06d",
			tm_time.tm_year + 1900, // 年
			tm_time.tm_mon + 1, // 月
			tm_time.tm_mday, // 日
			tm_time.tm_hour,
			tm_time.tm_min,
			tm_time.tm_sec,
			microseconds);
	}
	else
	{
		snprintf(buf,
			sizeof(buf),
			"%4d%02d%02d %02d:%02d:%02d",
			tm_time.tm_year + 1900,
			tm_time.tm_mon + 1,
			tm_time.tm_mday,
			tm_time.tm_hour,
			tm_time.tm_min,
			tm_time.tm_sec);
	}
	return buf; // 居然是返回一个buf
}

Timestamp Timestamp::now()
{
	struct timeval tv;
	gettimeofday(&tv, NULL); // NULL表示我们不需要时区的信息
	int64_t seconds = tv.tv_sec; // 得到对应的秒数
	return Timestamp(seconds * kMicroSecondsPerSecond + tv.tv_usec); // timestamp表示的是微秒是吧！
}

