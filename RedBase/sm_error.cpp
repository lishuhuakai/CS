#include <cerrno>
#include <cstdio>
#include <iostream>
#include "sm_error.h"
#include "sm.h"
using namespace std;

//
// Error table
//
const char *SMWarnMsg[] = {
	"key was not found in btree",
	"key attribute size is too small(<=0) or too big for a page",
	"key,rid already exists in index",
	"SM_NOSUCHENTRY relName/attrName do not exist in DB",
};

const char *SMErrorMsg[] = {
	"DB is already open",
	"Bad DB name - no such DB.",
	"DB is not open",
	"Bad Table name - no such table",
	"bad open",
	"file is not open",
	"Bad Attribute name specified for this relation",
	"Bad Table/Relation",
	"SM_INDEXEXISTS Index already exists.",
	"SM_TYPEMISMATCH Types do not match.",
	"SM_BADOP Bad operator in condition.",
	"SM_AMBGATTR Attribute ambiguous - more than 1 relation contains same attr.",
	"SM_BADPARAM Bad parameters used with set on commandline",
	"SM_BADAGGFUN Bad Aggregate function - not supported",
};

//
// SM_PrintError
//
void SMPrintError(RC rc)
{
	// Check the return code is within proper limits
	if (rc >= START_SM_WARN && rc <= SM_LASTWARN)
		// Print warning
		cerr << "SM warning: " << SMWarnMsg[rc - START_SM_WARN] << "\n";
	// Error codes are negative, so invert everything
	else if ((-rc >= -START_SM_ERR) && -rc <= -SM_LASTERROR)
	{
		// Print error
		cerr << "SM error: " << SMErrorMsg[-rc + START_SM_ERR] << "\n";
	}
	else if (rc == 0)
		cerr << "SM_PrintError called with return code of 0\n";
	else
	{
		// Print error
		cerr << "rc was " << rc << endl;
		cerr << "START_SM_ERR was " << START_SM_ERR << endl;
		cerr << "SM_LASTERROR was " << SM_LASTERROR << endl;
		cerr << "SM error: " << rc << " is out of bounds\n";
	}
}