#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "pf_internal.h"
#include "pf_buffer.h"
#include "pf_filehandle.h"
#include "util.h"
#include "pf_manager.h"
#include "pf_error.h"
using namespace RedBase;


RC PFManager::createFile(const char* pathname)
{
	int fd, n;
	// O_CREAT 若想打开的文件不存在的话，则自动建立该文件
	// O_EXCL 如果O_CREAT也被设置, 此指令会去检查文件是否存在. 文件若不存在则建立该文件, 
	// 否则将导致打开文件错误.
	// O_WRONLY 以只写的方式打开文件
	fd = Open(pathname, O_CREAT | O_EXCL | O_WRONLY, CREATION_MASK);
	char hdrBuf[PF_FILE_HDR_SIZE];
	memset(hdrBuf, 0, PF_FILE_HDR_SIZE);

	PFFileHdr *hdr_ = (PFFileHdr*)hdrBuf;
	hdr_->free = PF_PAGE_LIST_END;
	hdr_->size = 0;

	// 将头部写入到文件中
	n = Write(fd, hdrBuf, PF_FILE_HDR_SIZE);
	if (n != PF_FILE_HDR_SIZE) {
		Close(fd);
		Unlink(pathname); // 删除文件
		if (n < 0) return PF_UNIX;
		else return PF_HDRWRITE;
	}
	Close(fd);
	return 0; // 一切都OK
}

//
// destroyFile - 删除掉文件
// 
RC PFManager::destroyFile(const char *pathname)
{
	Unlink(pathname);
	return 0;
}

//
// openFile - 打开某个文件,并将文件的头部信息读入,写入到PFFileHandle类的实体中
// 
RC PFManager::openFile(const char* pathname, PFFilePtr &file)
{
	RC rc;
	file = make_shared<PFFileHandle>();
	file->fd_ = Open(pathname, O_RDWR);

	int n = Read(file->fd_, (Ptr)&file->hdr_, sizeof(PFFileHdr));
	if (n != sizeof(PFFileHdr)) {
		Close(file->fd_);
		return PF_HDRREAD;
	}
	file->changed_ = false;
	file->opened_ = true;
	return 0;
}

//
// closeFile - 关闭掉文件
// 
RC PFManager::closeFile(PFFilePtr &file)
{
	// 将buffer中的东西刷新到磁盘上
	int fd = file->fd_;
	file.reset();
	// 关闭文件
	Close(fd);
	return 0;
}


