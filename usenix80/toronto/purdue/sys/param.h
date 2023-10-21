/*
 * system conditional compilation
 */

#define XSWAP	/* run with 2 swap devices: pswapdev and sswapdev */
#define STATS	/* have clock.c and hp.c collect various statistics */
#define XBUF	/* run with large buffers out of Kernel address space */
#define POWERFAIL /* handle power failures */
#define MX	/* run with MX code */
#define INGRES  /* use interlock dev for ingres database (exit()/sys1.c) */
/* #define FLAKEYBUS /* retry traps to 4 in kernel mode - flakey hardware */
/*
 * tunable variables
 */

# ifndef XBUF
#define	NBUF	36		/* size of buffer cache */
# endif
# ifdef XBUF
#define NBUF1   14              /* buffers in Kernel space, for NODEV */
#define	NBUF2	110		/* buffers not in Kernel space */
#define NBUF NBUF1+NBUF2
# endif
#define TIMELIMIT 300		/* cpu seconds which gives SIGTIM */
#define	NINODE	350		/* number of in core inodes */
#define	NFILE	350		/* number of in core file structures */
#define	NMOUNT	16		/* number of mountable file systems */
#define	NEXEC	4		/* number of simultaneous exec's */
#define NCARGS	10240		/* length of arg list for exec */
#define	MAXMEM	(64*32)		/* max core per process - first # is Kw */
#define MAXBLK  650             /* max size (in blks) of a non su file */
#define	MAXBLK2	10000		/* max size of file (uid < MAXBLKU) */
#define	MAXBLKU 10		/* max uid for MAXBLK2 */
#define	SSIZE	20		/* initial stack size (*64 bytes) */
#define	SINCR	20		/* increment of stack (*64 bytes) */
#define	NOFILE	15		/* max open files per process */
#define	CANBSIZ	256		/* max size of typewriter line */
#define	CMAPSIZ	300		/* size of core allocation area */
#define	SMAPSIZ	300		/* size of swap allocation area */
#define	NCALL	50		/* max simultaneous time callouts */
#define	NPROC	250		/* max number of processes */
#define MAXJOB  300              /* max number of processes per user */
#define	NTEXT	60		/* max number of pure texts */
#define	NCLIST	800		/* max total clist size */
#define	HZ	60		/* Ticks/second of the clock */

/*
 * priorities
 * probably should not be
 * altered too much
 */

#define	PSWP	-100
#define	PINOD	-90
#define	PRIBIO	-50
#define PFNT	-3	/* wait for more file table space */
#define	PPIPE	1
#define	PWAIT	40
#define	PSLEP	80
#define	PUSER	90

/*
 * signals
 * dont change
 */

#define	NSIG	20
#define		SIGHUP	1	/* hangup */
#define		SIGINT	2	/* interrupt (rubout) */
#define		SIGQIT	3	/* quit (FS) */
#define		SIGINS	4	/* illegal instruction */
#define		SIGTRC	5	/* trace or breakpoint */
#define		SIGIOT	6	/* iot */
#define		SIGEMT	7	/* emt */
#define		SIGFPT	8	/* floating exception */
#define		SIGKIL	9	/* kill */
#define		SIGBUS	10	/* bus error */
#define		SIGSEG	11	/* segmentation violation */
#define		SIGSYS	12	/* sys */
#define		SIGPIPE	13	/* end of pipe */
#define		SIGCLK	14	/* alarm clock signal */
#define		SIGTIM	16	/* time limit */

/*
 * fundamental constants
 * cannot be changed
 */

#define	USIZE	16		/* size of user block (*64) */
#define	NULL	0
#define	NODEV	(-1)
# ifdef XBUF
#define NODEV2	(-2)
# endif
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
 * Certain processor registers
 */
#define PS	0177776
#define KL	0177560
#define SW	0177570
