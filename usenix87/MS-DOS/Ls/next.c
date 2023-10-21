#include <stdio.h>
#include "intdefs.h"

/* find_next - find the next file in the same directory */

find_next(dta)
struct dta *dta;
{
STRUCTURE

RAX = 0x1A00;
RDX = (unsigned)dta;
CALLDOS;                                          /* set dta */

RAX = 0x4F00;
CALLDOS;                                   /* fill the table */
return(!RESULT);
}
