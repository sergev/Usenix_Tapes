/*
 * update - update argv names in statfile for cfs
 */

#include "cfs.h"

char   *mktemp ();
char   *calloc ();

update (statfile, argc, argv, verbose)
char   *statfile;
int     argc;
char  **argv;
int     verbose;
{
    extern char *myname;
    FILE   *sfp;		/* statfile file pointer */
    char   *tfname;		/* temporary statfile file name */
    FILE   *tfp;		/* tempstat file pointer */
    int     errflg = 0;		/* returned error flag */
    int     rstat;		/* readline status flag */
    char    name[MAXPATHLEN];	/* path name */
    char   *arg;		/* argument for processing */
    struct stat sbuf;		/* stat data */
    int    *ufl;		/* updated filenames found list */
    int    *tfl;		/* temporary filenames found list */
    int     nargs;		/* temporary copy of argc */
    char   *poparg ();
    FILE   *openread ();
    FILE   *openwrite ();

    sfp = openread (statfile);
    tfname = mktemp ("cfs.XXXXXX");
    tfp = openwrite (tfname);
/*
 *	get storage for ufl & tfl
 */
    if (!(ufl = (int *) calloc ((unsigned) argc, sizeof (int)))) {
	fprintf (stderr, "cfs: can't calloc %d ints\n", argc);
	exit (1);
    }
    if (!(tfl = (int *) calloc ((unsigned) argc, sizeof (int)))) {
	fprintf (stderr, "cfs: can't calloc %d ints\n", argc);
	exit (1);
    }
/*
 *  for each entry in statfile
 */
    while (!(rstat = readstat (sfp, statfile, name, &sbuf))) {
	if (!cavn (argc, argv, tfl, name)) {
/* 
 *	    no match - write entry to tempstat
 */
	    writestat (tfp, name, &sbuf);
	}
	else {
/*
 *	    match found - update tempstat
 */
	    nargs = argc;
	    while (nargs-- > 0) {
		if (*tfl && !*ufl)
		    if (!addname (tfp, name)) {
			if (verbose)
			    printf ("u %s\n", name);
			*ufl = 1;
		    }
		    else
			errflg++;
		*tfl = 0;
		tfl++;
		ufl++;
	    }
	    tfl -= argc;
	    ufl -= argc;
	}
    }

    if (rstat > 0) {
/*
 *    check each arg - add if not updated
 */
	arg = poparg (&argc, &argv);
	while (arg != NULL) {
	    if (!*ufl) {
		if (!addname (tfp, arg)) {
		    if (verbose)
			printf ("a %s\n", arg);
		}
		else
		    errflg++;
	    }
	    arg = poparg (&argc, &argv);
	    ufl++;
	}
	fclose (tfp);
	fclose (sfp);
	if (movefile (tfname, statfile)) {
	    fprintf (stderr, "%s: %s not changed\n", myname, statfile);
	    errflg++;
	}
    }
    else {
	fclose (tfp);
	fclose (sfp);
	unlink (tfname);
	fprintf (stderr, "%s: %s not changed\n", myname, statfile);
	errflg++;
    }
    return (errflg);
}
