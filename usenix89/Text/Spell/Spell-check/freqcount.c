
/* RCS Info: $Revision: 1.1 $ on $Date: 85/10/04 15:44:37 $
 *           $Source: /ic4/faustus/src/spellfix/RCS/freqcount.c,v $
 * Copyright (c) 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
 *	Permission is granted to do anything with this code except sell it
 *	or remove this message.
 *
 * Determine how frequent various words are in text. Usage is
 *	freq dictfile sample ...
 */

#include <stdio.h>
#include <ctype.h>

#define HASHSIZE 50021
#define OFFSET	13

struct hashent {
	char h_word[32];
	char h_extra[32];
	long h_freq;
} ;

struct hashent hashtab[HASHSIZE];

main(ac, av)
	char **av;
{
	FILE *fp, *fopen();
	int i = 2;

	if (ac < 3)
		exit(1);
	if (!(fp = fopen(av[1], "r"))) {
		perror(av[1]);
		exit(1);
	}
	fprintf(stderr, "loading dictionary... ");
	fflush(stderr);
	inpdict(fp);
	fclose(fp);
	fprintf(stderr, "done.\n");
	while (i < ac) {
		if (!(fp = fopen(av[i], "r"))) {
			perror(av[i]);
			exit(1);
		}
		fprintf(stderr, "reading file %s... ", av[i]);
		fflush(stderr);
		i++;
		addfile(fp);
		fclose(fp);
		fprintf(stderr, "done.\n");
	}
	printwords();
	exit(0);
}

inpdict(fp)
	FILE *fp;
{
	char buf[BUFSIZ];
	register unsigned int j, x = 0;
	unsigned int hash();
	char *extra;

	while (fgets(buf, BUFSIZ, fp)) {
		for (extra = buf; isalpha(*extra); extra++)
			;
		if (*extra && (*extra != '\n')) {
			*extra = '\0';
			extra++;
		} else {
			*extra = '\0';
			extra = NULL;
		}
		j = hash(buf);
		while (hashtab[j].h_word[0]) {
			j += OFFSET;
			j %= HASHSIZE;
		}
		strcpy(hashtab[j].h_word, buf);
		if (extra && *extra)
			strcpy(hashtab[j].h_extra, extra);
		hashtab[j].h_freq = 0;
		if ((++x % 1000) == 0) {
			fprintf(stderr, ".");
			fflush(stderr);
		}
	}
	return;
}

unsigned
hash(word)
	register char *word;
{
	register unsigned int i = 0;
	register unsigned int p = 5003;

	while (*word) {
		i += (*word++ * (i + p));
		i %= HASHSIZE;
	}
	return (i);
}

addfile(fp)
	FILE *fp;
{
	char buf[BUFSIZ];
	register char *s;
	register int j, ct, x = 0;

	while (fgets(buf, BUFSIZ, fp)) {
		for (s = buf; isalpha(*s); s++)
			;
		*s = '\0';
		j = hash(buf);
		if (!hashtab[j].h_word[0]) {
			/* printf("= %s\n", buf); */
			continue;
		}
		ct = 0;
		while (strcmp(buf, hashtab[j].h_word)) {
			j += OFFSET;
			j %= HASHSIZE;
			if (ct++ > 100) {
				/* printf("= %s\n", buf); */
				goto moe;
			}
		}
		hashtab[j].h_freq++;
moe:		;
		if ((++x % 1000) == 0) {
			fprintf(stderr, ".");
			fflush(stderr);
		}
	}
	return;
}

printwords()
{
	register int i;

	for (i = 0; i < HASHSIZE; i++)
		if (hashtab[i].h_word[0])
			printf("%s %d\n", hashtab[i].h_word,
					hashtab[i].h_freq);
	return;
}

