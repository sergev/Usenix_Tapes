#
/*
 *
 *	This is a program to analyse the contents
 *	of the proc array, found in a dump of memory
 *	after a crash.
 *
 *	P. Ivanov.	UNSW.	14-4-77.
 *
 *	C. McGregor.	UNSW.	Jan-78.
 *
 *	P. Ivanov.	UNSW.	April-78.
 */

#define	UNSW
#define	DZ11
#include <local-system>
#include <defines.h>
#include <param.h>
#include <proc.h>
#include <tty.h>
#include <user.h>
#include <inode.h>
#include <file.h>
#ifdef	AUSAM
#include <passwd.h>
#endif	AUSAM
#ifdef	ZOMBIE
#include <pzomb.h>
#endif	ZOMBIE
#define	NBUF	1
#include <buf.h>
#include <text.h>
struct tty	tty;

#define	NOSTAT	7		/* number of status codes possible */
#define	NOFLAG	8		/* number of flag codes possible */
#define	NOTFLAG	2		/* number of flag codes possible */
#define	NOTTYFLG	16	/* number of stty flag codes */
#define	NOTTYIST	16	/* number of tty internal states */
#define	MAXMEM	024000	/* the real amount of memory on this system */
#define	MINSWAP	15960	/* start of swap area on this system */
#define	MAXSWAP	3990+15960	/* the end of swap on this system */
#define	U_PTAB	"%5t%r"	/* user area indent */
#define	T_PTAB	"%5t%r"	/* text area indent */
#define	S_PTAB	"          %r"	/* stack trace indent */
#define	SS_PTAB	"%50t%r"	/* secondary stack indent - local variables */
#define	Y_PTAB	"     %r"	/* ttydecode indent */
#define	C_PTAB	"          %r"	/* char list decode indent */
#define	DEBUG	if (dflg)
#define	WARNING	if (!wflg)
#define	LINEWIDTH	132	/* line size for line and star */
#define	TAB	"     %r"	/* tab indent */
#define	USERADR	0140000		/* present user start address */
#ifdef	_1170
#define	FULLPAGE	020000
#endif	_1170
#ifdef	BIG_UNIX
#define	PAGEADR	0120000		/* start address of re-mapped page */
#define	KISA5	0172352
#endif

int	coremap[CMAPSIZ];	/* space for core allocation */
int	swapmap[SMAPSIZ];	/* space for swap allocation */

char	*stat[];
char	*flag[NOFLAG];
char	*signals[NSIG+1];
char	*tflag[];

struct	symtab
	{
	char	st_sym[8];
	int	st_sz;
	};
struct symtab	symbols[];

struct	regs {
	int	r_word0;
	int	r_word1;
	int	r_reg[7];
	int	r_kisa6;
#ifdef	BIG_UNIX
	int	r_kisa5;
#endif
	} regs;

struct	symbol {
	char	s_symbol[8];
#ifndef	BIG_UNIX
	int	s_symflg;
#endif
#ifdef	BIG_UNIX
	char	s_symflg;
	char	s_symseg;
#endif
	unsigned s_symval;
	} names;

struct map
	{
	unsigned m_size;
	unsigned m_addr;
	};

#ifdef	BIG_UNIX
#define	MAXSEG	30
int	segbase[MAXSEG];
int	maxseg;
#endif

unsigned	txtoff;

char	wkbuf[512];
int	*regbuf;		/* equivalenced to wkbuf */
int	corefd;
int	namefd;

int	aflg, bflg, cflg, dflg;
int	fflg, gflg;
int	capfflg;
int	iflg;
int	kflg, lflg;
int	nflg;
int	oflg, pflg;
int	sflg, tflg, uflg;
int	wflg, xflg, yflg, zflg;
int	capcflg, capsflg;

/*
 *	flags:
 *	a - dump in ascii
 *	b - dump in bytes
 *	c - dump in chars
 *	C - dump the coremap
 *	d - debugging output flag
 *	f - decode file structure references
 *	F - (capital F) full dump flag
 *	i - decode inode structure references
 *	k - expects ka6 and aps values to be given	(not yet implemented)
 *	l - list all process slots, even unassigned ones
 *	n - give alternate namelist file (default /unix) as last arg
 *	o - dump in octal
 *	p - followed by digits dumps only that process
 *	s - user structure stack trace
 *	S - dump the swap map
 *	t - examine forking trees
 *	u - dump user area, including ka6 if given
 *	w - warnings off  flag
 *	x - dump only text structures
 *	y - dump only tty structures
 *	z - do it all baby (very wordy)
 */

/* other random declarations */

struct {
	int	hiword;
	int	loword;
	};

struct {
	unsigned unsign;
	};

struct symbol	nullnum;
