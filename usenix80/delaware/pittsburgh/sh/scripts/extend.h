/*
 * header file for shell extensions
 *
 * George K. Rosenberg
 * 17 Mar 80
 *
 * modified:
 *	24 Apr 80	GKR
 *	8 Jun 80	GKR
 */


char *dotfile();
char *getpthd();
long lseek();
int uniqid;
int lock1;
int lock2;
int itexists;
extern char dot[];
extern char badseek[];
extern char nonascii[];
int implcd;


/*
 * if RTS is a defined preprocessor token
 * then special code to invoke an appropriate
 * emulator program to interpret a file
 * in terms of a special run time system
 * is generated
 *
 * Command files beginning with "#" are
 * also recognized and an appropriate
 * interpreter may be invoked.
 * This is a convention for csh (i.e. C Shell)
 * files.
 *
 * the emulators are seperate programs
 * there are currently three
 * they all interpret pdp11 code
 * they should be run on a pdp11 or vax11
 *
 * pdp11 UNIX a.out files as UNIX version 6
 * pdp11 UNIX a.out files as UNIX version 7
 * pdp11 RT-11 .sav files as RT11 version 2
 */

#ifdef pdp11
#define	eleven 1
#endif

#ifdef vax
#define	eleven 1
#endif


/*
 * NRTS is the number of possible recursions
 * through the rts routine
 * this prevents bad run time systems from
 * causing infinitely descending calls to rts
 * yet still allows one system to be code to be
 * interpreted by another system (but not itself)
 * NRTS should be the maximum of 1 and
 * the number of run time systems
 */

#ifdef vax
#define NRTS 4
#else

#ifdef pdp11
#define NRTS 2
#else

#define NRTS 1
#endif

#endif


#ifndef IMPLCD
#define	IMPLCD	1
#endif

#ifndef SLASH
#define	SLASH	1
#endif


/* file type bits */
#define	RT11typ		(1<<0)
#define	UNIX7typ	(1<<1)
#define	UNIX6typ	(1<<2)
#define	SEPIDtyp	(1<<3)		/* UNIX seperate I & D spaces */
#define	OVERtyp		(1<<4)		/* UNIX overlay */
#define	BINtyp		(1<<5)		/* binary (non-ASCII) */
#define	CSHtyp		(1<<6)


struct argenv {
	char	 *ae_cmd;	/* the command name (#0) */
	char	**ae_list;	/* the argument list (#@) */
	int	  ae_nargs;	/* the length of the list (##) */
	int	  ae_id;	/* unique id (#$) */
};

struct argenv shrp;
struct argenv tlb;

typedef short int11;
typedef unsigned short unsign11;

struct exec11 {
	int11		magic11;
	unsign11	text11;
	unsign11	data11;
	unsign11	bss11;
	unsign11	syms11;
	unsign11	entry11;
	unsign11	unused11;
	unsign11	flag11;
};

/* these RT-11 specs are heresay (and suspect) */
#define	RT11MAGIC	0

struct rt11sav {
	int11	rtfill[6];
	int11	rtmagic;
};

#define	CSHMAGIC	'#'

union header {
	struct exec11	UNIX;
	struct rt11sav	rt11;
	char		bytes[20];
	char		csh;
};
