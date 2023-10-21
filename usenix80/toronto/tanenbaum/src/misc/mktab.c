#
/*
#define cEM1	1
	/* compile flag for use with 'cem' */
/*
 * This program converts the file 'tables' (standard input)
 * into 3 files:
 :	1.  an include file with definitions of the mnemonic constants
 *	2.  a C file with mnemonic info
 *	3.  a C file with optimising info.
 * Generation of a file can be suppressed by specifying
 * the appropriate option:
 *	-h  no define file
 *	-m  no mnem file
 *	-o  no opt file.
 */

struct mnems
{
	char	m_name[3];
	char	m_nminis;
	char	m_mbase;
	char	m_nshorties;
	char	m_sbase;
	char	m_obase;
	int	m_flags;
};

struct {
	int	intarr[];
};

/* flagbits of m_flags */
#define MN0		0
#define MNS		01
#define MNE		MNS
#define MNL		02
#define MNXYZ		014
#define		MNX	004
#define		MNY	010
#define		MNZ	014
#define MNABMC		0160
#define		MNA	0020
#define		MNB	0040
#define		MNM	0060
#define		MNC	0100
#define MNO		0200

struct pattern
{
	struct element
	{
		char	instr;
		char	operand;
	}
		sub[7];
	int	optkind;
};

struct const {
	char *name;
	int value;
} consttab[] {
	"fmnem",	0,
	"nmnem",	0,
	"fpseu",	0,
	"npseu",	0,
	"filb0",	0,
	"nilb0",	0,
	"fcst0",	0,
	"ncst0",	0,
	"ilb1",		0,
	"ilb2",		0,
	"dlb1",		0,
	"dlb2",		0,
	"dnam",		0,
	"pnam",		0,
	"scon",		0,
	"rcon",		0,
	"cst1",		0,
	"cstm",		0,
	"cst2",		0,
	"cend",		0,
	0,		0,
};
#define	FPSEU	2	/*index in consttab*/

struct {
	int word;
};

#define	EXCESS		33
#define CONLIM		(2*EXCESS)
#define ARANGE		17
#define ARITBASE	(CONLIM+4*ARANGE)

struct mnems mnemon[256], *lastmnem;
int first[26],count[26];
int firstopt[256],optcount[256];
int peekc;
int line 1;
char curinstr;
int optno;
int suppress;

int input[259],output[259];	/* IO buffers */

#ifdef cEM1
	extern int atoi(),getc(),dup(),fcreat();
	extern void printf(()),fflush(),close();
	int getlett(),getchar(),getbyte();
#endif
int h_flag 1;
int m_flag 1;
int o_flag 1;
char deffile[]	"em1.h";
char mnemfile[]	"mntab.c";
char optfile[]	"optab.c";

main (ac,av) char **av;
{
	while (--ac && **++av == '-')
		doflags(*av);
	if (!(h_flag || m_flag || o_flag)) return;
	firstpart();
	if (m_flag) writemnems();
	if (o_flag) dosecondpart();
}

doflags (arg) char *arg;
{	register char *ap;

	ap=arg;
	while (*++ap)
		switch (*ap) {
		  case 'h':	h_flag = 0; break;
		  case 'm':	m_flag = 0; break;
		  case 'o':	o_flag = 0; break;
		  default:	printf(" -%c?\n",*ap);
}		}

firstpart () {

	/*
	 * Create and write the file ops.h
	 */
	if (h_flag)
		if (fcreat(deffile,output)<0)
			error("can't create ops.h");
	read_const();
	read_pseudo();
	read_instr();
	if (h_flag) {
		fflush(output);
		close(output);
	}
}

read_const() {
	register struct const *cp;
	register char *p;
	register n;

	for (cp=consttab; cp->name; cp++) {
		p = cp->name;
		while (*p) {
			n = getchar();
			if (n != *p++)
				error("constant expected");
		}
		n = getbyte();
		cp->value = n;
		if (peekc != '\n')
			error("bad constant value");
		skipline();
		if (h_flag)
			printf("#define sp_%s %d\n",cp->name,n);
	}
	putchar('\n');
	if (getchar() != '\n')
		error("empty line expected");
	line++;
}

read_pseudo() {
	register c0,c1,c2;
	int n,pn;

	n = 0;
	for (;;) {
		if ((c0=getchar()) == '\n') {
			line++;
			if (h_flag)
				printf("#define sp_lpseu %d\n\n",
					--n + consttab[FPSEU].value);
			return;
		}
		checklett(c0);
		c1 = getlett();
		c2 = getlett();
		pn = getbyte();
		skipline();
		if (pn != n++)
			error("pseudo's not sorted on number");
		if (h_flag)
			printf("#define ps_%c%c%c %d\n",c0,c1,c2,
				pn + consttab[FPSEU].value);
	}
}

int inmnem(am) {
	register struct mnems *m;
	register c,flags;
	static curc;

	/*
	 * read one line of first part of tables.
	 * Syntax:
	 * mnm  flags  nmini  minibase  nshorties  shortiebase  otherbase
	 */
	m = am;
	if ((c=getchar()) == '\n') {
		line++;
		return(0);
	}
	checklett(c);
	if (c != curc) {
		if (c < curc)
			error("instructions not sorted");
		first[c - 'a'] = m-mnemon;
		curc = c;
	}
	++count[c - 'a'];
	m->m_name[0] = c;
	m->m_name[1] = getlett();
	m->m_name[2] = getlett();
	skiplayout();
	flags = 0;
	while ((c=getchar())!='\t' && c != ' ')
		switch(c) {
		default:
			error("bad flag");
		case '-':
			break;
		case 'a':
			null(flags&MNABMC);
			flags =| MNA;
			break;
		case 'e':
			null(flags&MNE);
			flags =| MNE;
			break;
		case 's':
			null(flags&MNS);
			flags =| MNS;
			break;
		case 'l':
			null(flags&MNL);
			flags =| MNL;
			break;
		case 'o':
			null(flags&MNO);
			flags =| MNO;
			break;
		case 'b':
			null(flags&MNABMC);
			flags =| MNB;
			break;
		case 'm':
			null(flags&MNABMC);
			flags =| MNM;
			break;
		case 'c':
			null(flags&MNABMC);
			flags =| MNC;
			break;
		case 'x':
			null(flags&MNXYZ);
			flags =| MNX;
			break;
		case 'y':
			null(flags&MNXYZ);
			flags =| MNY;
			break;
		case 'z':
			null(flags&MNXYZ);
			flags =| MNZ;
			break;
		}
	m->m_flags  = flags;
	m->m_nminis = getbyte();
	m->m_mbase  = getbyte();
	m->m_nshort = getbyte();
	m->m_sbase  = getbyte();
	m->m_obase  = getbyte();
	skipline();
	if (h_flag)
		printf("#define op_%3.3s %d\n",m->m_name,m-mnemon);
	return(1);
}

read_instr() {
	register struct mnems *m;

	m = mnemon;
	m->m_name[0] = '-';
	m->m_name[1] = '-';
	m->m_name[2] = '-';
	m->m_flags = MNZ;		/* ???? */
	while (inmnem(++m))
		;
	if (h_flag)
		printf("#define sp_lmnem %d\n\n",m-mnemon-1);
	lastmnem = m;
}

writemnems () {
	register struct mnems *mnem;
	register i,j;

	if (fcreat(mnemfile,output)<0)
		error("can't create mnems-file\n");
	printf("int mnemon[][5] {\n");
	for (mnem = mnemon; mnem <= lastmnem ; mnem++) {
		for (j=0; j < ((sizeof *mnem)>>1); j++) {
			printf("\t%d,",mnem->intarr[j]);
		}
		putchar('\n');
	}
	printf("};\n");
	fflush(output); close(output);
}

checklett(c) {

	if (c <'a' || c >'z')
		error("bad letter");
}

null(word) {

	if(word != 0)
		error("overspecified flag");
}

int getchar() {
	register c;

	if(peekc) {
		c = peekc;
		peekc = 0;
	} else {
		c = getc(input);
		if ( c >= 'A' && c <= 'Z' )
			c =+ 'a' - 'A';
	}
	return(c);
}

skiplayout() {
	register c;

	while ((c=getchar())=='\t'||c==' ')
		;
	peekc = c;
}

int getlett() {
	register c;

	c = getchar();
	checklett(c);
	return(c);
}

int getbyte() {
	char buf[10];
	register char *p;
	register c;

	skiplayout();
	p = buf;
	while((c=getchar())>= '0' && c <= '9')
		*p++ = c;
	peekc = c;
	*p=0;
	c = atoi(buf);
	if ( c < 0 || c > 255 )
		error("number too large");
	return(c);
}

skipline() {
	register c;

	while ((c = getchar()) != '\n')
		;
	peekc = getchar();
	line++;
}

error(s) {

	fflush(output);
	output[0]=2;
	printf("Mktab - error in line %d: %s\n",line,s);
	fflush(output);
	exit(-1);
}

dosecondpart () {
	register i,last;
	register char *s;
	char *ss;
	int optline();

	/*
	 * Process second part of tables that contain optimisations
	 * and produce output.
	 */

	if (fcreat(optfile,output)<0)
		error("can't create opts-file\n");
	printf("int pattern[][8] {\n");
	while (optline())
		;
	last = lastmnem - mnemon;
	printf("};\n\nint firstopt[] {\n");
	for(i=0; i<=last; i++)
		printf("\t%d,\n",firstopt[i]);
	printf("};\n\nint optcount[] {\n");
	for(i=0; i<=last; i++)
		printf("\t%d,\n",optcount[i]);
	printf("};\n");
	fflush(output); close(output);
}

int optline () {
	struct pattern pattern;
	register i;

	/*
	 * Read one optimisation and produce output for it.
	 */
	if (peekc<0)
		return(0);
	/*
	 * not yet at eof
	 */
	for (i=0;i<7;i++)
		inpart(&pattern.sub[i]);
	skiplayout();
	if (peekc!='\n')
		pattern.optkind = getbyte();
	else
		pattern.optkind = 0;
	skipline();
	if (pattern.sub[0].instr != curinstr) {
		curinstr = pattern.sub[0].instr;
		firstopt[curinstr&0377] = optno;
	}
	++optcount[curinstr&0377];
	printf("\t%d,\t%d,\t%d,\t%d,",
		pattern.sub[0].word,
		pattern.sub[1].word,
		pattern.sub[2].word,
		pattern.sub[3].word
	);
	printf("\t%d,\t%d,\t%d,\t%d,\n",
		pattern.sub[4].word,
		pattern.sub[5].word,
		pattern.sub[6].word,
		pattern.optkind
	);
	optno++;
	return(1);
}

inpart(ap) struct element *ap; {
	register struct element *p;
	register c,i;
	int ad1,ad2,op;
	char buf[4];
	int mnemlookup();

	/*
	 * read one part of an optimisation line
	 */
	p = ap;
	skiplayout();
	c = getchar();
	if (c == ',') {
		p->instr = 0;
		p->operand = 0;
		return;
	}
	checklett(c);
	buf[0] = c;
	buf[1] = getlett();
	buf[2] = getlett();
	buf[3] = 0;
	p->instr = mnemlookup(buf);
	skiplayout();
	c = getchar();
	if (c == '*') {
		p->operand = 0;
		return;
	}
	/*
	 * there is an operand, it could start with an 'a' or a digit
	 */
	if (c>= '0' && c<= '9' || c=='-') {
		buf[0] = c;
		i=1;
		while((c=getchar())>='0' && c<='9')
			buf[i++] = c;
		buf[i] = 0;
		i = atoi(buf);
		p->operand = i+EXCESS;
		peekc = c;
		return;
	}
	/*
	 * It should be an 'a' by now.
	 */
	if (c != 'a')
		error("bad letter in operand of optimisation");
	c = getchar();
	if (c<'0' || c>'3')
		error("In Ax x should be 0,1,2 or 3");
	ad1 = c - '0';
	c = getchar();
	switch(c) {
	default:
		error("bad char after Ax");
	case ' ':
	case '\t':
	case '\n':
		peekc = c;
		p->operand = CONLIM + ad1*ARANGE +ARANGE/2;
		return;
	case '+':
		op = 0;	break;
	case '-':
		op = 1;	break;
	case '*':
		op = 2;	break;
	case '/':
		op = 3;	break;
	}
	c = getchar();
	if ( c>='0' && c<='9') {
		peekc = c;
		ad2 = getbyte();
		if ( ad2>ARANGE/2 )
			error("offset too big");
		if (op > 1)
			error("bad char between Ax and number");
		op = 1 - 2*op;
		/*
		 * now op = -1 for char '-'
		 *          +1 for char '+'
		 */
		p->operand = CONLIM + ad1*ARANGE + ARANGE/2 + op*ad2;
		return;
	}
	if ( c != 'a' )
		error("bad char after operator");
	c = getchar();
	if (c <'0' || c >'3')
		error("bad digit after 'a' after operator");
	ad2 = c - '0';
	p->operand = ARITBASE + (op<<4) + (ad1<<2) + (ad2);
}

int mnemlookup(m) char *m; {
	register char *key;
	register struct mnems *mnem;
	register candidates;

	key = m;
	candidates = count[*key - 'a'];
	mnem = &mnemon[first[*key++ - 'a']];
	while (candidates--) {
		if (mnem->m_name[1] == *key++ &&
		    mnem->m_name[2] == *key)
			return(mnem-mnemon);
		mnem++;
		key--;
	}
	error("bad mnemonic in optimisation");
}

putchar (c) {
	putc(c,output);
}
