%{
/*
 *  z80 -- macro cross-assembler for the Zilog Z80 microprocessor
 *
 *  Bruce Norskog	4/78
 *
 *  Last modification - 8/29/78
 *  This assembler is modeled after the Intel 8080 macro cross-assembler
 *  for the Intel 8080 by Ken Borgendale.  The major features are:
 *	1.  Full macro capabilities
 *	2.  Conditional assembly
 *	3.  A very flexible set of listing options and pseudo-ops
 *	4.  Symbol table output
 *	5.  Error report
 *	6.  Elimination of sequential searching
 *	7.  Commenting of source
 *	8.  Facilities for system definiton files
 *
 */
#define ITEMTABLESIZE	650
#define TEMPBUFSIZE	200
#define LINEBUFFERSIZE	200
#define EMITBUFFERSIZE	200
#define MAXSYMBOLSIZE	7
#define IFSTACKSIZE	20
#define MAXIFS		150
#define TITLELEN	50
#define BINPERLINE	16
#define	PARMMAX		25
#define MAXEXP		25
#define SYMMAJIC	07203


#define loop	for(;;)

yyerror(){}		/*we will do our own error printing*/

struct	item	{
	char	*i_string;
	int	i_value;
	int	i_token;
};

extern int	fin[259];
extern int	fout[259];
int		fbuf[259];


int	pass2;	/*set when pass one completed*/

char	*dollarsign;	/*location counter*/
char	*olddollar;	/* kept to put out binary */

#define bflag	0	/* balance error */
#define eflag	1	/* expression error */
#define fflag	2	/* format error */
#define iflag	3	/* bad digits */
#define mflag	4	/* multiply defined */
#define pflag	5	/* phase error */
#define uflag	6	/* undeclared used */
#define vflag	7	/* value out of range */

#define FLAGS	8	/* number of flags */

char	err[FLAGS];
int	keeperr[FLAGS];
char	errlet[]	"BEFIMPUV";
char	*errname[] {
	"Balance",
	"Expression",
	"Format",
	"Digit",
	"Mult. def.",
	"Phase",
	"Undeclared",
	"Value",
};


char	linebuf[LINEBUFFERSIZE];
char	*lineptr;
char	*linemax  &linebuf[LINEBUFFERSIZE];

char	outbin[BINPERLINE];
char	*outbinp	&outbin;
char	*outbinm	&outbin[BINPERLINE];

char	emitbuf[EMITBUFFERSIZE];
char	*emitptr;

char	ifstack[IFSTACKSIZE];
char	*ifptr;
char	*ifstmax  &ifstack[IFSTACKSIZE-1];


char	expif[MAXIFS];
char	*expifp;
char	*expifmax  &expif[MAXIFS];

int	expstack[MAXEXP];
int	expptr;


int	nitems;
int	linein;
int	linecnt;
int	nbytes;
int	invented;


char	tempbuf[TEMPBUFSIZE];
char	*tempmax	&tempbuf[TEMPBUFSIZE-1];

char	inmlex;
char	arg_flag;
char	quoteflag;
int	parm_number;
int	exp_number;
char	*symlong	"Symbol too long";

int	disp;
#define FLOC	PARMMAX
#define TEMPNUM	PARMMAX+1
char	**est;
char	**est2;

int	floc;
int	mfptr;
int	mfile;
int	mfileo;
char	*writesyms;


char	*title;
char	titlespace[TITLELEN];
char	*timp;
char	*sourcef;
char	src[15];
char	bin[15];
char	mtmp[15];
char	listf[15];

char	bopt	1;
char	edef	1;
char	eopt	1;
char	fdef	0;
char	fopt	0;
char	gdef	1;
char	gopt	1;
char	lstoff	0;
char	lopt	0;
char	mdef	0;
char	mopt	0;
char	nopt	0;
char	oopt	0;
char	popt	0;
char	sopt	0;
char	topt	1;
char	saveopt;

char	xeq_flag	0;
int	xeq;

int	now[2];
int	line;
int	page	1;

struct stab {
	char	t_name[MAXSYMBOLSIZE+1];
	int	t_value;
	int	t_token;
};

/*
 *  push back character
 */
char	peekc;


/*
 *  add a character to the output line buffer
 */
addtoline(ac)
int	ac;
{
	if (inmlex) return(ac);
	if (lineptr >= linemax)
		error("line buffer overflow");
	*lineptr++ = ac;
	return(ac);
}


/*
 *  put values in buffer for outputing
 */
emit(bytes, args)
int	args;
{
	register  i, *argp;

	i = bytes;
	argp = &args;
	while	(--i >= 0)
		if (emitptr >= &emitbuf[EMITBUFFERSIZE])
			error("emit buffer overflow");
		else {
			*emitptr++ = *argp;
			argp++;
		}
}


emit1(opcode,regvalh,data16,type)
int	opcode,regvalh,data16,type;
{
	if (regvalh < 0) {
		if (type & 1 == 0 && (disp > 127 || disp < -128))
			err[vflag]++;
		switch(type) {
		case 0:
			if (opcode < 0)
				emit(4, regvalh >> 8, opcode >> 8, disp, opcode);
			else
				emit(3, regvalh >> 8, opcode, disp);
			break;
		case 1:
			emit(2, regvalh >> 8, opcode);
			break;
		case 2:
			if (data16 > 255 || data16 < -128)
				err[vflag]++;
			emit(4, regvalh >> 8, opcode, disp, data16);
			break;
		case 5:
			emit(4, regvalh >> 8, opcode, data16, data16 >> 8);
		}
	} else
		switch(type) {
		case 0:
			if (opcode < 0)
				emit(2, opcode >> 8, opcode);
			else
				emit(1, opcode);
			break;
		case 1:
			if (opcode < 0)
				emit(2, opcode >> 8, opcode);
			else
				emit(1, opcode);
			break;
		case 2:
			if (data16 > 255 || data16 < -128)
				err[vflag]++;
			emit(2, opcode, data16);
			break;
		case 3:
			if (data16 >255 || data16 < -128)
				err[vflag]++;
			emit(2, opcode, data16);
			break;
		case 5:
			if (opcode < 0)
				emit(4, opcode >> 8, opcode, data16, data16 >> 8);
			else
				emit(3, opcode, data16, data16 >> 8);
		}
}
emitdad(rp1,rp2)
	int rp1,rp2;
{
	if (rp1 < 0)
		emit(2,rp1 >> 8, rp2 + 9);
	else
		emit(1,rp2 + 9);
}
emitjr(opcode,expr)
int	opcode,expr;
{
	disp = expr - dollarsign - 2;
	if (disp > 127 || disp < -128)
		err[vflag]++;
	emit(2, opcode, disp);
}




/*
 *  put out a byte of binary 
 */
putbin(v)
{
	if(!pass2 || !bopt) return;
	*outbinp++ = v;
	if (outbinp >= outbinm) flushbin();
}



/*
 *  output one line of binary in INTEL standard form
 */
flushbin()
{
	register  char *p;
	register check;

	if (!pass2 || !bopt) return;
	nbytes =+ outbinp-outbin;
	if (check = outbinp-outbin) {
		putc(':', fbuf);
		puthex(check, fbuf);
		puthex(olddollar>>8, fbuf);
		puthex(olddollar, fbuf);
		puthex(0, fbuf);
		check =+ (olddollar >> 8) + olddollar;
		olddollar =+ (outbinp-outbin);
		for (p=outbin; p<outbinp; p++) {
			puthex(*p, fbuf);
			check =+ *p;
		}
		puthex(256-check, fbuf);
		putc('\n', fbuf);
		outbinp = outbin;
	}
}



/*
 *  put out one byte of hex
 */
puthex(byte, buf)
char	byte;
int	*buf;
{
	putc("0123456789ABCDEF"[(byte >> 4) & 017],buf);
	putc("0123456789ABCDEF"[byte & 017],buf);
}

/*
 *  put out a line of output -- also put out binary
 */
list(optarg)
int	optarg;
{
	register char *	p;
	register int	i;
	register char	check;
	int  lst;

	linein++;
	if (!expptr) linecnt++;
	addtoline('\0');
	if (pass2) {
		lst = iflist();
		if (lst) {
			lineout();
			if (nopt) printf("%4d:\t", linein);
			puthex(optarg >> 8,fout);
			puthex(optarg,fout);
			printf("  ");
			for (p = emitbuf; (p < emitptr) && (p - emitbuf < 4); p++) {
				puthex(*p,fout);
			}
			for (i = 4 - (p-emitbuf); i > 0; i--)
				printf("  ");
			putchar('\t');
			printf("%s",linebuf);
		}

		if (bopt) {
			for (p = emitbuf; p < emitptr; p++)
				putbin(*p);
		}


		p = emitbuf+4;
		while (lst && gopt && p < emitptr) {
			lineout();
			if (nopt) putchar('\t');
			printf("      ");
			for (i = 0; (i < 4) && (p < emitptr);i++) {
				puthex(*p, fout);
				p++;
			}
			putchar('\n');
		}


		lsterr2(lst);
	} else
		lsterr1();
	dollarsign =+ emitptr - emitbuf;
	emitptr = emitbuf;
	lineptr = linebuf;
}



/*
 *  keep track of line numbers and put out headers as necessary
 */
lineout()
{
	if (line == 60) {
		if (popt) putchar('\13');
		else printf("\n\n\n\n\n");
		line = 0;
	}
	if (line == 0) {
		printf("\n\n%s %s\t%s\t Page %d\n\n\n",
		&timp[4], &timp[20], title, page++);
		line = 4;
	}
	line++;
}


/*
 *  cause a page eject
 */
eject()
{
	if (!pass2 || !iflist()) return;
	if (popt) {
		putchar('\13');
		return;
	}
	while (line < 65) {
		line++;
		putchar('\n');
	}
	line = 0;
}


/*
 *  space n lines on the list file
 */
space(n)
{
	register  i;
	if (!pass2 || !iflist()) return;
	for (i = 0; i<n; i++) {
		lineout();
		putchar('\n');
	}
}


/*
 *  Error handling - pass 1
 */
lsterr1() {
	register int i;
	if (topt)
		for (i = 0; i <= 4; i++)
			if (err[i]) {
				errorprt(i);
				err[i] = 0;
			}
}


/*
 *  Error handling - pass 2.
 */
lsterr2(lst)
int	lst;
{
	register int i;
	for (i=0; i<FLAGS; i++)
		if (err[i]) {
			if (lst) {
				lineout();
				putchar(errlet[i]);
				putchar('\n');
			}
			err[i] = 0;
			keeperr[i]++;
			if (i > 4 && topt)
				errorprt(i);
		}

	fflush(fout);	/*to avoid putc(har) mix bug*/
}

/*
 *  print diagnostic to error terminal
 */
errorprt(errnum)
int	errnum;
{
	writed(linecnt);
	write(2, ":  ", 3);
	write(2, errname[errnum], size(errname[errnum])-2);
	write(2, " error\n", 7);
	write(2, linebuf, size(linebuf)-2);
	write(2, "\n", 1);
}
writed(num)
int	num;
{
	int	a;
	char	str[2];
	if (a=num/10)
		writed(a);
	str[0] = num % 10 + '0';
	str[1] = 0;
	write(2, str, 1);
}



/*
 *  list without address -- for comments and if skipped lines
 */
list1()
{
	int lst;
	addtoline('\0');
	lineptr = linebuf;
	linein++;
	if (!expptr) linecnt++;
	if (pass2)
		if (lst = iflist()) {
			lineout();
			if (nopt) printf("%4d:\t", linein);
			printf("\t\t%s", linebuf);
			lsterr2(lst);
		}
	else
		lsterr1();
}


/*
 *  see if listing is desired
 */
iflist()
{
	register  i, j;

	if (lopt) return(0);
	if (*ifptr && !fopt) return(0);
	if (!lstoff && !expptr) return(1);
	j = 0;
	for (i=0; i<FLAGS; i++) if (err[i]) j++;
	if (expptr) return(mopt || j);
	if (eopt && j) return(1);
	return(0);
}


%}
%token STRING
%token NOOPERAND
%token ARITHC
%token ADD
%token LOGICAL
%token BIT
%token CALL
%token INCDEC
%token DJNZ
%token EX
%token IM
%token IN
%token JP
%token JR
%token LD
%token OUT
%token PUSHPOP
%token RET
%token SHIFT
%token RST
%token REGNAME
%token ACC
%token C
%token RP
%token HL
%token INDEX
%token AF
%token SP
%token MISCREG
%token F
%token COND
%token SPCOND
%token NUMBER
%token UNDECLARED
%token END
%token ORG
%token DEFB
%token DEFS
%token DEFW
%token EQU
%token DEFL
%token LABEL
%token EQUATED
%token WASEQUATED
%token DEFLED
%token MULTDEF
%token MOD
%token SHL
%token SHR
%token NOT
%token IF
%token ENDIF
%token ARGPSEUDO
%token LIST
%token MINMAX
%token MACRO
%token MNAME
%token OLDMNAME
%token ARG
%token ENDM
%token MPARM
%token ONECHAR
%token TWOCHAR
%left '|' '^'
%left '&'
%nonassoc NOT
%left '+' '-'
%left '*' '/' MOD SHL SHR
%left UNARY
%%

%{
char  *cp;
int  ch;
int  i;
%}

program:
	statements
|
	error
	= {	error("file bad");	}
;


statements:
	statement
|
	statements statement
|
	statements error
	= {
		err[fflag]++;
		quoteflag = 0;
		while(yychar != '\n' && yychar != '\0') yychar = yylex();
		list(dollarsign);
		yyclearin;yyerrok;
	}
;


statement:
	label.part '\n'
	= { 
		if ($1) list(dollarsign);
		else  list1();
	}
|
	label.part operation '\n'
	= {
		list(dollarsign);
	}
|
	symbol EQU expression '\n'
	= {
		switch($1->i_token) {
		case UNDECLARED: case WASEQUATED:
			$1->i_token = EQUATED;
			$1->i_value = $3;
			break;
		default:
			err[mflag]++;
			$1->i_token = MULTDEF;
		}
		list($3);
	}
|
	symbol DEFL expression '\n'
	= {
		switch($1->i_token) {
		case UNDECLARED: case DEFLED:
			$1->i_token = DEFLED;
			$1->i_value = $3;
			break;
		default:
			err[mflag]++;
			$1->i_token = MULTDEF;
		}
		list($3);
	}
|
	symbol MINMAX expression ',' expression '\n'
	= {
		switch ($1->i_token) {
		case UNDECLARED: case DEFLED:
			$1->i_token = DEFLED;
			if ($2->i_value)	/* max */
				list($1->i_value = ($3 > $5? $3:$5));
			else list($1->i_value = ($3 < $5? $3:$5));
			break;
		default:
			err[mflag]++;
			$1->i_token = MULTDEF;
			list($1->i_value);
		}
	}
|
	IF expression '\n'
	= {
		if (ifptr >= ifstmax)
			error("Too many ifs");
		else {
			if (pass2) {
				*++ifptr = *expifp++;
				if (*ifptr != !yypv[2]) err[pflag]++;
			} else {
				if (expifp >= expifmax)
					error("Too many ifs!");
				*expifp++ = !yypv[2];
				*++ifptr = !yypv[2];
			}
		}
		saveopt = fopt;
		fopt = 1;
		list(yypv[2]);
		fopt = saveopt;
	}
|
	ENDIF '\n'
	= {
		if (ifptr == ifstack) err[bflag]++;
		else --ifptr;
		list1();
	}
|
	label.part END '\n'
	= {
		list(dollarsign);
		peekc = 0;
	}
|
	label.part END expression '\n'
	= {
		xeq_flag++;
		xeq = $3;
		list($3);
		peekc = 0;
	}
|
	label.part DEFS expression '\n'
	= {
		if ($3 < 0) err[vflag]++;
		list(dollarsign);
		if ($3) {
			flushbin();
			dollarsign =+ $3;
			olddollar = dollarsign;
		}
	}
|
	ARGPSEUDO arg_on ARG  arg_off '\n'
	= {
		list1();
		switch ($1->i_value) {

		case 0:		/* title */
			lineptr = linebuf;
			cp = tempbuf;
			title = titlespace;
			while ((*title++ = *cp++) && (title < &titlespace[TITLELEN]));
			*title = 0;
			title = titlespace;
			break;

		case 1:		/* rsym */
			if (pass2) break;
			insymtab(tempbuf);
			break;

		case 2:		/* wsym */
			writesyms = alloc(size(tempbuf));
			copy(tempbuf, writesyms);
			break;
		}
	}
|
	ARGPSEUDO arg_on '\n' arg_off
	= {
		err[fflag]++;
		list(dollarsign);
	}
|
	LIST '\n'
	= {	if($1 != -1) $2 = 1;	goto dolopt; }
|
	LIST expression '\n'
	= {  dolopt:
		linein++;
		linecnt++;
		if (pass2) {
			lineptr = linebuf;
			switch ($1->i_value) {
			case 0:	/* list */
				if ($2 < 0) lstoff = 1;
				if ($2 > 0) lstoff = 0;
				break;

			case 1:	/* eject */
				if ($2) eject();
				break;

			case 2:	/* space */
				if ((line + $2) > 60) eject();
				else space($2);
				break;

			case 3:	/* elist */
				eopt = edef;
				if ($2 < 0) eopt = 0;
				if ($2 > 0) eopt = 1;
				break;

			case 4:	/* fopt */
				fopt = fdef;
				if ($2 < 0) fopt = 0;
				if ($2 > 0) fopt = 1;
				break;

			case 5:	/* gopt */
				gopt = gdef;
				if ($2 < 0) gopt = 1;
				if ($2 > 0) gopt = 0;
				break;

			case 6: /* mopt */
				mopt = mdef;
				if ($2 < 0) mopt = 0;
				if ($2 > 0) mopt = 1;
			}
		}
	}
|
	UNDECLARED MACRO parm.list '\n' 
	= {
		$1->i_token = MNAME;
		$1->i_value = mfptr;
		list1();
		mlex($1->i_string);
		parm_number = 0;
	}
|
	OLDMNAME MACRO
	= {
		$1->i_token = MNAME;
		while (yychar != ENDM && yychar) {
			while (yychar != '\n' && yychar)
				yychar = yylex();
			list1();
			yychar = yylex();
		}
		while (yychar != '\n' && yychar) yychar = yylex();
		list1();
		yychar = yylex();
	}
|
	label.part MNAME al '\n'
	= {	goto expand;	}
|
	label.part MNAME al arg.list '\n'
	= {
	expand:
		arg_flag = 0;
		parm_number = 0;
		list(dollarsign);
		expptr++;
		est = est2;
		est[FLOC] = floc;
		est[TEMPNUM] = exp_number++;
		seek(mfile, $2->i_value, 0);
		floc = $2->i_value;
	}
;


label.part:
	/*empty*/
	= {	$$ = 0;	}
|
	symbol ':'
	= {
		switch($1->i_token) {
		case UNDECLARED:
			if (pass2)
				err[pflag]++;
			else {
				$1->i_token = LABEL;
				$1->i_value = dollarsign;
			}
			break;
		case LABEL:
			if (!pass2) {
				$1->i_token = MULTDEF;
				err[mflag]++;
			} else if ($1->i_value != dollarsign)
				err[pflag]++;
			break;
		default:
			err[mflag]++;
			$1->i_token = MULTDEF;
		}
	}
;


operation:
	NOOPERAND
	= {
		emit1($1->i_value, 0, 0, 1);
	}
|
	JP expression
	= {	emit(3, 0303, $2, $2 >> 8);	}
|
	CALL expression
	= {	emit(3, 0315, $2, $2 >> 8);	}
|
	RST	expression
	= {
		if ($2 > 7 || $2 < 0)
			err[vflag]++;
		emit(1, $1->i_value + (($2 & 7) << 3));
	}
|
	ADD ACC ',' expression
	= {
		emit1(0306, 0, $4, 3);
	}
|
	ARITHC expression
	= {
		emit1(0306 + ($1->i_value << 3), 0, $2, 3);
	}
|
	LOGICAL expression
	= {
		emit1(0306 | ($1->i_value << 3), 0, $2, 3);
	}
|
	ADD ACC ',' reg
	= {
		emit1(0200 + ($4 & 0377), $4, 0, 0);
	}
|
	ARITHC reg
	= {
		emit1(0200 + ($1->i_value << 3) + ($4 & 0377), $4, 0, 0);
	}
|
	LOGICAL reg
	= {
		emit1(0200 + ($1->i_value << 3) + ($2 & 0377), $2, 0, 0);
	}
|
	SHIFT reg
	= {
		emit1(0145400 + ($1->i_value << 3) + ($2 & 0377), $2, 0, 0);
	}
|
	INCDEC	reg
	= {
		emit1($1->i_value + (($2 & 0377) << 3) + 4, $2, 0, 0);
	}
|
	ARITHC HL ',' bcdehlsp
	= {
		if ($1->i_value == 1)
			emit(2,0355,0112+$4);
		else
			emit(2,0355,0102+$4);
	}
|
	ADD mar ',' bcdesp
	= {
		emitdad($2,$4);
	}
|
	ADD mar ',' mar
	= {
		if ($2 != $4)
			err[fflag]++;
		emitdad($2,$4);
	}
|
	INCDEC evenreg
	= {
		emit1(($1->i_value << 3) + ($2 & 0377) + 3, $2, 0, 1);
	}
|
	PUSHPOP pushable
	= {
		emit1($1->i_value + ($2 & 0377), $2, 0, 1);
	}
|
	BIT expression ',' reg
	= {
		if ($2 < 0 || $2 > 7)
			err[vflag]++;
		emit1($1->i_value + (($2 & 7) << 3) + ($4 & 0377), $4, 0, 0);
	}
|
	JP condition ',' expression
	= {
		emit(3, 0302 + $2, $4, $4 >> 8);
	}
|
	JP '(' mar ')'
	= {
		emit1(0351, $3, 0, 1);
	}
|
	CALL condition ',' expression
	= {
		emit(3, 0304 + $2, $4, $4 >> 8);
	}
|
	JR expression
	= {
		emitjr(030,$2);
	}
|
	JR spcondition ',' expression
	= {
		emitjr($1->i_value + $2, $4);
	}
|
	DJNZ expression
	= {
		emitjr($1->i_value, $2);
	}
|
	RET
	= {
		emit(1, $1->i_value);
	}
|
	RET condition
	= {
		emit(1, 0300 + $2);
	}
|
	LD reg ',' reg
	= {
		if (($2 & 0377) == 6 && ($4 & 0377) == 6)
			err[fflag]++;
		emit1(0100 + (($2 & 7) << 3) + ($4 & 7),$2 | $4, 0, 0);
	}
|
	LD reg ',' expression
	= {
		emit1(6 + (($2 & 0377) << 3), $2, $4, 2);
	}
|
	LD reg ',' '(' RP ')'
	= {	if ($2 != 7) err[fflag]++;
		else emit(1, 012 + $5->i_value);
	}
|
	LD reg ',' parenexpr
	= {
		if ($2 != 7) err[fflag]++;
		else emit(3, 072, $4, $4 >> 8);
	}
|
	LD '(' RP ')' ',' ACC
	= {
		emit(1, 2 + $3->i_value);
	}
|
	LD parenexpr ',' ACC
	= {
		emit(3, 062, $2, $2 >> 8);
	}
|
	LD reg ',' MISCREG
	= {
		if ($2 != 7) err[fflag]++;
		else emit(2, 0355, 0127 + $4->i_value);
	}
|
	LD MISCREG ',' ACC
	= {
		emit(2, 0355, 0107 + $2->i_value);
	}
|
	LD evenreg ',' lxexpression
	= {
		emit1(1 + ($2 & 060), $2, $4, 5);
	}
|
	LD evenreg ',' parenexpr
	= {
		if (($2 & 060) == 040)
			emit1(052, $2, $4, 5);
		else
			emit(4, 0355, 0113 + $2, $4, $4 >> 8);
	}
|
	LD parenexpr ',' evenreg
	= {
		if (($4 & 060) == 040)
			emit1(042, $4, $2, 5);
		else
			emit(4, 0355, 0103 + $4, $2, $2 >> 8);
	}
|
	LD evenreg ',' mar
	= {
		if ($2 != 060)
			err[fflag]++;
		else
			emit1(0371, $4, 0, 1);
	}
|
	EX RP ',' HL
	= {
		if ($2->i_value != 020)
			err[fflag]++;
		else
			emit(1, 0353);
	}
|
	EX AF ',' AF setqf '\'' clrqf
	= {
		emit(1, 010);
	}
|
	EX '(' SP ')' ',' mar
	= {
		emit1(0343, $6, 0, 1);
	}
|
	IN realreg ',' expression
	= {
		if ($2 != 7)
			err[fflag]++;
		else	{
			if ($4 < 0 || $4 > 255)
				err[vflag]++;
			emit(2, $1->i_value, $4);
		}
	}
|
	IN realreg ',' '(' C ')'
	= {
		emit(2, 0355, 0100 + ($2 << 3));
	}
|
	IN F ',' '(' C ')'
	= {
		emit(2, 0355, 0160);
	}
|
	OUT expression ',' ACC
	= {
		if ($2 < 0 || $2 > 255)
			err[vflag]++;
		emit(2, $1->i_value, $2);
	}
|
	OUT '(' C ')' ',' realreg
	= {
		emit(2, 0355, 0101 + ($6 << 3));
	}
|
	IM expression
	= {
		if ($2 > 2 || $2 < 0)
			err[vflag]++;
		else
			emit(2, $1->i_value >> 8, $1->i_value + (($2 + ($2 > 0)) << 3));
	}
|
	ORG expression
	= {
		if ($2-dollarsign) {
			flushbin();
			olddollar = $2;
			dollarsign = $2;
		}
	}
|
	DEFB db.list
|
	DEFW dw.list
|
	ENDM
;


parm.list:
|
	parm.element
|
	parm.list ',' parm.element
;


parm.element:
	UNDECLARED
	= {
		$1->i_token = MPARM;
		if (parm_number >= PARMMAX)
			error("Too many parameters");
		$1->i_value = parm_number++;
	}
;


arg.list:
	arg.element
|
	arg.list ',' arg.element
;


arg.element:
	ARG
	= {
		cp = alloc(size(tempbuf));
		est2[parm_number++] = cp;
		copy(tempbuf, cp);
	}
;
reg:
	realreg
|
	mem
;
realreg:
	REGNAME
	=	{
		$$ = $1->i_value;
	}
|
	ACC
	=	{
		$$ = $1->i_value;
	}
|
	C
	=	{
		$$ = $1->i_value;
	}
;
mem:
	'(' HL ')'
	=	{
		$$ = 6;
	}
|
	'(' INDEX '+' expression ')'
	=	{
		disp = $4;
		$$ = ($2->i_value & 0177400) | 6;
	}
|
	'(' INDEX ')'
	=	{
		disp = 0;
		$$ = ($2->i_value & 0177400) | 6;
	}
;
evenreg:
	bcdesp
|
	mar
;
pushable:
	RP
	=	{
		$$ = $1->i_value;
	}
|
	AF
	= {
		$$ = $1->i_value;
	}
|
	mar
;
bcdesp:
	RP
	=	{
		$$ = $1->i_value;
	}
|
	SP
	=	{
		$$ = $1->i_value;
	}
;
bcdehlsp:
	bcdesp
|
	HL
	=	{
		$$ = $1->i_value;
	}
;
mar:
	HL
	=	{
		$$ = $1->i_value;
	}
|
	INDEX
	=	{
		$$ = $1->i_value;
	}
;
condition:
	spcondition
|
	COND
	=	{
		$$ = $1->i_value;
	}
;
spcondition:
	SPCOND
	=	{
		$$ = $1->i_value;
	}
|
	C
	=	{	$$ = 030;	}
;
db.list:
	db.list.element
|
	db.list ',' db.list.element
;
db.list.element:
	TWOCHAR
	=	{
		emit(2, $1, $1>>8);
		}
|
	STRING
	=	{
		cp = $1;
		while (*cp != '\0') emit(1,*cp++);
		}
|
	expression
	=	{
		if ($1 < -128 || $1 > 255)
				err[vflag]++;
		emit(1, $1 & 0377);
		}
;


dw.list:
	dw.list.element
|
	dw.list ',' dw.list.element
;


dw.list.element:
	expression
	= {
		emit(2, $1, $1>>8);
	}
;



lxexpression:
	expression
|
	TWOCHAR
;

parenexpr:
	'(' expression ')'
	= {	$$ = $2;	}
;

expression:
	error
	= {
		err[eflag]++;
		$$ = 0;
	}
|
	LABEL
	= {	$$ = $1->i_value;	}
|
	NUMBER
|
	ONECHAR
|
	EQUATED
	= {	$$ = $1->i_value;	}
|
	WASEQUATED
	= {	$$ = $1->i_value;	}
|
	DEFLED
	= {	$$ = $1->i_value;	}
|
	'$'
	= {	$$ = dollarsign;	}
|
	UNDECLARED
	= {
		err[uflag]++;
		$$ = 0;
	}
|
	MULTDEF
	= {	$$ = $1->i_value;	}
|
	expression '+' expression
	= {	$$ = $1 + $3;	}
|
	expression '-' expression
	= {	$$ = $1 - $3;	}
|
	expression '/' expression
	= {	$$ = $1 / $3;	}
|
	expression '*' expression
	= {	$$ = $1 * $3;	}
|
	expression MOD expression
	= {	$$ = $1 % $3;	}
|
	expression '&' expression
	= {	$$ = $1 & $3;	}
|
	expression '|' expression
	= {	$$ = $1 | $3;	}
|
	expression '^' expression
	= {	$$ = $1 ^ $3;	}
|
	expression SHL expression
	= {	$$ = $1 << $3;	}
|
	expression SHR expression
	= {	$$ = (($1 >> 1) & 077777) >> ($3 - 1);	}
|
	'[' expression ']'
	= {	$$ = $2;	}
|
	NOT expression
	= {	$$ = ~$2;	}
|
	'+' expression %prec UNARY
	= {	$$ = $2;	}
|
	'-' expression %prec UNARY
	= {	$$ = -$2;	}
;

symbol:
	UNDECLARED
|
	LABEL
|
	MULTDEF
|
	EQUATED
|
	WASEQUATED
|
	DEFLED
;


al:
	= {
		if (expptr >= MAXEXP)
			error("Macro expansion level");
		est2 = expstack[expptr] = alloc(PARMMAX*2 + 4);
		for (i=0; i<PARMMAX; i++)
			est2[i] = 0;
		arg_flag++;
	}
;


arg_on:
	= {	arg_flag++;	}
;

arg_off:
	= {	arg_flag = 0;	}
;

setqf:
	= {	quoteflag++;	}
;

clrqf:
	= {	quoteflag = 0;	}

;

%%
extern int	yylval;

#define EOF	0
#define OTHER	1
#define SPACE	2
#define DIGIT	3
#define LETTER	4
#define STARTER 5


/*
 *  This is the table of character classes.  It is used by the lexical
 *  analyser. (yylex())
 */
char	charclass[]	{
	EOF,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,
	OTHER,	SPACE,	OTHER,	OTHER,	OTHER,	SPACE,	OTHER,	OTHER,
	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,
	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,
	SPACE,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,
	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,
	DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,
	DIGIT,	DIGIT,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	STARTER,
	STARTER,LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER,
	LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER,
	LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER,
	LETTER, LETTER, LETTER, OTHER,	OTHER,	OTHER,	OTHER,	OTHER,
	OTHER,	LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER,
	LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER,
	LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER,
	LETTER, LETTER, LETTER, OTHER,	OTHER,	OTHER,	OTHER,	OTHER,
};


/*
 *  the following table tells which characters are parts of numbers.
 *  The entry is non-zero for characters which can be parts of numbers.
 */
char	numpart[]	{
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	'0',	'1',	'2',	'3',	'4',	'5',	'6',	'7',
	'8',	'9',	0,	0,	0,	0,	0,	0,
	0,	'A',	'B',	'C',	'D',	'E',	'F',	0,
	'H',	0,	0,	0,	0,	0,	0,	'O',
	0,	'Q',	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	'a',	'b',	'c',	'd',	'e',	'f',	0,
	'h',	0,	0,	0,	0,	0,	0,	'o',
	0,	'q',	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0};




/*
 *  the following table is a list of assembler mnemonics;
 *  for each mnemonic the associated machine-code bit pattern
 *  and symbol type are given.
 */
struct	item	keytab[]	{
	"a",	7,	ACC,
	"adc",	1,	ARITHC,
	"add",	0,	ADD,
	"af",	060,	AF,
	"and",	4,	LOGICAL,
	"ascii",0,	DEFB,
	"b",	0,	REGNAME,
	"bc",	0,	RP,
	"bit",	0145500,BIT,
	"block",0,	DEFS,
	"byte",	0,	DEFB,
	"c",	1,	C,
	"call", 0315,	CALL,
	"ccf",	077,	NOOPERAND,
	"cp",	7,	LOGICAL,
	"cpd",	0166651,NOOPERAND,
	"cpdr",	0166671,NOOPERAND,
	"cpi",	0166641,NOOPERAND,
	"cpir",	0166661,NOOPERAND,
	"cpl",	057,	NOOPERAND,
	"d",	2,	REGNAME,
	"daa",	0047,	NOOPERAND,
	"de",	020,	RP,
	"dec",	1,	INCDEC,
	"defb",	0,	DEFB,
	"defl",0,	DEFL,
	"defs",	0,	DEFS,
	"defw",	0,	DEFW,
	"di",	0363,	NOOPERAND,
	"djnz",	020,	DJNZ,
	"e",	3,	REGNAME,
	"ei",	0373,	NOOPERAND,
	"eject",1,	LIST,
	"elist",3,	LIST,
	"end",	0,	END,
	"endif",0,	ENDIF,
	"endm", 0,	ENDM,
	"equ",	0,	EQU,
	"ex",	0,	EX,
	"exx",	0331,	NOOPERAND,
	"f",	0,	F,
	"flist",4,	LIST,
	"glist",5,	LIST,
	"h",	4,	REGNAME,
	"halt",	0166,	NOOPERAND,
	"hl",	040,	HL,
	"i",	0,	MISCREG,
	"if",	0,	IF,
	"im",	0166506,IM,
	"in",	0333,	IN,
	"inc",	0,	INCDEC,
	"ind",	0166652,NOOPERAND,
	"indr",	0166672,NOOPERAND,
	"ini",	0166642,NOOPERAND,
	"inir",	0166662,NOOPERAND,
	"ix",	0156440,INDEX,
	"iy",	0176440,INDEX,
	"jp",	0303,	JP,
	"jr",	040,	JR,
	"l",	5,	REGNAME,
	"ld",	0,	LD,
	"ldd",	0166650,NOOPERAND,
	"lddr",	0166670,NOOPERAND,
	"ldi",	0166640,NOOPERAND,
	"ldir",	0166660,NOOPERAND,
	"list",	0,	LIST,
	"m",	070,	COND,
	"macro",0,	MACRO,
	"max",	1,	MINMAX,
	"min",	0,	MINMAX,
	"mlist",6,	LIST,
	"mod",	0,	MOD,
	"nc",	020,	SPCOND,
	"neg",	0166504,NOOPERAND,
	"nolist",-1,	LIST,
	"nop",	0,	NOOPERAND,
	"not",	0,	NOT,
	"nv",	040,	COND,
	"nz",	0,	SPCOND,
	"or",	6,	LOGICAL,
	"org",	0,	ORG,
	"otdr",0166673,NOOPERAND,
	"otir",0166663,NOOPERAND,
	"out",	0323,	OUT,
	"outd",	0166653,NOOPERAND,
	"outi",	0166643,NOOPERAND,
	"p",	060,	COND,
	"pe",	050,	COND,
	"po",	040,	COND,
	"pop",	0301,	PUSHPOP,
	"push", 0305,	PUSHPOP,
	"r",	010,	MISCREG,
	"res",	0145600,BIT,
	"ret",	0311,	RET,
	"reti",	0166515,NOOPERAND,
	"retn",	0166505,NOOPERAND,
	"rl",	2,	SHIFT,
	"rla",	027,	NOOPERAND,
	"rlc",	0,	SHIFT,
	"rlca",	07,	NOOPERAND,
	"rld",	0166557,NOOPERAND,
	"rr",	3,	SHIFT,
	"rra",	037,	NOOPERAND,
	"rrc",	1,	SHIFT,
	"rrca",	017,	NOOPERAND,
	"rrd",	0166547,NOOPERAND,
	"rst",	0307,	RST,
	"rsym",	1,	ARGPSEUDO,
	"sbc",	3,	ARITHC,
	"scf",	067,	NOOPERAND,
	"set",	0145700,BIT,
	"shl",	0,	SHL,
	"shr",	0,	SHR,
	"sla",	4,	SHIFT,
	"sp",	060,	SP,
	"space",2,	LIST,
	"sra",	5,	SHIFT,
	"srl",	7,	SHIFT,
	"sub",	2,	LOGICAL,
	"title",0,	ARGPSEUDO,
	"v",	050,	COND,
	"word",	0,	DEFW,
	"wsym",	2,	ARGPSEUDO,
	"xor",	5,	LOGICAL,
	"z",	010,	SPCOND,
};

/*
 *  user-defined items are tabulated in the following table.
 */

struct item	itemtab[ITEMTABLESIZE];
struct item	*itemmax  &itemtab[ITEMTABLESIZE];





/*
 *  lexical analyser, called by yyparse.
 */
yylex()
{
	register char	c;
	register char *	p;
	register int	radix;
	int  limit;

	if (arg_flag) return(getarg());
loop switch(charclass[c = nextchar()]) {
	case EOF:
		if (expptr) {
			popsi();
			continue;
		} else return(0);

	case SPACE:
		break;
	case LETTER:
	case STARTER:
		p = tempbuf;
		do	{
			if (p >= tempmax)
				error(symlong);
			*p++ = (c >= 'A' && c <= 'Z') ? c + 'a' - 'A' : c;
			while	((c = nextchar()) == '$');
			}
			while	(charclass[c]==LETTER||charclass[c]==DIGIT);
		if (p - tempbuf > MAXSYMBOLSIZE)
			p = tempbuf + MAXSYMBOLSIZE;
		*p++ = '\0';
		peekc = c;
		return(tokenofitem(UNDECLARED));
	case DIGIT:
		if (*ifptr) return(skipline(c));
		p = tempbuf;
		do	{
			if (p >= tempmax)
				error(symlong);
			*p++ = (c >= 'A' && c <= 'Z') ? c + 'a' - 'A' : c;
			while	((c = nextchar()) == '$');
			}
			while(numpart[c]);
		peekc = c;
		*p-- = '\0';
		switch(*p)	{
			case 'o':
			case 'q':
				radix = 8;
				limit = 020000;
				*p = '\0';
				break;
			case 'd':
				radix = 10;
				limit = 3276;
				*p = '\0';
				break;
			case 'h':
				radix = 16;
				limit = 010000;
				*p = '\0';
				break;
			case 'b':
				radix = 2;
				limit = 077777;
				*p = '\0';
				break;
			default:
				radix = 10;
				limit = 3276;
				p++;
				break;
			}

		/*
		 *  tempbuf now points to the number, null terminated
		 *  with radix 'radix'.
		 */
		yylval = 0;
		p = tempbuf;
		do	{
			c = *p - (*p > '9' ? ('a' - 10) : '0');
			if (c >= radix)
				{
				err[iflag]++;
				yylval = 0;
				break;
				}
			if (yylval < limit ||
				(radix == 10 && yylval == 3276 && c < 8) ||
				(radix == 2 && yylval == limit))
				yylval = yylval * radix + c;
			else {
				err[vflag]++;
				yylval = 0;
				break;
				}
			}
			while(*++p != '\0');
		return(NUMBER);
	default:
		if (*ifptr) return(skipline(c));
		switch(c)	{
		case ';':
			return(skipline(c));
		case '\'':
			if (quoteflag) return('\'');
			p = tempbuf;
			p[1] = 0;
			do	switch(c = nextchar())	{
			case '\0':
			case '\n':
				err[bflag]++;
				goto retstring;
			case '\'':
				if ((c = nextchar()) != '\'') {
				retstring:
					peekc = c;
					*p = '\0';
					if ((p-tempbuf) >2) {
						yylval = tempbuf;
						return(STRING);
					} else if (p-tempbuf == 2)	{
						p = tempbuf;
						yylval = *p++ | *p<<8;
						return(TWOCHAR);
					} else	{
						p = tempbuf;
						yylval = *p++;
						return(ONECHAR);
					}
				}
			default:
				*p++ = c;
			} while (p < tempmax);
			/*
			 *  if we break out here, our string is longer than
			 *  our input line
			 */
			error("string buffer overflow");
		default:
			return(c);
		}
	}
}

/*
 *  return the token associated with the string pointed to by
 *  tempbuf.  if no token is associated with the string, associate
 *  deftoken with the string and return deftoken.
 *  in either case, cause yylval to point to the relevant
 *  symbol table entry.
 */

tokenofitem(deftoken)
int	deftoken;
{
	register  char *p;
	register struct item *	ip;
	register  i;
	int  r, l, u, hash;


	/*
	 *  binary search
	 */
	l = 0;
	u = (sizeof keytab/sizeof keytab[0])-1;
	while (l <= u) {
		i = (l+u)/2;
		ip = &keytab[i];
		if ((r = cmp(tempbuf, ip->i_string)) == 0) goto found;
		if (r < 0) u = i-1;  else  l = i+1;
	}

	/*
	 *  hash into item table
	 */
	hash = 0;
	p = tempbuf;
	while (*p) hash =+ *p++;
	hash =% ITEMTABLESIZE;
	ip = &itemtab[hash];

	loop {
		if (ip->i_token == 0) break;
		if (cmp(tempbuf, ip->i_string) == 0) goto found;
		if (++ip >= itemmax) ip = itemtab;
	}

	if (!deftoken) return(0);

	if (++nitems > ITEMTABLESIZE-20)
		error("item table overflow");
	ip->i_string = alloc(size(tempbuf));
	ip->i_token = deftoken;

	copy(tempbuf, ip->i_string);

found:
	if (*ifptr) {
		if (ip->i_token == ENDIF) return(ENDIF);
		if (ip->i_token == IF) {
			if (ifptr >= ifstmax)
				error("Too many ifs");
			else *++ifptr = 1;
		}
		return(skipline(' '));
	}
	yylval = ip;
	return(ip->i_token);
	}

/*
 *  compare two character strings.  return -1 for a<b, zero for equal
 *  and 1 for b>a
 */
cmp(a, b)
{
	register  char *sa, *sb;

	sa = a;
	sb = b;
	while (*sa == *sb++)
		if (*sa++ == '\0') return(0);
	if (*sa < *--sb) return(-1);
	return(1);
}



/*
 *  interchange two entries in the item table -- used by qsort
 */
interchange(i, j)
{
	register  struct item *fp, *tp;
	struct item temp;

	fp = &itemtab[i];
	tp = &itemtab[j];
	temp.i_string = fp->i_string;
	temp.i_value = fp->i_value;
	temp.i_token = fp->i_token;

	fp->i_string = tp->i_string;
	fp->i_value = tp->i_value;
	fp->i_token = tp->i_token;

	tp->i_string = temp.i_string;
	tp->i_value = temp.i_value;
	tp->i_token = temp.i_token;
}



/*
 *  quick sort -- used by putsymtab to sort the symbol table
 */
qsort(m, n)
{
	register  i, j;

	if (m < n) {
		i = m;
		j = n+1;
		loop {
			do i++; while(cmp(itemtab[i].i_string,
					itemtab[m].i_string) < 0);
			do j--; while(cmp(itemtab[j].i_string,
					itemtab[m].i_string) > 0);
			if (i < j) interchange(i, j); else break;
		}
		interchange(m, j);
		qsort(m, j-1);
		qsort(j+1, n);
	}
}



/*
 *  get the next character
 */
nextchar()
{
	register  c;
	int  ch;
	static  char  *earg;

	if (peekc != -1) {
		c = peekc;
		peekc = -1;
		return(c);
	}

start:
	if (earg) {
		if (*earg) return(addtoline(*earg++));
		earg = 0;
	}

	if (expptr) {
		if ((ch = getm()) == '\1') {	/*  expand argument  */
			ch = getm() - 'A';
			if (ch >= 0 && ch < PARMMAX && est[ch])
				earg = est[ch];
			goto start;
		}
		if (ch == '\2') {	/*  local symbol  */
			ch = getm() - 'A';
			if (ch >= 0 && ch < PARMMAX && est[ch]) {
				earg = est[ch];
				goto start;
			}
			earg = getlocal(ch, est[TEMPNUM]);
			goto start;
		}

		return(addtoline(ch));
	}

	return(addtoline(getchar()));
}


/*
 *  skip to rest of the line -- comments and if skipped lines
 */
skipline(ac)
{
	register  c;

	c = ac;
	while (c != '\n' && c != '\0') c = nextchar();
	return('\n');
}



main(argc, argv)
char  **argv;
{
	register  struct item *ip;
	register  i;
	int  files;
	int  save;
#ifdef DBUG
	extern  yydebug;
#endif

	for (i=3; i<11; i++) close(i);
	files = 0;

	for (i=1; i<argc; i++) {
		if (*argv[i] == '-') while (*++argv[i]) switch(*argv[i]) {

		case 'b':	/*  no binary  */
			bopt = 0;
			continue;

#ifdef DBUG
		case 'd':	/*  debug  */
			yydebug++;
			continue;
#endif

		case 'e':	/*  error list only  */
			eopt = 0;
			edef = 0;
			continue;

		case 'f':	/*  print if skipped lines  */
			fopt++;
			fdef++;
			continue;

		case 'g':	/*  do not list extra code  */
			gopt = 0;
			gdef = 0;
			continue;

		case 'l':	/*  no list  */
			lopt++;
			continue;

		case 'm':	/*  print macro expansions  */
			mdef++;
			mopt++;
			continue;

		case 'n':	/*  put line numbers on  */
			nopt++;
			continue;

		case 'o':	/*  list to standard output  */
			oopt++;
			continue;

		case 'p':	/*  put out VT for eject */
			popt++;
			continue;

		case 's':	/*  produce a symbol list  */
			sopt++;
			continue;

		case 't':
			topt = 0;
			continue;
		default:	/*  error  */
			error("Unknown option");

		} else if (files++ == 0) {
			sourcef = argv[i];
			copy(sourcef,src);
			suffix(src,".z");
			if (fopen(src, fin) < 0)
				error("Cannot open source file");
		} else if (files)
			error("Too many arguments");
	}


	if (files == 0) error("No source file");
	copy(sourcef,bin);
	suffix(bin,".hex");
	if (bopt)
		if (fcreat(bin, fbuf) < 0)
			error("Cannot create binary file");
	if (!lopt && !oopt) {
		copy(sourcef,listf);
		suffix(listf,".lst");
		if ((fout[0] = creat(listf, 0604)) < 0)
			error("Cannot create list file");
	} else
		fout[0] = dup(1);
	copy(sourcef,mtmp);
	suffix(mtmp,".tmp");
	if ((mfileo = creat(mtmp, 0644)) < 0)
		error("Cannot create temp file");
	mfile = open(mtmp, 0);
	unlink(mtmp);

	/*
	 *  get the time
	 */
	time(now);
	timp = ctime(now);
	timp[16] = 0;
	timp[24] = 0;

	title = sourcef;
	setvars();
	yyparse();
	pass2++;
	ip = &itemtab[-1];
	while (++ip < itemmax)
		switch	(ip->i_token) {

			case MNAME:
				ip->i_token = OLDMNAME;
				break;

			case EQUATED:
				ip->i_token = WASEQUATED;
				break;

			case DEFLED:
				ip->i_token = UNDECLARED;
				break;
			}
	setvars();
	seek(fin[0], 0, 0);

	yyparse();


	if (bopt) {
		flushbin();
		putc(':',fbuf);
		if (xeq_flag) {
			puthex(0,fbuf);
			puthex(xeq >> 8, fbuf);
			puthex(xeq,fbuf);
			puthex(1,fbuf);
			puthex(255-(xeq >> 8)-xeq, fbuf);
		} else
			for	(i = 0; i < 10; i++)
				putc('0',fbuf);
		putc('\n',fbuf);
		fflush(fbuf);
	}

	if (!lopt) fflush(fout);
	if (writesyms) outsymtab(writesyms);

	if (eopt) erreport();
	if (!lopt && !sopt) putsymtab();
	if (!lopt) {
		eject();
		fflush(fout);
	}
	exit(0);
}


/*
 *  set some data values before each pass
 */
setvars()
{
	register  i;

	peekc = -1;
	linein = linecnt = 0;
	exp_number = 0;
	emitptr = emitbuf;
	lineptr = linebuf;
	ifptr = ifstack;
	expifp = expif;
	*ifptr = 0;
	dollarsign = 0;
	olddollar = 0;
	for (i=0; i<FLAGS; i++) err[i] = 0;
}



/*
 *  print out an error message and die
 */
error(as)
char *	as;
{
	register  char *sp;

	*linemax = 0;
	printf("%s\n", linebuf);
	fflush(fout);
	sp = as;
	while (*sp++);
	write(2, as, sp-as);
	write(2, "\n", 1);
	exit(1);
}



/*
 *  output the symbol table
 */
putsymtab()
{
	register  struct item *tp, *fp;
	int  i, j, k, t, rows;

	if (!nitems) return;
	tp = &itemtab[-1];
	for (fp = itemtab; fp<itemmax; fp++) {
		if (fp->i_token == UNDECLARED) {
			nitems--;
			continue;
		}
		if (fp->i_token == 0) continue;
		tp++;
		if (tp != fp) {
			tp->i_string = fp->i_string;
			tp->i_value = fp->i_value;
			tp->i_token = fp->i_token;
		}
	}

	tp++;
	tp->i_string = "{";

	/*
	 *  sort the table
	 */
	qsort(0, nitems-1);

	title = "**  Symbol Table  **";
	rows = (nitems+3) / 4;
	if (rows+5+line > 60) eject();
	lineout();
	printf("\n\n\nSymbol Table:\n\n");
	line =+ 4;

	for (i=0; i<rows; i++) {
		for(j=0; j<4; j++) {
			k = rows*j+i;
			if (k < nitems) {
				tp = &itemtab[k];
				t = tp->i_token;
				printf("%-7s%c%4x     ", tp->i_string,
				(t==EQUATED || t==DEFLED)?'=':' ',
				tp->i_value);
			}
		}
		lineout();
		putchar('\n');
	}
}



/*
 *  put out error report
 */
erreport()
{
	register i, numerr;

	if (line > 50) eject();
	lineout();
	numerr = 0;
	for (i=0; i<FLAGS; i++) numerr =+ keeperr[i];
	if (numerr) {
		printf("\n\n\nError report:\n\n");
		printf("%6d errors\n", numerr);
		line =+ 5;
	} else {
		printf("\n\n\nStatistics:\n");
		line =+ 3;
	}

	for (i=0; i<FLAGS; i++)
		if (keeperr[i]) {
			lineout();
			printf("%6d %c -- %s error\n",
				keeperr[i], errlet[i], errname[i]);
		}

	if (line > 55) eject();
	lineout();
	printf("\n%6d\tsymbols\n", nitems);
	printf("%6d\tbytes\n", nbytes);
	line =+ 2;
	if (mfptr) {
		if (line > 53) eject();
		lineout();
		printf("\n%6d\tmacro calls\n", exp_number);
		printf("%6d\tmacro bytes\n", mfptr);
		printf("%6d\tinvented symbols\n", invented/2);
		line =+ 3;
	}
}


/*
 *  lexical analyser for macro definition
 */
mlex(nam)
{
	register  char  *p;
	register  c;
	int  t;

	inmlex++;

	/*
	 *  move text onto macro file, changing formal parameters
	 */
	c = nextchar();
loop {
	switch(charclass[c]) {

	case DIGIT:
		while (numpart[c]) {
			putm(c);
			c = nextchar();
		}
		continue;

	case STARTER:
	case LETTER:
		t = 0;
		p = tempbuf+MAXSYMBOLSIZE+2;
		do {
			if (p >= tempmax)
				error(symlong);
			*p++ = c;
			if (t < MAXSYMBOLSIZE)
				tempbuf[t++] = (c >= 'A' && c <= 'Z')  ?
					c+'a'-'A' : c;
			c = nextchar();
		} while (charclass[c]==LETTER || charclass[c]==DIGIT);

		tempbuf[t] = 0;
		*p++ = '\0';
		p = tempbuf+MAXSYMBOLSIZE+2;
		t = tokenofitem(0);
		if (t != MPARM) while (*p) putm(*p++);
		else {
			if (*yylval->i_string == '?') putm('\2');
			else putm('\1');
			putm(yylval->i_value + 'A');
		}
		if (t == ENDM) goto done;
		continue;

	case EOF:
		if (expptr) {
			popsi();
			c = nextchar();
			continue;
		}

		goto done;

	default:
		if (c == '\n') {
			linein++;
			linecnt++;
		}
		if (c != '\1') putm(c);
		c = nextchar();
	}
}

	/*
	 *  finish off the file entry
	 */
done:
	while(c != '\n' && c != '\0') c = nextchar();
	linein++;
	linecnt++;
	putm('\n');
	putm('\n');
	putm(0);

	for (c=0; c<ITEMTABLESIZE; c++)
		if (itemtab[c].i_token == MPARM) {
			itemtab[c].i_token = UNDECLARED;
		}
	inmlex = 0;
}



/*
 *  lexical analyser for the arguments of a macro call
 */
getarg()
{
	register  char  c;
	register  char  *p;
	static  comma;

	*tempbuf = 0;
	yylval = tempbuf;
	while(charclass[c = nextchar()] == SPACE);

	switch(c) {

	case '\0':
		popsi();
	case '\n':
	case ';':
		comma = 0;
		return(skipline(c));

	case ',':
		if (comma) {
			comma = 0;
			return(',');
		}
		else {
			comma++;
			return(ARG);
		}

	case '\'':
		p = tempbuf;
		do switch (c = nextchar()) {
			case '\0':
			case '\n':
				peekc = c;
				*p = 0;
				err[bflag]++;
				return(ARG);
			case '\'':
				if ((c = nextchar()) != '\'') {
					peekc = c;
					*p = '\0';
					comma++;
					return(ARG);
				}
			default:
				*p++ = c;
		} while (p < tempmax);
		error(symlong);

	default:  /* unquoted string */
		p = tempbuf;
		peekc = c;
		do switch(c = nextchar()) {
			case '\0':
			case '\n':
			case '\t':
			case ' ':
			case ',':
				peekc = c;
				*p = '\0';
				comma++;
				return(ARG);
			default:
				*p++ = c;
		} while (p < tempmax);
	}
}



/*
 *  copy a string
 */
copy(st1, st2)
{
	register  char  *s1, *s2;

	s1 = st1;
	s2 = st2;
	while (*s2++ = *s1++);
}


/*
 *  add a suffix to a string
 */
suffix(str,suff)
char *str,*suff;
{
	while(*str != '\0' && *str != '.')
		*str++;
	copy(suff,str);
}


/*
 *  find the size of a string
 */
size(st)
{
	register  char *sp;

	sp = st;
	while (*sp++);
	return((sp-st) + 1);
}



/*
 *  put out a byte to the macro file, keeping the offset
 */
putm(c)
{
	mfptr++;
	write(mfileo, &c, 1);
}



/*
 *  get a byte from the macro file
 */
getm()
{
	int  ch;

	floc++;
	ch = 0;
	if (read(mfile, &ch, 1) < 0) return(0);
	return(ch);
}



/*
 *  pop standard input
 */
popsi()
{
	register  i;

	for (i=0; i<PARMMAX; i++) {
		if (est[i]) free(est[i]);
	}
	floc = est[FLOC];
	free(est);
	expptr--;
	est = expptr? expstack[expptr-1] : 0;
	seek(mfile, floc, 0);
	if (lineptr > linebuf) lineptr--;
}



/*
 *  return a unique name for a local symbol
 */
getlocal(c, n)
{
	char  *st;
	int   i;

	invented++;
	st = "?A0000";
	st[1] = c+'a';
	for (i=2; i<6; i++) st[i] = (n << ((5-i)*3)&07) + '0';
	if (c >= 26) st[2] =+ 'a' - '0';
	return(st);
}



/*
 *  read in a symbol table
 */
insymtab(name)
{
	register struct stab *t;
	int  s, i, sfile;

	t = tempbuf;
	if ((sfile = open(name, 0)) < 0) return(1);
	read(sfile, t, sizeof *t);
	if (t->t_value != SYMMAJIC) return(1);

	s = t->t_token;
	for (i=0; i<s; i++) {
		read(sfile, t, sizeof *t);
		if (tokenofitem(UNDECLARED) != UNDECLARED)
			continue;
		yylval->i_token = t->t_token;
		yylval->i_value = t->t_value;
		if (t->t_token == MACRO)
			yylval->i_value =+ mfptr;
	}

	while ((s = read(sfile, tempbuf, TEMPBUFSIZE)) > 0) {
		write(mfileo, tempbuf, s);
		mfptr =+ s;
	}
	return(0);
}



/*
 *  write out symbol table
 */
outsymtab(name)
{
	register struct stab *t;
	register struct item *ip;
	int  i, sfile;

	t = tempbuf;
	if ((sfile = creat(name, 0644)) < 0) return(1);
	for (ip=itemtab; ip<itemmax; ip++) {
		if (ip->i_token == UNDECLARED) {
			ip->i_token = 0;
			nitems--;
		}
	}

	copyname(title, t);
	t->t_value = SYMMAJIC;
	t->t_token = nitems;
	write(sfile, t, sizeof *t);

	for (ip=itemtab; ip<itemmax; ip++) {
		if (ip->i_token != 0) {
			t->t_token = ip->i_token;
			t->t_value = ip->i_value;
			copyname(ip->i_string, t);
			write(sfile, t, sizeof *t);
		}
	}

	seek(mfile, 0, 0);
	while((i = read(mfile, tempbuf, TEMPBUFSIZE)) > 0)
		write(sfile, tempbuf, i);
	return(0);
}



/*
 *  copy a name into the symbol file
 */
copyname(st1, st2)
{
	register  char  *s1, *s2;
	register  i;

	i = (MAXSYMBOLSIZE+2) & ~01;
	s1 = st1;
	s2 = st2;

	while(*s2++ = *s1++) i--;
	while(--i > 0) *s2++ = '\0';
}
