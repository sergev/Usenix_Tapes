#include <stdio.h>

#define ERR(S, A)	if (opterr) error((S), (A)); else

int	opterr = 1;
int	optind = 1;
int	optopt;
char	*optarg;
char	*program;

extern	char *sname();
extern	char *index();

#ifndef lint
#ifndef NSCCS
static char sccsid[] = "%W%";
#endif
#endif

int
getopt(argc, argv, opts)
char **argv, *opts;
{
	static int sp = 1;
	register c;
	register char *cp;

	if (program == NULL)
		program = sname(argv[0]);

	if (sp == 1) {
		if (optind >= argc || argv[optind][0] != '-' || argv[optind][1] == '\0')
			return(EOF);
		else if (strcmp(argv[optind], "--") == NULL) {
			optind++;
			return(EOF);
		}
	}
	optopt = c = argv[optind][sp];
	if (c == ':' || (cp = index(opts, c)) == NULL) {
		ERR("illegal option -%c", c);
		if (argv[optind][++sp] == '\0') {
			optind++;
			sp = 1;
		}
		return('?');
	}
	if (*++cp == ':') {
		if (argv[optind][sp + 1] != '\0')
			optarg = &argv[optind++][sp + 1];
		else if (++optind >= argc) {
			ERR("option -%c requires an argument", c);
			sp = 1;
			return('?');
		} else
			optarg = argv[optind++];
		sp = 1;
	} else {
		if (argv[optind][++sp] == '\0') {
			sp = 1;
			optind++;
		}
		optarg = NULL;
	}
	return(c);
}
