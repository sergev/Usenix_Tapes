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

c_delete(argc, argv)
char *argv[];
{
register unsigned cmdsize, n, arg;
unsigned number;
char buf[12];

if (argc < 4) usage(E_NMA);

if (strncmp(argv[2], EVENTS, sizeof(EVENTS) - 2) == 0)
    cmdsize = EVCMD;
else if (strcmp(argv[2], DATA) == 0)
    cmdsize = DICMD;
else error("unknown delete request");

buf[0] = DATALOAD;

for (arg = 3; arg < argc; arg++)
    {
    if (!sscanf(argv[arg], "%d", &number))
	(void) fprintf(stderr,
	    "ignored non-numeric event number\n");

    if (cmdsize == EVCMD)
	{
	if (number > ETOTAL - 1)
	    {
	    (void) fprintf(stderr,
		"ignored event number greater than 127\n");
	    continue;
	    }
	buf[1] = number << 3;
	buf[2] = (number >> 5) & 0x3;
	}
    else
	{
	if (number > DTOTAL - 1)
	    {
	    (void) fprintf(stderr,
		"ignored data slot number greater than 255\n");
	    continue;
	    }
	buf[1] = number << 1;
	buf[2] = (number >> 7) | 0x4;
	}

    for (n = 3; n < cmdsize; n++)
	buf[n] = 0;

    sendsync();
    (void) write(tty, buf, cmdsize);

    chkack();
    }
}
