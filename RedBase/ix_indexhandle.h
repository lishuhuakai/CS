#ifndef IX_INDEX_HANDLE_H
#define IX_INDEX_HANDLE_H

#include <string.h>
#include <tr1/memory>
#include <vector>
#include "pf.h"
#include "ix.h"
#include "redbase.h"
#include "rm_rid.h"
#include "bplus_node.h"
#include "ix_error.h"
#include "pf_filehandle.h"
#include "noncopyable.h"
using namespace std;

class BPlusNode;
class IXIndexHandle : public noncopyable {
	friend class IXManager;
	friend class IXIndexScan;
	friend class IXIndexHandleTest;
public:
	IXIndexHandle(PFFilePtr &file);
	~IXIndexHandle();
private:
	bool search(NodePtr &curr, int h, KeyPtr key, RID &rid);
	bool insertIndex(NodePtr& node, int height, KeyPtr key, RID& rid, bool &fixed);
	void split(NodePtr &lhs, NodePtr &rhs);
	int findRebalance(NodePtr curr, NodePtr left, NodePtr right, NodePtr lanchor,
		NodePtr ranchor, KeyPtr key, RID &rid, int h, bool &updateKey);
	int rebalance(NodePtr curr, NodePtr left, NodePtr right, NodePtr lanchor, 
		NodePtr ranchor, bool &updateKey);
	void shift(NodePtr curr, NodePtr sib, NodePtr anchor, bool leftSib, bool &updateKey);
	int rootCollapse(NodePtr oldRoot, bool leaf);
	int merge(NodePtr curr, NodePtr sib, NodePtr anchor, bool leftSib, bool &updateKey);
	bool fixKeys(NodePtr curr, KeyPtr &key, RID &rid, bool &updateKey, int h);
public:
	RC insertIndex(KeyPtr key, RID rid);
	RC eraseIndex(KeyPtr key, RID rid);
	bool search(KeyPtr pKey, RID &rid);
public:
	NodePtr loadNode(Page num);
	NodePtr allocNode();
	void disposeNode(Page num);
public:
	int height() { return hdr_.height; }
	int capacity() { return hdr_.capacity; }
	AttrType type() { return hdr_.type; }
private:
	IXFileHdr hdr_;
	PFFilePtr file_;
	bool changed_;
	NodePtr root_;
	Page leaf_;
};

using IXIndexPtr = shared_ptr<IXIndexHandle>;

#endif /* IX_INDEX_HANDLE_H */