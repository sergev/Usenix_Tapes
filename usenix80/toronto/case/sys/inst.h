#ifdef MONITOR

/* Setup binary vector RF-HS-DV-RK-CPU for indexing swp_stat c_stat */

#define CPU	01	/* Lsb of index vector (0=idle, 1=busy) */
#define RK	02

/* Declare (maximum) logical number of each device type present */

#define LNRK	5

#define RKSIZ 64	/* 2**(LNRK+1) for rk-cpu overlap stats */

#define NSIZ	4	/* Index vector size 2(=devices) --> 4 combinations */

/* Instrumentation "flags" */

struct inst {
int inst_go;		/* Set to enable data collection */
int inst_conf;		/* Bit vector describes system configuration */
long inst_tim;		/* Time data collection enabled */

/* Misc. data gathered by System Instrumentation (see clock.c) */

long c_ticks;	/* Accumulates # of clock interrupts */
long c_intrpt;	/* Total number of interrupts handled */
long c_sysint;	/* # of times in system mode with pri>0 */
long c_idle;	/* # of times sched() has no process to run */
long c_user;		/* # of times observed in user mode */
long c_rnrun;	/* # of times higher pri. proc. prempts cpu */

long swp_stat[4];	/* B_BUSY <xy>: y=cpu idle/busy, x=B_WANTED */

long swp_rnout;	/* # of times sched() with none to swap in */
long swp_rnin;	/* # of times sched() waiting for free memory */
long swp_cnt;	/* Total # of swaps - set by swapping routine bio.c  */
long swp_wds;	/* Total # of words swapped - set by bio.c */

/* Disk Instrumentation */

int rk_busy;	/* bit vector telling which rks busy (set by driver) */

long tk_nin;		/* Total number of tty input characters */
long tk_nout;		/* Total number of tty output characters */

long ik_hit;		/* Total number of inodes found in core */
long ik_miss;		/* Total number of inodes not found in core */

long bk_hit;		/* Total number of disk blocks found in buffer pool */
long bk_miss;		/* Total number of disk blks not found in core */


/* Arrays for disk-cpu overlap statistics (per device) sampled at line freq.*/

long rk_stat[RKSIZ];

/* Statistics maintained by disk drivers */

struct {
	long d_start;	/* Time I/O was started on disk */
	long d_accum;	/* Total accumulated time disk active */
} rk_time[LNRK];
long rk_numb[LNRK];	/* Total # of RK accesses */
long rk_wds[LNRK];	/* total # of words of RK I/O */


long c_stat[NSIZ];	/* All cpu-device overlap combinations (see clock.c) */
} I;
#endif
