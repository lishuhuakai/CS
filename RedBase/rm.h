// 
//	Record Manager component interface
//	

#ifndef RM_H
#define RM_H

#include "comp.h"

//
// RMFileHdr - 文件头部
// 
struct RMFileHdr {
	int free;				// 链表中第一个空闲页
	int size;				// 文件中已经分配了的页的数目
	uint rcdlen;			// 记录的大小
} __attribute__((packed));

struct RMPageHdrEx {
	int free;
	int slots;
	int remain;
	char slotMap[];
} __attribute__((packed));

/*
 
		|-----------------------|
		|	free				|
		|-----------------------|
		|	free				|
		|-----------------------|
		|	slots				|
		|-----------------------|
		|	remain				|
		|-----------------------|
		|	slotMap				|
		|						|
		|						|
		|						|
		|-----------------------|

 */





#endif /* RM_H */