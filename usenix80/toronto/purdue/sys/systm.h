/*
 * Random set of variables
 * used by more than one
 * routine.
 */
char	canonb[CANBSIZ];	/* buffer for erase and kill (#@) */
int	coremap[CMAPSIZ];	/* space for core allocation */
int	swapmap[SMAPSIZ];	/* space for swap allocation */
int	*rootdir;		/* pointer to inode of root directory */
int	cputype;		/* type of cpu =40, 45, or 70 */
int	execnt;			/* number of processes in exec */
int	*runq;			/* head of linked list of running procs */
int	lbolt;			/* time of day in 60th not in time */
int	time[2];		/* time in sec from 1970 */
int	tout[2];		/* time of day of next sleep */
/*
 * The callout structure is for
 * a routine arranging
 * to be called by the clock interrupt
 * (clock.c) with a specified argument,
 * in a specified amount of time.
 * Used, for example, to time tab
 * delays on teletypes.
 */
struct	callo
{
	int	c_time;		/* incremental time */
	int	c_arg;		/* argument to routine */
	int	(*c_func)();	/* routine */
} callout[NCALL];
/*
 * Mount structure.
 * One allocated on every mount.
 * Used to find the super block.
 */
struct	mount
{
	int	m_dev;		/* device mounted */
	int	*m_bufp;	/* pointer to superblock */
	int	*m_inodp;	/* pointer to mounted on inode */
} mount[NMOUNT];
int	mpid;			/* generic for unique process id's */
char	runin;			/* scheduling flag */
char	runout;			/* scheduling flag */
char	runrun;			/* scheduling flag */
char	curpri;			/* more scheduling */
#ifdef XSWAP
int	runstg;			/* nz if sched to stage out to sswapdev*/
int	pswapdev;		/* primary swap (see low.s) */
int	sswapdev;		/* sec swap dev (see low.s) */
int	pswplo;			/* first block to use on pswapdev */
int	sswaplo;		/* first block to use on sswapdev */
int	ptimx;			/* max time on pri swap dev */
int	paddrx;			/* end if pswap (see low.s) */
int	npriswp;		/* numb of primary swaps (for stats) */
int	nsecswp;		/* numb of secondary swaps (stats) */
int	nstages;		/* numb of stage outs from pri swap */
#endif
int	maxmem;			/* actual max memory per process */
int	maxblk;			/* largest block of non su file, set in main.c */
int	maxblk2;		/* max size for file (uid < MAXBLKU) */
# ifdef XBUF
char	*bufaddr;		/* addr/64 of NBUF2 pool (init in data.h) */
char	*bufsiz;		/* size in words*32 of NBUF2 pool (init in data.h) */
char	*praddr;		/* addr/64 of proc table (see data.h) */
char	*prsiz;			/* size in words*32 of proc table (see data.h) */
# ifdef MX
char	*ntaddr;		/* addr/64 of net buffers (see data.h) */
char	*ntsiz;			/* size of net buffers (see data.h) */
# endif
int	*ka5;			/* 11/40 KISA5, 11/45-70 KDSA5 */
# endif
int	*lks;			/* pointer to clock device */
int	rootdev;		/* dev of root see conf.c */
int	pipedev;		/* pipe dev (init in low.s) */
int	swapdev;		/* dev of swap see conf.c */
int	swplo;			/* block number of swap space */
int	nswap;			/* size of swap space */
int	updlock;		/* lock for sync */
int	rablock;		/* block to be read ahead */
char	regloc[];		/* locs. of saved user registers (trap.c) */
/* System statistics block
 */
int	ccount;			/* count of characters currently on clist */
int	cblkct;			/* count of allocated blocks in clist */
