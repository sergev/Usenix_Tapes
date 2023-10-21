#include <stdio.h>
#include "intdefs.h"

/* putname - convert name to lower case and print */

putname(i)
int i;
{
register int c, j = 0;

while((c = tolower(obuf[i].oname[j])) != 0) {
    PUTC(c);
    j++;
    }
while(j++ < NAMESIZ - 1)                            /* pad to columnarize */
    PUTC(' ');
}
