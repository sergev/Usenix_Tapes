
/* RCS Info: $Revision: 1.3 $ on $Date: 85/10/08 18:35:20 $
 *           $Source: /ic4/faustus/src/spellfix/RCS/readdict.c,v $
 * Copyright (c) 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
 *	Permission is granted to do anything with this code except sell it
 *	or remove this message.
 */

#include "spellfix.h"

/* This routine creates the word table from the dictionary -- it returns
 * the number of words read.
 */

readdict(dictfile, ptr)
	char *dictfile;
	char ***ptr;
{
	register FILE *fp;
	FILE *fopen();
	register int i = 0, c, j = 0;
	char buf[BUFSIZ];
	register char **words = NULL, *s, *field;

	if (!dictfile)
		dictfile = DICTFILE;
	if (!(fp = fopen(dictfile, "r"))) {
		perror(dictfile);
		return (0);
	}
	while ((c = getc(fp)) != EOF) {
		if (c == '\n')
			i++;
		j++;
	}
	rewind(fp);

	/* Now get one big chunk of memory for this file. */
	field = malloc(j);
	words = (char **) malloc(i * sizeof (char *));
	if (!words || !field) {
		fprintf(stderr, "drat, malloc failed\n");
		return (0);
	}

	for (c = 0; c < i; c++) {
		fgets(buf, BUFSIZ, fp);
		words[c] = field;
		for (s = buf; *s && (*s != ' ') && (*s != '\t') &&
				(*s != '\n'); s++)
			*field++ = *s;
		*field++ = '\0';
	}
	*ptr = words;

	return (i);
}

/* main() { char **p; printf("%d entries\n", readdict((char *) NULL, &p)); }*/

