// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef MUDUO_BASE_PROCESSINFO_H
#define MUDUO_BASE_PROCESSINFO_H

#include <muduo/base/StringPiece.h>
#include <muduo/base/Types.h>
#include <muduo/base/Timestamp.h>
#include <vector>

namespace muduo
{

	namespace ProcessInfo // 用于记录进程的一些信息吗？
	{
		pid_t pid(); // 比如说，得到进程的信息
		string pidString(); // 这个函数其实没有别的意思，就是将pid的数字形式，转换为字符形式
		uid_t uid(); // 得到用户的id
		string username();
		uid_t euid();
		Timestamp startTime();
		int clockTicksPerSecond();
		int pageSize();
		bool isDebugBuild();  // constexpr

		string hostname();
		string procname();
		StringPiece procname(const string& stat);

		  /// read /proc/self/status
		string procStatus();

		  /// read /proc/self/stat
		string procStat();

		  /// read /proc/self/task/tid/stat
		string threadStat();

		  /// readlink /proc/self/exe
		string exePath();

		int openedFiles();
		int maxOpenFiles();

		struct CpuTime
		{
			double userSeconds;
			double systemSeconds;

			CpuTime()
				: userSeconds(0.0)
				, systemSeconds(0.0) {}
		};
		CpuTime cpuTime();

		int numThreads();
		std::vector<pid_t> threads();
	}

}

#endif  // MUDUO_BASE_PROCESSINFO_H
