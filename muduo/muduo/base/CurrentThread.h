// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_CURRENTTHREAD_H
#define MUDUO_BASE_CURRENTTHREAD_H

#include <stdint.h>

namespace muduo
{
	namespace CurrentThread
	{
	  // internal
		extern __thread int t_cachedTid; // 保存的tid，这样的话，就不用每一次都系统调用了!
		extern __thread char t_tidString[32]; // 线程的名称
		extern __thread int t_tidStringLength; // 表示名称的长度
		extern __thread const char* t_threadName;
		void cacheTid();

		inline int tid()
		{
			if (__builtin_expect(t_cachedTid == 0, 0))
			{
				cacheTid(); // 好吧，果然要缓存thread id
			}
			return t_cachedTid;
		}

		inline const char* tidString() // for logging
		{
			return t_tidString;
		}

		inline int tidStringLength() // for logging
		{
			return t_tidStringLength;
		}

		inline const char* name()
		{
			return t_threadName;
		}

		bool isMainThread();

		void sleepUsec(int64_t usec);
	}
}

#endif
