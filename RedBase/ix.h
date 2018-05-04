#ifndef IX_H
#define IX_H

#include "redbase.h"
#include "pf.h"

using KeyPtr = void *;

//
// IXFileHdr - 索引文件的头部
// 
struct IXFileHdr {
	uint size;		// 页的数目
	uint limitSpace; // 每一页最多只能够使用的空间大小
	Page root;		// 第一页的位置
	Page leaf;		// 第一个叶子所在的页面编号
	uint pairSize;  // 键值对的大小
	uint height;	// 二叉树的高度
	AttrType type;  // 键的类型
	uint len;		// 键的长度
	uint capacity;  // 记录下一个node里面可以放入key的数目
};

#endif /* IX_H */