#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

namespace RedBase {
	int Open(const char *pathname, int flags);
	int Open(const char *pathname, int flags, mode_t mode);
	ssize_t Read(int fd, void *buf, size_t count);
	ssize_t Write(int fd, const void *buf, size_t count);
	off_t Lseek(int fildes, off_t offset, int whence);
	void Close(int fd);
	int Unlink(const char *pathname);
	char *Getcwd(char *buf, size_t size);
	int Chdir(const char* path);
}

#endif /* UTIL_H */