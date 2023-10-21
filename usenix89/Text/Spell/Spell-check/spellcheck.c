
/* RCS Info: $Revision: 1.3 $ on $Date: 86/04/02 10:33:31 $
 *           $Source: /ic4/faustus/src/spellfix/RCS/spellcheck.c,v $
 * Copyright (c) 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
 *	Permission is granted to do anything with this code except sell it
 *	or remove this message.
 *
 * spellcheck -- takes a list of sets of words on the standard input,
 *	of the form <word1> <word2> <junk> <word3> <junk> ... <wordn> <junk>
 * 	and outputs those lines for which none of the wordi are in the
 *	dictionary. We are very liberal with memory, unlike spell...
 */

#include "spellfix.h"
#include <sys/file.h>
#include <pwd.h>

int debug = 0;

/* Usage is spellcheck [badfile goodfile]. */
main(ac, av)
	char **av;
{
	char **words, **rcwords, *text;
	int nwords, nrcwords;
	char buf[BUFSIZ], wbuf[BUFSIZ];
	register int i, j, k, found;
	register char *s, *t;
	char c, d;
	FILE *good, *bad, *fopen();
	struct passwd *pwd, *getpwuid();

	if (ac == 1) {
		bad = stdout;
		good = NULL;
	} else if (ac == 3) {
		if (!(bad = fopen(av[1], "w"))) {
			perror(av[1]);
			exit(1);
		}
		if (!(good = fopen(av[2], "w"))) {
			perror(av[2]);
			exit(1);
		}
	} else {
		fprintf(stderr, "Usage: %s [badfile] [goodfile]\n", av[0]);
		exit(1);
	}

	pwd = getpwuid(getuid());
	if (!pwd && !pwd->pw_dir)
		exit(1);
	
	sprintf(buf, "%s/.spellrc", pwd->pw_dir);
	if (!access(buf, R_OK))
		nrcwords = readdict(buf, &rcwords);
	else
		nrcwords = 0;

	nwords = readdict((char *) NULL, &words);
	if (nwords <= 0)
		exit(1);

	while (fgets(buf, BUFSIZ, stdin)) {
		found = 0;
		for (text = buf; *text; ) {
			/* Grab the next word. */
			for (s = text, t = wbuf; *s && (*s != ' ') && 
					(*s != '\n'); )
				*t++ = *s++;
			*t = '\0';
			if (!*wbuf)
				break;
			/* Make sure text points to the next good stuff. */
			if (text != buf) {
				text = s;
				while (*text && (*text == ' '))
					text++;
				while (*text && (*text != ' '))
					text++;
				while (*text && (*text == ' '))
					text++;
			} else {
				text = s;
				while (*text && (*text == ' '))
					text++;
			}

			/* Now wbuf is the word we want. Make sure it
			 * isn't a number first...
			 */
			for (s = wbuf; *s && isdigit(*s); s++)
				;
			if (!*s) {
				found = 1;
				break;
			}
			i = -1;
			j = nwords;
			for (;;) {
				k = (i + j) / 2;
				if ((k == i) || (k == j))
					break;
				if (debug)
					printf("%s =? %s\n", wbuf, words[k]);
				for (s = wbuf, t = words[k]; ; ) {
					/* Ignore all this junk they allow
					 * in /usr/dict/words.
					 */
					while ((*s == '\'') || (*s == '&') ||
							(*s == '.'))
						s++;
					while ((*t == '\'') || (*t == '&') ||
							(*t == '.'))
						t++;
					c = isupper(*s) ? tolower(*s) : *s;
					d = isupper(*t) ? tolower(*t) : *t;
					if (c > d) {
						i = k;
						break;
					} else if (c < d) {
						j = k;
						break;
					} else if (!c && !d && words[k][1]) {
						/* No 1-letter matches... */
						found = 1;
						break;
					} else {
						s++;
						t++;
					}
				}
				if (found)
					break;
			}
			if (found)
				break;
			else {
				/* See if it was in .spellrc... */
				for (i = 0; i < nrcwords; i++)
					if (!strcmp(wbuf, rcwords[i])) {
						found = 1;
						break;
					}
			}
		}
		if (found) {
			if (good)
				fputs(buf, good);
			continue;
		} else {
			fputs(buf, bad);
		}
	}
	exit(0);
}

