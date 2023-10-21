#include	<stdio.h>
#include	"tp.h"

extern char *strcpy();
extern char *sprintf();
extern char *index();

#define	NL	'\n'
#define	TAB	'\t'
#define	SP	' '
#define	EOS	'\0'
#define	Ascii95(c)	(040 <= (c) && (c) <= 0176)

#define	TRUE	1
#define	FALSE	0

#define	MAXSTR	128

/*
 * It is tempting to use structs for the declaration of the label data
 * structures, access the fields through selectors and let the C-compiler
 * do the offset calculation. There are, however, two problems:
 *    1. alignment is different on different machines (VAX vs. PDP11),
 *    2. according to the book the fields may even be arranged in inverse
 *       order.
 * So structs are out and defines are in.
 * A field is implemented as a pair: an address and a length.
 */

/* the general fields */
#define	Whole(p)	&(p)[0], sizeof (p)
#define	Labidf(p)	&(p)[0], 4

/* the VOL1 label */
#define	Volidf(p)	&(p)[4], 6
#define	Volacc(p)	&(p)[10], 1
#define	Sp1(p)		&(p)[11], 26
#define	Ownidf(p)	&(p)[37], 14
#define	Sp2(p)		&(p)[51], 28
#define	Labvers(p)	&(p)[79], 1

/* the date */
#define	Sp(p)		&(p)[0], 1
#define	Year(p)		&(p)[1], 2
#define	Yday(p)		&(p)[3], 3

/* the HDR1 label */
#define	Fileidf(p)	&(p)[4], 17
#define	Filesetidf(p)	&(p)[21], 6
#define	Filsecnum(p)	&(p)[27], 4
#define	Filseqnum(p)	&(p)[31], 4
#define	Gennum(p)	&(p)[35], 4
#define	Genversnum(p)	&(p)[39], 2
#define	Creatdate(p)	&(p)[41], 6
#define	Expirdate(p)	&(p)[47], 6
#define	Fileacc(p)	&(p)[53], 1
#define	Blkcount(p)	&(p)[54], 6
#define	Syscode(p)	&(p)[60], 13
#define	Sp3(p)		&(p)[73], 7

/* the HDR2 label */
#define	Recformat(p)	&(p)[4], 1
#define	Blklength(p)	&(p)[5], 5
#define	Reclength(p)	&(p)[10], 5
#define	Syssoftw(p)	&(p)[15], 35
#define	Bufoffset(p)	&(p)[50], 2
#define	Sp4(p)		&(p)[52], 28

/* the USRn label */
#define	Contents(p)	&(p)[4], 76


int unit = 0;
char *nmdns = TP_DENN;
TPFILE *tf = NULL;

#define	ASK_NO	0	/* use default and check */
#define	ASK_YES	1	/* ask and check */
#define	ASK_SUG	2	/* suggest default and check */
#define	ASK_ERR	3	/* ask again and check */

#define	inmood(md)	{int mood = md; for (;;mood=ASK_ERR){
#define	iferr(cond)	if(cond){locate();
#define	enderr		continue;}
#define	endmood		break;}}

char *
sps2txt(p)	{
	return p == 0 ? "<empty>" : p == 1 ? "<space>" : "<spaces>";
}

char *
str2txt(s) char *s;	{
	int p = 0;

	while (s[p] != EOS)	{
		if (s[p] != SP)
			return s;
		p++;
	}
	return sps2txt(p);
}

/* An expl(anation) is an explanatory string which contains the name of the
 * object being explained as the first item between ` and ' . The expl may
 * contain one %s which is replaced by the default (string) value.
 */

char * /* transient */
expl2str(expl) char *expl;	{
	static char str[MAXSTR];
	char *nm, cnt = 0;

	for (nm = index(expl, '`') + 1; *nm != EOS && *nm != '\''; nm++)
		if (cnt < MAXSTR-1)
			str[cnt++] = *nm;
	str[cnt] = EOS;
	return str;
}

char * /* transient */
tty_line()	{
	static char ans[MAXSTR];
	int cnt = 0; int ch;

	while ((ch = getchar()) != NL)	{
		if (ch == EOF)
			errors("\n*** No interaction!!!");
		if (cnt < MAXSTR-1)
			ans[cnt++] = ch;
	}
	ans[cnt] = EOS;
	return ans;
}

char * /* transient */
tty_str(prompt, expl, md, def) char *prompt, *expl, md, *def;	{
	static char conversation = FALSE;
	char *str;
	int err = 0;

	if (!conversation)	{
		prf_s("\n\
	I shall have to ask some questions; to get more information,\n\
	answer a question with a single question mark (?).\n\n", "");
		conversation = TRUE;
	}
	while (err < 3)	{
		printf(prompt, expl2str(expl));
		str= tty_line();
		if (strcmp(str, "?") == 0)	{
			printf("\n");
			prf_s(expl, def);
			printf("\n");
		}
		else
		if (strlen(str) > 0)
			return str;
		else
		if (md == ASK_SUG)
			return NULL;
		else	{
			printf("I do need an answer!\n");
			err++;
		}
	}
	errors("\nSorry");
	return str;
}

char * /* transient */
enq_str(expl, md, def) char *expl, *def;	{
	char *str;

	switch (md)	{
	case ASK_NO:
		str = def;
		break;
	case ASK_YES:
		locate();
		str = tty_str("Please type your %s: ", expl, md, def);
		break;
	case ASK_SUG:
		locate();
		printf("Default %s: ", expl2str(expl));
		prf_s("%s\n", str2txt(def));
		str = tty_str("Please type a new %s, or press RETURN: ",
			expl, md, def);
		if (str == NULL)
			str = def;
		break;
	case ASK_ERR:
		str = tty_str("Please type a new %s: ", expl, md, def);
		break;
	}

	return str;
}

int
enq_int(expl, md, def, max) char *expl;	{
	int res;
	char deftxt[MAXSTR];

	VOID(sprintf(deftxt, "%d", def));
	inmood (md)
		char *str = enq_str(expl, mood, deftxt);
		char *ptr = str;
		char ch;

		res = 0;
		while ((ch = *ptr++) != EOS)	{
			int dig = char2int(ch) -'0';

			if (dig < 0 || dig > 9)	{
				res = -1;
				break;
			}
			if (res > max/10 || res*10 > max - dig)	{
			/* airtight, waterproof and overflow-resistant */
				res = -2;
				break;
			}
			res = res*10 + dig;
		}
		iferr (res == -1)
			printf("The %s `%s' is not numeric\n", expl2str(expl), str);
		enderr;
		iferr (res == -2)
			printf("The %s `%s' is too large\n", expl2str(expl), str);
			printf("The maximum value is %d\n", max);
		enderr;
	endmood;
	return res;
}

int
isascstr(str) char *str;	{
	char ch;

	while ((ch = *str++) != EOS)
		if (!Ascii95(ch))
			return FALSE;
	return TRUE;
}

int
fld2int(addr, size) char *addr;	{
	int res = 0, i;

	for (i = 0; i < size; i++)	{
		char ch = addr[i];
		int dig = char2int(ch) - '0';
		if (ch == SP)
			continue;
		if (dig < 0 || dig > 9)
			return -1;
		res = res * 10 + dig;
	}
	return res;
}

char * /* transient */
fld2str(addr, size) char *addr;	{
	static char str[MAXSTR];
	int i;

	for (i = 0; i < size; i++)
		if (i < MAXSTR-1)
			str[i] = addr[i];
	while (i > 0 && str[i-1] == SP)
		i--;
	str[i] = EOS;
	return str;
}

fld2fld(str, s1, addr, s2) char *str, *addr;	{

	if (s1 != s2)
		abort();
	while (s1-- > 0)
		*addr++ = *str++;
}

str2fld(str, addr, size) char *str, *addr;	{

	while (size-- > 0)
		*addr++ = *str != EOS ? *str++ : SP;
}

char * /* transient */
char2str(ch)	{
	static char buff[7];

	if (Ascii95(ch))
		VOID(sprintf(buff, "%c", ch));
	else
		VOID(sprintf(buff, "\\[%03o]", char2int(ch)));
	return buff;
}

/*
 * `prf_s' prints a possibly ugly string `s' under a format `f' that may
 * be so long that it normally blows up the `printf' routine.
 */
prf_s(f, s) char *f, *s;	{
	char fch;

	while ((fch = *f++) != EOS)	{
		if (fch != '%')
			putchar(fch);
		else	{
			int sch;

			f++;
			while ((sch = *s++) != EOS)
				printf("%s", char2str(sch));
		}
	}
}

errors(str) char *str;	{
	printf("%s\n", str);
	if (tf != NULL)
		tpclose(tf);
	exit(1);
}

struct format	{
	char type;
	int (*checkpar)();
	int (*cpblk)();
};
#define	FORMAT	struct format

extern FORMAT formats[];

FORMAT *
format(ch) char ch;	{
	FORMAT *fm;

	for (fm = &formats[0]; fm->type != EOS; fm++)
		if (fm->type == ch)
			return fm;
	return NULL;
}

char filename[MAXSTR];
FILE *file = NULL;
int filseqnum = 0;
int filsecnum = 1;
char rectype[1] = 'F';
FORMAT *recformat;
int blklength = 1920;
int reclength = 80;
int bufoffset = 0;
int reccount;
int blkcount;

char VOL1buf[80];
char HDR1buf[80];
char HDR2buf[80];
