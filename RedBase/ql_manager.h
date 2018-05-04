#ifndef QL_MANAGER_H
#define QL_MANAGER_H
#include <stdlib.h>
#include <string.h>
#include "parser_interp.h"
#include "pf_manager.h"
#include "rm_manager.h"
#include "ql_manager.h"
using namespace std;

struct Value;
struct Condition;
struct AggRelAttr;
struct RelAttr;

class QLManager {
public:
	QLManager() {}
	~QLManager() {}
public:
	RC insert(const char* relname, int nvals, Value vals[]);
	RC select(int nselattrs,             // # attrs in select clause
		const AggRelAttr selattrs[],     // attrs in select clause              
		int   nRelations,                // # relations in from clause
		const char * const relations[],  // relations in from clause
		int   nconditions,               // # conditions in where clause
		const Condition conditions[],    // conditions in where clause
		int order,                       // order from order by clause
		RelAttr orderattr,               // the single attr ordered by
		bool group,
		RelAttr groupattr);
};

#endif /* QL_MANAGER_H */