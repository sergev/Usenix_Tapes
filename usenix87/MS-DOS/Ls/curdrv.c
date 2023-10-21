#include <stdio.h>
#include "intdefs.h"

/* curdrv - get current default drive */

 unsigned char *
curdrv(sp)
unsigned char *sp;
{
STRUCTURE

RAX = 0x1900;                                         /* DOS interrupt 19 */
CALLDOS;                                     /* gets current drive number */
*sp++ = RAX + 'a';                      /* convert to symbolic drive name */
*sp++ = ':';
*sp = '\0';
return(sp);
}
