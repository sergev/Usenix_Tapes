#ifndef lint
static char *what = "@(#)main.c 11/29/85 Warren Lavallee";
#endif

#include <stdio.h>
#include <strings.h>

#define TRUE 1
#define FALSE 0


main (argc, argv)
int     argc;
char   *argv[];
{
    register int i;

    if (!strcmp (argv[1], "-a")) {/* see if -a set  */
	(void) all ();
	exit (0);
    }

    if (argc == 1) {		/* if not arguments print yours */
	(void) me ();
	exit (0);
    }

    for (i = 1; i < (argc); i++)/* loop through arguments */
	if (!one (argv[i])) {	/* if login name, print   */
	    if (strcmp (argv[i], "0") && !atoi (argv[i])) {
		(void) fprintf (stderr, "%s not valid entry\n", argv[i]);
		(void) fflush (stderr);
	    }
	    else
		if (!uid (atoi (argv[i]))) {/* if uid print */
		    (void) fprintf (stderr, "%s not valid entry\n", argv[i]);
		    (void) fflush (stderr);
		}
	}
}
