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

#include <stdio.h
#include "x10.h"

struct evitem event;
struct ditem data;

c_finfo(argc, argv)
char *argv[];
{
if (argc != 3) usage(E_WNA);

if (strcmp(argv[2], EVENTS) == 0)
    while (read(0, (char *) &event, EVSIZE) == EVSIZE)
	{
	if (event.e_buf[0] & 0xF0) error("invalid MODE field in file");
	if (event.e_buf[1] & 0x80) error("invalid DAYS field in file");
	if (event.e_buf[2] > 23) error("invalid HOUR field in file");
	if (event.e_buf[3] > 59) error("invalid MINUTE field in file");
	if (event.e_buf[6] & 0x0F) error("invalid HOUSECODE field in file");
	pevent(event.e_buf, event.e_num);
	}
else if (strcmp(argv[2], DATA) == 0)
    {
    readid();
    while (read(0, (char *) &data, DISIZE) == DISIZE)
	pdata(data.d_buf, data.d_num);
    }
else error("unknown finfo request");
}
