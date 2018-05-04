#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "redbase.h"
#include "parser_node.h"
#include "parser_interp.h"
#include "sm_manager.h"
#include "ql_manager.h"
#include "pf_manager.h"

extern SMManager smManager;
extern QLManager qlManager;
extern PFManager pfManager;
extern bool stop;

#define E_OK                0
#define E_INCOMPATIBLE      -1
#define E_TOOMANY           -2
#define E_NOLENGTH          -3
#define E_INVINTSIZE        -4
#define E_INVREALSIZE       -5
#define E_INVFORMATSTRING   -6
#define E_INVSTRLEN         -7
#define E_DUPLICATEATTR     -8
#define E_TOOLONG           -9
#define E_STRINGTOOLONG     -10

#define ERRFP stderr

/*
* print_error: prints an error message corresponding to errval
*/
static void print_error(const char *errmsg, RC errval)
{
	if (errmsg != NULL)
		fprintf(stderr, "%s: ", errmsg);
	switch (errval) {
	case E_OK:
		fprintf(ERRFP, "no error\n");
		break;
	case E_INCOMPATIBLE:
		fprintf(ERRFP, "attributes must be from selected relation(s)\n");
		break;
	case E_TOOMANY:
		fprintf(ERRFP, "too many elements\n");
		break;
	case E_NOLENGTH:
		fprintf(ERRFP, "length must be specified for STRING attribute\n");
		break;
	case E_INVINTSIZE:
		fprintf(ERRFP, "invalid size for INTEGER attribute (should be %d)\n",
			(int)sizeof(int));
		break;
	case E_INVREALSIZE:
		fprintf(ERRFP, "invalid size for REAL attribute (should be %d)\n",
			(int)sizeof(float));
		break;
	case E_INVFORMATSTRING:
		fprintf(ERRFP, "invalid format string\n");
		break;
	case E_INVSTRLEN:
		fprintf(ERRFP, "invalid length for string attribute\n");
		break;
	case E_DUPLICATEATTR:
		fprintf(ERRFP, "duplicated attribute name\n");
		break;
	case E_TOOLONG:
		fprintf(stderr, "relation name or attribute name too long\n");
		break;
	case E_STRINGTOOLONG:
		fprintf(stderr, "string attribute too long\n");
		break;
	default:
		fprintf(ERRFP, "unrecognized errval: %d\n", errval);
	}
}

/*
* parse_format_string: deciphers a format string of the form: xl
* where x is a type specification (one of `i' INTEGER, `r' REAL,
* `s' STRING, or `c' STRING (character)) and l is a length (l is
* optional for `i' and `r'), and stores the type in *type and the
* length in *len.
*
* Returns
*    E_OK on success
*    error code otherwise
*/
static int parse_format_string(char *format_string, AttrType *type, int *len)
{
	int n;
	char c;

	/* extract the components of the format string */
	n = sscanf(format_string, "%c%d", &c, len);

	/* if no length given... */
	if (n == 1) {

		switch (c) {
		case 'i':		/* int */
			*type = INT;
			*len = sizeof(int);
			break;
		case 'f':
		case 'r':		/* float or real */
			*type = FLOAT;
			*len = sizeof(float);
			break;
		case 's':		/* string */
		case 'c':		/* char */
			return E_NOLENGTH;
		default:
			return E_INVFORMATSTRING;
		}
	}

	/* if both are given, make sure the length is valid */
	else if (n == 2) {

		switch (c) {
		case 'i':
			*type = INT;
			if (*len != sizeof(int))
				return E_INVINTSIZE;
			break;
		case 'f':
		case 'r':
			*type = FLOAT;
			if (*len != sizeof(float))
				return E_INVREALSIZE;
			break;
		case 's':
		case 'c':
			*type = STRING;
			if (*len < 1 || *len > MAXSTRINGLEN)
				return E_INVSTRLEN;
			break;
		default:
			return E_INVFORMATSTRING;
		}
	}

	/* otherwise it's not a valid format string */
	else
		return E_INVFORMATSTRING;

	return E_OK;
}

/*
* mk_attr_infos: converts a list of attribute descriptors (attribute names,
* types, and lengths) to an array of AttrInfo's so it can be sent to
* Create.
*
* Returns:
*    length of the list on success ( >= 0 )
*    error code otherwise
*/
static int mk_attr_infos(NODE *list, int max, AttrInfo infos[])
{
	int i;
	int len;
	AttrType type;
	NODE *n;
	RC errval;

	/* for each element of the list... */
	for (i = 0; list != NULL; ++i, list = list->u.LIST.next) {

		/* if the list is too long, then error */
		if (i == max)
			return E_TOOMANY;

		n = list->u.LIST.curr;

		/* Make sure the attribute name isn't too long */
		if (strlen(n->u.ATTRTYPE.attrname) > MAXNAME)
			return E_TOOLONG;

		/* interpret the format string */
		errval = parse_format_string(n->u.ATTRTYPE.type, &type, &len);
		if (errval != E_OK)
			return errval;

		/* add it to the list */
		infos[i].attrname = n->u.ATTRTYPE.attrname;
		infos[i].type = type;
		infos[i].len = len;
	}
	return i;
}

/*
* mk_values: converts a single value node into a Value
*/
static void mk_value(NODE *node, Value &value)
{
	value.type = node->u.VALUE.type;
	switch (value.type) {
	case INT:
		value.data = (void *)&node->u.VALUE.ival;
		break;
	case FLOAT:
		value.data = (void *)&node->u.VALUE.rval;
		break;
	case STRING:
		value.data = (void *)node->u.VALUE.sval;
		break;
	}
}


/*
* mk_values: converts a list of values into an array of values
*
* Returns:
*    the lengh of the list on success ( >= 0 )
*    error code otherwise
*/
static int mk_values(NODE *list, int max, Value values[])
{
	int i;

	/* for each element of the list... */
	for (i = 0; list != NULL; ++i, list = list->u.LIST.next) {
		/* If the list is too long then error */
		if (i == max)
			return E_TOOMANY;

		mk_value(list->u.LIST.curr, values[i]);
	}

	return i;
}


/*
 * mk_agg_rel_attr: converts a single relation-attribute (<relation,
 * attribute> pair) into a AggRelAttr
 * 将一个relation-attribute对转化为AggRelAttr
 */
static void mk_agg_rel_attr(NODE *node, AggRelAttr &relAttr)
{
	relAttr.func = node->u.AGGRELATTR.func;
	relAttr.relname = node->u.AGGRELATTR.relname; /* relation表示关系，对应数据库中的表,relname即表名 */
	relAttr.attrname = node->u.AGGRELATTR.attrname; /* attribute表示属性，attrname即属性名称 */
}


/*
 * mk_agg_rel_attrs: converts a list of relation-attributes (<relation,
 * attribute> pairs) into an array of AggRelAttrs
 *
 * Returns:
 *    the lengh of the list on success ( >= 0 )
 *    error code otherwise
 */
static int mk_agg_rel_attrs(NODE *list, int max, AggRelAttr relAttrs[])
{
	int i;

	/* For each element of the list... */
	for (i = 0; list != NULL; ++i, list = list->u.LIST.next) {
		/* If the list is too long then error */
		if (i == max)
			return E_TOOMANY;

		mk_agg_rel_attr(list->u.LIST.curr, relAttrs[i]);
	}

	return i;
}

//
// mk_relations - 从list中提取出table的名称
//
static int mk_relations(NODE *list, int max, char *relations[])
{
	int i;
	NODE *current;

	/* for each element of the list... */
	for (i = 0; list != NULL; ++i, list = list->u.LIST.next) {
		/* If the list is too long then error */
		if (i == max)
			return E_TOOMANY;

		current = list->u.LIST.curr;
		relations[i] = current->u.RELATION.relname;
	}

	return i;
}


//
// mk_conditions - 从列表中提取出conditons
//
static int mk_conditions(NODE *list, int max, Condition conditions[])
{
	int i;
	NODE *current;

	/* for each element of the list... */
	for (i = 0; list != NULL; ++i, list = list->u.LIST.next) {
		/* If the list is too long then error */
		if (i == max)
			return E_TOOMANY;

		current = list->u.LIST.curr;
		/* 条件构成 -> relname.attrname op relname.attrname */
		conditions[i].lhsAttr.relname =
			current->u.CONDITION.lhsRelattr->u.RELATTR.relname;
		conditions[i].lhsAttr.attrname =
			current->u.CONDITION.lhsRelattr->u.RELATTR.attrname;
		conditions[i].op = current->u.CONDITION.op;
		if (current->u.CONDITION.rhsRelattr) { /* 右操作数也是属性 */
			conditions[i].bRhsIsAttr = true;
			conditions[i].rhsAttr.relname =
				current->u.CONDITION.rhsRelattr->u.RELATTR.relname;
			conditions[i].rhsAttr.attrname =
				current->u.CONDITION.rhsRelattr->u.RELATTR.attrname;
		}
		else { /* 右操作数是值 */
			conditions[i].bRhsIsAttr = false;
			mk_value(current->u.CONDITION.rhsValue, conditions[i].rhsValue);
		}
	}
	return i;
}


//
// mk_rel_attr: converts a single relation-attribute (<relation, attribute> pair) into a RelAttr
//
static void mk_rel_attr(NODE *node, RelAttr &relAttr)
{
	relAttr.relname = node->u.RELATTR.relname;
	relAttr.attrname = node->u.RELATTR.attrname;
}

//
// mk_order_relattr: converts an int and a single relation-attribute (<relation, attribute> pair) into a int and a RelAttr
//
static void mk_order_relattr(NODE *node, int& order, RelAttr &relAttr)
{
	order = node->u.ORDERATTR.order;
	if (order != 0)
		mk_rel_attr(node->u.ORDERATTR.relattr, relAttr);
}

int interp(NODE *n)
{
	RC errval = 0;
	switch (n->kind)
	{
		case N_EXIT:
			stop = true;
			break;
		case N_CREATETABLE:	/* Create Table */
		{
			int nattrs;
			AttrInfo infos[MAXATTRS];
			if (strlen(n->u.CREATETABLE.relname) > MAXNAME) {
				print_error("create", E_TOOLONG);
				break;
			}
			nattrs = mk_attr_infos(n->u.CREATETABLE.attrlist, MAXATTRS, infos);
			if (nattrs < 0) {
				print_error("create", nattrs);
				break;
			}
			errval = smManager.createTable(n->u.CREATETABLE.relname, nattrs, infos);
			break;
		}
		case N_DROPTABLE:
			smManager.dropTable(n->u.DROPTABLE.relname);
			break;
		case N_DROPINDEX:
			smManager.dropIndex(n->u.DROPINDEX.relname, n->u.DROPINDEX.attrname);
			break;
		case N_INSERT:
		{
			int nvals = 0;
			Value values[MAXATTRS];
			nvals = mk_values(n->u.INSERT.valuelist, MAXATTRS, values);
			if (nvals < 0) {
				print_error("insert", nvals);
				break;
			}

			/* Make the call to insert */
			errval = qlManager.insert(n->u.INSERT.relname, nvals, values);
			break;
		}
		case N_LOAD:
			errval = smManager.load(n->u.LOAD.relname, n->u.LOAD.filename);
			break;
		case N_PRINT:
			errval = smManager.print(n->u.PRINT.relname);
			break;
		case N_HELP:
			if (n->u.HELP.relname)
				errval = smManager.help(n->u.HELP.relname);
			else
				errval = smManager.help();
			break;
		case N_QUERY: /* 查询语句 */
		{
			int nselattrs = 0;
			AggRelAttr relAttrs[MAXATTRS];
			int nrelations = 0; /* 表的数目 */ 
			char* relations[MAXATTRS]; /* 表的名称 */
			int nconditions = 0; /* 条件的数目 */
			Condition conditions[MAXCONDS];
			int order = 0; /* 升序or降序 */
			RelAttr orderAttr; /* 按照orderAttr来排序 */
			bool group = false;
			RelAttr groupAttr; /* 按照groupAttr来分组 */

			/* 开始解析所选择的属性 */
			nselattrs = mk_agg_rel_attrs(n->u.QUERY.relattrlist, MAXATTRS, relAttrs);
			if (nselattrs < 0) {
				print_error("select", nselattrs);
				break;
			}

			/* 开始解析table的名称了 */
			nrelations = mk_relations(n->u.QUERY.rellist, MAXATTRS, relations);
			if (nrelations < 0) {
				print_error("select", nrelations);
				break;
			}

			/* 开始解析条件 */
			nconditions = mk_conditions(n->u.QUERY.conditionlist, MAXCONDS, conditions);
			if (nconditions < 0) {
				print_error("select", nconditions);
				break;
			}

			/* Make the order by attr suitable for sending to Query */
			/* 开始解析排序属性 */
			mk_order_relattr(n->u.QUERY.orderrelattr, order, orderAttr);

			/* Make the group by attr suitable for sending to Query */
			mk_rel_attr(n->u.QUERY.grouprelattr, groupAttr);
			if (groupAttr.attrname != NULL)
				group = true;

			/* 开始调用select函数 */
			errval = qlManager.select(nselattrs, relAttrs, /* 选择的属性 */
				nrelations, relations, /* 选择的表格 */
				nconditions, conditions, /* 条件 */
				order, orderAttr,
				group, groupAttr);
			break;
		}
		default:
			break;
	}
	return errval;
}