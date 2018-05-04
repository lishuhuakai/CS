
#ifndef RM_RID_H
#define RM_RID_H

#include "redbase.h"
#include "pf.h"
#include <iostream>
#include <assert.h>
#include <string.h>
using namespace std;

using Slot = int;

//
// RID - Record id interface
// 
struct RID {
	Page num;
	Slot st;
	RID() : num(-1), st(-1) {}
	RID(Page num, Slot st) : num(num), st(st) {}
	~RID() {};
	Page page() const { return num; }
	Slot slot() const { return st; }
	bool operator==(const RID& rhs) const
	{
		return (num == rhs.num) && (st == rhs.st);
	}
	RID& operator=(const RID& rhs)
	{
		if (this == &rhs) return *this;
		num = rhs.num;
		st = rhs.st;
		return *this;
	}
};

ostream& operator<<(ostream& os, const RID& r);
#endif /* RM_RID_H */