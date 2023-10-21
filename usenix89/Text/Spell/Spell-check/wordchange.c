
/* RCS Info: $Revision: 1.2 $ on $Date: 85/10/08 18:35:24 $
 *           $Source: /ic4/faustus/src/spellfix/RCS/wordchange.c,v $
 * Copyright (c) 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
 *	Permission is granted to do anything with this code except sell it
 *	or remove this message.
 *
 * This program is called by wordchange file ..., and it reads the
 * standard input for pairs of words -- an old word, and a new word
 * to replace it with.
 */

#include "spellfix.h"

main(ac, av)
	char **av;
{
	char *tempf = "/tmp/wchXXXXXX", *mktemp();
	FILE *fp, *tp, *fopen();
	char buf[BUFSIZ];
	register char **old, **new, *s, *t;
	register int i = 0, j, k;
	int nwords = 0;
	char *beg;

	/* First collect all the changes. */
	tempf = mktemp(tempf);
	if (!(tp = fopen(tempf, "w+"))) {
		perror(tempf);
		exit(1);
	}
	while (fgets(buf, BUFSIZ, stdin)) {
		nwords++;
		fputs(buf, tp);
	}
	rewind(tp);

	old = (char **) malloc(nwords * sizeof (char *));
	new = (char **) malloc(nwords * sizeof (char *));

	/* Now read them back in. */
	while (fgets(buf, BUFSIZ, tp)) {
		old[i] = malloc(strlen(buf) + 1);
		strcpy(old[i], buf);
		for (s = old[i]; *s && (*s != ' '); s++)
			;
		if (*s) {
			*s = '\0';
			new[i] = s + 1;
		} else
			new[i] = s;
		for (s = new[i]; *s && (*s != ' ') && (*s != '\t') &&
				(*s != '\n'); s++)
			;
		*s = '\0';
		i++;
	}

	/* Now for each file, go through and do the replacements. */
	for (av++; *av; av++) {
		if (!(fp = fopen(*av, "r+"))) {
			perror(*av);
			continue;
		}
		rewind(tp);

		while (fgets(buf, BUFSIZ, fp)) {
			/* Now do the substitutions on this line. */
			for (i = 0; i < nwords; i++) {
				for (s = buf; *s; ) {
					/* Find the beginning of a word. */
					while (!isvalid(*s))
						s++;
					while (*s && (*s == '\''))
						s++;
					beg = s;
					for (t = old[i]; *t; t++)
						if (*t != *s)
							break;
						else
							s++;
					if (*t || isvalid(*s)) {
						while (isvalid(*s))
							s++;
						continue;
					}
					/* Now splice the new word in. Don't
					 * look at this code or you will die.
					 */
					k = strlen(old[i]);
					j = k - strlen(new[i]);
					if (j > 0) {
						for (t = beg + j; *t; t++)
							t[-j] = t[0];
						t[-j] = '\0';
					} else if (j < 0) {
						for (t = s; *t; t++)
							;
						for (; t >= beg + k; t--)
							t[-j] = t[0];
					}
					for (t = new[i]; *t; t++)
						*beg++ = *t;
					s = beg;	/* Important... */
				}
			}
			fputs(buf, tp);
		}
		ftruncate(fileno(tp), ftell(tp));
		rewind(tp);
		rewind(fp);
		while ((i = fread(buf, 1, BUFSIZ, tp)) > 0)
			fwrite(buf, 1, i, fp);
		ftruncate(fileno(fp), ftell(fp));
		fclose(fp);
	}
	fclose(tp);
	unlink(tempf);
	exit(0);
}

