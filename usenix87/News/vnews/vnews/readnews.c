/*
 * readnews - read news articles.
 */

/* static char	*SccsId = "%W%	%G%"; */

#include "rparams.h"
#include "artfile.h"
#include "ng.h"

/*
 * readnews - article reading program
 */


/*
 *	Authors:
 *		Matt Glickman	ucbvax!glickman
 *		Mark Horton	cbosg!mark
 *		Stephen Daniels	duke!swd
 *		Tom Truscott	duke!trt
 */

main(argc, argv)
int	argc;
register char	**argv;
{
	FILE *rcfp;
	FILE *openrc();
	time_t convdate() ;

	/* set up defaults and initialize. */
	pathinit();

#ifndef SHELL
	if ((SHELL = getenv("SHELL")) == NULL)
		SHELL = "/bin/sh";
#endif
	getuser();

	rcfp = openrc();
	roptions(argv, rcfp);

	if (datebuf) {
		atime = convdate(datebuf) ;
		free(datebuf) ;
	}

	/*
	 * ALL of the command line has now been processed. (!)
	 */

	if (sflag) {
		printf("Subscription list:  %s\n", sublist);
		xxit(0);
	}

	afopen();
	if (xflag)
		readinrc((FILE *)NULL);
	else {
		fseek(rcfp, 0L, 0);
		readinrc(rcfp);
	}
	fclose(rcfp);

#ifdef DEBUG
	fprintf(stderr, "sublist = %s\n", sublist);
#endif

	readr();

	fflush(stdout);
	if (xflag || lflag || tflag)
		xxit(0);
	writeoutrc();
	xxit(0);

	/* Camel, R.O.H. */
}



/*
 * convert a date to UNIX internal format.
 */

time_t
convdate(s)
      char *s ;
      {
      char buf[512] ;
      FILE *datefp, *popen() ;
      long cgtdate() ;
      long atol() ;

      sprintf(buf, "%s/cgtdate '%s'", LIB, s) ;
      if ((datefp = popen(buf, "r")) == NULL || fgets(buf, 512, datefp) == NULL)
            xerror("Can't convert -a date") ;
      return atol(buf) ;
}
