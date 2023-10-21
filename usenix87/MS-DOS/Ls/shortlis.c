#include <stdio.h>
#include "intdefs.h"

/* shortlist - print a list of names in 5 columns */

shortlist(k)
int k;                                           /* total number to print */
{
register int i, m, n;

if(colm)
    n = k;                                    /* set for 1-column listing */
else
    n = (k + 4)/5;                                         /* or 5-column */

for(i=0; i < n; i++){
    for(m = 0; (i+m) < k; m += n) {
        if(obuf[i+m].oattr & 0x10)
            strcat(obuf[i+m].oname, &qs);             /* mark directories */
        putname(i+m);                                   /* print the name */
        PUTS("   ");       
        }
    fputc(endlin(), stdout);
    }
return;
}
