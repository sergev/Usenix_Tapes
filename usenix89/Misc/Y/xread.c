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
#include <signal.h>
#include <setjmp.h>
#include "x10.h"

unsigned alarm();
void sigtimer();

/*
 * xread(fd, buf, count, timeout)
 *
 *	Timed read.  Works just like read(2) but gives up after
 *	timeout seconds, returning whatever's been read so far.
 */

static jmp_buf jb;

xread(fd, buf, count, timeout)
unsigned char *buf;
{
int total;

total = 0;
if (setjmp(jb))
    return(total);

(void) signal(SIGALRM, sigtimer);
(void) alarm((unsigned) timeout);

while (count--)
    {
    if (read(fd, (char *) buf, 1) < 1)
    	{
    	(void) alarm(0);
    	(void) signal(SIGALRM, SIG_IGN);
    	return(total);
    	}
    buf++;
    total++;
    }
(void) alarm(0);
(void) signal(SIGALRM, SIG_IGN);
return(total);
}

void
sigtimer()
{
longjmp(jb, 1);
}
