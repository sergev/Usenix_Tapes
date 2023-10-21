/*
** getopt.c	- argument parser
**
static	char	rcsid[] = "$HEADER$";
**
*/

#include "cw.h"

/*LINTLIBRARY*/
#ifndef	EOF
#	define	EOF	-1
#endif
#ifndef	SYSV
#	define	strchr	index
#endif
#define ERR(s, c)	if(opterr){\
		extern int strlen(), write();\
		char errbuf[2];\
		errbuf[0] = c; errbuf[1] = '\n';\
		(void) write(2, argv[0], (unsigned)strlen(argv[0]));\
		(void) write(2, s, (unsigned)strlen(s));\
		(void) write(2, errbuf, 2);}

extern	int	strcmp();
extern	char	*strchr();

static	int	opterr = 1;
int	optind = 1;
static	int	optopt;
char	*optarg;

int	getopt (argc, argv, opts)
int	argc;
char	**argv, *opts;
{
	static	int	sp = 1;
	reg	int	c;
	reg	char	*cp;

	if (sp == 1)
	{
		if (optind >= argc ||
		    argv[optind][0] != '-' ||
		    !argv[optind][1])
			return(EOF);
		if (!strcmp(argv[optind], "--"))
			return(optind++, EOF);
	}
	optopt = c = argv[optind][sp];
	if (c == ':' || !(cp=strchr(opts, c)))
	{
		ERR(": illegal option -- ", c);
		if (!argv[optind][++sp])
			optind++, sp = 1;
		return('?');
	}
	if (*++cp == ':')
	{
		if (argv[optind][sp+1])
			optarg = &argv[optind++][sp+1];
		else if (++optind >= argc)
		{
			ERR(": option requires an argument -- ", c);
			sp = 1;
			return('?');
		}
		else
			optarg = argv[optind++];
		sp = 1;
	}
	else
	{
		if (!argv[optind][++sp])
		{
			sp = 1;
			optind++;
		}
		optarg = (char *) 0;
	}
	return(c);
}
