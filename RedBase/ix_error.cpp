#include "ix_error.h"
#include <errno.h>
#include <stdio.h>
#include <iostream>
using namespace std;

static const char *IXWarnMsg[] = {
	"key was not found in btree",
	"key attribute size is too small(<=0) or too big for a page",
	"key,rid already exists in index",
	"key,rid combination does not exist in index",
};

static const char *IXErrorMsg[] = {
	"entry is too big",
	"error is in the PF component",
	"Index page with btree node is no longer valid",
	"Index file creation failed",
	"attempt to open already open file handle",
	"bad parameters specified to IX open file handle",
	"file is not open",
	"Bad RID - invalid page num or slot num",
	"Bad Key - null or invalid",
	"end of file",
};

//
// IX_PrintError - 根据返回码,打印相应的出错信息
//
void IXPrintError(RC rc)
{
	// 检查返回码在可控的范围之内
	if (rc >= START_IX_WARN && rc <= IX_LASTWARN)
		cerr << "IX warning: " << IXWarnMsg[rc - START_IX_WARN] << "\n";
	// Error codes are negative, so invert everything
	else if ((-rc >= -START_IX_ERR) && -rc <= -IX_LASTERROR)
	{
		cerr << "IX error: " << IXErrorMsg[-rc + START_IX_ERR] << "\n";
	}
	else if (rc == 0)
		cerr << "IX_PrintError called with return code of 0\n";
	else
	{
		// Print error
		cerr << "rc was " << rc << endl;
		cerr << "START_IX_ERR was " << START_IX_ERR << endl;
		cerr << "IX_LASTERROR was " << IX_LASTERROR << endl;
		cerr << "IX error: " << rc << " is out of bounds\n";
	}
}
