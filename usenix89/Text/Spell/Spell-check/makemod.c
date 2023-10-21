
/* RCS Info: $Revision: 1.1 $ on $Date: 85/10/08 18:36:16 $
 *           $Source: /ic4/faustus/src/spellfix/RCS/makemod.c,v $
 * Copyright (c) 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
 *	Permission is granted to do anything with this code except sell it
 *	or remove this message.
 *
 * This routine carries out modifications on base words (see the description
 * in baseword.c for details).
 * Defined: makemod
 */

#include "spellfix.h"

char *
makemod(word, mod)
	char *word, *mod;
{
	char buf[BUFSIZ];
	register char *s, *t, *r;
	int addmode = 1;

	strcpy(buf, word);
	s = buf;
	for (t = mod; *t; t++) {
		switch (*t) {
			case '$':
				while (s[1])
					s++;
				break;
			case '^':
				s = buf - 1;
				break;
			case '.':
				for (r = s; *r; r++)
					;
				for ( ; r >= s; r--)
					r[1] = r[0];
				s++;
				break;
			case '-':
				addmode = 0;
				break;
			case '+':
				addmode = 1;
				break;
			default:
				if (!isvalid(*t)) {
					/* Ack.. */
					return (NULL);
				} else if (addmode) {
					for (r = s + 1; *r; r++)
						;
					for ( ; r > s; r--)
						r[1] = r[0];
					*++s = *t;
				} else {
					if (*s != *t) {
						/* What now? */
						return (NULL);
					}
					/* Strange case... */
					while (s < buf)
						s++;
					for (r = s; *r; r++)
						r[0] = r[1];
					if (!*s)
						s--;
				}
		}
	}
	s = malloc(strlen(buf) + 1);
	strcpy(s, buf);
	return (s);
}

/* main(ac, av) char **av;{ printf("%s\n", makemod(av[1], av[2])); exit(0); } */

