#include	<stdio.h>

int	optind = 1;
char	*optarg;

getopt(argc, argv, ostr)
int	argc;
char	**argv, *ostr;
{
	int c;
	char *cp, *strchr();

	if( optind >= argc ||
	   argv[optind][0] != '-' || argv[optind][1] == '\0' )
		return(EOF);
	c = argv[optind][1];
	if( (cp = strchr(ostr, c)) == NULL ) {
		optind++;
		return((int)'?');
	}
	if( *++cp == ':' ) {
		if( argv[optind][2] != '\0' ) {
			optarg = &argv[optind][2];
		} else if( ++optind >= argc ) {
			fprintf(stderr, "missing argument to %c option\n", c);
			return((int)'?');
		} else {
			optarg = argv[optind];
		}
	}
	optind++;
	return(c);
}
