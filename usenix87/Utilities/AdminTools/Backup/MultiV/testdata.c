#include <stdio.h>
main(argc, argv)	/* garbo main() */
	int	argc;
	char	*argv[];
{
	long	atol();
	register long	j;
	register char	*str;
	register int	no_eoln, i;
	
	if (no_eoln = (*argv[1] == '-' && *++argv[1] == 'n')) {
		argv++;
		argc--;
	}
	for (i = 1; i < argc; i += 2) {
		str = argv[i+1];
		for (j = atol(argv[i]); j--; ) {
			printf("%s", str);
			if (!no_eoln)
				putc('\n', stdout);
		}
	}
}
