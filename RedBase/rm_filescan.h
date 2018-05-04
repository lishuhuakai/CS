#ifndef RM_FILESCAN_H
#define RM_FILESCAN_H
#include "rm.h"
#include "rm_error.h"
#include "rm_filehandle.h"
#include "rm_record.h"

//
// RMFileScan - 用作记录的扫描
//
class RMFileScan {
public:
	RMFileScan() 
		: comp_(nullptr), curr_(1, -1), op_(NO_OP)
		, offset_(0), val_(nullptr) {};
	~RMFileScan() {};
public:
	RC openScan(RMFilePtr& rmfile, AttrType attr, int len,
		int offset, Operator op,const void* val);
	RC getNextRcd(RMRecord &rcd);
	RC rewind();
	RC closeScan();
private:
	Comp *comp_;
	int offset_;
	Operator op_;
	const void* val_;
private:
	RID curr_;
	RMFilePtr rmfile_;
};

#endif /* RM_FILESCAN_H */