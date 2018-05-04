#ifndef CATALOG_H
#define CATALOG_H
#include <string.h>
#include "printer.h"

struct DataRel
{
	int rcdlen;			// 记录的长度
	int attrcount;		// 属性的数量
	int pages;			// 使用的页的数目
	int rcds;
	char relname[MAXNAME + 1];

	DataRel()
	{
		memset(relname, 0, MAXNAME + 1);
	}

	static int members() { return 5; }
	//
	// spaceUsage - 占用空间大小,即存储在文件中应当占用多少空间,需要注意的是,在文件中
	// 一条该结构的记录是紧凑存储的,不会为了对齐而出现空洞.
	// 
	static int spaceUsage()
	{
		return sizeof(int) * 4 + MAXNAME + 1;
	}
}__attribute__((packed));
#endif /* CATALOG_H */