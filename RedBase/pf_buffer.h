#ifndef PF_BUFFER_H
#define PF_BUFFER_H

#include <string.h>
#include "pf_internal.h"
#include "pf_hashtable.h"
#include "pf_error.h"
using namespace std;


//
// PFBuffer - 用于缓存文件的页面
//
class PFBuffer {
public:
	const static int capacity = 40;
	const static int pagesize = PF_PAGE_SIZE + sizeof(PFPageHdr);
private:
	struct BufferNode {
		bool dirty;
		uint count;
		int fd;
		Page num;
		char buffer[pagesize];
		BufferNode() 
			: dirty(false), count(0)
			, fd(-1), num(-1)
		{
			bzero(buffer, sizeof(buffer));
		}
	};
private:
	PFBuffer() : table_(capacity / 2) {}
public:
	static PFBuffer* instance()
	{
		if (instance_ == nullptr) {
			instance_ = new PFBuffer();
		}
		return instance_;
	}

	static void destroyBuffer()
	{
		if (instance_ != nullptr) {
			delete instance_;
			instance_ = nullptr;
		}
	}

public:
	RC getPage(int fd, Page num, Ptr &addr);
	RC allocPage(int fd, Page num, Ptr &addr);
	RC markDirty(int fd, Page num);	
	RC unpin(int fd, Page num);
	RC pin(int fd, Page num);
	RC forcePages(int fd, Page num);
	void clearFilePages(int fd);
	RC flush(int fd);
private:
	int searchAvaliableNode();
	RC readPage(int fd, Page num, Ptr dst);
	RC writeBack(int fd, Page num, Ptr src);
private:
	static PFBuffer* instance_;
	BufferNode nodes_[capacity];
	PFHashTable table_;
	int first_;				
	int last_;
	int free_;
};

#endif