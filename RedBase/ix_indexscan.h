
#ifndef IX_INDEXSCAN_H
#define IX_INDEXSCAN_H

#include "redbase.h"
#include "rm_rid.h"
#include "comp.h"
#include "ix_indexhandle.h"
class BPlusNode;
class IXIndexHandle;

class IXIndexScan {
public:
	IXIndexScan() 
		: opened_(false), eof_(false), comp_(nullptr)
		, index_(nullptr), curr_(nullptr)
		, pos_(0), op_(NO_OP), val_(nullptr)
	{}
	~IXIndexScan();
public:
	RC openScan(const IXIndexPtr &index, Operator comp, const void* val);
	RC getNextEntry(RID &rid);
	RC rewind();
	RC closeScan();
public:
	RC getNextEntry(KeyPtr &key, RID& rid);
private:
	Comp *comp_;
	IXIndexPtr index_;
	NodePtr curr_;
	int pos_;
	bool opened_;
	bool eof_;
	Operator op_;
	const void* val_;
};

#endif /* IX_INDEXSCAN_H */