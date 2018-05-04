//
// File:		pf_internal.h
// Description:	Declarations internal to the paged file component
//

#ifndef PF_INTERNAL_H
#define PF_INTERNAL_H

#include <cstdlib>
#include <cstring>
#include "pf.h"

// Constants and defines
#define CREATION_MASK		0600		// r/w privileges to owner only
#define PF_PAGE_LIST_END	-1			// end of list of free_ pages
#define PF_PAGE_USED		-2			// Page is being used


//
// PageHdr: Header structure for pages
//
struct PFPageHdr {
	int free;		// free可以有以下几种取值:
					// - 下一个空闲页的编号,此时该页面也是空闲的
					// - PF_PAGE_LIST_END => 页面是最后一个空闲页面
					// - PF_PAGE_USED => 页面并不是空闲的
};

// Justify the file header to the length of one Page
const int PF_FILE_HDR_SIZE = PF_PAGE_SIZE + sizeof(PFPageHdr);

#endif /* PF_INTERNAL_H */