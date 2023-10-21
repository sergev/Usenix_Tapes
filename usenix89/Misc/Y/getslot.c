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
#include "x10.h"

extern int
    tty,
    timeout;

getslot(type)
{
register scnt, dcnt;
unsigned char buf[8];
unsigned i, n, cksum, count;
int firstfree;

if (type == GETEVENTS)
    {
    scnt = ETOTAL;
    dcnt = ESIZE;
    }
else
    {
    scnt = DTOTAL;
    dcnt = DSIZE;
    }

sendsync();
buf[0] = type;
(void) write(tty, (char *) buf, 1);
getsync();
n = xread(tty, buf, 1, timeout);
if (n != 1) error("UPLOAD: no response");
cksum = 0;
count = 0;
firstfree = -1;
for (i = 0; i < scnt; i++)
    {
    n = xread(tty, buf, 1, timeout);
    if (n != 1) error("UPLOAD: reply truncated");
    if (buf[0] == 0xFF)
    	{
    	if (firstfree == -1) firstfree = i;
    	continue;
    	}
    n = xread(tty, &buf[1], dcnt - 1, timeout);
    if (n != dcnt - 1) error("UPLOAD: information truncated");
    for (n = 0; n < dcnt; n++)
    	cksum += buf[n];
    }
n = xread(tty, buf, 1, timeout);
if (n != 1) error("UPLOAD: checksum not received");
if (count && (cksum & 0xFF) != buf[0]) error("UPLOAD: bad checksum received");
if (firstfree == -1) error("No more storage slots available");
return(firstfree);
}
