// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//

#include <muduo/base/ProcessInfo.h>
#include <muduo/base/CurrentThread.h>
#include <muduo/base/FileUtil.h>

#include <algorithm>

#include <assert.h>
#include <dirent.h>
#include <pwd.h>
#include <stdio.h> // snprintf
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/times.h>

// 估计这里面的很多东西，我们暂时都用不到。
namespace muduo
{
	namespace detail
	{
		__thread int t_numOpenedFiles = 0; // 打开的文件的数目
		int fdDirFilter(const struct dirent* d) // 用于文件目录的过滤吗？
		{
			if (::isdigit(d->d_name[0]))
			{
				++t_numOpenedFiles;
			}
			return 0;
		}

		__thread std::vector<pid_t>* t_pids = NULL; // 用于记录开启了多少个线程吧!
		int taskDirFilter(const struct dirent* d)
		{
			if (::isdigit(d->d_name[0]))
			{
				t_pids->push_back(atoi(d->d_name));
			}
			return 0;
		}

		int scanDir(const char *dirpath, int(*filter)(const struct dirent *))
		{
			struct dirent** namelist = NULL;
			int result =:: scandir(dirpath, &namelist, filter, alphasort);
			assert(namelist == NULL);
			return result;
		}

		Timestamp g_startTime = Timestamp::now(); // 获取现在的时间
		// assume those won't change during the life time of a process.
		int g_clockTicks = static_cast<int>(::sysconf(_SC_CLK_TCK)); // 获取时钟每一秒大概跳多少次吧
		int g_pageSize = static_cast<int>(::sysconf(_SC_PAGE_SIZE)); // 每一页的大小
	}
}

using namespace muduo;
using namespace muduo::detail;

pid_t ProcessInfo::pid()
{
	return ::getpid(); // 获取进程的id号
}

string ProcessInfo::pidString()
{
	char buf[32];
	snprintf(buf, sizeof buf, "%d", pid()); // 这个玩意是将pid_t类型的数转换成为char类型啊!
	return buf; // 这里返回的时候其实会启动构造函数的
}

uid_t ProcessInfo::uid()
{
	return ::getuid(); // 获取用户的识别码
}

string ProcessInfo::username()
{
	struct passwd pwd;
	struct passwd* result = NULL;
	char buf[8192];
	const char* name = "unknownuser";

	getpwuid_r(uid(), &pwd, buf, sizeof buf, &result);
	if (result)
	{
		name = pwd.pw_name;
	}
	return name;
}

uid_t ProcessInfo::euid()
{
	return ::geteuid(); // geteuid()用来取得执行目前进程有效的用户识别码
}

Timestamp ProcessInfo::startTime()
{
	return g_startTime; // 开始运行的时间吗？
}

int ProcessInfo::clockTicksPerSecond()
{
	return g_clockTicks;
}

int ProcessInfo::pageSize()
{
	return g_pageSize;
}

bool ProcessInfo::isDebugBuild()
{
#ifdef NDEBUG
	return false;
#else
	return true;
#endif
}

string ProcessInfo::hostname() // 获取host的信息
{
  // HOST_NAME_MAX 64
  // _POSIX_HOST_NAME_MAX 255
	char buf[256];
	if (::gethostname(buf, sizeof buf) == 0)
	{
		buf[sizeof(buf) - 1] = '\0';
		return buf;
	}
	else
	{
		return "unknownhost";
	}
}

string ProcessInfo::procname()
{
	return procname(procStat()).as_string(); // 总之构建一个string啦!
}

StringPiece ProcessInfo::procname(const string& stat) // 返回进程的名称，是吧！
{
	StringPiece name;
	size_t lp = stat.find('(');
	size_t rp = stat.rfind(')');
	if (lp != string::npos && rp != string::npos && lp < rp)
	{
		name.set(stat.data() + lp + 1, static_cast<int>(rp - lp - 1));
	}
	return name;
}

string ProcessInfo::procStatus()
{
	string result;
	FileUtil::readFile("/proc/self/status", 65536, &result);
	return result;
}

string ProcessInfo::procStat()
{
	string result;
	FileUtil::readFile("/proc/self/stat", 65536, &result);
	return result;
}

string ProcessInfo::threadStat()
{
	char buf[64];
	snprintf(buf, sizeof buf, "/proc/self/task/%d/stat", CurrentThread::tid());
	string result;
	FileUtil::readFile(buf, 65536, &result);
	return result;
}

string ProcessInfo::exePath()
{
	string result;
	char buf[1024];
	ssize_t n =:: readlink("/proc/self/exe", buf, sizeof buf);
	if (n > 0)
	{
		result.assign(buf, n);
	}
	return result;
}

int ProcessInfo::openedFiles() // 打开的文件数目吗？
{
	t_numOpenedFiles = 0;
	scanDir("/proc/self/fd", fdDirFilter);
	return t_numOpenedFiles;
}

int ProcessInfo::maxOpenFiles() // 最大的可以打开的文件数目
{
	struct rlimit rl;
	if (::getrlimit(RLIMIT_NOFILE, &rl))
	{
		return openedFiles();
	}
	else
	{
		return static_cast<int>(rl.rlim_cur);
	}
}

ProcessInfo::CpuTime ProcessInfo::cpuTime()
{
	ProcessInfo::CpuTime t;
	struct tms tms;
	if (::times(&tms) >= 0)
	{
		const double hz = static_cast<double>(clockTicksPerSecond());
		t.userSeconds = static_cast<double>(tms.tms_utime) / hz;
		t.systemSeconds = static_cast<double>(tms.tms_stime) / hz;
	}
	return t;
}

int ProcessInfo::numThreads()
{
	int result = 0;
	string status = procStatus();
	size_t pos = status.find("Threads:");
	if (pos != string::npos)
	{
		result =:: atoi(status.c_str() + pos + 8);
	}
	return result;
}

std::vector<pid_t> ProcessInfo::threads()
{
	std::vector<pid_t> result;
	t_pids = &result;
	scanDir("/proc/self/task", taskDirFilter);
	t_pids = NULL;
	std::sort(result.begin(), result.end());
	return result;
}

