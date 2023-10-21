#include <stdio.h>
#include "intdefs.h"

/* comp - compare size of two entries */

comp(a,b)
struct outbuf *a, *b;
{
int y;

if(tsrt) {
    if(a->odate != b->odate)                           /* if dates differ */
        y = (a->odate < b->odate) ? -1 : 1;            /* that settles it */
    else
        y = (a->otime < b->otime) ? -1 : 1;         /* else compare times */
    return((rev) ? y : -y);
    }
else {
    y = strcmp(a->oname, b->oname);                    /* name comparison */
    return((rev) ? -y : y);
    }
}
