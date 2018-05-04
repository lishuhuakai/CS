#include "pf_internal.h"
#include "pf_pagehandle.h"
#include "pf_filehandle.h"
#include "pf_error.h"


PFPageHandle::~PFPageHandle()
{
	// 一般而言,file的生命周期长于page
	// PFPageHandle对象一旦析构,立刻要unpin
	shared_ptr<PFFileHandle> file = pffile_.lock();
	if (file) {
		file->unpin(num_);
	}
}

// 
// operator= - 赋值运算符
// 
PFPageHandle& PFPageHandle::operator=(const PFPageHandle& rhs)
{
	if (this != &rhs) {
		// 首先要消除对原来页面的引用
		PFFilePtr file = pffile_.lock();
		if (file) {
			file->unpin(num_);
		}
		num_ = rhs.num_;
		addr_ = rhs.addr_;
		file = rhs.pffile_.lock();
		// 赋值的时候必须要增加应用计数
		if (file) {
			file->pin(num_);
		}
		pffile_ = file;
	}
	return *this;
}

//
// PFPageHandle - 拷贝构造函数
//
PFPageHandle::PFPageHandle(const PFPageHandle& rhs)
	: pffile_(rhs.pffile_)
	, num_(rhs.num_)
	, addr_(rhs.addr_)
{
	PFFilePtr file = pffile_.lock();
	if (file) {
		file->pin(num_);
	}
}

//
// setDirty - 将页面设置为脏页面
//
void PFPageHandle::setDirty()
{
	if (dirty_) return;
	PFFilePtr file = pffile_.lock();
	if (file) {
		file->markDirty(num_);
	}
}

//
// dispose - 释放页面
//
void PFPageHandle::dispose()
{
	PFFilePtr file = pffile_.lock();
	if (file) {
		file->disposePage(num_);
	}
}
