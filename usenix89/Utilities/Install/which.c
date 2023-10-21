#include <stdio.h>
static char SCCSID[] = "@(#)which.c	Ver. 1.1, 86/12/29 13:46:31";
char *progname;
int all = 0;		/* find all occurrences */
int cwd = 1;		/* check cwd */
int warn = 1;		/* give a warning if not found */

main(argc,argv)
int argc;
char *argv[];
{
	char	*getenv(), *path = getenv("PATH");
	int c, getopt();
	extern int optind;
	extern char *optarg;

	progname = *argv;

	while((c = getopt(argc, argv, "aohq")) != EOF)
		switch(c) {
		case 'a':
			all++;
			break;
		case 'o':
			cwd = 0;
			break;
		case 'q':
			warn = 0;
			break;
		case 'h':
		default:
			help();
			exit(1);
		}
	switch(argc - optind) {
	case 0:
		help();
		break;
	default:
		for(; optind<argc; optind++) {
			if(cwd) putchar('\n');
			which(argv[optind], path);
		}
	}
	if(cwd) putchar('\n');
	exit(0);
}

help()
{
	fprintf(stderr, "Usage: %s [-ao] file [...]\n", progname);
	fprintf(stderr, "\t\tOptions are:\n");
	fprintf(stderr, "\t-a\tfind all occurrences in path\n");
	fprintf(stderr, "\t-o\tfind only first occurrence outside cwd\n");
}

/* which - C version of the unix/csh 'which' command
 * vix 23jul86 [written]
 * vix 24jul86 [don't use dynamic memory]
 */

which(name, path)
char	*name, *path;
{
	char	test[1000], *pc, *malloc(), save;
	int	len, namelen = strlen(name), found =0;
	int	count = 0;

	pc = path;
	if(*path == ':' && cwd && access(name, 01) == 0) { /* in cwd! */
		printf("./%s\n", name); /* go on to find other location */
		count++;
	}
	while (*pc != '\0' && (found == 0 || all) )
	{
		len = 0;
		while (*pc != ':' && *pc != '\0')
		{
			len++;
			pc++;
		}

		save = *pc;
		*pc = '\0';
		sprintf(test, "%s/%s", pc-len, name);
		*pc = save;
		if (*pc)
			pc++;

		found = (0 == access(test, 01));	/* executable */
		if (found) {
			puts(test);
			count++;
		}
	}
	if (count == 0 && warn)
	{
		printf("No %s (%s)\n", name, path);
	}
	return(count);
}
