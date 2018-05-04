#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "pf_buffer.h"
#include "util.h"

using namespace RedBase;
using namespace std;

PFBuffer* PFBuffer::instance_ = nullptr;

//
// getPage - 获取一个指向缓存空间的指针，如果页面已经在缓存之中的话，(re)pin 页面，然后返回一个
//	指向它的指针。如果页面没有在缓存中，将其从文件中读出来，pin it，然后返回一个指向它的指针。如果
//  缓存是满的话，替换掉一个页。
//	
RC PFBuffer::getPage(int fd, Page num, Ptr &addr)
{
	int idx = table_.search(fd, num);
	if (idx >= 0) {
		// 页面已经在内存中了，现在只需要增加引用即可
		nodes_[idx].count++;
	}
	else {
		// 分配一个空的页面,页面的在buff中的位置由idx记录
		idx = searchAvaliableNode();
		if (idx < 0) return PF_NOBUF;
		readPage(fd, num, nodes_[idx].buffer);
		nodes_[idx].fd = fd;
		nodes_[idx].num = num;
		nodes_[idx].count = 1;
		nodes_[idx].dirty = false;
		table_.insert(fd, num, idx);
	}
	// dst指针直接指向页的首部
	addr = nodes_[idx].buffer;
	return 0;
}

//
// pin - 将页面锁在缓冲区里,大致流程和getPage类似
//
RC PFBuffer::pin(int fd, Page num)
{
	int idx = table_.search(fd, num);
	if (idx >= 0) {
		// 页面已经在内存中了，现在只需要增加引用即可
		nodes_[idx].count++;
	}
	else {
		// 分配一个空的页面,页面的在buff中的位置由idx记录
		idx = searchAvaliableNode();
		if (idx < 0) return PF_NOBUF;
		readPage(fd, num, nodes_[idx].buffer);
		nodes_[idx].fd = fd;
		nodes_[idx].num = num;
		nodes_[idx].count = 1;
		nodes_[idx].dirty = false;
		table_.insert(fd, num, idx);
	}
	return 0;
}

//
// allocatePage - 分配一个页面
// 
RC PFBuffer::allocPage(int fd, Page num, Ptr &addr)
{
	int idx = table_.search(fd, num);
	if (idx > 0) return PF_PAGEINBUF;

	// 分配空的页
	idx = searchAvaliableNode();
	if (idx < 0) return PF_NOBUF;

	table_.insert(fd, num, idx);
	nodes_[idx].fd = fd;
	nodes_[idx].num = num;
	nodes_[idx].dirty = false;
	nodes_[idx].count = 1;
	addr = nodes_[idx].buffer;
	return 0;
}

//
// markDirty - 将一个页面标记为脏,当它从bufffer中丢弃的时候,它会被回到文件中
// 
RC PFBuffer::markDirty(int fd, Page num)
{
	int idx = table_.search(fd, num);
	// 页面必须存在于缓存中
	if (idx < 0)  return PF_PAGENOTINBUF;

	if (nodes_[idx].count == 0) return PF_PAGEUNPINNED;
	// 标记为脏页
	nodes_[idx].dirty = true;
	return 0;
}

//
// unpin - 减少一个引用量
//	
RC PFBuffer::unpin(int fd, Page num)
{
	int idx = table_.search(fd, num);
	if (idx < 0) return PF_PAGENOTINBUF;
	nodes_[idx].count -= 1;
	if (nodes_[idx].count == 0) {
		//printf("num = %d, count = 0, dirty = %d\n", num, nodes_[idx].count, nodes_[idx].dirty);
		if (nodes_[idx].dirty) {
			//printf("write, back\n");
			writeBack(nodes_[idx].fd, nodes_[idx].num, nodes_[idx].buffer);
			nodes_[idx].dirty = false;
		}
	}
	return 0;
}

//
// flush - 将脏的页面全部都刷到磁盘中
// 
RC PFBuffer::flush(int fd)
{
	for (int i = 0; i < capacity; i++) {
		if (nodes_[i].fd == fd) {
			if (nodes_[i].dirty) {
				writeBack(fd, nodes_[i].num, nodes_[i].buffer);
				nodes_[i].dirty = false;
			}
		}
	}
	return 0;
}

//
// forcePages
// 
RC PFBuffer::forcePages(int fd, Page num)
{
	for (int i = 0; i < capacity; i++) {
		if ((nodes_[i].fd == fd) && (nodes_[i].num == num)) {
			if (nodes_[i].dirty) {
				writeBack(fd, nodes_[i].num, nodes_[i].buffer);
				nodes_[i].dirty = false;
			}
		}
	}
	return 0;
}

//
// clearFilePages - 一般在文件前调用,将所有缓存的页面全部移除
//
void PFBuffer::clearFilePages(int fd)
{
	for (int i = 0; i < capacity; i++) {
		if (nodes_[i].fd == fd) {
			table_.remove(nodes_[i].fd, nodes_[i].num);
			if (nodes_[i].dirty) {
				writeBack(fd, nodes_[i].num, nodes_[i].buffer);
				nodes_[i].dirty = false;
			}
			nodes_[i].fd = -1;
			nodes_[i].num = -1;
			nodes_[i].count = 0;
		}
	}
}

//
// searchAvaliableNode - 内部使用的函数
// 
int PFBuffer::searchAvaliableNode()
{
	for (int i = 0; i < capacity; i++) {
		if (nodes_[i].count == 0) {
			table_.remove(nodes_[i].fd, nodes_[i].num);
			return i;
		}
	}
	return -1;
}

//
// readPage - 从磁盘中读取一个页面
//	
RC PFBuffer::readPage(int fd, Page num, Ptr dst)
{
	long offset = num * (long)pagesize + PF_FILE_HDR_SIZE;
	Lseek(fd, offset, L_SET);
	// 读取数据
	int n = Read(fd, dst, pagesize);
	if (n != pagesize) return PF_INCOMPLETEREAD;
	return 0; // 一切正常
}


//
// writeBack - 将src处的内容写回到磁盘
// 
RC PFBuffer::writeBack(int fd, Page num, Ptr src)
{
	// 这里的页面计数,从文件头部之后开始
	long offset = num * pagesize + PF_FILE_HDR_SIZE;
	Lseek(fd, offset, L_SET);

	int n = Write(fd, src, pagesize);
	if (n != pagesize) return PF_INCOMPLETEWRITE;
	return 0;
}


