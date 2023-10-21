#include <stdio.h>
#include "intdefs.h"

/* mname - convert month number to month name */

 unsigned char *
mname(n)
int n;
{
static unsigned char *name[] = {
    "???",
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "Jun",
    "Jul",
    "Aug",
    "Sep",
    "Oct",
    "Nov",
    "Dec"
    };
return((n < 1 || n > 12) ? name[0] : name[n]);
}
