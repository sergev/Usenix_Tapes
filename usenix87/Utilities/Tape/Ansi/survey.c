#define	MSGUSE	"Usage is: survey [-cfhlmpx]\n"
#include	"tp.h"
#include	<ctype.h>
#define	WIDTH	64

/*
 * Name: survey, survey contents of magtape
 * Author: Dick Grune
 * Version: 820314
 */

int unit = 0;
char *nmdns = TP_DENN;
char *rx = "r";
TPFILE *tf;
extern FILE *tperr;
char buff[TP_MAXB];
int size;

char pflag = 0;

main(argc, argv) char *argv[];	{

	argc--; argv++;
	if (argc > 0)	{
		if (argv[0][0] == '-')	{
			char *pp = argv[0];

			while (*++pp)
			switch (*pp)	{
			case 'c':
				unit = TP_CDEV;
				if (argc < 2)
					goto Lbad;
				nmdns = argv[1];
				argc--; argv++;
				break;
			case 'f':
				unit = TP_IMAG;
				if (argc < 2)
					goto Lbad;
				nmdns = argv[1];
				argc--; argv++;
				break;
			case 'h':
				nmdns = TP_DENH;
				break;
			case 'l':
				nmdns = TP_DENL;
				break;
			case 'm':
				unit = *++pp - '0';
				break;
			case 'p':
				pflag = 1;
				break;
			case 'x':
				rx = "rx";
				break;
			default:
				goto Lbad;
			}
			argc--; argv++;
		}
	}

	if (argc != 0)
		goto Lbad;

	tf = tpopen(unit, nmdns, rx);

	while ((size = tpread(tf, buff, TP_MAXB)) != EOF)	{
		printf("%6d", size);
		if (pflag)
			expose();
		printf("\n");
	}
	exit(0);

Lbad:
	fprintf(stderr, MSGUSE);
	exit(1);
}

int
hex(c)	char c;	{
	return "0123456789ABCDEF"[c&017];
}

expose()	{
	if (size == 0)
		printf("\t* * * TAPE MARK * * *\n");
	else	{
		int i;
		printf("\t");
		for (i = 0; i < WIDTH && i < size; i++)	{
			char c = buff[i];
			printf("%c", isascii(c) &&
				(isprint(c) || c == ' ') ? c : '?');
		}
		printf("\n  EBC:\t");
		for (i = 0; i < WIDTH && i < size; i++)	{
			char c = ebc2asc(buff[i]);
			printf("%c", isascii(c) &&
				(isprint(c) || c == ' ') ? c : '?');
		}
		printf("\n  HEX:\t");
		for (i = 0; i < WIDTH/2 && i < size; i++)	{
			char c = buff[i];
			printf("%c%c", hex(c>>4), hex(c));
		}
		printf("\n");
	}
}
