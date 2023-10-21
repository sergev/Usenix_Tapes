/* vi: set tabstop=4 : */

/*
 * Version 1.3 December 1986
 *
 * sp - spell word
 *
 * Usage:	sp [-f dictionary-list] [-eavc] [word ...]
 *
 * Compute the Soundex code for each word on the command line
 * (or each word on the standard input) and compare against a
 * dictionary
 *
 * The soundex dictionary list may be specified on the command line
 * The environment variable SPPATH may be set to a list of colon
 * separated pathnames of soundex dictionaries.
 * If a command line dictionary-list (a colon separated list of pathnames) is
 * given in addition to the SPPATH variable, all dictionaries are used.
 *
 * To reduce the size of the word list, certain heuristics are used:
 * the -a option causes all words matched to be printed
 * The output is alphabetically sorted and indicators are printed
 * beside each word:
 *	X   == exact match
 *	!   == close match
 *	*   == near match
 * ' '  == matched
 *
 * Note that the maximum number of colliding words is MAXCOUNT due to the
 * data structure used.
 *
 * Permission is given to copy or distribute this program provided you
 * do not remove this header or make money off of the program.
 *
 * Please send comments and suggestions to:
 *
 * Barry Brachman
 * Dept. of Computer Science
 * Univ. of British Columbia
 * Vancouver, B.C. V6T 1W5
 *
 * .. {ihnp4!alberta, uw-beaver}!ubc-vision!ubc-cs!brachman
 * brachman@cs.ubc.cdn
 * brachman%ubc.csnet@csnet-relay.arpa
 * brachman@ubc.csnet
 */

#include <sys/types.h>
#include <sys/file.h>
#include <ctype.h>
#include <stdio.h>

#ifdef NEWDBM
#include <ndbm.h>
#else !NEWDBM
#include <dbm.h>
#endif NEWDBM

#include "sp.h"

#define streq(X, Y)	(!strcmp(X, Y))
#define range(S)	((strlen(S) + 4) / 5)

#define USAGE		"Usage: sp [-f dictionary-list] [-eavc] [word ...]"

char word[MAXWORDLEN + 2];

datum FETCH();

char *fileptr[MAXDICT + 1];	/* Up to MAXDICT dictionaries + sentinel */
int dict_ptr = 0;

char *wordptr[MAXWORDS], *wordlistptr;
char wordlist[WORDSPACE];
int nmatched;

/*
 * Soundex codes
 * The program depends upon the numbers zero through six being used
 * but this can easily be changed
 */
char soundex_code_map[26] = {
/***	 A  B  C  D  E  F  G  H  I  J  K  L  M  N  O  P	***/ 
		 0, 1, 2, 3, 0, 1, 2, 0, 0, 2, 2, 4, 5, 5, 0, 1,

/***	 Q  R  S  T  U  V  W  X  Y  Z			***/
		 2, 6, 2, 3, 0, 1, 0, 2, 0, 2
};

int aflag, cflag, eflag, vflag;

main(argc, argv)
int argc;
char **argv;
{
	register int fflag, i;
	register char *p;
	char *getenv();

	argc--; argv++;
	fileptr[0] = (char *) NULL;
	while (argc > 0 && argv[0][0] == '-') {
		fflag = 0;		/* to break out of following loop... */
		for (i = 1; argv[0][i] != '\0' && fflag == 0; i++) {
			switch (argv[0][i]) {
			case 'a':
				aflag = 1;
				break;
			case 'c':
				cflag = 1;
				break;
			case 'e':
				eflag = 1;
				break;
			case 'f':
				if (argc == 1) {
					fprintf(stderr, "%s\n", USAGE);
					exit(1);
				}
				mkfilelist(argv[1]);
				argc--;
				argv++;
				fflag = 1;		/* break out of loop */
				break;
			case 'v':
				vflag = 1;
				break;
			default:
				fprintf(stderr, "%s\n", USAGE);
				exit(1);
			}
		}
		argc--, argv++;
	}

	if ((p = getenv("SPPATH")) != (char *) NULL)
		mkfilelist(p);
	if (fileptr[0] == (char *) NULL)
		mkfilelist(DEFAULT_SPPATH);
	if (vflag) {
		printf("Using dictionaries:\n");
		for (i = 0; fileptr[i] != (char *) NULL; i++)
			if (strlen(fileptr[i]) > 0)
				printf("\t%s\n", fileptr[i]);
	}
	if (argc) {
		for (i = 0; i < argc; i++) {
			if (!eflag)
				printf("%s:\n", argv[i]);
			apply(argv[i]);
			if (!eflag)
				printf("\n");
		}
	}
	else {
		int ch, len;

		while (1) {
			printf("Word? ");
			if (fgets(word, sizeof(word), stdin) == (char *) NULL) {
				printf("\n");
				break;
			}
			len = strlen(word);
			if (word[len - 1] != '\n') {
				fprintf(stderr, "sp: Word too long: %s", word);
				while ((ch = getchar()) != '\n')	/* flush rest of line */
					putc(ch, stderr);
				putc('\n', stderr);
				continue;
			}
			word[--len] = '\0';
			if (len > MAXWORDLEN) {
				fprintf(stderr, "sp: Word too long: %s\n", word);
				continue;
			}

			apply(word);
			if (!eflag)
				printf("\n");
		}
	}
}

/*
 * Apply the Soundex search for a word to each dictionary in turn
 * Note that 'DBMINIT' opens both the '.dir' and the '.pag' files
 * and we must close them to avoid running out of file descriptors
 *
 * This routine gets called each time a word is looked up and therefore
 * the dbm files may be repeatedly opened and closed.  Since the vast majority
 * of the time this program is invoked for just a single word it doesn't seem
 * worthwhile to do the right thing by saving file descriptors/DBM pointers.
 * There probably won't be more than two dictionaries in use anyway.
 */
apply(word)
char *word;
{
	register int code, i, nodicts;

	nmatched = 0;
	wordlistptr = wordlist;
	if ((code = soundex(word, 3)) == BAD_WORD)
		return;
	nodicts = 1;
	for (i = 0; fileptr[i] != (char *) NULL; i++) {
		if (strlen(fileptr[i]) == 0)
			continue;
		if (DBMINIT(fileptr[i], O_RDONLY) != -1) {
			proc(code);
			nodicts = 0;
		}
		DBMCLOSE();
	}
	if (nodicts) {
		fprintf(stderr, "sp: Can't open any dictionaries\n");
		exit(1);
	}
	if (vflag && !eflag && nmatched == 0)
		printf("%s: no match\n", word);
	else
		choose(word);
}

/*
 * Look the word up in the current dictionary
 * and save all the matches
 * Note that only three digits are of the Soundex code are stored
 * in a dictionary
 */
proc(soundex)
int soundex;
{
	register int c, len;
	datum dbm_key, dbm_content;
	key_t *key, keyvec[KEYSIZE];
	char *mk_word(), *p;

	key = keyvec;
	dbm_key.dptr = (char *) key;
	dbm_key.dsize = KEYSIZE;
	c = 0;
	while (1) {
		mk_key(key, soundex, c);
		dbm_content = FETCH(dbm_key);

		if (dbm_content.dptr == 0)
			break;

		if (IS_DELETED(dbm_content)) {
			if (++c > MAXCOUNT) {
				fprintf(stderr, "sp: entry count overflow\n");
				exit(1);
			}
			continue;
		}

		if (nmatched == MAXWORDS) {
			fprintf(stderr, "sp: Too many matches\n");
			exit(1);
		}

		p = mk_word(dbm_content.dptr, dbm_content.dsize, soundex);
		len = strlen(p);
		if (wordlistptr + len >= &wordlist[WORDSPACE]) {
			fprintf(stderr, "sp: Out of space for words\n");
			exit(1);
		}
		strncpy(wordlistptr, p, len);
		wordlistptr[len] = '\0';
		wordptr[nmatched++] = wordlistptr;
		wordlistptr += len + 1;
		if (++c > MAXCOUNT) {
			fprintf(stderr, "sp: entry count overflow\n");
			exit(1);
		}
	}
}

/*
 * Select and print those words which we consider
 * to have matched 'word'
 */
choose(word)
register char *word;
{
	register int c, code, i, len, mcount, wordlen;
	register char *p;
	int compar();

	code = soundex(word, 4);
	qsort(wordptr, nmatched, sizeof(char *), compar);
	c = range(word);
	wordlen = strlen(word);
	mcount = 0;
	for (i = 0; i < nmatched; i++) {
		p = wordptr[i];
		if (strmatch(word, p) == 0) {
			printf("X");
			if (eflag) {
				printf(" %s\n", word);
				return;
			}
		}
		else if (eflag)
			continue;
		else if (soundex(p, 4) == code)
			printf("!");
		else if (aflag &&
			(wordlen < (len = strlen(p)) - c || len > wordlen + c))
			printf(" ");
		else if (!cflag)
			printf("*");
		else
			continue;
		printf("%3d. %s\n", mcount + 1, p);
		mcount++;
	}
	if (vflag)
		printf("(%d total matches)\n", nmatched);
}

/*
 * Compute an 'n' digit Soundex code for 'word' 
 * See mksp.c
 */
soundex(word, n)
register char *word;
int n;
{
	register int c, digit_part, previous_code, soundex_length;
	register char *p, *w;
	char wcopy[MAXWORDLEN + 2];

	strcpy(wcopy, word);
	p = w = wcopy;
	while (*p != '\0') {
		if (isupper(*p))
			*p = tolower(*p);
		p++;
	}
	if (!isalpha(*w)) {
		fprintf(stderr, "sp: Improper word: %s\n", word);
		return(BAD_WORD);
	}
	digit_part = 0;
	soundex_length = 0;
	previous_code = soundex_code_map[*w - 'a'];
	for (p = w + 1; *p != '\0' && soundex_length < n; p++) {
		if (!isalpha(*p))
			continue;
		c = soundex_code_map[*p - 'a'];
		if (c == 0 || previous_code == c) {
			previous_code = c;
			continue;
		}
		digit_part = digit_part * 7 + c;
		previous_code = c;
		soundex_length++;
	}
	while (soundex_length++ < n)
		digit_part *= 7;
	return((digit_part << 5) + *w - 'a');
}

/*
 * Process a path string (environment variable SPPATH, DEFAULT_SPPATH, or an
 * arg) by separating the pathnames into strings pointed to by elements
 * of 'fileptr'
 * End of list indicated by fileptr entry of NULL
 *
 * No attempt made to ignore duplicate pathnames
 */
mkfilelist(p)
register char *p;
{
	register int len;
	register char *path, *start;
	char *malloc();

	while (*p != '\0' && dict_ptr < MAXDICT) {
		start = p;
		while (*p != ':' && *p != '\0')
			p++;
		if (start == p && *p == ':') {	/* colon with nothing else */
			p++;
			continue;
		}
		len = p - start;
		path = (char *) malloc((unsigned) (len + 1));
		if (path == (char *) NULL) {
			fprintf(stderr, "sp: Out of dictionary space\n");
			exit(1);
		}
		strncpy(path, start, len);
		path[len] = '\0';
		fileptr[dict_ptr++] = path;
	}
	fileptr[dict_ptr] = (char *) NULL;
}

compar(p, q)
char **p, **q;
{

	return(strmatch(*p, *q));
/*	return(strcmp(*p, *q)); */	/* use if you prefer case sensitive */
}

