/*
 * movefile - move old to new; bomb if unsuccessful
 */

#include "cfs.h"

movefile (old, new)
char   *old;
char   *new;
{
    extern char *myname;
    char	sysline[5 + (2 * MAXPATHLEN)];

    if (rename (old, new)) {	/* if rename doesn't do it, try cp */
	sprintf (sysline, "cp %s %s", old, new);
	if (system (sysline))
	    return (1);
	else
	    unlink (old);
    }
    return (0);
}
