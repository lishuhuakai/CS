//
// rm_record.cpp
// 类RMRecord的实现.
//
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include "rm.h"
#include "rm_rid.h"
#include "rm_record.h"

using namespace std;


RMRecord::RMRecord()
	:rcdlen_(-1), addr_(nullptr), rid_(-1, -1)
{}

RMRecord::~RMRecord()
{
	if (addr_ != nullptr) {
		free(addr_);
	}
}

//
// set - 设定记录的信息,从src处拷贝出一条记录,并记录下记录的大小,位置信息
// 
RC RMRecord::set(Ptr src, uint len, RID rid)
{
	if ((rcdlen_ != -1) && (len != rcdlen_)) return RM_RECSIZEMISMATCH;
	rcdlen_ = len;		 // 记录下每一条记录的大小
	rid_ = rid;		 // 记录下对应关系
	if (addr_ == nullptr) addr_ = (char *)malloc(sizeof(char) * len);
	memcpy(addr_, src, len);
	return 0;
}

RC RMRecord::getData(Ptr &p) const
{
	if (addr_ != nullptr && rcdlen_ != -1) {
		p = addr_;
		return 0;
	}
	else return RM_NULLRECORD;
}

RC RMRecord::getRid(RID &rid) const
{
	if (addr_ != nullptr && rcdlen_ != -1)
	{
		rid = rid_;
		return 0;
	}
	else return RM_NULLRECORD;
}

ostream& operator<<(ostream& os, const RID& r)
{
	os << "[" << r.page() << "," << r.slot() << "]";
	return os;
}

