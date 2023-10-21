#define	MSGUSE	"Usage is: NOSsplit [-cfhlm[s N]] [out_name]\n"
#include	<stdio.h>
#include	"tp.h"
extern char *sprintf();

/*
 * Name: NOSsplit, split Cyber system tape (data format is SI)
 * Author: Dick Grune
 * Version: 820314
 */

#define	BSIZE	3840	/* blocksize binary files */
#define	CSIZE	960	/* blocksize coded files */
#define	SIZE	3840	/* maximum blocksize */
#define	EORM	"\055\023\035\052\027\054\000"
#define	EORL	7
#define	EOS	'\0'

#define	lastblock(n)	((n)!=BSIZE && (n)!=CSIZE)
#define	cybln(n)	((n)*8/60*10)

int fnumber = 0;
char *ofil = "x";
char fname[128];
FILE *outf;

char buff[SIZE];
int size;	/* number of chars in `buff' */
int bpos;	/* number of bits consumed by `get6bits' */
int bstat;

int unit = 0;
int skipf = 0;	/* number of logical records to skip (forwards) */
char *nmdns = TP_DENN;
TPFILE *tf;
extern FILE *tperr;

char *progname;

main(argc, argv) char *argv[];	{

	progname = argv[0];
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
			case 's':
				if (argc < 2)
					goto Lbad;
				if (sscanf(argv[1], "%d", &skipf) != 1)
					goto Lbad;
				argc--; argv++;
				break;
			default:
				goto Lbad;
			}
			argc--; argv++;
		}
	}

	if (argc == 1)	{
		ofil = argv[0];
		argc--; argv++;
	}

	if (argc != 0)
		goto Lbad;

	read_tape();
	return 0;

Lbad:
	fprintf(stderr, MSGUSE);
	return 1;

}

read_tape()	{
	int i;
	int bcnt;

	tf = tpopen(unit, nmdns, "rx");
	tperr = stdout;

	fillbuff();
	if (size == 80 && strncmp(buff, "VOL1", 4) == 0)	{
		printf("This is a labelled tape\n");
		printf("For label information use `ansir -p'\n");
		while (size > 0)
			fillbuff();
		if (size == 0)
			fillbuff();
	}
	else	printf("This is a non-labelled tape\n");

	for (i = 0; i < skipf; i++)	{
		if (size <= 0)	{
			printf("%d record%s missing\n", english(skipf-i));
			exit(1);
		}
		fnumber++;
		while (!lastblock(size))
			fillbuff();
		fillbuff();
	}

	if (skipf > 0)
		printf("%d logical record%s skipped\n", english(skipf));

	while (size > 0)	{
		newcreat(fnumber++);

		bcnt = 1;
		while (putbuff(cybln(size)), !lastblock(size))	{
			fillbuff();
			bcnt++;
		}

		proc_eor(bcnt);

		VOID(fclose(outf));
		fillbuff();
	}

	printf("%d record%s retrieved\n", english(fnumber-skipf));
}

newcreat(fn)	{
	int i;

	for(i=0; ofil[i] != EOS; i++)
		fname[i] = ofil[i];
	VOID(sprintf(&fname[i], "%04d", fn));
	outf = fopen(fname, "w");
	if (outf == NULL)	{
		printf("%s: cannot create `%s'\n", progname, fname);
		exit(1);
	}
}

fillbuff()	{

	do	size = tpread(tf, buff, sizeof buff);
	while (size > 0 && size < 6 /* noise record */);
	bpos = bstat = 0;
}

putbuff(n) int n; /* number of 6bit chars */	{

	while (n-- > 0)
		putc(get6bits(), outf);
}


proc_eor(bcnt)	{
	char eor[EORL];
	int i;

	for (i = 0; i < EORL; i++)
		eor[i] = get6bits();

	printf("%s: ", fname);
	printf("%d block%s, ", english(bcnt));
	if (strncmp(eor, EORM, EORL) != 0)
		printf("no proper EOR\n");
	else
		printf("EOR%2o\n", get6bits());
}

#define	left(c,n)	(((c)&((077<<(8-(n)))&0377))>>(8-(n)))
#define	right(c,n)	(((c)&((077>>(6-(n)))&0377))<<(6-(n)))
/*
 * These forms are constructed through program transformations; the
 * author cannot, by any stretch of imagination, guess why they work.
 */

int
get6bits()	{
	int res = 0;

	switch (bstat++)	{
	case 0:	res = left(buff[bpos+0], 6);
		break;
	case 1:	res = right(buff[bpos+0], 2) + left(buff[bpos+1], 4);
		break;
	case 2:	res = right(buff[bpos+1], 4) + left(buff[bpos+2], 2);
		break;
	case 3:	res = right(buff[bpos+2], 6);
		break;
	}
	if (bstat == 4)	{
		bpos += 3;
		bstat = 0;
	}
	return res;
}
