#ifndef PARSER_INTERP_H
#define PARSER_INTERP_H
#include <iostream>
#include "parser.h"
#include "parser_node.h"
using namespace std;

struct AttrInfo {
	char *attrname;
	AttrType type;
	int len;
};

struct Value {
	AttrType type;
	void* data;
	friend ostream& operator<<(ostream& os, const Value& v);
};

struct AggRelAttr {
	AggFun func;
	char *relname;  /* 表名 */
	char *attrname; /* 属性名 */
	friend std::ostream &operator<<(std::ostream &s, const AggRelAttr &ra);
};

struct RelAttr {
	char* relname;
	char* attrname;
	friend std::ostream &operator<<(std::ostream &s, const RelAttr &ra);
};

struct Condition {
	RelAttr  lhsAttr;    /* left-hand side attribute            */
	Operator op;         /* comparison operator                 */
	bool      bRhsIsAttr;/* TRUE if the rhs is an attribute,    */
						 /* in which case rhsAttr below is valid;*/
						 /* otherwise, rhsValue below is valid.  */
	RelAttr  rhsAttr;    /* right-hand side attribute            */
	Value    rhsValue;   /* right-hand side value                */
						 /* print function                       */
	friend std::ostream &operator<<(std::ostream &s, const Condition &c);

};

int interp(NODE *n);
#endif /* PARSER_INTERP_H */

