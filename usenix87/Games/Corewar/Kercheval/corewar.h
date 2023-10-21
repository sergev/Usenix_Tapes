/*
 *	General defines for CoreWar, based on A.K. Dewdney's May 1984
 *	column in Scientific American.
 *
 *	Copyright 1984, Berry Kercheval.  All rights reserved.  Permission
 *	for distribution is granted provided no direct commercial
 *	advantage is gained, and that this copyright notice appears 
 *	on all copies.
 */

#define TIMEOUT 120		/* only play for two minutes */
struct	memword
	{
		short op;
		short a_mode;
		short b_mode;
		int   a;
		int   b;
		short owner;
	};

typedef struct memword memword;
typedef	int	address;

#define 	MEMSIZE		8000
#define 	DISTANCE	1000	/* programs at laest this far apart */
#define		LDW		10	/* width of local dump */

/*
 *	Redcode Instruction Format:
 *
 *	N    N     N   NNNN NNNN	<-- decimal digits
 *	OP ModeA ModeB AAAA BBBB
 *
 *	Max. instruction: 82279997999 = 0x1 3245 222F
 *	Too bad it won't fit in a Long
 *
 */

/*
 * The redcode Op-code values
 */

#define		MOV	1
#define		ADD	2
#define		SUB	3
#define		JMP	4
#define		JMZ	5
#define		JMG	6
#define		DJZ	7
#define		CMP	8
#define		FORK	9
#define		DAT	0

char *op_str[] =
{
	"dat",	/* 0 */
	"mov",	/* 1 */
	"add",	/* 2 */
	"sub",	/* 3 */
	"jmp",	/* 4 */
	"jmz",	/* 5 */
	"jmg",	/* 6 */
	"djz",	/* 7 */
	"cmp",	/* 8 */
	"fork",	/* 9 */
	0
};
	
/*
 * Operand modes
 */

#define IMMEDIATE	0
#define DIRECT		1
#define INDIRECT	2

char mode_char[] = { '#', ' ', '@', 0};

struct proc {
	struct proc *next_proc;
	struct proc *ancestor;
	address	pc;
	int	status;			/* 0: dead. */
	short	from;			/* 1 or 2 (original ancestor) */
	long	time_of_birth;
	long	no_of_alive_offspring;
	long	no_of_dead_offspring;
};

extern struct proc * makeproc();
#define	MAXPROCNUM	20	/* max no of processes at top level */


