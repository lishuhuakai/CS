#ifndef PF_H
#define PF_H
#include <assert.h>
#include "redbase.h"
//
// Page用来唯一地标志文件中一个页
//
typedef int Page;
typedef unsigned int uint;


// Page Size
// 每一个页面中都保存着头部的一些信息。PageHdr定义在pf_internal.h文件中。它包含着
// 我们应当保存的一些信息。但是很不幸，在这里，我们不能使用sizeof(PageHdr),但是实际上
// sizeof(PageHdr) == sizeof(int).
const int PF_PAGE_SIZE = 4096 - sizeof(int);

//
// FileHdr - 每个数据文件的开头都是这样一个结构,用于描述这个文件的使用情况
//
struct PFFileHdr {
	int free;		/* 空闲页组成的链表 */
	uint size;		/* 页的数目 */
};


#endif /* PF_H */