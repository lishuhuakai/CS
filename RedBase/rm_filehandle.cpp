#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <math.h>
#include "rm.h"
#include "rm_error.h"
#include "rm_filehandle.h"
#include "rm_pagehdr.h"
using namespace std;


//
// CalcSlotsSize - 计算一个数据块最多所支持的存储的记录的条数
// 
uint static CalcSlotsCapacity(uint len)
{
	uint remain = PF_PAGE_SIZE - sizeof(RMPageHdrEx); // 剩余可用的字节数目
	// floor - 向下取整
	// 每一条记录都需要1个bit,也就是1/8字节来表示是否已经记录了数据
	uint slots = floor((1.0 * remain) / (len + 1 / 8));
	uint hdr_size = sizeof(RMPageHdrEx) + BitMap::bytes(slots);
	// 接下来需要不断调整
	while ((slots * len + hdr_size) > PF_PAGE_SIZE) {
		slots--;
		hdr_size = sizeof(RMPageHdrEx) + BitMap::bytes(slots);
	}
	return slots;
}


RMFileHandle::RMFileHandle(PFFilePtr file) 
	: changed_(false)
{
	pffile_ = file;
	PFPageHandle page = PFGetPage(file, 0);
	Ptr buff = page.rawPtr();
	memcpy(&rmhdr_, buff, sizeof(RMFileHdr));
	rcdlen_ = rmhdr_.rcdlen;
	capacity_ = CalcSlotsCapacity(rcdlen_);
}

RMFileHandle::~RMFileHandle()
{
	if (changed_) {
		PFPageHandle page = PFGetPage(pffile_, 0);
		Ptr buff = page.rawPtr();
		memcpy(buff, &rmhdr_, sizeof(RMFileHdr));
		page.setDirty();
	}
}


//
// isValidPage - 判断是否为有效的页面
// 
bool RMFileHandle::isValidPage(Page num) const
{
	return (num >= 0) && (num < rmhdr_.size);
}

bool RMFileHandle::isValidRID(const RID& rid) const
{
	Slot slot = rid.slot();
	return isValidPage(rid.page()) && slot >= 0;
}


//
// insertRcd - 插入一条记录
// 
RC RMFileHandle::insertRcd(const Ptr addr, RID &rid)
{
	if (addr == nullptr) return RM_NULLRECORD; // 指向的内容为空
	PFPageHandle page;
	Page num;
	Slot slot;
	// 从一个拥有空闲的pos的空闲页面中获取一个空闲的pos
	nextFreeSlot(page, num, slot);
	// 从页面头部提取出该页面的信息
	Ptr ptr = page.rawPtr();
	RMPageHdr hdr(capacity_, ptr);
	uint offset = hdr.lenOfHdr() + slot * rcdlen_;
	ptr = ptr + offset;
	
	rid = RID(num, slot);	// rid用于记录存储的位置
	memcpy(ptr, addr, rcdlen_);
	hdr.map.reset(slot); // 设定为0,表示已经被使用了
	hdr.setRemain(hdr.remain() - 1);
	if (hdr.remain() == 0) {
		rmhdr_.free = hdr.next();
		hdr.setNext(PAGE_FULLY_USED);
	}
	page.setDirty();
	return 0;
}

//
//  updateRcd 更新一条记录, rcd中记录了记录在磁盘中的位置
// 
RC RMFileHandle::updateRcd(const RMRecord &rcd)
{
	RID rid;
	rcd.getRid(rid);
	if (!isValidRID(rid)) return RM_BAD_RID;
	Page num = rid.page();
	Slot pos = rid.slot();

	Ptr src;
	PFPageHandle page = PFGetPage(pffile_, num);
	Ptr dst = page.rawPtr();

	RMPageHdr hdr(capacity_, dst);
	if (hdr.map.available(pos)) return RM_NORECATRID;
	rcd.getData(src);
	dst = dst + hdr.lenOfHdr() + pos * rcdlen_;
	memcpy(dst, src, rcdlen_);
	page.setDirty();
	return 0;
}

RC RMFileHandle::deleteRcd(const RID &rid)
{
	if (!isValidRID(rid)) return RM_BAD_RID;
	Page num = rid.page();
	Slot pos = rid.slot();
	PFPageHandle page = PFGetPage(pffile_, num);	// 获取对应的页面
	Ptr addr = page.rawPtr();
	RMPageHdr hdr(capacity_, addr);

	if (hdr.map.available(pos)) return RM_NORECATRID; // 早已经为free
	hdr.map.set(pos); // 将对应的pos设置为0
	int remain = hdr.remain();
	if (hdr.remain() == 0) {
		hdr.setNext(rmhdr_.free);
		rmhdr_.free = num;
	}
	hdr.setRemain(hdr.remain() + 1);
	page.setDirty();
	return 0;
}


//
// getRcd - 获取一条记录
// 
RC RMFileHandle::getRcd(const RID &rid, RMRecord &rcd)
{
	if (!isValidRID(rid)) return RM_BAD_RID;
	Page num = rid.page();
	Slot pos = rid.slot();
	PFPageHandle page = PFGetPage(pffile_, num);
	Ptr addr = page.rawPtr();
	RMPageHdr hdr(capacity_, addr);
	if (hdr.map.available(pos)) return RM_NORECATRID; // 该pos必定不会空闲
	addr = addr + hdr.lenOfHdr() + pos * rcdlen_;
	rcd.set(addr, rmhdr_.rcdlen, rid);		// 设定记录
	return 0;
}

RC RMFileHandle::forcePages(Page num /* = ALL_PAGES */)
{
	if (!isValidPage(num) && num != ALL_PAGES)
		return RM_BAD_RID;
	return pffile_->forcePages(num);
}

//
// getNextFreeSlot - 获取下一个空闲的pos,一个页面包含很多pos
// 
bool RMFileHandle::nextFreeSlot(PFPageHandle& page, Page &num, Slot& slot)
{
	Ptr addr;
	if (rmhdr_.free > 0) { // 仍然有空闲的页面
		page = PFGetPage(pffile_, rmhdr_.free);
		num = rmhdr_.free;
		addr = page.rawPtr();
	}
	else { // 需要重新分配页面
		page = PFAllocPage(pffile_);
		addr = page.rawPtr();
		num = page.page();
		RMPageHdr hdr(capacity_, addr);
		hdr.setNext(PAGE_LIST_END);
		hdr.setRemain(capacity_);
		int remain = hdr.remain();
		hdr.map.setAll();
		page.setDirty();
		rmhdr_.free = num; // 将分配的页面的页号添加到空闲链表中
		rmhdr_.size++;
		changed_ = true;
	}
	
	RMPageHdr hdr(capacity_, addr);
	for (int i = 0; i < capacity_; i++) {
		if (hdr.map.available(i)) { // 该位置恰好可用
			slot = i;
			return true;
		}
	}
	return false; // unexpected error
}