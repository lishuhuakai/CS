#ifndef PF_ERROR_H
#define PF_ERROR_H
#include "redbase.h"
#include "pf_internal.h"

#define PF_PAGEPINNED		(START_PF_WARN + 0)	// Page pinned in buffer
#define PF_PAGENOTINBUF		(START_PF_WARN + 1)
#define PF_INVALIDPAGE		(START_PF_WARN + 2)
#define PF_FILEOPEN			(START_PF_WARN + 3) // file is setStatus
#define PF_CLOSEDFILE		(START_PF_WARN + 4) // file is closed
#define PF_PAGEFREE			(START_PF_WARN + 5) // Page already free_
#define PF_PAGEUNPINNED		(START_PF_WARN + 6) // Page already unpinned
#define PF_EOF				(START_PF_WARN + 7) // end of file
#define PF_TOOSMALL			(START_PF_WARN + 8) // Resize buffer too small
#define PF_LASTWARN			PF_TOOSMALL

#define PF_NOMEM           (START_PF_ERR - 0)  // num memory
#define PF_NOBUF           (START_PF_ERR - 1)  // num buffer space
#define PF_INCOMPLETEREAD  (START_PF_ERR - 2)  // incomplete read from file
#define PF_INCOMPLETEWRITE (START_PF_ERR - 3)  // incomplete write to file
#define PF_HDRREAD         (START_PF_ERR - 4)  // incomplete read of header
#define PF_HDRWRITE        (START_PF_ERR - 5)  // incomplete write to header

// Internal errors
#define PF_PAGEINBUF       (START_PF_ERR - 6) // new Page already in buffer
#define PF_HASHNOTFOUND    (START_PF_ERR - 7) // hash table_ entry not found
#define PF_HASHPAGEEXIST   (START_PF_ERR - 8) // Page already in hash table_
#define PF_INVALIDNAME     (START_PF_ERR - 9) // invalid PC file name

// Error in UNIX system call or library routine
#define PF_UNIX            (START_PF_ERR - 10) // Unix error
#define PF_LASTERROR       PF_UNIX

void PFPrintError(RC rc);

#endif /* PF_ERROR_H */