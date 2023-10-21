#define	MSGUSE	"Usage is: cptp [-cfhlmx] [of=file | if=file]"
#include	<stdio.h>
#include	"tp.h"

/*
 * Name: cptp, copy tape
 * Author: Dick Grune
 * Version: 820314
 *
 * `Cptp' converts between real tapes and tape images on disk.
 */

int unit = 0;
char *nmdns = TP_DENN;
char *rx = "r";
TPFILE *from, *to;
extern FILE *tperr;
char *filename;
int size;
char buff[TP_MAXB];

main (argc, argv) char *argv[];	{
	char *arg;

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
			case 'x':
				rx = "rx";
				break;
			default:
				goto Lbad;
			}
			argc--; argv++;
		}
	}
	if (argc != 1)
		goto Lbad;
	arg = argv[0];
	if (arg[0] == '\0' || arg[1] != 'f' || arg[2] != '=')
		goto Lbad;
	filename = &arg[3];
	tperr = stdout;
	switch (arg[0])	{
	case 'o':
		if (open(filename, 0) > 0)
			error("Output file already exists");
		from = tpopen(unit, nmdns, rx);
		to = tpopen(TP_IMAG, filename, "w");
		break;
	case 'i':
		from = tpopen(TP_IMAG, filename, "r");
		to = tpopen(unit, nmdns, "w");
		break;
	default:
		goto Lbad;
	}

	while ((size = tpread(from, buff, TP_MAXB)) != EOF)	{
		if (size == TP_MAXB)
			printf("Block too long; information may be lost\n");
		tpwrite(to, buff, size);
	}
	tpclose(from);
	tpclose(to);
	exit(0);

Lbad:
	error(MSGUSE);
}

error(str)	char *str;	{

	fprintf(stderr, "%s\n", str);
	exit(1);
}
