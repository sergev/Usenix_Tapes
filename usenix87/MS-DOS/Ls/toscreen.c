#include <stdio.h>
#include "intdefs.h"

/* toscreen - find out if output is to console screen */

toscreen()
{
STRUCTURE

RAX = 0x4400;
RBX = 1;
CALLDOS;
return((RDX & 1) && (RDX & 0x80));                      /* isdev && iscin */
}
