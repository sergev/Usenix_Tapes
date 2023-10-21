/*
 * create - creates a statfile for cfs
 */

#include "cfs.h"

create (statfile, verbose)
char   *statfile;
int     verbose;
{
    extern char *myname;
    FILE   *sfp;
    int     errflg = 0;
    char    name[MAXPATHLEN];
    FILE   *openwrite ();

    sfp = openwrite (statfile);

/* 
 *	add each name in stdin
 */
    while (scanf ("%s", name) != EOF)
	if (!addname (sfp, name))
	    if (verbose)
		printf ("a %s\n", name);
	    else
		errflg++;

    return (errflg);
}
