#include <stdio.h>
#include "intdefs.h"

/* getpath - get absolute path into search path buffer */

getpath(sp, p)
register char *sp, *p;
{
extern unsigned char *curdrv(), *curpath();

if(*(p+1) == ':') {                         /* if drive specified, use it */
    *sp++ = *p++;
    *sp++ = *p++;
    }
else                                            /* else use current drive */
    sp = curdrv(sp);

if(*p == qs)                               /* if path is absolute, use it */
    *sp++ = *p++;
else
    sp = curpath(sp);                         /* else insert current path */

while(*p != '\0') {                  /* copy & translate rest of pathname */
    if(*p == '.') {                     /* ignore "." starting a pathname */
        if(*++p == '.') {
            while( *(--sp - 1) != qs)       /* if ".." back up 1 pathname */
                ;
            p++;
            }
        if(*p == qs)
            p++;
        }
    else {
        while(*p != qs && *p != '\0')                  /* move 1 pathname */
            *sp++ = *p++;
        if(*p == qs)
            *sp++ = *p++;
        }
    }
*sp = '\0';
return;
}
