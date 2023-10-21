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

extern int tty;
struct evitem event;
struct ditem data;

c_fload(argc, argv)
char *argv[];
{
unsigned char buf[12];
char *cptr;
register unsigned i, size, cmdsize;

if (argc != 3) usage(E_WNA);

if (strcmp(argv[2], EVENTS) == 0)
    {
    size = EVSIZE;
    cmdsize = EVCMD;
    cptr = (char *) &event;
    }
else if (strcmp(argv[2], DATA) == 0)
    {
    size = DISIZE;
    cmdsize = DICMD;
    cptr = (char *) &data;
    }
else error("unknown fload request");

while (read(0, cptr, size) == size)
    {
    buf[0] = DATALOAD;

    if (size == EVSIZE)
	{
	if (event.e_buf[0] & 0xF0) error("invalid MODE field in file");
	if (event.e_buf[1] & 0x80) error("invalid DAYS field in file");
	if (event.e_buf[2] > 23) error("invalid HOUR field in file");
	if (event.e_buf[3] > 59) error("invalid MINUTE field in file");
	if (event.e_buf[6] & 0x0F) error("invalid HOUSECODE field in file");
	buf[1] = event.e_num << 3;
	buf[2] = (event.e_num >> 5) & 0x3;
	for (i = 0; i < ESIZE; i++)
	    buf[i+3] = event.e_buf[i];
	}
    else
	{
	buf[1] = data.d_num << 1;
	buf[2] = data.d_num >> 7 | 0x4;
	buf[3] = data.d_buf[0];
	buf[4] = data.d_buf[1];
	}

    buf[cmdsize - 1] = 0;
    for (i = 3; i < cmdsize - 1; i++)		/* compute checksum */
    	buf[cmdsize - 1] += buf[i];

    sendsync();
    (void) write(tty, (char *) buf, cmdsize);

    chkack();
    }
}
