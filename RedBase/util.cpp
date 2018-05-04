#include "util.h"

namespace RedBase {
	void unix_error(const char *msg) /* unix-style error */
	{
		fprintf(stderr, "%s: %s\n", msg, strerror(errno));
		exit(0);
	}

	int Open(const char *pathname, int flags, mode_t mode)
	{
		int rc;

		if ((rc = open(pathname, flags, mode)) < 0)
			unix_error("Open error");
		return rc;
	}

	int Open(const char *pathname, int flags)
	{
		int rc;

		if ((rc = open(pathname, flags)) < 0)
			unix_error("Open error");
		return rc;
	}

	ssize_t Read(int fd, void *buf, size_t count)
	{
		ssize_t rc;

		if ((rc = read(fd, buf, count)) < 0)
			unix_error("Read error");
		return rc;
	}

	ssize_t Write(int fd, const void *buf, size_t count)
	{
		ssize_t rc;

		if ((rc = write(fd, buf, count)) < 0)
			unix_error("Write error");
		return rc;
	}

	off_t Lseek(int fildes, off_t offset, int whence)
	{
		off_t rc;

		if ((rc = lseek(fildes, offset, whence)) < 0)
			unix_error("Lseek error");
		return rc;
	}

	void Close(int fd)
	{
		int rc;

		if ((rc = close(fd)) < 0)
			unix_error("Close error");
	}

	int Unlink(const char *pathname)
	{
		int rc;
		if ((rc = unlink(pathname)) < 0)
			unix_error("Unlink error");
		return 0;
	}

	char * Getcwd(char *buf, size_t size)
	{
		char * src = getcwd(buf, size);
		if (src == nullptr) unix_error("Getcwd error");
		return src;
	}

	int Chdir(const char * path)
	{
		int rc;
		if ((rc = chdir(path)) < 0) unix_error("Chdir error");
		return 0;
	}

}