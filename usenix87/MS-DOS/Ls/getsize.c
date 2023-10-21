#include <stdio.h>
#include "intdefs.h"

getsize(path, nt)       /* get file cluster size */
unsigned char *path;
int nt;
{
if(!nt) {
    if((clsize = getcl(path)) == 0) {       /* get cluster size for drive */
        fprintf(stderr, "Invalid drive: %c\n", *path);
        return(1);
        }
    }
clmask = clsize-1;
return(0);
}
