//
// rm_error.cpp 
//

#include "rm_error.h"
#include "pf.h"
#include <iostream>
using namespace std;

const char * RMWarnMsg[] = {
	"bad record size <= 0",
	"This rid has no record",
};

const char *RMErrorMsg[] = {
	"record size is too big for a page",
	"error is in the PF component",
	"record null",
	"record size mismatch",
	"attempt to open already open file handle",
	"bad parameters specified to RM open/create file handle",
	"file is not open",
	"Bad RID - invalid page num or slot num",
	"end of file",
};

void RMPrintError(RC rc)
{
	if (rc >= START_RM_WARN && rc <= RM_LASTWARN)
		cerr << "RM warning: " << RMWarnMsg[rc - START_RM_WARN] << endl;
	else if ((-rc >= -START_RM_ERR) && (-rc <= -RM_LASTERROR))
		cerr << "RM error: " << RMErrorMsg[-rc + START_RM_ERR] << endl;
	else if (rc == 0)
		cerr << "RMPrintError called with return code of 0." << endl;
	else
	{
		// Êä³ö´íÎóÐÅÏ¢
		cerr << "rc was" << rc << endl;
		cerr << "START_RM_ERR was " << START_RM_ERR << endl;
		cerr << "RM_LASTERROR was " << RM_LASTERROR << endl;
		cerr << "RM error: " << rc << "is out of bounds" << endl;
	}
}