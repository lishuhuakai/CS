#ifndef QL_ERROR_H
#define	QL_ERROR_H

#include "redbase.h"
//
// Print-error function
//
void QLPrintError(RC errval);

#define QL_KEYNOTFOUND    (START_QL_WARN + 0)  // cannot find key
#define QL_INVALIDSIZE    (START_QL_WARN + 1)  // invalid number of attributes
#define QL_ENTRYEXISTS    (START_QL_WARN + 2)  // key,rid already
// exists in index
#define QL_NOSUCHENTRY    (START_QL_WARN + 3)  /* 不存在此种key */
#define QL_EOF            (START_QL_WARN + 4)  /* 迭代器到达了所有分组的结束位置 */
#define QL_GEOF			  (START_QL_WARN + 5)  /* 迭代器到达了一个分组的结束位置 */

#define QL_LASTWARN QL_ENTRYEXISTS


#define QL_BADJOINKEY      (START_QL_ERR - 0)
#define QL_ALREADYOPEN     (START_QL_ERR - 1)
#define QL_BADATTR         (START_QL_ERR - 2)  
#define QL_DUPREL          (START_QL_ERR - 3)
#define QL_RELMISSINGFROMFROM (START_QL_ERR - 4)
#define QL_FNOTOPEN        (START_QL_ERR - 5)
#define QL_JOINKEYTYPEMISMATCH (START_QL_ERR - 6)
#define QL_BADOPEN         (START_QL_ERR - 7)

#define QL_LASTERROR QL_EOF

#endif /* QL_ERROR_H */