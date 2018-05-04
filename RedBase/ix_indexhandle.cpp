#include <unistd.h>
#include <stdio.h>
#include "ix_indexhandle.h"
#include "bplus_node.h"
#include "pf_filehandle.h"
#include "pf_pagehandle.h"

IXIndexHandle::IXIndexHandle(PFFilePtr &file)
{
	file_ = file;
	PFPageHandle zeropage = PFGetPage(file, 0);
	Ptr addr = zeropage.rawPtr();
	memcpy(&hdr_, addr, sizeof(IXFileHdr));
	PFPageHandle rootpage;
	bool load = true;
	if (hdr_.height > 0) {
		leaf_ = hdr_.leaf;
		rootpage = PFGetPage(file, hdr_.root);
	}
	else { // 高度为0,那么要构建一个root节点
		rootpage = PFAllocPage(file);
		Page num = rootpage.page();
		load = false;
		hdr_.size++;
		hdr_.root = num;
		hdr_.height = 1;
		hdr_.leaf = num;		// 这个节点一定为叶子节点
		leaf_ = num;
	}
	// 将root所在的页保存在内存中
	root_ = make_shared<BPlusNode>(hdr_, rootpage, load);
	changed_ = true;
}

IXIndexHandle::~IXIndexHandle()
{
	if (changed_) {
		PFPageHandle page = PFGetPage(file_, 0);
		Ptr addr = page.rawPtr();
		memcpy(addr, &hdr_, sizeof(IXFileHdr));
		page.setDirty();
	}
}

RC IXIndexHandle::insertIndex(KeyPtr k, RID rid)
{
	if (k == nullptr) return IX_BADKEY;
	KeyPtr key = malloc(hdr_.len);
	memcpy(key, k, hdr_.len);
	bool fixed = true;
	bool stat = insertIndex(root_, 1, key, rid, fixed);
	if (stat && !fixed) {
		NodePtr root = allocNode();
		if (root_->right() == rid.page()) {
			root->updateKey(0, root_->keyAt(0));
			root->updateKey(1, key);
			root->updateRid(0, RID(root_->num_, -1));
			root->updateRid(1, rid);
		}
		else {
			root->updateKey(0, key);
			root->updateKey(1, root->keyAt(0));
			root->updateRid(0, rid);
			root->updateRid(1, RID(root_->num_, -1));
		}
		root->size_ += 2;
		root->setSize(root->size_);
		root_ = root;
		hdr_.height++;
		free(key);
	}
	else if (!stat) return IX_ENTRYEXISTS;
	return 0;
}

void IXIndexHandle::split(NodePtr &lhs, NodePtr &rhs)
{
	int begin = lhs->size_ / 2;
	int size = lhs->size_ - begin;
	memmove(rhs->keyAt(0), lhs->keyAt(begin), size * lhs->len_);
	memmove(&rhs->rids_[0], &lhs->rids_[begin], size * sizeof(RID));
	lhs->size_ = begin;
	lhs->setSize(lhs->size_);
	rhs->size_ = size;
	rhs->setSize(rhs->size_);
}

int IXIndexHandle::findRebalance(NodePtr curr, NodePtr left, NodePtr right,
	NodePtr lanchor, NodePtr ranchor, KeyPtr key, RID &rid, int h, bool &updateKey)
{
	NodePtr nextLeft, nextRight, nextLanchor, nextRanchor;
	int removePtr = -1;
	KeyPtr currKey;
	RID nextRid;
	int keyPos = -1;

	if (h < hdr_.height) { // internal node
		int idx = curr->leftmostSearchPos(key);
		if (idx < 0) return -2;
		for (int i = idx; i < curr->size_; i++) {
			currKey = curr->keyAt(i);
			nextRid = curr->ridAt(i);
			keyPos = i;
			if (curr->keyComp(currKey, key) > 0) break;
			if (i == 0) {
				if (left != nullptr) nextLeft = loadNode(left->ridAt(left->size_ - 1).page());
				else nextLeft = nullptr;
				nextLanchor = left;
			}
			else {
				nextLeft = loadNode(curr->ridAt(i - 1).page());
				nextLanchor = curr;
			}

			if (i == curr->size_ - 1) {
				if (right != nullptr) nextRight = loadNode(right->ridAt(0).page());
				else nextRight = nullptr;
				nextRanchor = right;
			}
			else {
				nextRight = loadNode(curr->ridAt(i + 1).page());
				nextRanchor = curr;
			}

			removePtr = findRebalance(loadNode(nextRid.page()), nextLeft, nextRight, nextLanchor,
				nextRanchor, key, rid, h + 1, updateKey);
			if (updateKey) {
				curr->updateKey(i, key);
				curr->updateRid(i, rid);
			}
			updateKey = (removePtr == -1) && updateKey && (i == 0);
			if (removePtr != -3) break;
		}
	}
	else {
		keyPos = curr->search(key, rid);
		if (keyPos >= 0) {
			currKey = key;
			nextRid = rid;
			removePtr = nextRid.page();
		}
		else return -3;
	}

	if (removePtr == -3) return removePtr;

	int pos = -1;
	if (removePtr == nextRid.page()) {
		curr->erase(keyPos);
		updateKey = (keyPos == 0);
		if ((h == 1) && (curr->size_ == 1)) {
			pos = rootCollapse(root_, leaf_ == root_->num_);
		}
		else if ((h > 1) && (curr->size_ < curr->size_ / 2)) {
			pos = rebalance(curr, left, right, lanchor, ranchor, updateKey);
		}
	}

	if (updateKey) {
		//key = curr->keyAt(0);
		curr->copyKey(0, key);
		rid = RID(curr->num_, -1);
	}
	return pos;
}

int IXIndexHandle::rebalance(NodePtr curr, NodePtr left, NodePtr right, NodePtr lanchor, 
	NodePtr ranchor, bool & updateKey)
{
	NodePtr balance, anchor;
	bool leftSib = false;

	int removePtr = -1;
	if ((right == nullptr) || ((left != nullptr) && (left->size_ > right->size_))) {
		leftSib == true;
		balance = left;
		anchor = lanchor;
	}
	else {
		balance = right;
		anchor = ranchor;
	}

	if (balance->size_ > (balance->size_ / 2)) {
		shift(curr, balance, anchor, leftSib, updateKey);
	}
	else {
		removePtr = merge(curr, balance, anchor, leftSib, updateKey);
	}
	return removePtr;
}

void IXIndexHandle::shift(NodePtr curr, NodePtr sib, NodePtr anchor, bool leftSib, bool & updateKey)
{
	int idx;
	int count = ((sib->size_ + curr->size_) / 2 - curr->size_);
	int len = curr->len_;
	if (leftSib) {
		memmove(curr->keyAt(count), curr->keyAt(0), curr->size_ * len);
		memmove(&curr->rids_[count], &curr->rids_[0], curr->size_ * sizeof(RID));
		memmove(curr->keyAt(0), sib->keyAt(sib->size_ - count), count * len);
		memmove(&curr->rids_[0], &sib->rids_[sib->size_ - count], count * sizeof(RID));
		curr->size_ += count;
		sib->size_ -= count;
		updateKey = true;
	}
	else {
		KeyPtr origKey = malloc(sib->len_);
		sib->copyKey(0, origKey);
		memmove(curr->keyAt(curr->size_), sib->keyAt(0), count * len);
		memmove(&curr->rids_[curr->size_], &curr->rids_[0], count * sizeof(RID));
		memmove(curr->keyAt(0), sib->keyAt(count), (sib->size_ - count) * len);
		memmove(&curr->rids_[0], &sib->rids_[count], (sib->size_ - count) * sizeof(RID));
		curr->size_ += count;
		sib->size_ -= count;
		if ((idx == 0) && (sib->keyComp(sib->keyAt(0), origKey) != 0)) {
			bool update;
			RID rid(sib->num_, -1);
			fixKeys(root_, origKey, rid, update, 1);
		}
		else anchor->updateKey(idx, sib->keyAt(0));
		free(origKey);
	}
	
}

int IXIndexHandle::rootCollapse(NodePtr oldRoot, bool leaf)
{
	RID rid(oldRoot->num_, -1);
	if (leaf) {
		// 什么也不干,暂时不删除唯一的一个节点
	}
	else {
		root_ = loadNode(oldRoot->ridAt(0).page());
		disposeNode(rid.page());
		hdr_.height--;
	}
	return oldRoot->num_;
}


int IXIndexHandle::merge(NodePtr curr, NodePtr sib, NodePtr anchor, bool leftSib, bool & updateKey)
{
	int idx = anchor->search(sib->keyAt(0), RID(sib->num_, -1));
	int sibSize = sib->size_, curSize = curr->size_;
	int len = curr->len_;
	if (leftSib) {
		memcpy(sib->keyAt(sib->size_), curr->keyAt(0), curr->size_ * len);
		memcpy(&sib->rids_[sib->size_], &curr->rids_[0], curr->size_ * sizeof(RID));
		sib->size_ = sibSize + curSize;
		curr->size_ = 0;
	}
	else {
		KeyPtr origKey = malloc(sib->len_);
		sib->copyKey(0, origKey);
		for (int i = sibSize + curSize - 1; i >= sibSize - 1; i--) {
			sib->updateKey(i, sib->keyAt(i - curSize));
			sib->updateRid(i, sib->ridAt(i - curSize));
		}
		memcpy(sib->keyAt(0), curr->keyAt(0), curSize * len);
		memcpy(&sib->rids_[0], &curr->rids_[0], curSize * sizeof(RID));
		sib->size_ = sibSize + curSize;
		curr->size_ = 0;

		if (idx != 0) {
			anchor->updateKey(idx, sib->keyAt(0));
		}
		else if (sib->keyComp(origKey, sib->keyAt(0)) != 0) {
			bool update;
			RID rid(sib->num_, -1);
			fixKeys(root_, origKey, rid, update, 1);
		}
	}

	if (leftSib) {
		sib->setRight(curr->right());
		NodePtr right = loadNode(curr->right());
		if (right) right->setLeft(curr->num_);
	}
	else {
		if (leaf_ == curr->num_) leaf_ = sib->num_;
		sib->setLeft(curr->left());
		NodePtr left = loadNode(curr->left());
		if (left) left->setRight(sib->num_);
	}
	//anchor->keys_[idx] = sib->keys_[0]; // 更新上层的key
	updateKey = false; // curr节点被删除掉了,所以没有必要更新上层节点的key值了
	int removePtr = curr->num_;
	disposeNode(removePtr);
	return removePtr;
	return 0;
}

bool IXIndexHandle::fixKeys(NodePtr curr, KeyPtr & key, RID & rid, bool & updateKey, int h)
{
	if (curr->num_ == rid.page()) {
		key = curr->keyAt(0);
		updateKey = true;
		return true;
	}

	if (h < hdr_.height) {
		int idx = curr->leftmostSearchPos(key);
		if (idx < 0) return false;
		for (int i = idx; i < curr->size_; i++) {
			if (curr->keyComp(curr->keyAt(i), key) != 0) break;
			NodePtr node = loadNode(curr->ridAt(i).page());
			if (fixKeys(node, key, rid, updateKey, h + 1)) {
				if (updateKey) curr->updateKey(i, key);
				updateKey = (updateKey && (i == 0));
				return true;
			}
		}
	}
	return false;
}


bool IXIndexHandle::search(NodePtr &curr, int h, KeyPtr key, RID & rid)
{
	int idx = -1;
	if (h == hdr_.height) {
		idx = curr->lowerBound(key);
		if (idx < 0) return false;
		rid = curr->ridAt(idx);
		return true;
	}
	else {
		idx = curr->leftmostSearchPos(key);
		if (idx < 0) return false;
		for (int i = idx; i < curr->size_; i++) {
			NodePtr next = loadNode(curr->ridAt(i).page());
			if (search(next, h + 1, key, rid)) {
				return true;
			}
		}
	}
	return false;
}

bool IXIndexHandle::insertIndex(NodePtr& curr, int height, KeyPtr key, RID& rid, bool &fixed)
{
	int idx = -1;
	if (height == hdr_.height) { // leaf node
		idx = curr->search(key, rid);
		if (idx >= 0) {		// (key, rid)对已经存在
			fixed = true;
			return false;
		}
	}
	else { // internal node
		idx = curr->leftmostSearchPos(key);
		RID pos;
		if (idx < 0) { // key比tree中所有的key都小
			idx = 0;
			curr->updateKey(0, key);
			pos = curr->ridAt(0);
		}
		else {
			pos = curr->ridAt(idx);
		}
		NodePtr next = loadNode(pos.page());
		bool stat = insertIndex(next, height + 1, key, rid, fixed);
		if (fixed) return stat;
	}

	if (curr->size_ < curr->capacity_) {
		fixed = true;
		if (height == hdr_.height) curr->insert(key, rid);
		else curr->insert(idx + 1, key, rid);
	}
	else { // 需要进行分裂
		fixed = false;
		NodePtr sib = allocNode();
		split(curr, sib);
		if (sib->keyComp(sib->keyAt(0), key) < 0) sib->insert(key, rid);
		else curr->insert(key, rid);

		//key = sib->keyAt(0);
		sib->copyKey(0, key);
		rid = RID(sib->num_, -1);

		sib->setRight(curr->right());
		NodePtr right = loadNode(curr->right());
		if (right) right->setLeft(sib->num_);
		sib->setLeft(curr->num_);
		curr->setRight(sib->num_);
	}
	return true;
}

//
// eraseIndex - 试图从索引文件中删除该索引
// 
RC IXIndexHandle::eraseIndex(KeyPtr val, RID rid)
{
	if (val == nullptr) return IX_BADKEY;
	bool updateKey = false;
	KeyPtr key = malloc(hdr_.len);
	memcpy(key, val, hdr_.len);
	int res = findRebalance(root_, nullptr, nullptr, nullptr, nullptr, 
		key, rid, 1, updateKey);
	free(key);
	if (res == -3) return IX_KEYNOTFOUND;
	return 0;
}

bool IXIndexHandle::search(KeyPtr key, RID &rid)
{
	return search(root_, 1, key, rid);
}

NodePtr IXIndexHandle::loadNode(Page num)
{
	if (num < 0) return nullptr;
	PFPageHandle page = PFGetPage(file_, num);
	return make_shared<BPlusNode>(hdr_, page, true);
}

NodePtr IXIndexHandle::allocNode()
{
	PFPageHandle page = PFAllocPage(file_);
	return make_shared<BPlusNode>(hdr_, page, false);
}

void IXIndexHandle::disposeNode(Page num)
{
	file_->disposePage(num);
}

