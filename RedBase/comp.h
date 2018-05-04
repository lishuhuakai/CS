#ifndef COMP_H
#define COMP_H

#include "redbase.h"

class Comp {
public:
	Comp(AttrType type, int len) : type_(type), len_(len) {}
	virtual ~Comp() {}
public:
	virtual bool eval(const void* lhs, Operator op, const void* rhs) = 0;
protected:
	AttrType type_;  /* 类型 */
	int len_;		 /* 长度 */
};

Comp* make_comp(AttrType type, int len);

#endif /* COMP_H */