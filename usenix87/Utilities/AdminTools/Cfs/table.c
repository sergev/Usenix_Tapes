/*
 * table - print table of contents for statfile
 */
#include "cfs.h"

char   *calloc ();

table (statfile, argc, argv, verbose)
char   *statfile;
int     argc;
char  **argv;
int     verbose;
{
    extern char *myname;
    FILE   *fp;
    char    name[MAXPATHLEN];
    struct stat sbuf;
    int     errflg = 0;
    int     rstat;
    int    *foundlist;
    FILE   *openread ();

    fp = openread (statfile);
/* 
 *  if no arguments, table entire statfile
 */
    if (argc == 0)
	while (!(rstat = readstat (fp, statfile, name, &sbuf)))
	    if (verbose)
		printstat (name, &sbuf);
	    else
		printf ("%s\n", name);
    else {
/*
 *	get storage for foundlist
 */
	if (!(foundlist = (int *) calloc
		    ((unsigned) argc, sizeof (int)))) {
	    fprintf (stderr, "%s: can't calloc %d ints\n",
		    myname, argc);
	    exit (1);
	}
/*
 *  for each entry in statfile
 */
	while (!(rstat = readstat (fp, statfile, name, &sbuf))) {
	    if (cavn (argc, argv, foundlist, name)) {
		if (verbose)
		    printstat (name, &sbuf);
		else
		    printf ("%s\n", name);
	    }
	}
	if (rstat < 0)
	    errflg++;
    }
    fclose (fp);
    return (errflg);
}
