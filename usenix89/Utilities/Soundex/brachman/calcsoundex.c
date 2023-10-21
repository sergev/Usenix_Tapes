/* vi: set tabstop=4 : */

/*
 * calcsoundex - calculate soundex codes
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

#include <stdio.h>
#include <ctype.h>

#include "sp.h"

char word[MAXWORDLEN + 2];

char soundex_code_map[26] = {
/***	 A  B  C  D  E  F  G  H  I  J  K  L  M  N  O  P	***/ 
		 0, 1, 2, 3, 0, 1, 2, 0, 0, 2, 2, 4, 5, 5, 0, 1,

/***	 Q  R  S  T  U  V  W  X  Y  Z			***/
		 2, 6, 2, 3, 0, 1, 0, 2, 0, 2
};

main(argc, argv)
int argc;
char **argv;
{
	register int c, i, soundex_length, digit_part, previous_code;
	int ch, len, vflag;
	short soundex;
	char *gets();

	vflag = 0;
	if (argc > 2 || (argc == 2 && strcmp(argv[1], "-v"))) {
		fprintf(stderr, "Usage: calcsoundex [-v]\n");
		exit(1);
	}
	if (argc > 1)
		vflag = 1;

	while (fgets(word, sizeof(word), stdin) != (char *) NULL) {
		len = strlen(word);
		if (word[len - 1] != '\n') {
			fprintf(stderr, "calcsoundex: Word too long: %s", word);
			while ((ch = getchar()) != '\n')	/* flush rest of line */
				putc(ch, stderr);
			putc('\n', stderr);
			continue;
		}
		word[--len] = '\0';
		if (len > MAXWORDLEN) {
			fprintf(stderr, "calcsoundex: Word too long: %s\n", word);
			continue;
		}

		for (i = 0; word[i] != '\0'; i++) {
			if (isupper(word[i]))
				word[i] = tolower(word[i]);
		}
		if (!isalpha(word[0]))
			continue;

		digit_part = 0;
		soundex_length = 0;
		previous_code = soundex_code_map[word[0] - 'a'];
		for (i = 1; word[i] != '\0' && soundex_length < 3; i++) {
			if (!isalpha(word[i]))
				continue;
			c = soundex_code_map[word[i] - 'a'];
			if (c == 0 || previous_code == c) {
				previous_code = c;
				continue;
			}
			digit_part = digit_part * 10 + c;
			previous_code = c;
			soundex_length++;
		}
		while (soundex_length++ < 3)
			digit_part *= 10;
		soundex = digit_part << 5 + word[0] - 'a';
		printf("%c", word[0]);
		if (digit_part < 100)
			putchar('0');
		if (digit_part < 10)
			putchar('0');
		if (digit_part == 0)
			putchar('0');
		else
			printf("%d", digit_part);
		if (vflag)
			printf(" %s", word);
		putchar('\n');
	}
	putchar('\n');
	exit(0);
}

