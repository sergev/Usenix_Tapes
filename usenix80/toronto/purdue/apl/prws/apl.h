/*
 * Temp file names
 */

#define WSFILE	ws_file		/* work space file */
#define LPFILE	lp_file		/* label preprocessor scratch file */

/*
 * Magic Numbers
 */

#define MRANK	8
#define CANBS	300
#define STKS	500
#define NLS	200
#define NAMS	40
#define OBJS	500
#define MAXLAB	30
#define MAGIC	0100554
#define data	double

/*
 * derrived constants
 */

#define	SDAT	sizeof datum
#define	SINT	sizeof integ

/*
 * Interpreter Op Codes
 */

#define EOF	(-1)
#define EOL	0

#define ADD	1
#define PLUS	2
#define SUB	3
#define MINUS	4
#define MUL	5
#define SGN	6
#define DIV	7
#define RECIP	8
#define MOD	9
#define ABS	10
#define MIN	11
#define FLOOR	12
#define MAX	13
#define CEIL	14
#define PWR	15
#define EXP	16
#define LOG	17
#define LOGE	18
#define CIR	19
#define PI	20
#define COMB	21
#define FAC	22

#define DEAL	23
#define RAND	24
#define DRHO	25
#define MRHO	26
#define DIOT	27
#define MIOT	28
#define ROT0	29
#define REV0	30
#define DTRN	31
#define MTRN	32
#define DIBM	33
#define MIBM	34

#define GDU	35
#define GDUK	36
#define GDD	37
#define GDDK	38
#define EXD	39
#define SCAN	40
#define EXDK	41
#define SCANK	42
#define IPROD	43
#define OPROD	44
#define QUAD	45
#define QQUAD	46
#define BRAN0	47
#define BRAN	48
#define DDOM	49
#define MDOM	50

#define COM	51
#define RED	52
#define COMK	53
#define REDK	54
#define ROT	55
#define REV	56
#define ROTK	57
#define REVK	58
#define CAT	59
#define RAV	60
#define CATK	61
#define RAVK	62

#define PRINT	63
#define QUOT	64
#define ELID	65
#define CQUAD	66
#define COMNT	67
#define INDEX	68
#define HPRINT	69

#define LT	71
#define LE	72
#define GT	73
#define GE	74
#define EQ	75
#define NE	76
#define AND	77
#define OR	78
#define NAND	79
#define NOR	80
#define NOT	81
#define EPS	82
#define MEPS	83
#define REP	84
#define TAKE	85
#define DROP	86
#define ASGN	88
#define IMMED	89


#define NAME	90
#define CONST	91
#define FUN	92
#define ARG1	93
#define ARG2	94
#define AUTO	95
#define REST	96

#define COM0	97
#define RED0	98
#define EXD0	99
#define SCAN0	100
#define BASE	101
#define MENC	102	/*	monadic	encode	*/
#define LABEL	103	/* statement label */
#define PSI	104	/* PSI input character */
#define PSI1	105	/* PSI monadic half */
#define PSI2	106	/* PSI dyadic half */
#define ISP	107	/* ISP input code */
#define ISP1	108	/* ISP monadic half */
#define ISP2	109	/* ISP dyadic half */
#define QWID	110	/* quad fn1 */
#define QFUZZ	111
#define QRUN	112
#define QFORK	113
#define QWAIT	114
#define QEXEC	115
#define FDEF	116
#define QEXIT	117
#define QPIPE	118
#define QCHDIR	119
#define QOPEN	120
#define QCLOSE	121
#define QREAD	122
#define QWRITE	123
#define QCREAT	124
#define QSEEK	125
#define QUNLNK	126
#define QRD	127
#define QDUP	128
#define QAP	129
#define QKILL	130
#define QCRP	131
#define DFMT	132
#define MFMT	133
#define QNC	134
#define NILRET	135
#define XQUAD	136
#define SICLR	137
#define SICLR0	138
#define RVAL	139
#define QSIGNL	140
#define	QFLOAT	141		/* Float character string to data */

/*
 * Immediate sub-op codes
 */

#define	CLEAR	1
#define	DIGITS	2
#define	EDIT	3
#define	ERASE	4
#define	FNS	5
#define	FUZZ	6
#define	READ	7
#define	ORIGIN	8
#define	VARS	9
#define	WIDTH	10
#define	DEBUG	11
#define OFF	12
#define LOAD	13
#define SAVE	14
#define COPY	15
#define CONTIN	16
#define LIB	17
#define DROPC	18
#define VSAVE	19
#define SCRIPT	20
#define EDITF	21
#define TRACE	22
#define UNTRACE	23
#define WRITE	24
#define RESET	25
#define SICOM	26
#define CODE	27
#define	DEL	28
#define	SHELL	29
#define	LIST	30

struct
{
	char	c[0];
};

data	zero;
data	one;
data	pi;
data	maxexp;	/* the largest value such that exp(maxexp) is defined */
data	datum;
data	getdat();
int	labflag;	/* label processing enabled */
int	funtrace;	/* function trace enabled */
int	labgen;		/* label processing being done */
int	apl_term;	/* flag set if apl terminal mapping req'd */

/*
 * Several unrelated values, which appear
 * together in the header of an apl workspace file.
 */
struct
{
	double	fuzz;
	int	iorg;
	int	digits;
	int	width;
} thread;

/*
 * Data types
 * Each new type should be accomodated for 
 * in dealloc [a0.c]
 */

#define	DA	1
#define	CH	2
#define	LV	3
#define	QD	4
#define	QQ	5
#define	IN	6
#define	EL	7
#define	NF	8
#define	MF	9
#define	DF	10
#define	QC	11
#define	QV	12	/* quad variables */
#define DU	13	/* dummy -- causes fetch error except on print */
#define QX	14	/* latent expr. quad "Llx" */
#define LBL	15	/* locked label value */

/*
 * This is a descriptor for apl data, allocated by "newdat".
 * The actual data starts at item.dim[item.rank], and thus
 * &item.dim[item.rank] should always == item.datap.
 *
 * A null item is a vector(!), and is rank==1, size==0.
 *
 * the stack is the operand stack, and sp is the pointer to the
 * top of  the stack.
 */

struct item
{
	char	rank;
	char	type;
	int	size;
	int	index;
	data	*datap;
	int	dim[0];
} *stack[STKS], **sp;

/*
 * variable/fn (and file name) descriptor block.
 * contains useful information about all LVals.
 * Also kludged up to handle file names (only nlist.namep 
 * is then used.)
 *
 * For fns, nlist.itemp is an array of pointers to character
 * strings which are the compiled code for a line of the fn.
 * (Itemp == 0) means that the fn has not yet been compiled .
 * nlist.itemp[0] == the number of lines in the fn, and
 * nlist.itemp[1] == the function startup code, and
 * nlist.itemp[max] == the close down shop code.
 */

struct nlist
{
	char	use;
	char	type;	/* == LV */
	int	*itemp;
	char	*namep;
	int	label;
} nlist[NLS];

/*
 * This is the structure used to implement the
 * APL state indicator.
 *
 * The structure is allocated dynamically in ex_fun (ai.c),
 * but not explicitly.   Ex_fun declares a single, local
 * structure (allocated by C, itself), and links it to
 * previous instances of the structure.  SI is used for
 * two basic things:
 *
 *	1) error traceback (Including ")SI" stuff).
 *	2) Restoration of the global variable environment
 *	   (or any other, pending environment).
 *
 * The global variable "gsip" is a pointer to the
 * head of a chain of these structures, one for each
 * instance of an activated function.  (Gsip == 0) implies
 * an empty list, (gsip->sip == 0) implies the end of the list,
 * and (gsip->np == 0) implies a state indicator seperator.
 * (A new function was evoked with an old one pending.)
 *
 * Note that "gsip->funlc" is the same as the old global
 * variable "funlc", and 
 *
 *	(gsip && gsip->sip ? gsip->sip->funlc : 0)
 *
 * is the value of the old global, "ibeam36".
 */

struct si {
	int	suspended;	/* fn is suspended <=1, pending <= 0 */
	struct si *sip;		/* previous fn activation */
	struct nlist *np;	/* current fn vital stats. */
	int funlc;		/* current fn current line number */
	struct item **oldsp;	/* top of operand stack upon fn entry */
	char *oldpcp;		/* execution string upon fn entry */
	int	env;		/* pointer for restoration of local
				 * fn activation record */
} *gsip;

/*
 * exop[i] is the address of the i'th action routine.
 * Because of a "symbol table overflow" problem with C,
 * the table was moved from a1.c to it's own at.c
 */

int	(*exop[])();

double	floor();
double	fabs();
double	ceil();
double	log();
double	sin();
double	cos();
double	atan();
double	atan2();
double	sqrt();
double	exp();
double	gamma();
double	ltod();

int	integ;
int	signgam;
int	column;
int	intflg;
int	echoflg;
int	ifile;
int	wfile;
int	debug;
int	ttystat[3];
int	stime[2];
char	*pcp;	/* global copy of arg to exec */
int     rowsz;
int	mencflg;
int	aftrace;
char    *mencptr;
int oldlb[MAXLAB];
int pt;
int syze;
int pas1;
int ibeam36;
int protofile;
int lastop;	/* last (current) operator exec'ed */
char *scr_file;	/* scratch file name */
char *ws_file;	/* apl workspace file */
char *lp_file;	/* apl label preprocessor file */


struct
{
	char	rank;
	char	type;
	int	size;
	int	dimk;
	int	delk;
	int	dim[MRANK];
	int	del[MRANK];
	int	idx[MRANK];
} idx;

#define	APLBIT	020		/* Apl bit in byte 5 of stty call */


/* Following are definitions for buffered I/O.
 * To generate a version of APL without buffered I/O,
 * leave NBUF undefined.
 */

#define	NBUF	4		/* Number of I/O buffers */


#ifdef NBUF

#define	BLEN	256		/* Buffered I/O buffer length */

struct iobuf {			/* Buffered I/O buffer structure */
	int b_len;		/* Buffer length */
	int b_next;		/* Next available character */
	int b_fd;		/* Assigned file descriptor */
	char b_buf[BLEN];	/* Actual buffer */
} *iobuf;


struct fds {
	int fd_dev;             /* Device major/minor number */
	int fd_ind;             /* File inode number */
	int fd_buf;             /* Number of assigned buffer */
	char fd_lastop;         /* Last operation (0=read, 1=write) */
	char fd_uniq;           /* Unique flag (1=unique, 0=not unique) */
	char fd_dup;            /* Principal fd for dups */
	char fd_open;           /* (0=closed, 1=open) */
} files[16];


#define	READF	readf		/* Buffered read routine */
#define	WRITEF	writef		/* Buffered write routine */
#define	SEEKF	seekf		/* Buffered seek routine */
#define	OPENF	openf		/* Buffered file open routine */
#define	CREATF	creatf		/* Buffered file create routine */
#define	DUPF	dupf		/* Buffered file dup routine */
#define	CLOSEF	closef		/* Buffered file close routine */
#define	FSTATF	fstatf		/* Buffered "fstat" call */
#define	FORKF	forkf		/* Buffered "fork" call */

#endif


#ifndef	NBUF

#define	READF	read		/* Normal read routine */
#define	WRITEF	write		/* Normal write routine */
#define	SEEKF	seek		/* Normal seek routine */
#define	OPENF	open		/* Normal file open routine */
#define	CREATF	creat		/* Normal file create routine */
#define	DUPF	dup		/* Normal file dup routine */
#define	CLOSEF	close		/* Normal file close routine */
#define	FSTATF	fstat		/* Normal "fstat" call */
#define	FORKF	fork		/* Normal "fork" call */

#endif
