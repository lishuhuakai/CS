#ifndef QL_STREAM_H
#define QL_STREAM_H

#include <iostream>
#include <vector>
#include <map>
#include "redbase.h"
#include "data_attr.h"
#include "parser_interp.h"
#include "ix_indexscan.h"
#include "ix_indexhandle.h"
#include "rm_filescan.h"
#include "rm_manager.h"
#include "noncopyable.h"
using namespace std;


//
// CharCmp - 主要是要将char*作为map的key,所以需要自定义比较函数
// 
struct CharCmp {
	bool operator()(const char * lhs, const char * rhs) const
	{
		return strcmp(lhs, rhs) < 0;
	}
};


//
// Steam - 每一个流都是一个过滤器
//
class Stream : public noncopyable
{
	friend class CombWrapper;
	friend class Query;
public:
	Stream(int nattrs, const DataAttr attrs[], int nconds, const Condition conds[]) 
		: attrs_(attrs, attrs + nattrs), conds_(conds, conds + nconds)
	{
		if (attrs_.size() == 0) {
			rcdlen_ = 0;
		}
		else {
			auto &last = attrs_.back();
			rcdlen_ = last.offset + last.len;
		}
		/* 将属性所在的下标放入indicator中,方便查询 */
		for (int i = 0; i < attrs_.size(); i++) {
			indicator_[attrs[i].attrname] = i;
		}
	}
	virtual ~Stream() {}
public:
	virtual RC open() = 0;
	virtual RC next(uint8_t *data) = 0;
	virtual RC close() = 0;
	virtual RC rewind() = 0;
	virtual void clean() = 0;	/* 用于释放内部的资源 */
public:
	int rcdlen() { return rcdlen_; }
public:
	void appendCond(const Condition cond) { conds_.push_back(cond); }
protected:
	int rcdlen_; /* 记录的长度 */
	map<const char*, int, CharCmp> indicator_;
	vector<Comp *> comps_;
	vector<Condition> conds_;
	vector<DataAttr> attrs_;
};


class IdxWrapper : public Stream
{
public:
	IdxWrapper(const char* pathname, const DataAttr &idxattr, Operator op, const void* val,
		int nattrs, const DataAttr attrs[], int nconds, const Condition conds[]);
	~IdxWrapper();
public:
	virtual RC open();
	virtual RC next(uint8_t *data);
	virtual RC close();
	virtual RC rewind();
	virtual void clean() {}
private:
	IXIndexScan scan_;
	RMFilePtr file_;
	IXIndexPtr index_;
	vector<int> loffsets_; /* 条件左侧的变量在记录中的偏移量 */
	const char* pathname_;
};

class RcdWrapper : public Stream
{
	friend class CombStream;
public:
	RcdWrapper(const char* pathname,
		int nattrs, const DataAttr attrs[],
		int nconds, const Condition conds[])
		: Stream(nattrs, attrs, nconds, conds)
		, pathname_(pathname)
	{}
	~RcdWrapper();
public:
	virtual RC open();
	virtual RC next(uint8_t *data);
	virtual RC close();
	virtual void clean() {}
protected:
	virtual RC rewind();
private:
	RMFileScan scan_;
	RMFilePtr file_;
	vector<int> loffsets_;	/* 条件左侧的变量在记录中的偏移量 */
	const char* pathname_;
};

//
// CombWrapper - 处理两张表的合并操作
//
class CombWrapper : public Stream
{
public:
	CombWrapper(Stream *lhs, Stream *rhs)
		: Stream(0, nullptr, 0, nullptr), lhs_(lhs), rhs_(rhs), ldata_(nullptr)
	{
		auto &lattrs = lhs->attrs_; /* 左边表的属性 */
		auto &rattrs = rhs->attrs_; /* 右边表的属性 */
		int lrcdlen = 0;
		/* 两张表合并之后的属性 */
		for (int i = 0; i < lattrs.size(); i++) {
			attrs_.push_back(lattrs[i]);
		}
		
		auto &llast = lattrs.back();
		lrcdlen = llast.offset + llast.len;
		
		/* 右边的流的值需要添加一个偏移量 */
		for (int i = 0; i < rattrs.size(); i++) {
			DataAttr attr = rattrs[i];
			attr.offset += lrcdlen;
			attrs_.push_back(attr);
		}
		auto &last = attrs_.back();
		rcdlen_ = last.offset + last.len;
	}

	~CombWrapper();
public:
	virtual RC open();
	virtual RC next(uint8_t *data);
	virtual RC close();
	virtual void clean();
protected:
	virtual RC rewind();
private:
	Stream *lhs_;
	Stream *rhs_;
	vector<int> loffsets_;
	vector<int> roffsets_;
	uint8_t *ldata_;
};




#endif /* QL_STREAM_H */