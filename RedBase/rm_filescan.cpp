//
// rm_filescan.cpp
// 类RMFileScan的实现
//

#include <assert.h>
#include <stdio.h>
#include <iostream>
#include "rm.h"
#include "rm_filescan.h"
#include "pf_pagehandle.h"
using namespace std;

RC RMFileScan::openScan(RMFilePtr& file, AttrType type, 
	int len, int attroffset, Operator op, const void* val)
{
	rmfile_ = file;
	if (val != nullptr) {
		/* 做一些检查 */
		if ((len >= PF_PAGE_SIZE - sizeof(Page)) || (len <= 0)) return RM_RECSIZEMISMATCH;
		if (type == STRING && (len <= 0 || len > MAXSTRINGLEN))
			return RM_FCREATEFAIL;
	}
	comp_ = make_comp(type, len);
	op_ = op;
	val_ = val;
	offset_ = attroffset; /* 记录下属性的偏移量 */
	return 0;
}

//
// rewind - 从新开始遍历
// 
RC RMFileScan::rewind()
{
	curr_ = RID(1, -1);
	return 0;
}


//
// getNextRcd - 用于获取下一条记录
// 
RC RMFileScan::getNextRcd(RMRecord &rcd)
{
	uint pages = rmfile_->pagesSize();		/* 页的数目 */
	uint slots = rmfile_->capacity_;		/* slots的数目 */

	for (uint i = curr_.page(); i < pages; i++) {
		PFPageHandle page = PFGetPage(rmfile_->pffile_, i);
		Ptr addr = page.rawPtr();
		RMPageHdr hdr(slots, addr);
		uint slot = curr_.page() == i ? curr_.slot() + 1 : 0;

		for (;slot < slots; slot++) {
			if (!hdr.map.available(slot)) {		/* 这个slot已经被使用了 */
				curr_ = RID(i, slot);
				rmfile_->getRcd(curr_, rcd);	/* 获取一条记录 */
				addr = rcd.rawPtr();
				if (comp_->eval(addr + offset_, op_, val_)) return 0;
				/* 否则的话,获取下一条记录 */
			}
		}
	}
	return RM_EOF;
}

RC RMFileScan::closeScan()
{
	if (comp_ != nullptr) delete comp_;
	curr_ = RID(1, -1);
	return 0;
}