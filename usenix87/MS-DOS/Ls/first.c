#include <stdio.h>
#include "intdefs.h"

/* find_first - find first file in chosen directory */

find_first(path, dta, mask)
unsigned char *path;
struct dta *dta;
int mask;
{
STRUCTURE

RAX = 0x1A00;                                         /* DOS interrupt 1A */
RDX = (unsigned)dta;
CALLDOS;                                    /* sets data transfer address */

RAX = 0x4E00;                                         /* DOS interrupt 4E */
RCX = mask;        
RDX = (unsigned) path;
CALLDOS;                      /* fills the structure */
return(!RESULT);
}
