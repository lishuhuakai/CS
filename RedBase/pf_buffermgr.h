//
// File:		pf_buffermgr.h
// Description: PF_BufferMgr class interface
//


#ifndef PF_BUFFERMGR_H
#define PF_BUFFERMGR_H

#include "pf_internal.h"
#include "pf_hashtable.h"

//
// Defines
//

// INVALID_SLOT is used within the PF_BufferMgr class which tracks a list
// of PF_BufPageDesc. Inside the PF_BufPageDesc are integer "pointers" to
// next and prev items. INVALID_SLOT is used to indicate no previous or next.
#define INVALID_SLOT	(-1)

//
// PF_BufPageDesc - struct containing data about a page in the buffer
//

struct PF_BufPageDesc {
	char *pData;		// page contents
	int next;			// next in the linked list of buffer pages
	int prev;			// prev in the linked list of buffer pages
	int bDirty;			// TRUE if page is dirty
	short int pinCount;	// pin count
	Page pageNum;		// page number for this page
	int fd;				// OS file descriptor of this page
};


//
// PF_BufferMgr - manag the page buffer
//
class PF_BufferMgr {
public:
	PF_BufferMgr(int numPages);
	~PF_BufferMgr();

	// Read pageNum into buffer, point *ppBuffer to location
	RC GetPage(int fd, PageNum pageNum, char **ppBuffer, int bMultiplePins = TRUE);
	// Allocate a new page in the buffer, point *ppBuffer to its location
	RC AllocatePage(int fd, PageNum pageNum, char **ppBuffer);
	RC MarkDirty(int fd, PageNum pageNum);	// Mark page dirty
	RC UnpinPage(int fd, PageNum pageNum);	// Unpin page from the buffer
	RC FlushPages(int fd);	// Flush pages for file

	// Force a page to the disk, but do not remove from the buffer pool
	RC ForcePages(int fd, PageNum pageNum);

	// Remove all entries from thr Buffer Manager.
	RC ClearBuffer();

	// 显示buffer中所有的entries
	RC PrintBuffer();

	// 试图调整buffer的大小
	RC ResizeBuffer(int iNewSize);

	// Three Methods for manipulating raw memory buffers. These memory
	// locations are handled by the buffer manager, but are not
	// associated with a particular file. These should be used if you
	// want memory that is bounded by the size of buffer pool.

	// return the size of the block that can be allocated.
	RC GetBlockSize(int &length) const;

	// Allocate a memory chunk that lives in buffer manager
	RC AllocateBlock(char *&buffer);
	// Dispose of a memory chunk managed by the buffer manager.
	RC DisposeBlock(char* buffer);

private:
	RC InsertFree(int slot);
	RC LinkHead(int slot);
	RC Unlink(int slot);
	RC InternalAlloc(int &slot);

	// Read a page
	RC ReadPage(int fd, PageNum pageNum, char* dest);

	// Write a page
	RC WritePage(int fd, PageNum pageNum, char* source);

	// Init the page desc entry
	RC InitPageDesc(int fd, PageNum pageNum, char* source);
	
	PF_BufPageDesc *bufTable;			// info on buffer pages
	PF_HashTable hashTable;				// Hash table object
	int	nmPages;						// # of pages in the buffer
	int pageSize;						// MRU page slot
	int first;							
	int last;
	int free;
};




#endif