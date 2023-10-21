#define	MSGUSE	"Usage is: rawtp [-cfhlmx] XX [ param ... ]"
#include	<stdio.h>
#include	"tp.h"
/*
 * Name: rawtp, read raw tape
 * Author: Dick Grune
 * Version: 820314
 *
   Selected portions are read from tape and written to files.

*/

#define	TRUE	1
#define	FALSE	0
#define	EOS	'\0'

#define	EOB	0	/* End Of Block */
#define	EOX	-1	/* End Of File (to avoid confusion with EOF) */
#define	EOT	-2	/* End Of Tape */
#define	AT_EOB	(ilength <= EOB)
#define	AT_EOX	(ilength <= EOX)
#define	AT_EOT	(ilength <= EOT)

char name [128];
char *eoname = &name[0];
FILE *ofile = NULL;

TPFILE *tape;
extern FILE *tperr;
int unit = 0;
char *nmdns = TP_DENN;
char *rx = "r";
char buff[TP_MAXB];

char *strins();

main(argc, argv) char **argv;	{
	extern int ilength;

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
	if (argc < 1)
		goto Lbad;
	if (**argv == '+' || **argv == '-')
		goto Lbad;
	set_name(argv);
	argc--; argv++;

	ilength = EOT;	/* fake empty tape to test parameters */
	params(argc, argv);

	tape = tpopen(unit, nmdns, rx);
	tperr = stdout;
	ilength = EOB;	/* and now for keeps */
	skipIRG();
	params(argc, argv);
	tpclose(tape);
	exit(0);

Lbad:
	error(MSGUSE, "");
}

set_name(argv) char **argv;	{
	register char *pt;

	eoname = strins(eoname, *argv);
	eoname = strins(eoname, "01aaa01");
	*eoname = EOS;
	for (pt = eoname; pt > name; pt--)
		if (pt[-1] == '/') break;
	if (eoname - pt > 14)
		error("%s: file name too long", name);
}

params(argc, argv) char **argv;	{

	VOID(strins(eoname-7, "01"));
	if (!argc)
		param("+1x");
	else
		while (argc--)	{
			param(*argv++);
			incr(eoname-6);
		}
}

char *ppar;	/* parameter being processed */

param(arg) char *arg;	{
	register int repl;

	ppar = arg;
	repl = getxrepl(ppar);
	if (repl == 0) repl--;
	VOID(strins(eoname-5, "aaa"));
	while (repl-- && instr())
		incr(eoname-3);
}

int moved;

int
instr()	{
	char *p = ppar;

	moved = FALSE;
	VOID(strins(eoname-2, "01"));
	while (simp_instr(&p))
		incr(eoname-1);
	return moved;
}

int copy = FALSE;

int
simp_instr(pp) char **pp;	{
	register int cnt;

	switch (**pp)	{
	case EOS:
	case 'x':
		return FALSE;
	case '+':
		copy = TRUE;
		break;
	case '-':
		copy = FALSE;
		break;
	default:
		error("%s: bad parameter", ppar);
	}
	(*pp)++;
	cnt = getint(pp);
	while (cnt-- && copyfile()) {}
	if (**pp == '.')
		(*pp)++;
	cnt = getint(pp);
	while (cnt-- && copyblock()) {}
	if (**pp == '.')
		(*pp)++;
	cnt = getint(pp);
	while (cnt-- && copychar()) {}
	if (copy)
		dropfile();
	return TRUE;
}

int ilength;

/* ilength contains the number of characters the tape is ahead of the user;
 * or it is EOX or EOT
 */
char *iptr;

int
copyfile()	{

	if (AT_EOT)
		return FALSE;
	while (copyblock())	{}
	skipTM();
	return TRUE;
}

int
copyblock()	{

	if (AT_EOX)
		return FALSE;
	if (!copy)
		ilength = EOB;
	else
		while (copychar())	{}
	skipIRG();
	return TRUE;
}

int
copychar()	{

	if (AT_EOB)
		return FALSE;
	outchar(*iptr);
	iptr++;
	ilength--;
	return TRUE;
}

outchar(c)	{

	if (!copy)
		return;
	if (ofile == NULL)	{
		getfile();
		moved = TRUE;
	}
	putc(c, ofile);
}

/* physical tape movers */

skipTM()	{

	if (AT_EOT)
		return;
	ilength = EOB;
	skipIRG();
}

skipIRG()	{
	int size;

	if (AT_EOX)
		return;
	size = tpread(tape, buff, TP_MAXB);
	ilength = size == EOF ? EOT : size == 0 ? EOX : size;
	iptr = buff;
	if (!AT_EOT)
		moved = TRUE;
}

/* output file registration */

getfile()	{

	if ((ofile = fopen(name, "w")) == NULL)
		error("%s: cannot create", name);
}

dropfile()	{

	if (ofile != NULL)
		VOID(fclose(ofile));
	ofile = NULL;
}

/* service routines */

char *
strins(s1, s2) char *s1, *s2;	{

	while (*s2 != EOS)
		*s1++ = *s2++;
	return s1;
}

int
getint(pp) char **pp;	{
	register int val, res = 0;

	for (;;)	{
		val = **pp - '0';
		if (val < 0 || val > 9)
			return res;
		(*pp)++;
		res = res*10 + val;
	}
}

incr(p) char *p;	{

	(*p)++;
	if (*p == '9' + 1)	{
		*p = '0'; incr(p-1);
	}
	else
	if (*p == 'z' + 1)	{
		*p = 'a'; incr(p-1);
	}
}

int
getxrepl(p) char *p;	{
	register int r;

	while (*p != 'x')
		if (!*p++)
			return 1;
	p++;
	r = getint(&p);
	if (*p)
		error("%s: bad replicator", p);
	return r;
}

error(p1, p2) char *p1, *p2;	{

	printf(p1, p2);
	printf("\n");
	exit(1);
}
