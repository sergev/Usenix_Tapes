
/* RCS Info: $Revision: 1.1 $ on $Date: 85/09/27 23:06:37 $
 *           $Source: /ic4/faustus/src/spellfix/RCS/makeadj.c,v $
 * Copyright (c) 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
 *	Permission is granted to do anything with this code except sell it
 *	or remove this message.
 */

#include <stdio.h>
#include <ctype.h>

#define NLETTERS 26

long adjacent[NLETTERS][NLETTERS];

main(ac, av)
	char **av;
{
	char buf[512];
	register char *s;
	register int i, j, highest = 0;

	fprintf(stderr, "Reading words..."); fflush(stderr);
	while (gets(buf)) {
		for (s = buf; *s; s++)
			if (!isalpha(*s))
				goto moe;
		for (s = buf; (s[0] && s[1]); s++) {
			i = (isupper(s[0]) ? tolower(s[0]) : s[0]) - 'a';
			j = (isupper(s[1]) ? tolower(s[1]) : s[1]) - 'a';
			adjacent[i][j]++;
		}
moe:	;
	}
	fprintf(stderr, " done.\n"); fflush(stderr);
	for (i = 0; i < NLETTERS; i++)
		for (j = 0; j < NLETTERS; j++)
			if (adjacent[i][j] > highest)
				highest = adjacent[i][j];
	fprintf(stderr, "Highest frequency = %d, ", highest);
	highest /= 255;
	fprintf(stderr, "dividing by %d...", highest); fflush(stderr);
	for (i = 0; i < NLETTERS; i++)
		for (j = 0; j < NLETTERS; j++)
			adjacent[i][j] /= highest;
	fprintf(stderr, " done.\n"); fflush(stderr);

	for (i = 0; i < NLETTERS; i++) {
		printf("	{ ");	/* { */
		for (j = 0; j < NLETTERS; j++)
			printf("%d%s", adjacent[i][j],
					(j < NLETTERS - 1) ? ", " : "");
		if (i < NLETTERS - 1)
			printf(" } ,\n");
		else
			printf(" }\n");
	}
	exit (0);
}

