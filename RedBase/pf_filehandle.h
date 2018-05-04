#ifndef PF_FILEHANDLE_H
#define PF_FILEHANDLE_H
#include "pf.h"
#include "pf_buffer.h"
#include "pf_pagehandle.h"
#include "noncopyable.h"
#include <tr1/memory>
using namespace std;

//
// PFFileHandle - 对文件的一种抽象,打开文件时创建该类的对象,要关闭文件时,销毁该对象
// 
class PFFileHandle : public noncopyable {
	friend class PFManager;
public:
	PFFileHandle()
		:opened_(false)
		, buff_(PFBuffer::instance())
	{}
	~PFFileHandle() 
	{
		flush();
		clearFilePages();
	};
public:
	RC firstPage(PFPageHandle &page) const;
	RC nextPage(Page curr, PFPageHandle &page) const;
	RC getPage(Page num, PFPageHandle &page) const;
	RC lastPage(PFPageHandle &page) const;
	RC prevPage(Page curr, PFPageHandle &page) const;

	RC allocPage(PFPageHandle &page);
	RC disposePage(Page num);
	RC markDirty(Page num) const;
	RC unpin(Page num) const;
	RC pin(Page num);
	RC forcePages(Page num = ALL_PAGES);
private:
	void clearFilePages();
	RC flush();
private:
	PFBuffer *buff_;				// PFBuffer这个类用于管理缓存
	PFFileHdr hdr_;					// 用于记录文件头部的信息
	bool opened_;					// 文件是否已经打开
	bool changed_;					// 文件头部是否已经改变了
	int fd_;						// 文件描述符
};

using PFFilePtr = shared_ptr<PFFileHandle>;

PFPageHandle PFAllocPage(PFFilePtr& file);
PFPageHandle PFGetPage(PFFilePtr& file, Page num);

#endif /* PF_FILEHANDLE_H */