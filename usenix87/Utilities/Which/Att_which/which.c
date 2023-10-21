/* which - C version of the unix/csh 'which' command
 * vix 23jul86 [written]
 * vix 24jul86 [don't use dynamic memory]
 */

#include <stdio.h>

static	char	*myname;

main(argc, argv)
int	argc;
char	*argv[];
{
	char	*getenv(), *path = getenv("PATH");

	myname = argv[0];
	for (argc--, argv++;  argc;  argc--, argv++)
		if (0 != which(*argv, path))
			exit(1);
	exit(0);
}

static which(name, path)
char	*name, *path;
{
	char	test[1000], *pc, *malloc(), save;
	int	len, namelen = strlen(name), found;

	pc = path;
	found = 0;
	while (*pc != '\0' && found == 0)
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
		if (found)
			puts(test);
	}
	if (found == 0)
	{
		printf("%s: no %s in (%s)\n", myname, name, path);
		return 1;
	}
	return 0;
}
