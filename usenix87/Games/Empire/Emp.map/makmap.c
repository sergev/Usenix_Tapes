
#include <stdio.h>

#define ROWS 60
#define COLS 100
#define MAXCIT 65

char map[ROWS][COLS];

main()
{
	unsigned int c, bit;
	int citcnt;
	int ch;
	register int i, j, cnt;

	/* Read the map */
	citcnt = 0;
	for (i = 0; i < COLS; i++) {
		for (j = 0; j < ROWS; j++) {
			ch = getchar();
			switch(ch) {
				case '+':
				case '.':
					map[j][i] = ch;
					break;
				case '*':
					citcnt++;
					map[j][i] = ch;
					break;
				case '\n':
					fprintf(stderr,
				"Line too short; should be %d characters\n", ROWS);
					exit(1);

				case EOF:
					fprintf(stderr, "File not large enough\n");
					exit(1);
				default:
					fprintf(stderr,
				"Bad character '%c' (0x%02x)\n", ch, ch);
					exit(1);
			}
		}
		if ((ch = getchar()) != '\n') {
			fprintf(stderr, "Line too long; should be %d characters\n", ROWS);
			exit(1);
		}
	}
	if ((ch = getchar()) != EOF) {
		fprintf(stderr, "File too large; should have %d lines\n", COLS);
		exit(1);
	}
	if (citcnt > MAXCIT) {
		fprintf(stderr, "too many cities!  You had %d, max is %d\n", citcnt, MAXCIT);
		exit(1);
	}

	/* convert the map */
	cnt = -1;
	c = map[0][0];
	for (i = 0; i < ROWS; i++)
		for (j = 0; j < COLS; j++) {
			/* New map character? */
			if (c != map[i][j] || cnt == 63) {	/* yes */
				bit = (cnt << 2) | MCODE(c);
				putchar(bit);
				c = map[i][j];
				cnt = 0;
			} else
				cnt++;
		}
	putchar('\0');
	putchar('\0');
}

#define	LAND	3
#define	WATER	2
#define	CITY	1

/*
 * code '+', '.', '*' as 3, 2, or 1
 */
MCODE(c)
unsigned int c;
{
	switch(c) {
		case '+':
			return(LAND);
		case '.':
			return(WATER);
		case '*':
			return(CITY);
	}
	fprintf(stderr, "internal error\n");
	exit(1);
}
