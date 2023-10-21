
#include <stdio.h>
#include <ctype.h>

main()
{
	char buf[BUFSIZ];
	register char *s, *t;
	int pl;

	while (fgets(buf, BUFSIZ, stdin)) {
		for (s = buf; *s && (*s != ':'); s++)
			if (!isalpha(*s))
				break;
		if (*s != ':')
			continue;
		while (*++s != ':')
			;
		while (*++s != ':')
			;
		if (*++s == 'p')
			pl = 1;
		else
			pl = 0;
		s += 2;
		if (((*s == 'v') && pl) || ((*s == 'n') && !pl) ||
				((*s != 'v') && (*s != 'n'))) {
			for (t = buf; isalpha(*t); t++)
				putchar(*t);
			printf(" %s", t);
		}
	}
	exit(0);
}

