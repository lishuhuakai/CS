#include <iostream>
#include "ql_error.h"
using namespace std;

//
// Error table
//
const char *QLWarnMsg[] = {
	"key was not found in btree",
	"QL_INVALIDSIZE invalid number of attributes",
	"key,rid already exists in index",
	"key,rid combination does not exist in index",
};

const char *QLErrorMsg[] = {
	"QL_BADJOINKEY Bad Join Key - is it present ?",
	"QL_ALREADYOPEN Iterator already open.",
	"QL_BADATTR Bad Attribute",
	"QL_DUPREL Duplicate Table names",
	"QL_RELMISSINGFROMFROM Relation in WHERE missing from FROM",
	"QL_FNOTOPEN Iterator is not open",
	"QL_JOINKEYTYPEMISMATCH Type mismatch",
	"QL_BADOPEN QL Manager is in bad state or not open",
	"QL_EOF end of input on iterator",
};

//
// QL_PrintError
//
// Desc: Send a message corresponding to a QL return code to cerr
// In:   rc - return code for which a message is desired
//
void QLPrintError(RC rc)
{
	// Check the return code is within proper limits
	if (rc >= START_QL_WARN && rc <= QL_LASTWARN)
		// Print warning
		cerr << "QL warning: " << QLWarnMsg[rc - START_QL_WARN] << "\n";
	// Error codes are negative, so invert everything
	else if ((-rc >= -START_QL_ERR) && -rc <= -QL_LASTERROR)
	{
		// Print error
		cerr << "QL error: " << QLErrorMsg[-rc + START_QL_ERR] << "\n";
	}
	else if (rc == 0)
		cerr << "QL_PrintError called with return code of 0\n";
	else
	{
		// Print error
		cerr << "rc was " << rc << endl;
		cerr << "START_QL_ERR was " << START_QL_ERR << endl;
		cerr << "QL_LASTERROR was " << QL_LASTERROR << endl;
		cerr << "QL error: " << rc << " is out of bounds\n";
	}
}