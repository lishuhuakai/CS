#ifndef IX_MANAGER_H
#define IX_MANAGER_H

#include "redbase.h"
#include "rm_rid.h"
#include "pf_manager.h"
#include "pf.h"
#include "ix_indexhandle.h"

class IXIndexHandle;
class IXManager {
public:
	IXManager() {};
	~IXManager() {};
public:
	RC createIndex(const char* prefix, int num, AttrType type, int len, uint limitSpace=PF_PAGE_SIZE);
	RC destroyIndex(const char* prefix, int num);
	RC openIndex(const char* prefix, int num, IXIndexPtr &index);
	RC closeIndex(IXIndexPtr &index);
};

#endif