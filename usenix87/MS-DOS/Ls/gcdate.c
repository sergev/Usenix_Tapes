#include <stdio.h>
#include "intdefs.h"


/* gcdate - get current date (in months) for comparison */

gcdate()
{
STRUCTURE

RAX = 0x2A00;
CALLDOS;
return(RCX * 12 + (RDX >> 8));                         /* yr * 12 + month */
}
