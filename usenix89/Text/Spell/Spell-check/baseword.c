
/* RCS Info: $Revision: 1.4 $ on $Date: 86/04/02 10:33:10 $
 *           $Source: /ic4/faustus/src/spellfix/RCS/baseword.c,v $
 * Copyright (c) 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
 *	Permission is granted to do anything with this code except sell it
 *	or remove this message.
 *
 * This program takes words with possible prefixes and suffixes and strips
 * them off. The output is a list of pairs: <base word, trans info>, where
 * the transformation info describes how to change the base word into the
 * word given. Each pair describes one possible way to parse the word (so
 * "catting" would produce "catt $+ing cat $.+ing"). The transformation data
 * is basically an editing script: characters have the following
 * interpretation:
 *	$ go to the character at the end of the word
 *	^ go to before the beginning of the word
 *	- delete the following characters
 *	+ add the following characters (the default mode)
 *	. duplicate the character we are on now
 *	c any other character means either add this character after the
 *		current one or delete the current one and move to the next
 *		one (if it isn't there the match fails), depending on 
 *		whether we last saw a + or a -.
 * The first word on the output line is the origonal unmodified input,
 * and it doesn't have a transformation word. Note that the first
 * character in the mod must be either '^' or '$', and neither character
 * can appear anywhere else. This makes things a bit easier to do,
 * and this is really all that is needed for this application.
 */

#include "spellfix.h"

/* The table of all possible word modifiers. The trans info we output will
 * be one of these. Note that the process below is the opposite of what we
 * are specifying in the mods.
 */

char *mods[] = {
	"$+ive",
	"$-e+ive",
	"$+ism",
	"$-e+ism",
	"$+y",
	"$+es",
	"$-y+ies",
	"$+al",
	"$+s",
	"$+ing",
	"$.+ing",
	"$-e+ing",
	"$+ion",
	"$-e+ion",
	"$+ness",
	"$-y+iness",
	"$+like",
	"$+less",
	"$-y+iless",
	"$+ize",
	"$-e+ize",
	"$+'s",
	"$-t+ce",
	"$+ity",
	"$-y+iable",
	"$+able",
	"$-e+able",
	"$+.able",
	"$+ity",
	"$-e+ity",
	"$+ly",
	"$+.ly",
	"$-y+ily",
	"$+ment",
	"$+ater",
	"$+er",
	"$-e+er",
	"$+ed",
	"$-e+ed",
	"$+.ed",
	"$-y+ied",
	"$+est",
	"$+.est",
	"$-e+est",
	"$-y+iest",
	"$-y+ication",
	"$+ship",
	"^+anti",
	"^+bio",
	"^+dis",
	"^+electro",
	"^+en",
	"^+fore",
	"^+hyper",
	"^+intra",
	"^+inter",
	"^+iso",
	"^+kilo",
	"^+magneto",
	"^+meta",
	"^+micro",
	"^+milli",
	"^+mis",
	"^+mono",
	"^+multi",
	"^+non",
	"^+out",
	"^+over",
	"^+photo",
	"^+poly",
	"^+pre",
	"^+pseudo",
	"^+re",
	"^+semi",
	"^+stereo",
	"^+sub",
	"^+super",
	"^+thermo",
	"^+ultra",
	"^+under",
	"^+un",
} ;

/* For each word, we have to try to apply as many decompositions as
 * possible. So we keep a list of words and repeatedly try to
 * decompose them, adding the results to the list. We may get
 * several copies of each decomposition this way, so we should try
 * and print only one. (but we don't...)
 */

main(ac, av)
	char **av;
{
	char buf[BUFSIZ];
	char *poss[NALTS], *change[NALTS], done[NALTS];
	char **words, **changes;
	register char *t, *s;
	register int i, j, k, l;
	int nchanges, changemade;
	char modmade[sizeof (mods) / sizeof (char *)];

	while (fgets(buf, BUFSIZ, stdin)) {
		for (t = buf; *t && (*t != '\n') && (*t != ' ') && 
				(*t != '\t'); t++)
			;
		*t = '\0';
		bzero(done, NALTS);
		bzero(poss, sizeof (char *) * NALTS);
		bzero(change, sizeof (char *) * NALTS);
		bzero(modmade, sizeof (modmade));
		poss[0] = buf;
		do {
			changemade = 0;
			for (i = 0; i < NALTS; i++) {
				/* See what we can do with this word. */
				if (poss[i] && !done[i]) {
					nchanges = decomp(poss[i], &words,
							&changes, modmade);
				} else
					continue;
				for (j = 0, k = 0; j < nchanges; j++) {
					/* First make sure we don't already
					 * have this one.
					 */
					for (l = 0; poss[l]; l++)
						if (!strcmp(poss[l], words[j]))
							break;
					if (poss[l])
						continue;
					/* Find a free place. */
					while (poss[k] && (k < NALTS))
						k++;
					if (k == NALTS) {
						/* None left... */
						fprintf(stderr, "Gasp...\n");
						goto newword;
					}
					poss[k] = words[j];

					/* Now concatenate the two mods. */
					s = malloc(strlen(changes[j]) +
							strlen(change[i]) + 1);
					sprintf(s, "%s%s", changes[j],
							change[i]);
					change[k] = s;
				}
				if (nchanges) {
					/* Mark this as already dealt with. */
					done[i] = 1;
					changemade = 1;
				}
			}
		} while (changemade);

		fputs(poss[0], stdout);
		for (i = 1; poss[i]; i++) {
			printf(" %s %s", poss[i], change[i]);
			free(poss[i]);
			free(change[i]);
		}
newword:	putchar('\n');
	}
	exit(0);
}

/*  Try to decompose this word, stripping off one prefix or suffix. */

decomp(word, wptr, cptr, modmade)
	char *word;
	char ***wptr, ***cptr;
	char *modmade;
{
	register int i, j, k;
	register char *s, *t, *r;
	char buf[BUFSIZ], mbuf[BUFSIZ];
	int addmode, forward, decount = 0;
	static char *poss[NALTS], *changes[NALTS];
	int nmods = sizeof (mods) / sizeof (char *);

	for (i = 0; i < nmods; i++) {
		if (modmade[i])
			continue;
		/* Try to apply the reverse of mods[i] to buf. */
		strcpy(buf, word);
		strcpy(mbuf, mods[i]);
		addmode = 1;
		switch (*mbuf) {
			case '^':
				forward = 1;
				s = buf;
				break;
			case '$':
				forward = 0;
				for (s = buf; s[1]; s++)
					;
				break;
			default:
				fprintf(stderr, "Bad mod %s...\n",
						mbuf);
				exit(1);
		}
		if (forward) {	/* Damn tabs. */
			for (t = mbuf + 1; *t; t++) {
				switch (*t) {
					case '+':
						addmode = 1;
						break;
					case '-':
						addmode = 0;
						break;
					case '.':
						if ((s > buf) &&
							(s[-1] == s[0])) {
							for (r = s; *r; r++)
								r[0] = r[1];
							s--;
						} else
							goto out;
						break;
					default:

					if (!isvalid(*t)) {
						fprintf(stderr, 
						"Bad character %c in mod %s\n",
								*t, t);
						exit(1);
					} else if (addmode) {
						if (*s == *t) {
							for (r = s; *r; r++)
								r[0] = r[1];
						} else if (isupper(*s) &&
							(tolower(*s) == *t)) {
							for (r = s; *r; r++)
								r[0] = r[1];
							*t = toupper(*t);
						} else
							goto out;
					} else {
						for (r = s; *r; r++)
							;
						for (; r >= s; r--)
							r[1] = r[0];
						*s++ = *t;
					}
				}
			}
		} else {
			/* In this case, we have to go back from the
			 * end. This is seriously ugly stuff.
			 */
			for (t = mbuf; t[1]; t++)
				;
			for (r = t; (r > mbuf) && (*r != '-') &&
					(*r != '+'); r--)
				;
			switch (*r) {
				case '-':
					addmode = 0;
					break;
				case '+':
					addmode = 1;
					break;
				default:
					fprintf(stderr, "Bad mod %s\n",
							mbuf);
					exit(1);
			}
			for (; t > mbuf; t--) {
				switch (*t) {
					case '+':
					case '-':
						for (r = t - 1; (r > mbuf) &&
						(*r != '-') && (*r != '+'); r--)
							;
						if (*r == '-')
							addmode = 0;
						else
							addmode = 1;
						break;
					case '.':
						if ((s > buf) && 
							(s[-1] == s[0])) {
							for (r = s; *r; r++)
								r[0] = r[1];
							s--;
						} else
							goto out;
						break;
					default:
					if (!isvalid(*t)) {
						fprintf(stderr, 
						"Bad character %c in mod %s\n",
									*t, t);
						exit(1);
					} else if (addmode) {
						if (*s == *t) {
							for (r = s; *r; r++)
								r[0] = r[1];
							s--;
						} else if (isupper(*s) &&
							(tolower(*s) == *t)) {
							for (r = s; *r; r++)
								r[0] = r[1];
							s--;
							*t = toupper(*t);
						} else
							goto out;
					} else {
						for (r = s; *r; r++)
							;
						for (; r > s; r--)
							r[1] = r[0];
						s[1] = t[0];
					}
				}
			}
		}

out:		if ((forward && !*t) || (t == mbuf)) {
			/* Ok, this modification works. */
			poss[decount] = malloc(strlen(buf) + 1);
			strcpy(poss[decount], buf);
			changes[decount] = malloc(strlen(mbuf) + 1);
			strcpy(changes[decount], mbuf);
			decount++;
			modmade[i] = 1;
		}
	}
	*wptr = poss;
	*cptr = changes;
	return (decount);
}

