#ifndef RM_RECORD_H
#define RM_RECORD_H
#include "rm_error.h"
//
// RMRecord
// 
class RMRecord {
	friend class RMRecordTest;
public:
	RMRecord();
	~RMRecord();
public:
	RC getData(Ptr &p) const;
	RC set(Ptr p, uint size, RID rid);
	RC getRid(RID &rid) const;
	Ptr rawPtr() { return addr_;}
	RID& rid() { return rid_; }
private:
	uint rcdlen_;	// 记录所占的字节大小
	Ptr addr_;		// 字节的地址
	RID rid_;		// 记录在文件中的位置
};

#endif