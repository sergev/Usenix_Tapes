#include <stdio.h>
#include "intdefs.h"

/* curpath - get path to directory on default drive */

 unsigned char *
curpath(sp)
register char *sp;
{
STRUCTURE

*sp++ = qs;

RAX = 0x4700;                        /* DOS interrupt 47 gets path string */
if((RDX = (unsigned)*(sp - 3) - '`') < 0)
    RDX += ' ';                            /* convert drive name to index */
RSI = (unsigned) sp;
CALLDOS;

while(*sp != '\0') {                              /* make string readable */
    *sp = tolower(*sp);
    if(*sp == '\\')
        *sp = qs;
    sp++;
    }
if(*(sp-1) != qs)
    *sp++ = qs;
*sp = '\0';
return(sp);
}
