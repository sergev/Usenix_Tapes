#
/*
 *	C pass 2 header
 */

#define	swsiz	200
#define	ossiz	500

struct	tnode {
	int	op;	/* used to index into opdope table for info	*/
	int	type;	/* operator type - FLOAT, DOUBLE, LONG, INT, CHAR..	*/
	int	degree;
	struct	tnode *tr1, *tr2;
};

struct	bnode {
	int	bop;
	int	lbl;
	int	cond;
	struct	tnode *btree;
};

struct	tname {
	int	nop;
	int	ntype;
	int	elsize;
	char	class;
	char	regno;
	int	offset;
	int	nloc;
};

/*
 * for field selections
 */
struct tsel {
	int	op;
	int	type;
	int	degree;
	struct	tnode *tr1;
	char	flen;
	char	bitoffs;
};

struct	tconst {
	int	cop;
	int	ctype;
	int	cdeg;
	int	value;
};

struct	optab {
	char	tabdeg1;
	char	tabtyp1;
	char	tabdeg2;
	char	tabtyp2;
	char	*tabstring;
};

struct	table {
	int	tabop;
	struct	optab *tabp;
};

struct	instab {
	int	iop;
	char	*str1;
	char	*str2;
};

struct	swtab {
	int	swlab;
	int	swval;
};

char	maprel[];
char	notrel[];
int	nreg;
int	isn;
int	namsiz;
int	line;
char	ascbuf[518];
int	nerror;
int	acctype;
int	tmplabel;
/*struct	instab	branchtab[];*/
int	opdope[];
char	*opntab[];
int	nstack;
int	nfloat;
int	*spacep;
int	treespace[ossiz];
int	autosize;
int	eolflg;
int	depth;
int	xvalid;
int	multused, divused, modused;
struct tconst czero, cone, fczero;

/*
	operators
*/
#define	EOF	0
#define	SEMI	1	/* semicolon ';'	*/
#define	LBRACE	2	/* left brace '{'	*/
#define	RBRACE	3	/* right brace '}'	*/
#define	LBRACK	4	/* left bracket '['	*/
#define	RBRACK	5	/* right bracket ']'	*/
#define	LPARN	6	/* left parens '('	*/
#define	RPARN	7	/* right parens ')'	*/
#define	COLON	8	/* colon ':'	*/
#define	COMMA	9	/* comma ','	*/
#define	FSEL	10	/* field selection	*/

#define	KEYW	19	/*+*/
#define	NAME	20	/*+*/
#define	CON	21	/* constant	*/
#define	STRING	22	/* string constant	*/
#define	FCON	23	/* floating constant	*/
#define	SFCON	24	/* double constant	*/

#define	AUTOI	27	/* register auto-increment	*/
#define	AUTOD	28	/* register auto-decremnet	*/
#define	INCBEF	30	/* pre-increment	*/
#define	DECBEF	31	/* pre-decrement	*/
#define	INCAFT	32	/* post-increment	*/
#define	DECAFT	33	/*post-decrement	*/
#define	EXCLA	34	/* unary !	*/
#define	AMPER	35	/* unary &	*/
#define	STAR	36	/* unary *	*/
#define	NEG	37	/* unary -	*/
#define	COMPL	38	/* unary ~	*/

#define	DOT	39	/* structure reference	*/
#define	PLUS	40	/* +	*/
#define	MINUS	41	/* -	*/
#define	TIMES	42	/* *	*/
#define	DIVIDE	43	/* /	*/
#define	MOD	44	/* %	*/
#define	RSHIFT	45	/* >>	*/
#define	LSHIFT	46	/* <<	*/
#define	AND	47	/* &	*/
#define	NAND	55	/* &~	*/
#define	OR	48	/* |	*/
#define	EXOR	49	/* ^	*/
#define	ARROW	50	/* ->	*/
#define	ITOF	51	/* int -> double	*/
#define	FTOI	52	/* double -> int	*/
#define	LOGAND	53	/* &&	*/
#define	LOGOR	54	/* ||	*/
#define	FTOL	56	/* double -> long	*/
#define	LTOF	57	/* long -> double	*/
#define	ITOL	58	/* integer -> long	*/
#define	LTOI	59	/* long -> integer	*/

#define	EQUAL	60	/* ==	*/
#define	NEQUAL	61	/* !=	*/
#define	LESSEQ	62	/* <=	*/
#define	LESS	63	/* <	*/
#define	GREATEQ	64	/* >=	*/
#define	GREAT	65	/* >	*/
#define	LESSEQP	66	/* <= pointer	*/
#define	LESSP	67	/* < pointer	*/
#define	GREATQP	68	/* >= pointer	*/
#define	GREATP	69	/* > pointer	*/

#define	ASPLUS	70	/* =+	*/
#define	ASMINUS	71	/* =-	*/
#define	ASTIMES	72	/* =*	*/
#define	ASDIV	73	/* =/	*/
#define	ASMOD	74	/* =%	*/
#define	ASRSH	75	/* =>>	*/
#define	ASLSH	76	/* =<<	*/
#define	ASSAND	77	/* =&	*/
#define	ASOR	78	/* =|	*/
#define	ASXOR	79	/* =^	*/
#define	ASSIGN	80	/* =	*/
#define	TAND	81	/* & for tests	*/
#define	LTIMES	82	/* * long	*/
#define	LDIV	83	/* / long	*/
#define	LMOD	84	/* % long	*/
#define	ASSNAND	85	/* =&~	*/
#define	LASTIMES 86	/* =* long	*/
#define	LASDIV	87	/* =/ long	*/
#define	LASMOD	88	/* =% long	*/

#define	QUEST	90	/* ?	*/
#define	LLSHIFT	91	/* long <<	*/
#define	ASLSHL	92	/* long =<<	*/
#define	CALL1	98	/*+*/
#define	CALL2	99	/*+*/
#define	CALL	100	/* call	*/
#define	MCALL	101	/* mcall	*/
#define	JUMP	102	/* goto	*/
#define	CBRANCH	103	/* jump conditional	*/
#define	INIT	104	/* global initialization	*/
#define	SETREG	105	/* set number of regs	*/
#define	LOAD	106	/*+*/
#define	RFORCE	110	/* force r0	*/

/*
 * Intermediate code operators
 */
#define	BRANCH	111
#define	LABEL	112
#define	NLABEL	113
#define	RLABEL	114
#define LSTAR	115	/* * op on left side of assignment  */
#define	BDATA	200
#define	WDATA	201
#define	PROG	202
#define	DATA	203
#define	BSS	204
#define	CSPACE	205
#define	SSPACE	206
#define	SYMDEF	207
#define	SAVE	208
#define	RETRN	209
#define	EVEN	210
#define	PROFIL	212
#define	SWIT	213
#define	EXPR	214
#define	SNAME	215
#define	RNAME	216
#define	ANAME	217
#define	NULL	218

/*
    code generator types
*/
#define PUSH	219
#define POP	220
#define LOADAC	221
#define SAVEAC	222
#define LOADADD	223

/*
 *	types
 */
#define	INT	0
#define	CHAR	1
#define	FLOAT	2
#define	DOUBLE	3
#define	STRUCT	4
#define	RSTRUCT	5
#define	LONG	6

#define	TYLEN	2
#define	TYPE	07
#define	XTYPE	(03<<3)
#define	PTR	010
#define	FUNC	020
#define	ARRAY	030
/* this is a problem, since compound types can cause problems */
#define ACC	010000  /* accumulator resident type  */
#define STACK	020000  /* stack resident type (intermediate storage) */
#define INDEX	040000  /* used with LSTAR */
#define FIXED	0100000 /* used to fix argument references */

#define TYPEBITS 07777	/* bits which hold type info */
/*
	storage	classes
*/
#define	KEYWC	1
#define	MOS	10
#define	AUTO	11
#define	EXTERN	12
#define	STATIC	13
#define	REG	14
#define	STRTAG	15
#define	ARG	16
#define	OFFS	20
#define	XOFFS	21
#define	SOFFS	22

/*
	Flag	bits
*/

#define	BINARY	01
#define	LVALUE	02
#define	RELAT	04
#define	ASSGOP	010
#define	LWORD	020
#define	RWORD	040
#define	COMMUTE	0100
#define	RASSOC	0200
#define	LEAF	0400
#define	CNVRT	01000

/*  types used by pick routine  */
#define CONCHR		1
#define STKCHR		2
#define AUTOCHR0	3
#define AUTOCHR1	4
#define EXTCHR		5
#define STATCHR		6
#define UNKCHR		7
#define	CONINT		8
#define STKINT		9
#define AUTOINT0	10
#define AUTOINT1	11
#define EXTINT		12
#define STATINT		13
#define UNKINT		14
#define INDXCHR		15
#define INDXINT		16
