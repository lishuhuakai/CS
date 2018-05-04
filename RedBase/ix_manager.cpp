#include "ix_manager.h"
#include "ix_indexhandle.h"
#include "pf_error.h"
#include <math.h>
#include <sstream>
using namespace std;

extern PFManager pfManager;

//
// createIndex - 构建一个索引
// 
RC IXManager::createIndex(const char* prefix, int num, AttrType type, int len, uint limitSpace)
{
	// 每一次都要做细致的检查
	if ((num < 0) || (type < INT) || (type > STRING) ||
		(prefix == nullptr))
		return IX_FCREATEFAIL;
	if (((type == STRING) && ((len <= 0) || (len > MAXSTRINGLEN))))
		return IX_FCREATEFAIL;
	char filepath[64];
	snprintf(filepath, 64, "%s.%d", prefix, num);
	pfManager.createFile(filepath);
	PFFilePtr file;
	pfManager.openFile(filepath, file);
	PFPageHandle page = PFAllocPage(file);
	Ptr addr = page.rawPtr();

	IXFileHdr *ixHdr = reinterpret_cast<IXFileHdr *>(addr);
	ixHdr->size = 1;
	ixHdr->limitSpace = limitSpace;		// 受限的页面大小,当然,这里仅仅是为了测试
	ixHdr->pairSize = len + sizeof(Page);
	ixHdr->height = 0;
	ixHdr->root = -1;
	ixHdr->leaf = -1;
	ixHdr->type = type;
	ixHdr->len = len;
	uint remain = limitSpace - sizeof(int) - 2 * sizeof(Page);
	ixHdr->capacity = floor(remain / (sizeof(RID) + len));
	page.setDirty();
	pfManager.closeFile(file);
	return 0;
}

//
// destroyIndex - 销毁掉索引
// 
RC IXManager::destroyIndex(const char* prefix, int num)
{
	if ((num < 0) || (prefix == nullptr)) return IX_FCREATEFAIL;
	char filepath[64];
	snprintf(filepath, 64, "%s.%d", prefix, num);
	pfManager.destroyFile(filepath);
	return 0;
}


//
// openIndex
// 
RC IXManager::openIndex(const char* prefix, int num, IXIndexPtr &index)
{
	if (num < 0 || prefix == nullptr) return IX_FCREATEFAIL;
	char filepath[64];
	snprintf(filepath, 64, "%s.%d", prefix, num);
	PFFilePtr file;
	pfManager.openFile(filepath, file);
	index = make_shared<IXIndexHandle>(file);
	return 0;
}


//
// closeIndex
// 
RC IXManager::closeIndex(IXIndexPtr &index)
{
	PFFilePtr file = index->file_;
	index.reset();
	pfManager.closeFile(file);
	return 0;
}


