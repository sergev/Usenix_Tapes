/*
 *  cfs - check file states
 *
 *	Rex Sanders, US Geological Survey, 12/86
 */

#include "cfs.h"

char   *myname;

main (argc, argv)
int     argc;
char  **argv;
{
    char   *statfile;
    int     verbose = 0;
    int     option = 0;
    int     errflg = 0;


/*
 * Switch to the proper action procedure
 */
    switch (option = deco (&argc, &argv, &statfile, &verbose)) {
	case 's': 
	    errflg = statcheck (statfile, verbose);
	    break;
	case 'c': 
	    errflg = create (statfile, verbose);
	    break;
	case 'u': 
	    errflg = update (statfile, argc, argv, verbose);
	    break;
	case 'd': 
	    errflg = delete (statfile, argc, argv, verbose);
	    break;
	case 't': 
	    errflg = table (statfile, argc, argv, verbose);
	    break;
	default: 
	    fprintf (stderr, "usage:  %s [-scudt][v] ", myname);
	    fprintf (stderr, "statfile [filename ...]\n");
	    errflg = option;
	    break;
    }

    exit (errflg);
}
