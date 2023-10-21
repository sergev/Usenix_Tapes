/*
 * statcheck - checks status of files in statfile
 */

#include "cfs.h"

statcheck (statfile, verbose)
char   *statfile;
int     verbose;
{
    extern char *myname;
    FILE   *fp;
    int     errflg = 0;
    int     rstat = 0;
    char    name[MAXPATHLEN];
    struct stat osbuf;
    struct stat nsbuf;
    FILE   *openread ();

    fp = openread (statfile);
/*
 *  for each entry in statfile, get stored status
 */
    while (!(rstat = readstat (fp, statfile, name, &osbuf))) {
/* 
 *	stat real file
 */
	if (stat (name, &nsbuf)) {
	    printf ("status check FAILED for %s\n", name);
	    if (verbose)
		printf ("%s: %s - %s\n\n", myname, name,
			sys_errlist[errno]);
	    errflg++;
	}
/*
 *	    compare old status with new status
 */
	else
	    if (statcomp (&osbuf, &nsbuf)) {
		printf ("status check FAILED for %s\n", name);
		if (verbose)
		    print2stat (&osbuf, &nsbuf);
		errflg++;
	    }
    }
    if (rstat < 0)
	errflg++;
    fclose (fp);
    return (errflg);
}
