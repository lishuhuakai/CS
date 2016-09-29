// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//

#include <muduo/base/FileUtil.h>
#include <muduo/base/Logging.h> // strerror_tl

#include <boost/static_assert.hpp>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>

using namespace muduo;

FileUtil::AppendFile::AppendFile(StringArg filename)
	: fp_(::fopen(filename.c_str(), "ae")),  // 'e' for O_CLOEXEC
	writtenBytes_(0)
{
	assert(fp_); // 一定要打开成功了才行
	::setbuffer(fp_, buffer_, sizeof buffer_); // 设置缓冲池的节奏啊！
	// posix_fadvise POSIX_FADV_DONTNEED ?
}

FileUtil::AppendFile::~AppendFile()
{
	::fclose(fp_); // 关闭文件
}

void FileUtil::AppendFile::append(const char* logline, const size_t len) // 往文件后面添加内容
{
	size_t n = write(logline, len);
	size_t remain = len - n;
	while (remain > 0)
	{
		size_t x = write(logline + n, remain);
		if (x == 0) // 也就是说一个字节也没有写
		{
			int err = ferror(fp_); // feeror用于检测文件读写是否出错
			if (err) // 如果出错了
			{
				fprintf(stderr, "AppendFile::append() failed %s\n", strerror_tl(err));
			}
			break;
		}
		n += x;
		remain = len - n; // remain -= x
	}

	writtenBytes_ += len; // 已经写入的字符数目增加了len
}

void FileUtil::AppendFile::flush() // 用于向fp_指向的文件里面刷新
{
	::fflush(fp_);
}

size_t FileUtil::AppendFile::write(const char* logline, size_t len)
{
	// #undef fwrite_unlocked
	return ::fwrite_unlocked(logline, 1, len, fp_);
}

FileUtil::ReadSmallFile::ReadSmallFile(StringArg filename)
	: fd_(::open(filename.c_str(), O_RDONLY | O_CLOEXEC)), // 读取文件
	err_(0)
{
	buf_[0] = '\0';
	if (fd_ < 0) // 打开文件出错，要保留errno
	{
		err_ = errno;
	}
}

FileUtil::ReadSmallFile::~ReadSmallFile()
{
	if (fd_ >= 0)
	{
		::close(fd_); // FIXME: check EINTR
	}
}

// return errno
template<typename String>
int FileUtil::ReadSmallFile::readToString(int maxSize,
	String* content,
	int64_t* fileSize,
	int64_t* modifyTime,
	int64_t* createTime)
{
	BOOST_STATIC_ASSERT(sizeof(off_t) == 8);
	assert(content != NULL);
	int err = err_;
	if (fd_ >= 0)
	{
		content->clear(); // 将string类型里面的内容清空

		if (fileSize)
		{
			struct stat statbuf;
			if (::fstat(fd_, &statbuf) == 0)
			{
				if (S_ISREG(statbuf.st_mode))
				{
					*fileSize = statbuf.st_size; // 获取文件的大小
					content->reserve(static_cast<int>(std::min(implicit_cast<int64_t>(maxSize), *fileSize)));
				}
				else if (S_ISDIR(statbuf.st_mode)) // 如果是个文件夹
				{
					err = EISDIR;
				}
				if (modifyTime) // 获取修改的时间
				{
					*modifyTime = statbuf.st_mtime;
				}
				if (createTime) // 获取创建的时间
				{
					*createTime = statbuf.st_ctime;
				}
			}
			else
			{
				err = errno;
			}
		}

		while (content->size() < implicit_cast<size_t>(maxSize))
		{
			size_t toRead = std::min(implicit_cast<size_t>(maxSize) - content->size(), sizeof(buf_)); // 需要读取的字节数
			ssize_t n = ::read(fd_, buf_, toRead);
			if (n > 0)
			{
				content->append(buf_, n);
			}
			else
			{
				if (n < 0) // 读取出错
				{
					err = errno;
				}
				break;
			}
		}
	}
	return err;
}

int FileUtil::ReadSmallFile::readToBuffer(int* size) // 表示要读到buffer里面吗？
{
	int err = err_;
	if (fd_ >= 0)
	{
		ssize_t n = ::pread(fd_, buf_, sizeof(buf_) - 1, 0);
		if (n >= 0)
		{
			if (size)
			{
				*size = static_cast<int>(n);
			}
			buf_[n] = '\0'; // 还记得在文件末尾加上一个'\0'结尾
		}
		else
		{
			err = errno;
		}
	}
	return err;
}

template int FileUtil::readFile(StringArg filename,
	int maxSize,
	string* content,
	int64_t*, int64_t*, int64_t*);

template int FileUtil::ReadSmallFile::readToString(
	int maxSize,
	string* content,
	int64_t*, int64_t*, int64_t*);

#ifndef MUDUO_STD_STRING
template int FileUtil::readFile(StringArg filename,
	int maxSize,
	std::string* content,
	int64_t*, int64_t*, int64_t*);

template int FileUtil::ReadSmallFile::readToString(
	int maxSize,
	std::string* content,
	int64_t*, int64_t*, int64_t*);
#endif

