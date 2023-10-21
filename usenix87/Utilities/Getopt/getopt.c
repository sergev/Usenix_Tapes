/*
**  GETOPT PROGRAM AND LIBRARY ROUTINE
**
**  I wrote main() and AT&T wrote getopt() and we both put our efforts into
**  the public domain via mod.sources.
**	Rich $alz
**	Mirror Systems
**	(mirror!rs, rs@mirror.TMC.COM)
**	August 10, 1986
*/

#include <stdio.h>


#ifndef INDEX
#define INDEX index
#endif


extern char	*INDEX();
extern int	 optind;
extern char	*optarg;


main(ac, av)
    register int	 ac;
    register char 	*av[];
{
    register char 	*flags;
    register int	 c;

    /* Check usage. */
    if (ac < 2) {
	fprintf(stderr, "usage: %s flag-specification arg-list\n", av[0]);
	exit(2);
    }

    /* Play games; remember the flags (first argument), then splice
       them out so it looks like a "standard" command vector. */
    flags = av[1];
    av[1] = av[0];
    av++;
    ac--;

    /* Print flags. */
    while ((c = getopt(ac, av, flags)) != EOF) {
	if (c == '?')
	    exit(1);
	/* We assume that shells collapse multiple spaces in `` expansion. */
	printf("-%c %s ", c, INDEX(flags, c)[1] == ':' ? optarg : "");
    }

    /* End of flags; print rest of options. */
    printf("-- ");
    for (av += optind; *av; av++)
	printf("%s ", *av);
    exit(0);
}

/*
**  This is the public-domain AT&T getopt(3) code.  I added the
**  #ifndef stuff because I include <stdio.h> for the program;
**  getopt, per se, doesn't need it.  I also added the INDEX/index
**  hack (the original used strchr, of course).  And, note that
**  technically the casts in the write(2) calls shouldn't be there.
*/

#ifndef NULL
#define NULL	0
#endif
#ifndef EOF
#define EOF	(-1)
#endif
#ifndef INDEX
#define INDEX index
#endif


#define ERR(s, c)	if(opterr){\
	extern int strlen(), write();\
	char errbuf[2];\
	errbuf[0] = c; errbuf[1] = '\n';\
	(void) write(2, argv[0], (unsigned)strlen(argv[0]));\
	(void) write(2, s, (unsigned)strlen(s));\
	(void) write(2, errbuf, 2);}

extern int strcmp();
extern char *INDEX();

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
			return(EOF);
		}
	optopt = c = argv[optind][sp];
	if(c == ':' || (cp=INDEX(opts, c)) == NULL) {
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
