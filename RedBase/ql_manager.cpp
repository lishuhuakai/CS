#include <algorithm>
#include "redbase.h"
#include "ql_manager.h"
#include "parser_interp.h"
#include "ql_error.h"
#include "printer.h"
#include "ql_query.h"
using namespace std;

#define MAXDATAATTR		(3 * MAXATTRS)
#define MAXCONDITIONS	(2 * MAXATTRS)
#define MAXCHAR			1024
/*~~~~~~~~~~~~~~~~~~~~~~~~~~Utilitys~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
extern PFManager pfManager;
extern RMManager rmManager;
extern IXManager ixManager;
extern SMManager smManager;

static char buffer[1024];

static DataAttr attrs[MAXDATAATTR];

//
// insert - 将数据插入到relname这张表中
//
RC QLManager::insert(const char* relname, int nvals, Value vals[])
{
	RC errval;
	int nattrs;
	/* 插入的时候有一个问题需要解决,那就是需要校验数据是否符合规范 */
	smManager.lookupAttrs(relname, nattrs, attrs);
	if (nattrs != nvals) {
		return QL_INVALIDSIZE;
	}

	int size = 0;
	for (int i = 0; i < nvals; i++) {
		if (vals[i].type != attrs[i].type) return QL_JOINKEYTYPEMISMATCH;
		size += attrs[i].len;
	}
	int offset = 0;
	for (int i = 0; i < nvals; i++) {
		memcpy(buffer + offset, vals[i].data, attrs[i].len);
		offset += attrs[i].len;
	}
	RMFilePtr file;
	RID rid;
	rmManager.openFile(relname, file);
	errval = file->insertRcd(buffer, rid);
	rmManager.closeFile(file);
	return 0;
}

//
// select : 解析select命令
// 
RC QLManager::select(int nselattrs, const AggRelAttr selattrs[],
	int nrelations, const char * const relations[],
	int nconditions, const Condition conditions[],
	int order, RelAttr orderattr, bool group, RelAttr groupattr)
{
	Query *query = query_new(nselattrs, selattrs, nrelations, relations,
		nconditions, conditions, order, orderattr, group, groupattr);
	Printer print(query->discs(), query->discsSize()); /* Printer具体负责输出 */
	query->open();
	Item it;
	RC errval;
	print.printHeader();
	for ( ; ; ) {
		errval = query->next(it);
		if (errval == QL_EOF) break;
		print.print(it.data);
	}
	print.printFooter();
	query->close();
	query_free(query);
	return 0;
}


