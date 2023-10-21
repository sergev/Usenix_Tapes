%{
#define ITEMTABLESIZE	600
#define NAMETABLESIZE	5000
#define LINEBUFFERSIZE	200
#define EMITBUFFERSIZE	200
#define STRINGBUFSIZE	200

#define check(a,b)	if(a){printf("b\n");a = 0;}

yyerror(){}		/*we will do our own error printing*/

struct	item	{
	char *	i_string;
	int	i_value;
	int	i_token;
	};

extern int	fin[259];
extern int	fout[259];
int		fbuf[259];

char *	progname;	/*name by which program was invoked*/

char *	globalp;	/*used by parser routines*/

int	pass1done;	/*set when pass one completed*/

char *	dollarsign;	/*location counter*/

int	rflag;		/*set on register usage error*/

int	eflag;		/*set on expression error*/

int	pflag;		/*set on phase error*/

int	uflag;		/*set when undeclared appears in expr.*/

int	mflag;		/*set when multiply defined occurs*/

int	vflag;		/*set when value out of bounds*/

int	bflag;		/*set on balance error*/

int	fflag;		/*set on statement format error*/

int	iflag;		/*set on bad digits*/

char	linebuf[LINEBUFFERSIZE];
char *	lineptr;

char	emitbuf[EMITBUFFERSIZE];
char *	emitptr;

addtoline(ac)
int	ac;
	{
	if	(lineptr >= &linebuf[LINEBUFFERSIZE])
		error("line buffer overflow");
	*lineptr++ = ac;
	return(ac);
	}

emit(args)
int	args;
	{
	register int	i;
	register int *	argp;

	i = nargs();
	argp = &args;
	while	(--i >= 0)
		if	(emitptr >= &emitbuf[EMITBUFFERSIZE])
			error("emit buffer overflow");
		else	{
			*emitptr++ = *argp;
			argp++;
/*
	. . .which we all know is equivalent to
		*emitptr++ = *argp++;
	but trying telling that to the C compiler.
*/
			}
	}

puthex(byte,buf)
char	byte;
int *	buf;
	{
	putc("0123456789ABCDEF"[(byte >> 4) & 017],buf);
	putc("0123456789ABCDEF"[byte & 017],buf);
	}

list(optarg)
int	optarg;
	{
	register char *	p;
	register int	i;
	register char	checksum;

	if	(pass1done)
		{
		addtoline('\0');
		if	(checksum = emitptr - emitbuf)
			{
			putc(':',fbuf);
			puthex(checksum,fbuf);
			puthex(dollarsign >> 8,fbuf);
			puthex(dollarsign,fbuf);
			puthex(0,fbuf);
			checksum =+ (dollarsign >> 8) + dollarsign;
			}
		i = (nargs() == 0) ? dollarsign : optarg;
		puthex(i >> 8,fout);
		puthex(i,fout);
		printf("  ");
		for	(p = emitbuf; (p < emitptr) && (p - emitbuf < 4); p++)
			{
			puthex(*p,fbuf);
			puthex(*p,fout);
			checksum =+ *p;
			}
		for	(i = 4 - (p-emitbuf); i > 0; i--)
			printf("  ");
		printf("  ");
		printf("%s",linebuf);
		while	(p < emitptr)
			{
			printf("      ");
			for	(i = 0; (i < 4) && (p < emitptr);i++)
				{
				puthex(*p,fbuf);
				puthex(*p,fout);
				checksum =+ *p;
				p++;
				}
			printf("\n");
			}
		if	(emitptr - emitbuf)
			{
			puthex(256 - checksum,fbuf);
			putc('\n',fbuf);
			}
		check(rflag,R);
		check(eflag,E);
		check(uflag,U);
		check(pflag,P);
		check(mflag,M);
		check(vflag,V);
		check(bflag,B);
		check(fflag,F);
		check(iflag,I);
		fflush(fout);	/*to avoid putc(har) mix bug*/
		}
	dollarsign =+ emitptr - emitbuf;
	emitptr = emitbuf;
	lineptr = linebuf;
	}

%}
%token STRING
%token NOOPERANDOP
%token DESTOP
%token LOGDATAOP
%token ARIDATAOP
%token EVENREGOP
%token DATA16OP
%token MOV
%token MVI
%token LXI
%token SOURCEOP
%token STAXLDAX
%token NUMBER
%token UNDECLARED
%token END
%token ORG
%token DB
%token DS
%token DW
%token EQU
%token SET
%token LABEL
%token EQUATED
%token WASEQUATED
%token SETED
%token MOD
%token AND
%token OR
%token XOR
%token SHL
%token SHR
%token NOT
%left OR XOR
%left AND
%nonassoc NOT
%left '+' '-'
%left '*' '/' MOD SHL SHR
%%
program:
	statements end.statement
|
	error
	=	{error("first line bad/bad end statement--assembly aborted");}
;
statements:
	statement
|
	statements statement
|
	statements error
	=	{
		fflag++;
		while(yychar != '\n' && yychar != '\0') yychar = yylex();
		list();
		yyclearin;yyerrok;
		}
;
end.statement:
	label.part END '\n'
	=	{list();}
;
statement:
	'\n'
	=	{
		addtoline('\0');
		if	(pass1done)	printf("\t\t%s",linebuf);
/*
		note that the listing must be done here because
		if 'list' were called it would insist on
		prepending the location counter to the listed line,
		which is a no-no when a blank (or comment-only)
		line is to be listed.
*/
		lineptr = linebuf;
		}
|
	label.part operation '\n'
	=	{list();}
|
	equatable EQU expression '\n'
	=	{
		$1->i_token = EQUATED;
		$1->i_value = $3;
		list($3);
		}
|
	UNDECLARED SET expression '\n'
	=	{setit:
		$1->i_token = SETED;
		$1->i_value = $3;
		list($3);
		}
|
	SETED SET expression '\n'
	=	{goto setit;}
|
	label.part DS expression '\n'
	=	{
		if	(dollarsign + $3 < dollarsign) eflag++;
		list();
		if	(dollarsign + $3 >= dollarsign) dollarsign =+ $3;
		}
;
label.part:
	/*empty*/
|
	UNDECLARED ':'
	=	{
		if	(pass1done)
			pflag++;
		else	{
			$1->i_token = LABEL;
			$1->i_value = dollarsign;
			}
		}
|
	LABEL ':'
	=	{
		if	(!pass1done)
			mflag++;
		else	if	($1->i_value != dollarsign)
				pflag++;
		}
;
operation:
	/*empty*/
|
	NOOPERANDOP
	=	{emit($1->i_value);}
|
	DATA16OP expression
	=	{
		emit($1->i_value,$2,$2>>8);
		}
|
	LOGDATAOP logdatavalue
	=	{emit($1->i_value,$2);}
|
	ARIDATAOP aridatavalue
	=	{emit($1->i_value,$2);}
|
	EVENREGOP evenregvalue
	=	{emit($1->i_value | ($2 << 3));}
|
	MOV regvalue ',' regvalue
	=	{if	($2 == 6 && $4 == 6)
			rflag++;
		emit($1->i_value | ($2 << 3) | $4);}
|
	MVI regvalue ',' logdatavalue
	=	{emit($1->i_value | ($2 << 3),$4);}
|
	LXI evenregvalue ',' expression
	=	{emit($1->i_value | ($2 << 3),$4,$4 >> 8);}
|
	SOURCEOP regvalue
	=	{emit($1->i_value | $2);}
|
	DESTOP regvalue
	=	{emit($1->i_value | ($2 << 3));}
|
	ORG expression
	=	{dollarsign = $2;}
|
	STAXLDAX expression
	=	{
		if	($2 != 0 && $2 != 2)	rflag++;
		emit($1->i_value | (($2 & 02) << 3));
		}
|
	DB db.list
|
	DW dw.list
;
logdatavalue:
	expression
	=	{
		if	(($1 < 0) || ($1 > 0377))
			vflag++;
		$$ = $1 & 0377;
		}
;
aridatavalue:
	expression
	=	{
		if	(($1 < -256) || ($1 > 255))
		/*(to conform with the Burroughs)*/
			vflag++;
		$$ = $1 & 0377;
		}
;
evenregvalue:
	expression
	=	{
		switch($1)	{
			default:
				rflag++;
			case 0:
			case 2:
			case 4:
			case 6:
				break;
			}
		$$ = $1 & 06;
		}
;
regvalue:
	expression
	=	{
		if	(($1 & ~07) != 0)
			rflag++;
		$$ = $1 & 07;
		}
;
db.list:
	db.list.element
|
	db.list ',' db.list.element
;
db.list.element:
	expression
	=	{
		if	($1 < -128 || $1 > 255)
			eflag++;
		emit($1 & 0377);
		}
|
	STRING
	=	{
		globalp = $1;
		while	(*globalp != '\0')	emit(*globalp++);
		}
;
dw.list:
	dw.list.element
|
	dw.list ',' dw.list.element
;
dw.list.element:
	expression
	=	{emit($1,$1 >> 8);}
;
expression:
	error
	=	{
		eflag++;
		$$ = 0;
		}
|
	LABEL
	=	{$$ = $1->i_value;}
|
	NUMBER
|
	EQUATED
	=	{$$ = $1->i_value;}
|
	WASEQUATED
	=	{$$ = $1->i_value;}
|
	SETED
	=	{$$ = $1->i_value;}
|
	'$'
	=	{$$ = dollarsign;}
|
	UNDECLARED
	=	{
		uflag++;
		$$ = 0;
		}
|
	expression '+' expression
	=	{$$ = $1 + $3;}
|
	expression '-' expression
	=	{$$ = $1 - $3;}
|
	expression '/' expression
	=	{$$ = $1 / $3;}
|
	expression '*' expression
	=	{$$ = $1 * $3;}
|
	expression MOD expression
	=	{$$ = $1 % $3;}
|
	expression AND expression
	=	{$$ = $1 & $3;}
|
	expression OR expression
	=	{$$ = $1 | $3;}
|
	expression XOR expression
	=	{$$ = $1 ^ $3;}
|
	expression SHL expression
	=	{$$ = $1 << $3;}
|
	expression SHR expression
	=	{$$ = (($1 >> 1) & 077777) >> ($3 - 1);}
|
	'(' expression ')'
	=	{$$ = $2;}
|
	NOT expression
	=	{$$ = ~$2;}
|
	'+' expression
|
	'-' expression
	=	{$$ = -$2;}
|
	STRING
	=	{
		globalp = $1;
		if	(globalp[1] != '\0' && globalp[2] != '\0')
			vflag++;
		$$ = globalp[0] | (globalp[1] << 8);
		}
;
equatable:
	UNDECLARED
|
	WASEQUATED
;
%%
extern int	yylval;

#define EOF	0
#define OTHER	1
#define SPACE	2
#define DIGIT	3
#define LETTER	4
#define STARTER 5

char	charclass[]	{
	EOF,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,
	OTHER,	SPACE,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,
	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,
	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,
	SPACE,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,
	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,
	DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,
	DIGIT,	DIGIT,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	STARTER,
	STARTER,LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER,
	LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER,
	LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER,
	LETTER, LETTER, LETTER, OTHER,  OTHER,  OTHER,  OTHER,  LETTER,
	OTHER,	LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER,
	LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER,
	LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER,
	LETTER, LETTER, LETTER, OTHER,	OTHER,	OTHER,	OTHER,	OTHER,
	0};

/*
	the following table tells which characters are parts of numbers.
	the entry for each such character is a non-zero value.
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
	the following table is a list of assembler mnemonics;
	for each mnemonic the associated machine-code bit pattern
	and symbol type are given.
*/

struct	item	keytab[]	{
"b",	0,	EQUATED,
"c",	1,	EQUATED,
"d",	2,	EQUATED,
"e",	3,	EQUATED,
"h",	4,	EQUATED,
"l",	5,	EQUATED,
"m",	6,	EQUATED,
"a",	7,	EQUATED,
"sp",	6,	EQUATED,
"psw",	6,	EQUATED,
"call", 0315,	DATA16OP,
"cc",	0334,	DATA16OP,
"cnc",	0324,	DATA16OP,
"cz",	0314,	DATA16OP,
"cnz",	0304,	DATA16OP,
"cp",	0364,	DATA16OP,
"cm",	0374,	DATA16OP,
"cpe",	0354,	DATA16OP,
"cpo",	0344,	DATA16OP,
"ret",	0311,	NOOPERANDOP,
"rc",	0330,	NOOPERANDOP,
"rnc",	0320,	NOOPERANDOP,
"rz",	0310,	NOOPERANDOP,
"rnz",	0300,	NOOPERANDOP,
"rp",	0360,	NOOPERANDOP,
"rm",	0370,	NOOPERANDOP,
"rpe",	0350,	NOOPERANDOP,
"rpo",	0340,	NOOPERANDOP,
"rst",	0307,	DESTOP,
"in",	0333,	LOGDATAOP,
"out",	0323,	LOGDATAOP,
"lxi",	0001,	LXI,
"push", 0305,	EVENREGOP,
"pop",	0301,	EVENREGOP,
"sta",	0062,	DATA16OP,
"lda",	0072,	DATA16OP,
"xchg", 0353,	NOOPERANDOP,
"xthl", 0343,	NOOPERANDOP,
"sphl", 0371,	NOOPERANDOP,
"pchl", 0351,	NOOPERANDOP,
"dad",	0011,	EVENREGOP,
"stax", 0002,	STAXLDAX,
"ldax", 0012,	STAXLDAX,
"inx",	0003,	EVENREGOP,
"mov",	0100,	MOV,
"hlt",	0166,	NOOPERANDOP,
"mvi",	0006,	MVI,
"inr",	0004,	DESTOP,
"dcr",	0005,	DESTOP,
"add",	0200,	SOURCEOP,
"adc",	0210,	SOURCEOP,
"sub",	0220,	SOURCEOP,
"sbb",	0230,	SOURCEOP,
"ana",	0240,	SOURCEOP,
"xra",	0250,	SOURCEOP,
"ora",	0260,	SOURCEOP,
"cmp",	0270,	SOURCEOP,
"adi",	0306,	ARIDATAOP,
"aci",	0316,	ARIDATAOP,
"sui",	0326,	ARIDATAOP,
"sbi",	0336,	ARIDATAOP,
"ani",	0346,	LOGDATAOP,
"xri",	0356,	LOGDATAOP,
"ori",	0366,	LOGDATAOP,
"cpi",	0376,	ARIDATAOP,
"rlc",	0007,	NOOPERANDOP,
"rrc",	0017,	NOOPERANDOP,
"ral",	0027,	NOOPERANDOP,
"rar",	0037,	NOOPERANDOP,
"jmp",	0303,	DATA16OP,
"jc",	0332,	DATA16OP,
"jnc",	0322,	DATA16OP,
"jz",	0312,	DATA16OP,
"jnz",	0302,	DATA16OP,
"jp",	0362,	DATA16OP,
"jm",	0372,	DATA16OP,
"jpe",	0352,	DATA16OP,
"jpo",	0342,	DATA16OP,
"dcx",	0013,	EVENREGOP,
"cma",	0057,	NOOPERANDOP,
"stc",	0067,	NOOPERANDOP,
"cmc",	0077,	NOOPERANDOP,
"daa",	0047,	NOOPERANDOP,
"shld", 0042,	DATA16OP,
"lhld", 0052,	DATA16OP,
"ei",	0373,	NOOPERANDOP,
"di",	0363,	NOOPERANDOP,
"nop",	0000,	NOOPERANDOP,
"end",	0,	END,
"org",	0,	ORG,
"db",	0,	DB,
"ds",	0,	DS,
"dw",	0,	DW,
"equ",	0,	EQU,
"set",	0,	SET,
"mod",	0,	MOD,
"and",	0,	AND,
"or",	0,	OR,
"xor",	0,	XOR,
"shl",	0,	SHL,
"shr",	0,	SHR,
"not",	0,	NOT,
0,	0,	0};

/*
	user-defined items are tabulated in the following table.
*/

struct item	itemtab[ITEMTABLESIZE];

/*
	the above table references strings which are stored in the
	following array.  nameptr points to the next available location
	within the array.
*/

char	nametab[NAMETABLESIZE];
char *	nameptr;

/*
	strings encountered in the input are stored below.
*/

char	stringbuf[STRINGBUFSIZE];

/*
	ye olde pushed-back scanner symbol.
*/

char	peekc;

yylex()
{
register char	c;
register char *	p;
register int	radix;
int		limit;

while(1) switch(charclass[c = nextchar()])
	{
	case EOF:
		return(0);
	case SPACE:
		break;
	case LETTER:
	case STARTER:
		p = nameptr;
		do	{
			if	(p >= &nametab[NAMETABLESIZE])
				error("name table overflow");
/************************************************************************/
/*	The next line was changed 4/10/78 by G. ORDY, CWRU from:	*/
/*			*p++ = (c >= 'A' && c <= 'Z') ? c+'a'-'A' : c ; */
/*	This change prohibits upper case ASCII letters from being	*/
/*	changed to lower case ASCII. Also, the underline was added as	*/
/*	a LETTER symbol (see charclass[]).				*/
/************************************************************************/
			*p++ = c;
			while	((c = nextchar()) == '$');
			}
			while	(charclass[c]==LETTER||charclass[c]==DIGIT);
		if	(p - nameptr > 8)
			p = nameptr + 8;
		if	(p >= &nametab[NAMETABLESIZE])
			error("name table overflow");
		*p++ = '\0';
		peekc = c;
		return(tokenofitem(UNDECLARED));
	case DIGIT:
		p = nameptr;
		do	{
			if	(p >= &nametab[NAMETABLESIZE])
				error("name table overflow");
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
				limit = 0100000;
				*p = '\0';
				break;
			default:
				radix = 10;
				limit = 3276;
				p++;
				break;
			}
/*nameptr now points to the number, terminated by a null to which p points*/
/*radix has been set to the radix*/
		yylval = 0;
		p = nameptr;
		do	{
			c = *p - (*p > '9' ? ('a' - 10) : '0');
			if	(c >= radix)
				{
				iflag++;
				yylval = 0;
				break;
				}
			if	(yylval < limit ||
				(radix == 10 && yylval == 3276 && c < 8))
				yylval = yylval * radix + c;
			else	{
				vflag++;
				yylval = 0;
				break;
				}
			}
			while(*++p != '\0');
		return(NUMBER);
	default:
		switch(c)	{
		case ';':
			while	((c = nextchar()) != '\n' && c != '\0');
			return(c);
		case '\'':
			p = stringbuf;
			do	switch(c = nextchar())	{
			case '\0':
			case '\n':
				peekc = c;
				*p = '\0';
				bflag++;
				yylval = stringbuf;
				return(STRING);
			case '\'':
				if	((c = nextchar()) != '\'')
					{
					peekc = c;
					*p = '\0';
					if	(p == stringbuf)
						fflag++;
					yylval = stringbuf;
					return(STRING);
					}
			default:
				*p++ = c;
			}
				while	(p < &stringbuf[STRINGBUFSIZE]);
/* if we ever break out, we have overflowed the bounds of good taste*/
			error("string buffer overflow");
		default:
			return(c);
		}
	}
}

/*
	return the token associated with the string pointed to by
	nameptr.  if no token is associated with the string, associate
	deftoken with the string and return deftoken.
	in either case, cause yylval to point to the relevant
	symbol table entry.
*/

tokenofitem(deftoken)
int	deftoken;
	{
	register char *		p;
	register char *		q;
	register struct item *	ip;

	ip = &keytab[-1];
	while	((++ip)->i_token != 0)
		{
		p = nameptr;
		q = ip->i_string;
		while	(*p == *q++)
			if	(*p++ == '\0')
				goto found;
		}
	ip = &itemtab[-1];
	while	((++ip)->i_token != 0)
		{
		p = nameptr;
		q = ip->i_string;
		while	(*p == *q++)
			if	(*p++ == '\0')
				goto found;
		}
	if	(ip >= &itemtab[ITEMTABLESIZE - 1])
		error("item table overflow");
/*
	the -1 is necessary to insure that an empty item
	(which is used to terminate the search)
	is left at the end of the table.
*/
	ip->i_string = nameptr;
	ip->i_token = deftoken;
	while	(*nameptr++);	/*update past used*/
found:
	yylval = ip;
	return(ip->i_token);
	}

nextchar()
	{
	register char c;

	if	(peekc != -1)
		{
		c = peekc;
		peekc = -1;
		return(c);
		}
	else	return(addtoline(getchar()));
	}

main(argc,argv)
int	argc;
char *	argv[];
	{
	register struct item *	ip;
	register int		i;

	progname = argv[0];
	if	(argc != 2)
		error("arg count");
	if	(fopen(argv[1],fin) != 0)
		error("can't open file");
	if	(fcreat("a.out",fbuf) < 0)
		error("can't create a.out");
	fout[0] = dup(1);
	nameptr = nametab;
	setvars();
	yyparse();
	pass1done++;
	ip = &itemtab[-1];
	while	((++ip)->i_token != 0)
		switch	(ip->i_token)
			{
			case EQUATED:
				ip->i_token = WASEQUATED;
				break;
			case SETED:
				ip->i_token = UNDECLARED;
				break;
			}
	close(fin[0]);
	if	(fopen(argv[1],fin) != 0)
		error("can't reopen file");
	setvars();
	yyparse();
	putc(':',fbuf);
	for	(i = 0; i < 10; i++)
		putc('0',fbuf);
	putc('\n',fbuf);
	fflush(fbuf);
	fflush(fout);
	exit(0);
	}

setvars()
	{
	peekc = -1;
	emitptr = emitbuf;
	lineptr = linebuf;
	dollarsign = 0;
	rflag = 0;
	eflag = 0;
	mflag = 0;
	uflag = 0;
	pflag = 0;
	vflag = 0;
	bflag = 0;
	fflag = 0;
	iflag = 0;
	}

error(as)
char *	as;
	{
	extern char *	progname;
	extern char *	sys_errlist[];
	extern int	errno;
	register int	saved;

	saved = errno;
	sys_errlist[errno = 0] = as;
	flush();
	perror(progname);
	exit(saved ? saved : -1);
	}
