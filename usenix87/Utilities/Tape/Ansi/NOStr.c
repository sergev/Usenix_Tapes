#define	MSGUSE	"Usage is: NOStr -C [file]"
#include	<stdio.h>
#include	"tp.h"

/*
 * Program: NOStr, translates a Cyber 6-bit file into a UNIX ASCII file.
 * Author: Dick Grune
 * Version: 820314
 */

#define	MAXERR	40
#define	EOS	'\0'

FILE	*ifile;		/* input file */
char	*iname;		/* input name */

int displZ(), asciiZ(), binary();
struct conv	{
	char *name;
	int (*command)();
	char *expl;
} conv[] =	{
	{"d", displZ, "Z-type records in DISPLAY code"},
	{"a", asciiZ, "Z-type records in ASCII95"},
	{"b", binary, "binary, = ASCII256"}
};

main(argc, argv) char *argv[];	{
	int p;

	if (argc < 2 || argc > 3 || argv[1][0] != '-' || strlen(argv[1]) != 2)
		goto Bad_usage;
	if (argc == 2 || strcmp(argv[2], "-") == 0)	{
		iname = "standard input";
		ifile = stdin;
	}
	else	{
		iname = argv[2];
		ifile = fopen(iname, "r");
		if (ifile == NULL)	{
			fprintf(stderr, "%s: cannot open %s\n",
					argv[0], iname);
			return 1;
		}
	}
	for (p = 0; p < n_items(conv); p++)
		if (strcmp(&argv[1][1], conv[p].name) == 0)	{
			(*conv[p].command)();
			return 0;
		}
Bad_usage:
	fprintf(stderr, MSGUSE);
	fprintf(stderr,
		"\nwhere the conversion code C is one of the following:\n");
	for (p = 0; p < n_items(conv); p++)
		fprintf(stderr, "`%s': %s\n", conv[p].name, conv[p].expl);
	return 1;
}

#define	NL	'\n'
#define	TEN	10

int cwrd[TEN];
long lcnt = 1;
long icnt = 0;

displZ()	{	/* recognize Z-type records in DISPLAY code */
	int nch;
	int zpend = 0;

	while (nch = get_cwrd(TEN), nch != 0)	{
		int i;

		if (nch != TEN)
			VOID(complain("short word", 0));
		while (nch != 0 && cwrd[nch-1] == 0)
			nch--;
		if (zpend && nch > 0)
			putdispl(0); /* zero pending */
		for (i = 0; i < nch; i++)
			putdispl(cwrd[i]);
		if (nch < TEN-1)
			putascii(NL);
		zpend = nch == TEN-1;
	}
}

#define	FIVE	5

asciiZ()	{	/* recognize Z-type records in ASCII95 */
	int nch;

	while (nch = get_cwrd(FIVE), nch != 0)	{
		int i;

		if (nch != FIVE)
			VOID(complain("short word", 0));
		while (nch != 0 && cwrd[nch-1] == 0)
			nch--;
		for (i = 0; i < nch; i++)
			putascii(cwrd[i]);
		if (nch < FIVE)
			putascii(NL);
	}
}

binary()	{	/* binary, also ASCII256 */
	int ch;

	while (ch = get12bits(), ch != EOF)
		putascii(ch);
}

int
get_cwrd(n)	{	/* gather n (60/n)-bit bytes into a Cyber word */
	int i;

	for (i = 0; i < n; i++)	{
		cwrd[i] = n == TEN ? get6bits() :
				n == FIVE ? get12bits() : abort();
		if (cwrd[i] == EOF)
			break;
	}
	return i;
}

int
get12bits()	{
	int ch1, ch2;
	static errcnt = 0;

	ch1 = get6bits();
	if (ch1 == EOF)
		return EOF;
	ch2 = get6bits();
	if (ch2 == EOF)
		ch2 = 0;

	ch1 = (ch1 << 6) + ch2;
	if (ch1 >> 8)
		errcnt = complain("composed char wider than 8 bits", errcnt);
	return ch1;
}

int
get6bits()	{
	int ch = getc(ifile);
	static errcnt = 0;

	if (ch == EOF)
		return EOF;
	icnt++;
	if (ch >> 6)
		errcnt = complain("input char wider than 6 bits", errcnt);
	return ch;
}

putdispl(ch)	{	/* convert display to ASCII */
	putchar(
	"%ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789+-*/()$= ,.#[]:\"_!&'?<>@\\^;"
		[ch]);
}

putascii(ch) char ch;	{
	putchar(ch);
	if (ch == NL)
		lcnt++;
}

int
complain(s, n) char *s;	{
	if (n > MAXERR)
		return n;
	fprintf(stderr,
		"At input char %D, at output line %D, %s in file `%s'\n",
				icnt, lcnt, s, iname);
	if (n == MAXERR)
		fprintf(stderr,
	"After %d complaints, further complaints of this type suppressed\n",
			MAXERR);
	return n+1;
}
