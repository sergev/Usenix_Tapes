/*
 *  decode option flags for cfs
 */
#include <stdio.h>
#include <strings.h>

deco (argc, argv, statfile, verbose)
int    *argc;
char ***argv;
char  **statfile;
int    *verbose;

{
    extern char *myname;
    char   *slash;
    int     option = 0;
    int     errflg = 0;
    char   *arg;
    char   *poparg ();

    myname = poparg (argc, argv);
    slash  = rindex (myname, '/');
    myname = slash ? (slash + 1) : myname;
/*
 *  looking for command line formed:
 *    cfs -[scudt][v] statfile [more args ...]
 *  where one and only one of s, c, u, d, t must be specified.
 */
    arg = poparg (argc, argv);
    if (arg != NULL && *arg == '-') {
	for (arg++; *arg != '\0'; arg++)
	    switch (*arg) {
		case 's': 
		case 'c': 
		case 'u': 
		case 'd': 
		case 't': 
		    if (!option) {
			option = *arg;
			*statfile = poparg (argc, argv);
			if (*statfile == NULL) {
			    fprintf (stderr,
				    "%s: no statfile specified\n",
				    myname);
			    errflg++;
			}
		    }
		    else {
			fprintf (stderr,
			    "%s: %c: only one of {scudt} may be specified\n",
			    myname, *arg);
			errflg++;
		    }
		    break;
		case 'v':
		    (*verbose)++;
		    break;
		default: 
		    fprintf (stderr, "%s: option %c not recognized\n",
			    myname, *arg);
		    errflg++;
		    break;
	    }
    }
    else {
	fprintf (stderr,
		"%s: one of {-s -c -u -d -t} must be specified\n",
		myname);
	errflg++;
    }

    return (errflg ? -1 : option);
}
