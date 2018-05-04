#ifndef PF_PAGE_HANDLE_H
#define PF_PAGE_HANDLE_H
#include "pf.h"
#include <tr1/memory>
using namespace std;

class PFFileHandle;
//
// PFPageHandle是对于页面的一种抽象。
//
class PFPageHandle {
	friend class PFFileHandle;
public:
	PFPageHandle() : num_(-1), addr_(nullptr), dirty_(false) {}
	~PFPageHandle();
	PFPageHandle(const PFPageHandle& rhs);
	PFPageHandle& operator=(const PFPageHandle &page);

	//
	// rawPtr - 获取指向实际内容的裸指针
	// 
	Ptr rawPtr() { return addr_; }
	Page page() { return num_; }
	void setOwner(shared_ptr<PFFileHandle> file) { pffile_ = file; }
	void setDirty();
	void dispose();		// 销毁所在的页
private:
	bool dirty_;
	weak_ptr<PFFileHandle> pffile_;	// 弱引用是有必要的
	Page num_;						// 页面的编号
	Ptr addr_;						// 指向实际的页面数据
};

#endif /* PF_PAGE_HANDLE_H */