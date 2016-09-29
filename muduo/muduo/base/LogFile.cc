#include <muduo/base/LogFile.h>

#include <muduo/base/FileUtil.h>
#include <muduo/base/ProcessInfo.h>

#include <assert.h>
#include <stdio.h>
#include <time.h>

using namespace muduo;

LogFile::LogFile(const string& basename,
	size_t rollSize,
	bool threadSafe,
	int flushInterval,
	int checkEveryN)
	: basename_(basename)
	, rollSize_(rollSize)
	, flushInterval_(flushInterval)
	, checkEveryN_(checkEveryN)
	, count_(0)
	, mutex_(threadSafe ? new MutexLock : NULL)
	, startOfPeriod_(0)
	, lastRoll_(0)
	, lastFlush_(0)
{
	assert(basename.find('/') == string::npos);
	rollFile(); // 打开一个新的文件
}

LogFile::~LogFile()
{
}

void LogFile::append(const char* logline, int len) // 往后面添加
{
	if (mutex_)
	{
		MutexLockGuard lock(*mutex_); // 先加锁
		append_unlocked(logline, len); // 然后调用无锁版本的往后面添加内容
	}
	else
	{
		append_unlocked(logline, len);
	}
}

void LogFile::flush()
{
	if (mutex_)
	{
		MutexLockGuard lock(*mutex_);
		file_->flush();
	}
	else
	{
		file_->flush(); // file_应该是一个文件指针吧！
	}
}

void LogFile::append_unlocked(const char* logline, int len)
{
	file_->append(logline, len);

	if (file_->writtenBytes() > rollSize_) // rollSize_一般指的是文件的大小吧！
	{
		rollFile(); // 这是回滚的意思吗？
	}
	else
	{
		++count_; // 计数加1,表示啥
		if (count_ >= checkEveryN_)
		{
			count_ = 0;
			time_t now =::time(NULL); // 当前的时间
			time_t thisPeriod_ = now / kRollPerSeconds_ * kRollPerSeconds_;
			if (thisPeriod_ != startOfPeriod_)
			{
				rollFile();
			}
			else if (now - lastFlush_ > flushInterval_)
			{
				lastFlush_ = now;
				file_->flush();
			}
		}
	}
}

bool LogFile::rollFile() // 话说，rollFile()究竟是用来干什么的
{
	time_t now = 0;
	string filename = getLogFileName(basename_, &now); // 得到文件的名称
	time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;

	if (now > lastRoll_) 
	{
		lastRoll_ = now;
		lastFlush_ = now; 
		startOfPeriod_ = start;
		file_.reset(new FileUtil::AppendFile(filename)); // 好吧，没想到是打开一个新文件
		return true;
	}
	return false;
}

string LogFile::getLogFileName(const string& basename, time_t* now) // 好吧，总之这个函数的话，就是用来得到log文件的文件名的！
{
	string filename;
	filename.reserve(basename.size() + 64);
	filename = basename;

	char timebuf[32];
	struct tm tm;
	*now = time(NULL);
	gmtime_r(now, &tm); // FIXME: localtime_r ?
	strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S.", &tm);
	filename += timebuf;

	filename += ProcessInfo::hostname();

	char pidbuf[32];
	snprintf(pidbuf, sizeof pidbuf, ".%d", ProcessInfo::pid());
	filename += pidbuf;

	filename += ".log";

	return filename;
}

