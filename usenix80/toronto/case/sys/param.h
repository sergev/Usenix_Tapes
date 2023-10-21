/*
 * tunable variables
 */

#define	NBUF	20		/* size of buffer cache */
#define	NINODE	100		/* number of in core inodes */
#define	NFILE	70		/* number of in core file structures */
#define	NMOUNT	5		/* number of mountable file systems */
#define	NEXEC	3		/* number of simultaneous exec's */
#define	NCARGS	5120		/* max number of args (multiple of 512) */
#define	MAXMEM	(64*32)		/* max core per process - first # is Kw */
#define	SSIZE	20		/* initial stack size (*64 bytes) */
#define	SINCR	20		/* increment of stack (*64 bytes) */
#define	NOFILE	15		/* max open files per process */
#define	CANBSIZ	256		/* max size of typewriter line */
#define	CMAPSIZ	50		/* size of core allocation area */
#define	SMAPSIZ	50		/* size of swap allocation area */
#define	NCALL	20		/* max simultaneous time callouts */
#define	NPROC	40		/* max number of processes */
#define	NTEXT	15		/* max number of pure texts */
#define	NCLIST	150		/* max total clist size */
#define	HZ	60		/* Ticks/second of the clock */
#define	MSGBUFS	128		/* Characters saved from error messages */
#define	NMON	6		/* number of statistics monitored in clock.c */
#define	MAX_NAPPING	5	/* max number of procs using nap sys call */
#define CPU_45	CPU_45		/* compile in 45 features  */

/*
 * priorities
 * probably should not be
 * altered too much
 */

#define	PSWP	-100
#define	PINOD	-90
#define	PRIBIO	-50
#define	PPIPE	1
#define	PNAP	10
#define	PWAIT	40
#define	PSLEP	90
#define	PUSER	100

/*
 * signals
 * dont change
 */

#define	NSIG	20
#define	SIGHUP	1	/* hangup */
#define	SIGINT	2	/* interrupt (rubout) */
#define	SIGQIT	3	/* quit (FS) */
#define	SIGINS	4	/* illegal instruction */
#define	SIGTRC	5	/* trace or breakpoint */
#define	SIGIOT	6	/* iot */
#define	SIGEMT	7	/* emt */
#define	SIGFPT	8	/* floating exception */
#define	SIGKIL	9	/* kill */
#define	SIGBUS	10	/* bus error */
#define	SIGSEG	11	/* segmentation violation */
#define	SIGSYS	12	/* sys */
#define	SIGPIPE	13	/* end of pipe */
#define	SIGCLK	14	/* alarm clock */
#define	SIGTRM	15	/* catchable termination */
#define	SIGLDS	16	/* LDS-2 interrupt */

/*
 * fundamental constants
 * cannot be changed
 */

#define	USIZE	16		/* size of user block (*64) */
#define	NULL	0
#define	NODEV	(-1)
#define	ROOTINO	1		/* i number of all roots */
#define	DIRSIZ	14		/* max characters per directory */

/*
 * structure to access an
 * integer in bytes
 */
struct
{
	char	lobyte;
	char	hibyte;
};

/*
 * structure to access an integer
 */
struct
{
	int	integ;
};

/*
 * structure to access a long as integers
 */
struct {
	int	hiword;
	int	loword;
};

/*
 * Certain processor registers
 */
#define PS	0177776
#define KL	0177560
#define SW	0177570
#define	SSW	050		/* software switch register */

/*
 *	Debugging flags
 */
/* #define	DEBUG	DEBUG	/* print debugging info */
/* #define	DUMP	DUMP	/* dump system on errors */
/* #define	PROF	PROF	/* run system profiler */
#define	MONITOR	MONITOR	/* accumulate system usage stats */
