/*
** cw.h	- misc. constants and macros used throughout corewar system
**
**	[cw by Peter Costantinidis, Jr. @ University of California at Davis]
*/

/*
**
static	char	rcsid[] = "$Header: cw.h,v 7.0 85/12/28 14:35:45 ccohesh Dist $";
**
*/
#include <sys/types.h>		/* necessary for void */

#ifdef	BSD42|BSD43
#	define	SETJMP	_setjmp
#	define	LONGJMP	_longjmp
#else
#	define	SETJMP	setjmp
#	define	LONGJMP	longjmp
#endif

/*
** curses conflicts
*/
#ifndef	WINDOW
#	define	TRUE	1
#	define	FALSE	0
#	undef	reg
#endif

#ifdef	DEBUG
#	define	reg
#else
#	define	reg	register
#endif

#ifndef	CTRL
#	define	CTRL(c)	('c'&037)
#endif
#ifndef	CINTR
#	define	CINTR	CTRL(?)
#endif
#define	min(a,b)	((a) < (b) ? (a) : (b))
#define	max(a,b)	((a) > (b) ? (a) : (b))
#define	abs(a)		((a) < 0 ? -(a) : (a))

/*
** Visual display related constants
*/
#define	MIN_LINES	24	/* minimum lines allowed in display */
#define	MIN_COLS	80	/* minimum columns allowed in display */

/*
** memory window
*/
#define	MLINES		16	/* # lines in memwin */
#define	MCOLS		(MAX_MEM/(MLINES*NUMMEM))/* # columns in memwin */
#define	MYBEGIN		1	/* y offset in stdscr of memwin */
#define	MXBEGIN		1	/* x offset in stdscr of memwin */

/*
** message window
*/
#define	MSGLINES	(MIN_LINES-MLINES-2)/* lines in status window */
#define	MSGCOLS		(MXBEGIN+MCOLS)/* columns in status window */
#define	MSGYBEGIN	(MYBEGIN+MLINES+1)/* y offset in stdscr of status win */
#define	MSGXBEGIN	0

/*
** status window
*/
#define	SLINES		MIN_LINES
#define	SCOLS		(MIN_COLS-MCOLS-2)
#define	SYBEGIN		0
#define	SXBEGIN		(MXBEGIN+MCOLS+1)

#define	NUMMEM		PAGESIZE/* # memwords per character in memwin */
#define	HORTBORDER	'-'	/* horizontal border char for memwin box */
#define	VERTBORDER	'|'	/* vertical border char for memwin box */

/*
** misc. characters in memory window
*/
#define	MEMPTY	'.'	/* nothing of note about memsect */
#define	MSHARE	'-'	/* more than one job writing */
#define	MHIT	'+'	/* hit near a PC */
#define	MFULL	'*'	/* >=10 PC's in memsect */

/*
** output controls
*/
#define	SIZE_START	0x0001L		/* print size and start of job */
#define	PASSNUM		0x0002L		/* print current cycle */
#define	PASSFREQ	10		/* print every n cycles */
#define	PINST		0x0004L		/* print current instruction */
#define	VISUAL		0x0008L		/* visual display */
#define	STATUS		0x0010L		/* status window for v.d. */
#define	DEF_CTRLS	(VISUAL|STATUS|PINST)

#define	ON(x)		(octrl&(x))
#define	TOGGLE(x)	octrl=(octrl&x)?(octrl&~x):(octrl|x)

#define	HALTED(j)	(j->pdied)
#define	PC	_pc

#define	uns		unsigned
#define	unss		unsigned short
#define	when		break;case
#define	otherwise	break;default

#define	RC_OUT		"rc.out"

/*
** memory constraints and other limits
*/
#define	PAGESIZE	16	/* jobs placed on page boundaries */
#define	MAX_PAGE	512	/* max. pages in mem. */
#define	MAX_MEM		(PAGESIZE*MAX_PAGE)/* size of mem. */
#define	JMAX_LEN	512	/* max. len for job */
#define	PAGESEP		8	/* min. job separation in pages */
#define	JOB_WIDTH	(JMAX_LEN+ (PAGESEP*PAGESIZE))
#define	CYCLES		1024	/* max. execution cycle limit */
#define	MAX_USERS	(SLINES-1)/* max. # jobs (some might not fit...) */
#define	MAX_LABELS	128	/* maximum number of labels allowed */
#define	LABEL_LEN	6	/* max. len of labels */

/*
** field bit-widths needed
*/
#define	OP_BITS		4
#define	MODE_BITS	2
#define	ADDR_BITS	13	/* # of bits needed to address all memory */

/*
** Redcode instruction set
*/
#define	DAT	0	/* initialize location to value B */
#define	MOV	1	/* move A into location B */
#define	ADD	2	/* add A to contents of B, results in B */
#define	SUB	3	/* subtract A from contents of B, results in B */
#define	JMP	4	/* jump to B */
#define	JMZ	5	/* if A == 0 then jump to B, otherwise continue */
#define	DJZ	6	/* --A; if A == 0 then jump to B */
#define	CMP	7	/* compare A with B; if != then skip next instruction */
#define	MUL	8	/* multiply B by A, results in B */
#define	DIV	9	/* divide B by A, results in B */
#define	RND	10	/* random number between 0 and MAX_MEM, results in B */

/*
** Addressing modes
*/
#define	IMM	0	/* number is operand */
#define	REL	1	/* number specifies offset from current instruction */
#define	IND	2	/* number specifies offset from current instruction */
			/* to location where relative address of operand is */
			/* found */

/*
** character and string constants representing the various modes
** and instructions
*/
#define	CIMM	'#'
#define	CREL	' '
#define	CIND	'@'
#define	SDAT	"DAT"
#define	SMOV	"MOV"
#define	SADD	"ADD"
#define	SSUB	"SUB"
#define	SJMP	"JMP"
#define	SJMZ	"JMZ"
#define	SDJZ	"DJZ"
#define	SCMP	"CMP"
#define SMUL	"MUL"
#define	SDIV	"DIV"
#define	SRND	"RND"

/*
** termination reasons for mars/processes
*/
#define	EALIVE		0	/* currently active */
#define	ECYCLE		1	/* max cycle limit exceeded */
#define	EWIN		2	/* normal termination (a winner) */
#define	EILLINST	3	/* illegal instruction */
#define	EBADMODE	4	/* invalid addressing mode */
#define	EINTERRUPT	5	/* process interrupted */

typedef	struct
{
	uns	op:OP_BITS,	/* opcode */
		moda:MODE_BITS,	/* mod argument A */
		modb:MODE_BITS,	/* mod argument B */
		arga:ADDR_BITS,	/* argument A */
		argb:ADDR_BITS;	/* argument B */
} memword;
#define	val	argb

typedef	struct
{
	char	*pname;		/* name of player (input file name) */
	int	pid;		/* process number */
	int	pc;		/* current program counter */
	int	psize;		/* # of instruction in program */
	int	picnt;		/* # of instructions executed at job halt */
	int	pdied;		/* non-zero if dead, if dead then reason */
	memword	*plstmem;	/* # of last memword modified */
} process;

extern	char	*argv0;
extern	int	jobcnt, jobsleft, min_sep;
extern	int	cycle, max_cyc;
extern	long	octrl;
extern	memword	memory[];

extern	char	*strcpy();

extern	char	*diedstr(), *punctrl();
extern	void	usage(), msg(), msgfini(), movmem(), statfini(),
		vfini(), vstatus(), vstajob();
extern	int	printit(), sum(), msginit(), statinit(), vupdate();
