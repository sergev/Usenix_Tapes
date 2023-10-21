#include <stdio.h>
#include "intdefs.h"

/* getcl - get cluster size & space left on requested drive */

getcl(pp)
unsigned char *pp;
{
STRUCTURE
int cs;
extern long left;
extern int drive;

if((RAX = *pp - 'a') < 0)
    RAX += ' ';
drive = RAX & 0x7F;
RDX = drive + 1;                               /* 0 = default, 1 = a, etc */
RAX = 0x3600;
CALLDOS;                                          /* DOS interrupt hex 36 */
if(RAX == 0xFFFF)                                 /* gets free disk space */
    return(0);                                       /* and other goodies */
else {
    cs = RAX * RCX;                               /* ax = sectors/cluster */
    left = (long)cs * RBX;                      /* bx = # unused clusters */
    return(cs);                                      /* cx = bytes/sector */
    }                                   /* dx = drive capacity (clusters) */
}
