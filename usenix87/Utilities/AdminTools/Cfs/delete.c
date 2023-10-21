/*
 * delete - deletes names from statfile for cfs
 */

#include "cfs.h"

char   *mktemp ();
char   *calloc ();

delete (statfile, argc, argv, verbose)
char   *statfile;
int     argc;
char  **argv;
int     verbose;
{
    extern char *myname;
    FILE   *sfp;
    char   *tfname;
    FILE   *tfp;
    int     errflg = 0;
    int     rstat = 0;
    char    name[MAXPATHLEN];
    struct stat sbuf;
    int    *foundlist;
    char   *arg;
    char   *mktemp ();
    char   *poparg ();
    FILE   *openread ();
    FILE   *openwrite ();

    sfp = openread (statfile);
    tfname = mktemp ("cfs.XXXXXX");
    tfp = openwrite (tfname);
/*
 *	get storage for foundlist
 */
/*NOSTRICT*/
    if (!(foundlist = (int *) calloc ((unsigned) argc, sizeof (int)))) {
	fprintf (stderr, "cfs: can't calloc %d ints\n", argc);
	exit (1);
    }
/*
 *  for each entry in statfile
 */
    while (!(rstat = readstat (sfp, statfile, name, &sbuf))) {
	if (!cavn (argc, argv, foundlist, name))
	    writestat (tfp, name, &sbuf);
    }
    fclose (tfp);
    fclose (sfp);
    if (rstat > 0) {
	while ((arg = poparg (&argc, &argv)) != NULL) {
	    if (*foundlist && verbose)
		printf ("d %s\n", arg);
	    foundlist++;
	}
	if (movefile (tfname, statfile)) {
	    fprintf (stderr, "%s: %s not changed\n", myname, statfile);
	    errflg++;
	}
    }
    else {
	unlink (tfname);
	fprintf (stderr, "%s: %s not changed\n", myname, statfile);
	errflg++;
    }
    return (errflg);
}
