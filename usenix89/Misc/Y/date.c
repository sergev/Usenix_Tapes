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
#ifdef SYSV
#include <sys/types.h>
#endif
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

#include <sys/param.h>
#include <sys/filsys.h>
#include "x10.h"

extern struct tm *localtime();

extern long lseek();

extern int
    Idays,
    Ihours,
    Iminutes;

/* ARGSUSED */

c_date(argc, argv)
char *argv[];
{
struct filsys f;
int rf, today;
struct tm *tp;

if (argc != 2) usage(E_2MANY);
rf = open(ROOTNAME, 0);
if (rf < 0) error("can't open root filesystem");
if (lseek(rf, 512L, 0) == -1L) error("can't lseek on root");
if (read(rf, (char *) &f, 512) != 512) error("can't read root");
(void) close(rf);
if (f.s_time < 515000000L) error("root has unreasonable timestamp");
tp = localtime(&f.s_time);
today = dowX2U(Idays);
while (tp->tm_wday % 7 != today)
    tp->tm_wday++, tp->tm_mday++;

#ifdef VENIX
(void) printf("%2d%02d%02d%02d%02d\n",
       tp->tm_year, tp->tm_mon+1, tp->tm_mday, Ihours, Iminutes);
#else
(void) printf("%02d%02d%02d%02d%2d\n",
       tp->tm_mon+1, tp->tm_mday, Ihours, Iminutes, t0->tm_year);
#endif
}
