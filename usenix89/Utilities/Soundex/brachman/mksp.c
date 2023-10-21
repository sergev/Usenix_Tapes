/* vi: set tabstop=4 : */

/*
 * mksp - make soundex dictionary
 * Version 1.3,  December 1986
 *
 * If <soundexfile.{dir, pag}> do not exist, try to create them
 * If they do exist and are not empty, then words will be added
 * from the standard input
 * Only when words are being added to an existing database are duplicate words
 * ignored
 * Valid words (words beginning with an alphabetic) are stored as given but
 * comparisons for duplicates is case independent.
 * Non-alphabetic characters are ignored in computing the soundex
 *
 * Permission is given to copy or distribute this program provided you
 * do not remove this header or make money off of the program.
 *
 * Please send comments and suggestions to:
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

#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <stdio.h>

#ifdef NEWDBM
#include <ndbm.h>
#include <sys/file.h>
#else !NEWDBM
#include <dbm.h>
#endif NEWDBM

#include "sp.h"

#define NEW_DICT			0
#define OLD_DICT			1

#define VFREQ				1000	/* frequency for verbose messages */

#define streq(X, Y)			(!strcmp(X, Y))
#define strneq(X, Y, N)		(!strncmp(X, Y, N))

#define USAGE				"Usage: mksp -tad [-v[#]] <soundexfile>"

int map[26][667];

datum FETCH(), FIRSTKEY(), NEXTKEY();

key_t keyvec[KEYSIZE];
key_t *key = keyvec;

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

int digit_part;

int aflag, dflag, tflag, vflag;

char *mk_word();

main(argc, argv)
int argc;
char **argv;
{
	register int i;
	char *file;

	if (argc != 3 && argc != 4) {
		fprintf(stderr, "%s\n", USAGE);
		exit(1);
	}
	aflag = dflag = tflag = vflag = 0;
	file = (char *) NULL;

	for (i = 1; i < argc; i++) {
		if (streq(argv[i], "-a"))
			aflag = 1;
		else if (streq(argv[i], "-d"))
			dflag = 1;
		else if (streq(argv[i], "-t"))
			tflag = 1;
		else if (strneq(argv[i], "-v", 2)) {
			if (isdigit(argv[i][2]))
				vflag = atoi(&argv[i][2]);
			else
				vflag = 1;
		}
		else if (file == (char *) NULL)
			file = argv[i];
		else {
			fprintf(stderr, "%s\n", USAGE);
			exit(1);
		}
	}

	if (file == (char *) NULL || (tflag + aflag + dflag) != 1) {
		fprintf(stderr, "%s\n", USAGE);
		exit(1);
	}

	if (aflag) {
		addwords(file);
		if (vflag > 1) {
			register int j, total;
			int m, max;

			fprintf(stderr, "Counters:\n");
			for (i = 0; i < 26; i++) {
				total = max = map[i][0];
				for (j = 1; j < 667; j++) {
					total += (m = map[i][j]);
					if (m > max)
						max = m;
				}
				if (max > 0)
					fprintf(stderr, "%c: max %d total %d\n", 'a'+i, max, total);
			}
		}
	}
	else if (dflag)
		deletewords(file);
	else if (tflag)
		prcontents(file);
	exit(0);
}

/*
 * Add words read from stdin to the database
 * The key is the 3 digit soundex code for the word plus a disambiguating
 * counter.  Different counter values are used for words with the same soundex
 * code.  The maximum counter value is MAXCOUNT.  If the counter overflows then
 * we lose, but given at least 8 bits this seems unlikely.
 */
addwords(name)
char *name;
{
	register int c, count, delete, duplicate, i, len;
	register int *p;
	int ch, s, status;
	char wcopy[MAXWORDLEN + 2], word[MAXWORDLEN + 2];
	datum dbm_key, dbm_content;

	status = setup(name);
	if (DBMINIT(name, O_RDWR) == -1) {
		fprintf(stderr, "mksp: Can't initialize\n");
		exit(1);
	}

	for (i = 0; i < 26; i++)
		for (c = 0; c < 667; c++)
			map[i][c] = 0;

	dbm_key.dptr = (char *) key;
	dbm_key.dsize = KEYSIZE;

	count = 0;

	while (fgets(word, sizeof(word), stdin) != (char *) NULL) {
		len = strlen(word);
		if (word[len - 1] != '\n') {
			fprintf(stderr, "mksp: Word too long: %s", word);
			while ((ch = getchar()) != '\n')	/* flush rest of line */
				putc(ch, stderr);
			putc('\n', stderr);
			continue;
		}
		word[--len] = '\0';
		if (len > MAXWORDLEN) {
			fprintf(stderr, "mksp: Word too long: %s\n", word);
			continue;
		}

		if ((s = soundex(word, 3)) == BAD_WORD) {
			if (vflag)
				fprintf(stderr, "Ignoring bad word: %s\n", word);
			continue;
		}
		ch = (isupper(word[0]) ? tolower(word[0]) : word[0]) - 'a';
		p = &(map[ch][digit_part]);

		/*
		 * If words are being added to an old dictionary,
		 * check for duplication and watch for a deleted entry
		 * The reason for only checking for duplicates in old dictionaries is
		 * that usually when you're creating a new dictionary the words are
		 * already sorted and unique and the creation of a large dictionary is
		 * slow enough already.
		 */
		duplicate = 0;
		delete = -1;			/* an 'impossible' counter */
		if (status == OLD_DICT) {
			c = 0;
			while (1) {
				mk_key(key, s, c);
				dbm_content = FETCH(dbm_key);
				if (dbm_content.dptr == 0)
					break;

				if (!IS_DELETED(dbm_content)) {
					char *str;

					str = mk_word(dbm_content.dptr, dbm_content.dsize, s);
					if (strmatch(word, str) == 0) {
						duplicate = 1;
						if (vflag)
							fprintf(stderr, "duplicate: %s\n", word);
						break;
					}
				}
				else if (delete < 0)	/* choose delete nearest front */
					delete = c;

				if (++c > MAXCOUNT) {
					fprintf(stderr, "mksp: Counter overflow\n");
					fprintf(stderr, "soundex: %c%d\n", ch+'a', b10(digit_part));
					exit(1);
				}
			}
			if (duplicate)
				continue;
			*p = c;
		}
		if (*p > MAXCOUNT) {
			fprintf(stderr, "mksp: Counter overflow\n");
			fprintf(stderr, "soundex: %c%d\n", ch+'a', b10(digit_part));
			exit(1);
		}
		mk_key(key, s, *p);
		*p = *p + 1;
		strcpy(wcopy, word);
		if (len == 1) {
			if (isupper(wcopy[0]))
				wcopy[0] |= UPPER_CHAR;
			wcopy[0] |= SINGLE_CHAR;
			dbm_content.dptr = wcopy;
		}
		else {
			if (isupper(wcopy[1]))
				wcopy[1] = wcopy[1] - 'A' + 26;
			else if (islower(wcopy[1]))
				wcopy[1] = wcopy[1] - 'a';
			else if ((wcopy[1] = tospchar(wcopy[1])) == '\0') {
				fprintf(stderr, "Bogus second char: can't happen!\n");
				exit(1);
			}
			if (isupper(wcopy[0]))
				wcopy[1] = wcopy[1] | UPPER_CHAR;
			dbm_content.dptr = wcopy + 1;
			len--;
		}
		dbm_content.dsize = len;					/* null not stored */
		if (delete < 0) {
			if (STORE(dbm_key, dbm_content) == -1) {
				fprintf(stderr, "mksp: Can't store\n");
				exit(1);
			}
		}
		else {
			if (vflag)
				fprintf(stderr, "reusing: %s\n", word);
			mk_key(key, s, delete);
			if (REPLACE(dbm_key, dbm_content) == -1) {
				fprintf(stderr, "mksp: Can't replace\n");
				exit(1);
			}
		}
		count++;
		if (vflag > 1)
			fprintf(stderr, "%5d: %s(%d)\n", count, word, ex_count(key));
		if (vflag && (count % VFREQ) == 0)
			fprintf(stderr, "%5d: %s\n", count, word);
	}
	if (vflag)
		fprintf(stderr, "%d words\n", count);
	DBMCLOSE();
}

/*
 * Print out everything
 */
prcontents(name)
char *name;
{
	register int s;
	datum dbm_key, dbm_content;

	if (DBMINIT(name, O_RDONLY) == -1)
		exit(1);

	dbm_key = FIRSTKEY();
	while (dbm_key.dptr != NULL) {
		dbm_content = FETCH(dbm_key);
		if (dbm_content.dptr == 0)
			break;						/* ??? */

		if (vflag)
			printf("%3d. ", ex_count((key_t *) dbm_key.dptr));
		if (IS_DELETED(dbm_content)) {
			if (vflag)
				printf("(deleted)\n");
		}
		else {
			s = ex_soundex((key_t *) dbm_key.dptr);
			printf("%s\n", mk_word(dbm_content.dptr, dbm_content.dsize, s));
		}
		dbm_key = NEXTKEY(dbm_key);
	}
	DBMCLOSE();
}

/*
 * When words are deleted they must be marked as such rather than deleted
 * using DELETE().  This is because the sequence of counters must remain
 * continuous.  If DELETE() is used then any entries with the same soundex
 * but with a larger counter value would not be accessible.  This approach
 * does cost some extra space but if an addition is made to the chain then
 * a deleted counter slot will be reused.  Also, the storage used by the word
 * should be made available to dbm.  This could be improved somewhat
 * by actually using DELETE() on the last entry of the chain.
 */
deletewords(name)
char *name;
{
	register int c, ch, len, s;
	register char *p;
	char word[MAXWORDLEN + 2];
	datum dbm_key, dbm_content;

	if (DBMINIT(name, O_RDWR) == -1)
		exit(1);

	while (fgets(word, sizeof(word), stdin) != (char *) NULL) {
		len = strlen(word);
		if (word[len - 1] != '\n') {
			fprintf(stderr, "mksp: Word too long: %s", word);
			while ((ch = getchar()) != '\n')	/* flush rest of line */
				putc(ch, stderr);
			putc('\n', stderr);
			continue;
		}
		word[--len] = '\0';
		if (len > MAXWORDLEN) {
			fprintf(stderr, "mksp: Word too long: %s\n", word);
			continue;
		}

		if ((s = soundex(word, 3)) == BAD_WORD) {
			if (vflag)
				fprintf(stderr, "Bad word: %s\n", word);
			continue;
		}

		c = 0;
		while (1) {
			dbm_key.dptr = (char *) key;
			dbm_key.dsize = KEYSIZE;
			mk_key(key, s, c);
			dbm_content = FETCH(dbm_key);
			if (dbm_content.dptr == NULL) {
				if (vflag)
					fprintf(stderr, "Not found: %s\n", word);
				break;
			}

			if (!IS_DELETED(dbm_content)) {
				p = mk_word(dbm_content.dptr, dbm_content.dsize, s);
				if (strmatch(word, p) == 0) {
					/*
					 * Aside:
					 * Since dptr points to static storage it must be reset
					 * if we want to retain the old content (content.dptr=word)
					 * This took a while to determine...
					 * Anyhow, since there is no need to store the old word
					 * we free up the space
					 */
					dbm_content.dptr = "";
					dbm_content.dsize = 0;
					if (REPLACE(dbm_key, dbm_content) == -1)
						fprintf(stderr, "mksp: delete of '%s' failed\n", word);
					else if (vflag) {
						if (vflag > 1)
							fprintf(stderr, "%d. %s ", c, p);
						fprintf(stderr, "deleted\n");
					}
					break;
				}
				else if (vflag > 1)
					fprintf(stderr, "%d. %s\n", c, p);
			}
			else if (vflag > 1)
				fprintf(stderr, "%d. (deleted)\n", c);

			if (++c > MAXCOUNT) {
				ch = isupper(word[0]) ? tolower(word[0]) : word[0];
				fprintf(stderr, "mksp: Counter overflow\n");
				fprintf(stderr, "soundex: %c%d\n", ch, b10(digit_part));
				exit(1);
			}
		}
	}
	DBMCLOSE();
}

/*
 * Setup the dictionary files if necessary
 */
setup(name)
char *name;
{
	register int s1, s2;

	s1 = check_dict(name, ".dir");
	s2 = check_dict(name, ".pag");
	if (s1 == NEW_DICT && s2 == NEW_DICT)
		return(NEW_DICT);
	return(OLD_DICT);
}

/*
 * Check if a dictionary file exists:
 *	- if not, try to create it
 *	- if so, see if it is empty
 * Return NEW_DICT if an empty file exists,
 * OLD_DICT if a non-empty file exists
 * Default mode for new files is 0666
 */
check_dict(name, ext)
char *name, *ext;
{
	register int len, s;
	char *filename;
	struct stat statbuf;
	char *malloc();
	extern int errno;

	len = strlen(name) + strlen(ext) + 1;
	filename = (char *) malloc((unsigned) len);
	if (filename == (char *) NULL) {
		fprintf(stderr, "mksp: Can't malloc '%s.%s'\n", name, ext);
		exit(1);
	}
	sprintf(filename, "%s%s", name, ext);
	if (stat(filename, &statbuf) == -1) {
		if (errno != ENOENT) {
			perror("mksp");
			exit(1);
		}
		if (creat(filename, 0666) == -1) {
			perror("mksp");
			exit(1);
		}
		s = NEW_DICT;
	}
	else {
		if (statbuf.st_size == 0)
			s = NEW_DICT;
		else
			s = OLD_DICT;
	}
	return(s);
}

/*
 * Compute an 'n' digit Soundex code for 'word' 
 * As a side effect, leave the digit part of the soundex in digit_part
 *
 * Since the soundex can be considered a base 7 number, if 'n' is:
 *	3	require  9 (10 if base 10) bits for digits
 *	4	require 12 (13) bits
 *	5	require 15 (17) bits
 *	6	require 17 (20) bits
 *
 * The three slightly different versions of this routine should be coalesced.
 */
soundex(word, n)
register char *word;
int n;
{
	register int c, soundex_length, previous_code;
	register char *p, *w;
	char wcopy[MAXWORDLEN + 2];

	if (!IS_VALID(word))
		return(-1);

	strcpy(wcopy, word);
	p = w = wcopy;

	while (*p != '\0') {
		if (isupper(*p))
			*p = tolower(*p);
		p++;
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

b10(n)
int n;
{
	register int b10, s;

	for (b10 = 0, s = 1; n != 0; n /= 7) {
		b10 += (n % 7) * s;
		s *= 10;
	}
	return(b10);
}

