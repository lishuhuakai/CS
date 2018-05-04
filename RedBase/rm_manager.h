#ifndef RM_MANAGER_H
#define RM_MANAGER_H
#include "pf.h"
#include "rm.h"
#include "rm_error.h"
#include "rm_filehandle.h"
#include "pf_manager.h"

class RMManager {
public:
	RMManager() {};
	~RMManager() {};
public:
	RC createFile(const char* pathname, uint rcdlen);
	RC destroyFile(const char* pathname);
	RC openFile(const char* pathname, RMFilePtr &rmfile);
	RC closeFile(RMFilePtr& rmfile);
};

#endif /* RM_MANAGER_H */