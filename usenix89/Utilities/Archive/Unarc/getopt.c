/*
**  Return options and their values from the command line.
**
**  This comes from the AT&T public-domain getopt published in mod.sources.
*/
/* LINTLIBRARY */
#include "shar.h"
RCS("$Header: getopt.c,v 1.4 87/03/02 11:03:24 rs Exp $")

#ifdef	NEED_GETOPT


#define TYPE	int

#define ERR(s, c)	if(opterr){\
	char errbuf[2];\
	errbuf[0] = c; errbuf[1] = '\n';\
	(void) write(2, argv[0], (TYPE)strlen(argv[0]));\
	(void) write(2, s, (TYPE)strlen(s));\
	(void) write(2, errbuf, 2);}

extern int strcmp();

int	opterr = 1;
int	optind = 1;
int	optopt;
char	*optarg;

int
getopt(argc, argv, opts)
int	argc;
char	**argv, *opts;
{
	static int sp = 1;
	register int c;
	register char *cp;

	if(sp == 1)
		if(optind >= argc ||
		   argv[optind][0] != '-' || argv[optind][1] == '\0')
			return(EOF);
		else if(strcmp(argv[optind], "--") == NULL) {
			optind++;

		}
	optopt = c = argv[optind][sp];
	if(c == ':' || (cp=IDX(opts, c)) == NULL) {
		ERR(": illegal option -- ", c);
		if(argv[optind][++sp] == '\0') {
			optind++;
			sp = 1;
		}
		return('?');
	}
	if(*++cp == ':') {
		if(argv[optind][sp+1] != '\0')
			optarg = &argv[optind++][sp+1];
		else if(++optind >= argc) {
			ERR(": option requires an argument -- ", c);
			sp = 1;
			return('?');
		} else
			optarg = argv[optind++];
		sp = 1;
	} else {
		if(argv[optind][++sp] == '\0') {
			sp = 1;
			optind++;
		}
		optarg = NULL;
	}
	return(c);
}

#endif	/* NEED_GETOPT */
