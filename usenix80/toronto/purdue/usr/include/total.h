#

/*
 *      Data structures for accounting summary
 *	(program: total)
 */

#include	<ktd.h>

struct sum2 {
	unsigned entries;
	unsigned t_cnt[NMON];
	unsigned t_u_cnt[NUSR];
} week[7][48];
