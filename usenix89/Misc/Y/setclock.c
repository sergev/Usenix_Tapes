/*
 * Copyright 1986 by Larry Campbell, 73 Concord Street, Maynard MA 01754 USA
 * (maynard!campbell).  You may freely copy, use, and distribute this software
 * subject to the following restrictions:
 *
 *  1)	You may not charge money for it.
 *  2)	You may not remove or alter this copyright notice.
 *  3)	You may not claim you wrote it.
 *  4)	If you make improvements (or other changes), you are requested
 *	to send them to me, so there's a focal point for distributing
 *	improved versions.
 *
 * John Chmielewski (tesla!jlc until 9/1/86, then rogue!jlc) assisted
 * by doing the System V port and adding some nice features.  Thanks!
 */

#include <stdio.h>
#include <time.h>
#include "x10.h"

extern struct tm *localtime();
extern long time();
extern int tty;
extern char *wdays[];

/* ARGSUSED */

c_setclock(argc, argv)
char *argv[];
{
unsigned char data[5];
struct tm *tp;
long dtime;

if (argc != 2) usage(E_2MANY);
dtime = time((long *) 0);
tp = localtime(&dtime);
data[0] = SETCLK;
data[1] = tp->tm_min;
data[2] = tp->tm_hour;
data[3] = dowU2X(tp->tm_wday);
data[4] = CHKSUM(data);
sendsync();
(void) write(tty, (char *) data, 5);
chkack();
(void) printf("X10 clock set to %s, %d:%02d\n",
    wdays[tp->tm_wday], tp->tm_hour, tp->tm_min);
}
