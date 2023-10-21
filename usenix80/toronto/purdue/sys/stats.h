/*
 * various system statistics counters.
 * Most of these counters are maintained by clock.c except where noted.
 * The counters can be read/written from a  user process via the
 * table device driver (/dev/kt).
 * --ghg 11/07/77.
 */

#define RP04ADDR	0176700	/* RP04 csr register */

#define USER_CNT	0	/* time spent running user mode processes */
#define SYS_CNT		1	/* time spent running kernel mode */
#define SYSI_CNT	2	/* system interrupt time */
#define SWAP_CNT	3	/* time swap buffer busy */
#define IDLE_CNT	4	/* CPU idle time */
#define RP04_CNT	5	/* time RP04 busy */
#define RPXF_CNT	6	/* number of RP04 transfers (in hp.c) */
#define NMON		7	/* number of entries in above table */

struct	stats {
	int mon_cnt[NMON];	/* average value */
	int mon_cur[NMON];	/* count during last second */
	int ken_pc;		/* kernel pc before timer int */
} stats;
