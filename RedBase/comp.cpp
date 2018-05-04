#include <string.h>
#include <math.h>
#include "comp.h"

#define EPSILON 0.000001


class IntComp : public Comp {
public:
	IntComp() : Comp(INT, sizeof(int)) {}
public:
	virtual bool eval(const void* lhs, Operator op, const void* rhs);
};

class FloatComp : public Comp {
public:
	FloatComp() : Comp(FLOAT, sizeof(float)) {}
public:
	virtual bool eval(const void* lhs, Operator op, const void* rhs);
};

class StrComp : public Comp {
public:
	StrComp(int len) : Comp(STRING, len) {}
public:
	virtual bool eval(const void* lhs, Operator op, const void* rhs);
};


bool IntComp::eval(const void* lhs, Operator op, const void* rhs)
{
	if (op == NO_OP) return true;
	int lval = *(int *)lhs, rval = *(int *)rhs;
	switch (op)
	{
		case GE_OP: return lval >= rval;
		case EQ_OP: return lval == rval;
		case GT_OP: return lval > rval;
		case LE_OP: return lval <= rval;
		case LT_OP: return lval < rval;
		case NE_OP: return lval != rval;
		default: return true;
	}
}

bool FloatComp::eval(const void* lhs, Operator op, const void* rhs)
{
	if (op == NO_OP) return true;
	float lval = *(float *)lhs, rval = *(float *)rhs;
	switch (op)
	{
		case EQ_OP: return fabs(lval - rval) < EPSILON;
		case NE_OP: return fabs(lval - rval) > EPSILON;
		case LT_OP: return lval - rval < -EPSILON;
		case GT_OP: return lval - rval > EPSILON;
		case LE_OP: return (lval - rval < -EPSILON) || (fabs(lval - rval) < EPSILON);
		case GE_OP: return (lval - rval > EPSILON) || (fabs(lval - rval) > EPSILON);
		default: return true;
	}
}

bool StrComp::eval(const void* lhs, Operator op, const void* rhs)
{
	if (op == NO_OP) return true;
	char* lval = (char *)lhs, *rval = (char *)rhs;
	switch (op)
	{
		case EQ_OP: return strncmp(lval, rval, len_) == 0;
		case NE_OP: return strncmp(lval, rval, len_) != 0;
		case LT_OP: return strncmp(lval, rval, len_) < 0;
		case GT_OP: return strncmp(lval, rval, len_) > 0;
		case LE_OP: return strncmp(lval, rval, len_) <= 0;
		case GE_OP: return strncmp(lval, rval, len_) >= 0;
		default: return true;
	}

}

Comp* make_comp(AttrType type, int len)
{
	switch (type)
	{
		case INT: return new IntComp();
		case FLOAT: return new FloatComp();
		case STRING: return new StrComp(len);
	}
	return nullptr;
}