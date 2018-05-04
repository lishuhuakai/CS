#ifndef QL_PROJECTION_H
#define QL_PROJECTION_H

#include <iostream>
#include <vector>
#include <tuple>
#include "ql_stream.h"
#include "data_attr.h"
#include "noncopyable.h"
#include "avl_map.h"
using namespace std;

struct Item : public noncopyable
{
	int rcdlen;
	uint8_t *data;
};

//
// ItemDiscrip 用于标记属性
// 
struct ItemDiscrip
{
	AggFun func;		/* 聚集函数 */
	char relname[MAXNAME + 1];  /* 表的名称 */
	char attrname[MAXNAME + 1]; /* 属性的名称 */
	AttrType type; /* 类型 */
	int len; /* 长度 */
};

// comp_func 比较函数
typedef bool(*comp_func)(const void* lhs, const void* rhs, int offset, int comp_len);

struct SortConf
{
	int comp_len;
	int offset;
	comp_func func;
};

//
// Query - 处理记录的投影
//
class Query
{
public:
	Query(Stream *stream, int nselattrs, const AggRelAttr selattrs[],
		int order, RelAttr &orderattr, bool group, RelAttr &groupattr);
	~Query();
public:
	const ItemDiscrip* discs() { return  &discs_[0]; }
	int discsSize() { return discs_.size(); }
	bool open() { stream_->open(); }
	RC next(Item &it);
	RC close() { return stream_->close(); }
	void clean();
private:
	int chooseGroup(uint8_t *data);
	void handleMax(vector<uint8_t*>& datas, DataAttr&attr, uint8_t *data);
	void handleMin(vector<uint8_t*>& datas, DataAttr& attr, uint8_t *data);
	void handleAvg(vector<uint8_t*>& datas, DataAttr& attr, uint8_t *data);
	void handleSum(vector<uint8_t*>& datas, DataAttr& attr, uint8_t *data);
private:
	bool collected_;	/* 是否已经采集了所有的数据 */
	int pos_;		   /* 记录已经传递了的数据的个数 */
	bool eof_;
	int rcdlen_;
	bool containAgg_;	/* 查询中是否包含了聚合函数 */
	Stream *stream_;
	vector<AggRelAttr> aggs_;
	vector<DataAttr> attrs_;
	vector<ItemDiscrip> discs_;
	vector<vector<uint8_t *>> groupdatas_;
	AVLMap *printeddatas_; /* 用于存储所有已经输出了的数据 */
	//AVLMap *groupdatass_;
private:
	/* 排序和分组相关 */
	int order_;
	SortConf conf_;
	bool group_;
	int groupattrOffset_;
	int groupattrLen_;
	int grouppos_;
};



//
// DataComp - 主要用来处理记录的比较
//
template<typename T>
class DataComp
{
public:
	DataComp(int offset)
		: offset_(offset)
	{}
public:

	bool findMax(vector<uint8_t *>& datas, uint8_t* data)
	{
		if (datas.size() == 0) return false;
		T max = *(T *)(datas[0] + offset_), val;
		for (int i = 1; i < datas.size(); i++) {
			val = *(T *)(datas[i] + offset_);
			if (max < val) {
				max = val;
			}
		}
		memcpy(data, &max, sizeof(T));
		return true;
	}

	bool findMin(vector<uint8_t *>& datas, uint8_t* data)
	{
		if (datas.size() == 0) return false;
		T min = *(T *)(datas[0] + offset_), val;
		for (int i = 1; i < datas.size(); i++) {
			val = *(T *)(datas[i] + offset_);
			if (min > val) {
				min = val;
			}
		}
		memcpy(data, &min, sizeof(T));
		return true;
	}

	bool calcAvg(vector<uint8_t *>& datas, uint8_t* data)
	{
		float avg = 0;
		for (int i = 0; i < datas.size(); i++) {
			avg += *(T *)(datas[i] + offset_);
		}
		avg = avg / datas.size();
		memcpy(data, &avg, sizeof(float));
		return true;
	}

	bool calcSum(vector<uint8_t *>& datas, uint8_t* data)
	{
		T sum = 0;
		for (int i = 0; i < datas.size(); i++) {
			sum += *(T *)(datas[i] + offset_);
		}
		memcpy(data, &sum, sizeof(T));
		return true;
	}
private:
	int offset_;
};

Query* query_new(int nselattrs, const AggRelAttr selattrs[],
	int nrelations, const char * const relations[],
	int nconditions, const Condition conditions[],
	int order, RelAttr &orderattr, bool group, RelAttr &groupattr);

void query_free(Query* query);

#endif /* QL_PROJECTION_H */