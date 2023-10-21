/*
 * addname - add name to statfile on fp for cfs
 */

#include "cfs.h"

addname (fp, name)
FILE   *fp;
char   *name;
{
    extern char *myname;
    int     errflg = 0;
    struct stat sbuf;

    if (stat (name, &sbuf)) {
	fprintf (stderr, "%s: ", myname);
	perror (name);
	errflg = 1;
    }
    else
	writestat (fp, name, &sbuf);

    return (errflg);
}
