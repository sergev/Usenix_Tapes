#include <stdio.h>
#include "intdefs.h"

/* setps - set pathname separator to conform to MS-DOS switchar value */

setps()
{
STRUCTURE

RAX = 0x3700;                              /* dos int 37 returns switchar */
CALLDOS;
return(((RDX & 0xFF) == '/') ? '\\' : '/');  /* return pathname separator */
}
