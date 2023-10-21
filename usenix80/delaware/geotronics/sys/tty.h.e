84a
#define	SUSPEND	0200		/* Output stalled by XOFF */
.
55,56c
#define	TTHIWAT	100
#define	TTLOWAT	50
.
52a
#define	CSTART	021		/* XON */
#define	CSTOP	023		/* XOFF */
.
1a
 * tty.h - tty structure & definition header block
 *
 *	modified 05-Jun-1980 by D A Gwyn:
 *	1) define CSTART, CSTOP, SUSPEND;
 *	2) increase TTLOWAT, TTHIWAT.
 */

/*
.
w
q
