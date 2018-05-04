#ifndef REDBASE_H
#define REDBASE_H


//
// 一些全局需要用到的变量
//
#define MAXNAME			20				// maximum length of a relname
										// or attribute name
#define MAXSTRINGLEN	255				// maximum length of a string-type attribute
#define MAXATTRS		40				// maximum number of attrubutes in a relname
#define MAXCONDS		40				// maximum number of conditions in a query



//
// Return codes
//
using RC = int;
using Ptr = char *;

#define	PAGE_LIST_END	-1
#define	PAGE_FULLY_USED -2

#define OK_RC			0				// OK_RC return code is guaranteed to always be 0

#define START_PF_ERR  (-1)
#define END_PF_ERR    (-100)
#define START_RM_ERR  (-101)
#define END_RM_ERR    (-200)
#define START_IX_ERR  (-201)
#define END_IX_ERR    (-300)
#define START_SM_ERR  (-301)
#define END_SM_ERR    (-400)
#define START_QL_ERR  (-401)
#define END_QL_ERR    (-500)

#define START_PF_WARN  1
#define END_PF_WARN    100
#define START_RM_WARN  101
#define END_RM_WARN    200
#define START_IX_WARN  201
#define END_IX_WARN    300
#define START_SM_WARN  301
#define END_SM_WARN    400
#define START_QL_WARN  401
#define END_QL_WARN    500

// ALL_PAGES is defined and used by the forcePages method defined in RM and PF layers
const int ALL_PAGES = -1;

//
// Attribute types
//
enum AttrType {
	INT,
	FLOAT,
	STRING
};

//
// Comparison operators
//
enum Operator {
	NO_OP,										// num comparison
	EQ_OP, NE_OP, LT_OP, GT_OP, LE_OP, GE_OP	// binary atomic operators
};

//
// 聚集函数
//
enum AggFun {
	NO_F,
	MIN_F, MAX_F, COUNT_F,
	SUM_F, AVG_F
};

#endif /* REDBASE_H */