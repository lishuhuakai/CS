#include <unistd.h>
#include <sys/types.h>
#include <assert.h>
#include "pf_internal.h"
#include "pf_buffer.h"
#include "pf.h"
#include "util.h"
#include "pf_filehandle.h"
#include "pf_pagehandle.h"
#include "pf_error.h"
using namespace RedBase;

// 这里的代码对页面的处理有一些问题,首先,磁盘中前后相邻的页并不是逻辑上前后相邻的页,
// 因为有的时候,我们要销毁一些页,会导致磁盘空洞的产生,为了避免这些空洞,我们将由于销毁而重获自由的页
// 拉成了一张链表,链表的头部在文件头部.
// 我想说一下,这样的结构不大好.有没有什么方法可以改进?


//
// getFirstPage - 获取第一个页面,将内容填入PFPageHandle这样一个实例中
//	
RC PFFileHandle::firstPage(PFPageHandle &page) const
{
	return nextPage((Page)-1, page);
}

//
// getLastPage - 获取最后一个页面,将内容填充如PFPageHandle实例中
//	
RC PFFileHandle::lastPage(PFPageHandle &page) const
{
	return prevPage(hdr_.size, page);
}

//
// getNextPage - 获取当前页面的下一个页面,并将内容填充如PFPageHandle的一个实例中
// 
RC PFFileHandle::nextPage(Page curr, PFPageHandle &page) const
{
	RC rc;
	if (!opened_) return PF_CLOSEDFILE;
	if (curr < -1 || curr >= hdr_.size) return PF_INVALIDPAGE;
	// 扫描文件直到一个有效的使用过的页面被找到
	for (curr++; curr < hdr_.size; curr++) {
		if (!(rc = getPage(curr, page))) return 0;
		if (rc != PF_INVALIDPAGE) return rc;
	}
	return PF_EOF;
}

//
// getPrevPage - 获取前一个页面的信息,并将内容填入PFPageHandle的一个实例中
//	
RC PFFileHandle::prevPage(Page curr, PFPageHandle &page) const {
	RC rc;
	if (!opened_) return PF_CLOSEDFILE;
	if (curr <= 0 || curr >= hdr_.size) return PF_INVALIDPAGE;

	// 扫描文件，直到找到一个有效的页,所谓的有效的页,指的是
	for (curr--; curr >= 0; curr--) {
		if (!(rc = getPage(curr, page)))
			return 0;

		// If unexpected error, return it
		if (rc != PF_INVALIDPAGE)
			return rc;
	}
	return PF_EOF;
}

RC PFFileHandle::pin(Page num)
{
	if (!opened_) return PF_CLOSEDFILE;
	if (num < 0 || num >= hdr_.size) return PF_INVALIDPAGE;
	return buff_->pin(fd_, num);
}

//
// getPage - 获取文件中指定的页, 并将内容填充入一个PFPageHandle的一个实体
// 
RC PFFileHandle::getPage(Page num, PFPageHandle &page) const
{

	if (!opened_) return PF_CLOSEDFILE;
	if (num < 0 || num >= hdr_.size) return PF_INVALIDPAGE;
	Ptr addr;
	buff_->getPage(fd_, num, addr);
	PFPageHdr *hdr = (PFPageHdr *)addr;
	// 如果页面已经被使用了，那么让pph指向这个页面
	if (hdr->free == PF_PAGE_USED) {
		page.num_ = num;
		// 注意到这里的data_,指向的并不是页的首部,而是首部后面4个字节处
		page.addr_ = addr + sizeof(PFPageHdr);
		return 0; // 一切正常
	}
	// 到了这里的话,页面多半为空
	unpin(num);
	return PF_INVALIDPAGE;
}

//
// allocatePage - 在文件中分配一个新的页面
//
RC PFFileHandle::allocPage(PFPageHandle &page)
{
	RC rc;		// 返回码
	Page num;
	Ptr addr;

	if (!opened_) return PF_CLOSEDFILE;

	if (hdr_.free != PF_PAGE_LIST_END) { // 仍然存在空闲页面,取出一块
		num = hdr_.free;
		if (rc = buff_->getPage(fd_, num, addr)) return rc;
		// tofix - 这里我是存在疑问的,那就是新得到的页面的free初始化过了吗? 初始化过
		hdr_.free = ((PFPageHdr *)addr)->free; // 空洞数目减1
	}
	else { // 空闲链表为空
		num = hdr_.size;
		// 分配一个新的页面
		if (rc = buff_->allocPage(fd_, num, addr)) return rc;
		hdr_.size++;
	}
	changed_ = true; // 文件发生了变动
	// 将这个页面标记为USED
	((PFPageHdr *)addr)->free = PF_PAGE_USED;
	memset(addr + sizeof(PFPageHdr), 0, PF_PAGE_SIZE);
	// 将页面标记为脏
	markDirty(num);
	// 将页面的信息填入pph中
	page.num_ = num;
	page.addr_ = addr + sizeof(PFPageHdr); // 指向实际的数据
	return 0;
}

//
// disposePage - 销毁一个页面,需要注意的是,PFPageHandle实例指向的对象
// 应该不再被使用了，在调用了这个函数之后。
// 
RC PFFileHandle::disposePage(Page num)
{
	Ptr addr;
	if (num < 0 || num >= hdr_.size) return PF_INVALIDPAGE;
	// 常规性检查，文件必须要打开，然后页面必须有效
	buff_->getPage(fd_, num, addr);
	PFPageHdr *hdr = (PFPageHdr *)addr;
	// 页面必须有效,free == PF_PAGE_USED表示页面正在被使用
	if (hdr->free != PF_PAGE_USED) {
		unpin(num);
		return PF_PAGEFREE;
	}
	hdr->free = hdr_.free; // 将销毁的页面放入链表中,这么做是为了避免产生空洞
	hdr_.free = num;
	changed_ = true;
	markDirty(num);
	unpin(num);
	return 0;
}

//
// markDirty - 将一个页面标记为脏页面,这个页面在移出缓存区的时候会被写入到磁盘中去。
// 
RC PFFileHandle::markDirty(Page num) const
{
	if (!opened_) return PF_CLOSEDFILE;
	if (num < 0 || num >= hdr_.size) return PF_INVALIDPAGE;
	return buff_->markDirty(fd_, num);
}

//
// flushPages - 将所有的脏的页面回写到磁盘中去.
// 
RC PFFileHandle::flush()
{
	if (!opened_) return PF_CLOSEDFILE;
	int n; 
	// 如果头部被修改了，那么将其写回到磁盘中
	if (changed_) {
		Lseek(fd_, 0, L_SET);
		// 写头部
		n = Write(fd_, &hdr_, sizeof(PFFileHdr));
		if (n != sizeof(PFFileHdr)) return PF_HDRWRITE;
		changed_ = false;
	}
	return buff_->flush(fd_); // 将内容刷到磁盘中
}


//
// unpin - 如果有必要的话，这个页面将会被写回到磁盘中。
//	
RC PFFileHandle::unpin(Page num) const
{
	if (!opened_) return PF_CLOSEDFILE;
	if (num < 0 || num >= hdr_.size) return PF_INVALIDPAGE;
	return buff_->unpin(fd_, num);
}

RC PFFileHandle::forcePages(Page num /* = ALL_PAGES */)
{
	if (!opened_) return PF_CLOSEDFILE;
	if (changed_) {
		Lseek(fd_, 0, L_SET);
		int size = sizeof(PFFileHdr);
		int n = Write(fd_, &hdr_, size);
		if (n != size) return PF_HDRWRITE;
		changed_ = false;
	}
	return buff_->forcePages(fd_, num);
}

void PFFileHandle::clearFilePages()
{
	buff_->clearFilePages(fd_);
}

PFPageHandle PFAllocPage(PFFilePtr& file)
{
	PFPageHandle page;
	RC rc = file->allocPage(page);
	if (rc != 0) {
		PFPrintError(rc);
		exit(0);
	}
	page.setOwner(file);
	return page;
}

PFPageHandle PFGetPage(PFFilePtr &file, Page num)
{
	PFPageHandle page;
	RC rc = file->getPage(num, page);
	if (rc != 0) {
		PFPrintError(rc);
		exit(0);
	}
	page.setOwner(file);
	return page;
}

