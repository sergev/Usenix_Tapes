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
#include <ctype.h>
#include "x10.h"

extern int tty;
extern struct hstruct
    housetab[];

/* ARGSUSED */

c_reset(argc, argv)
char *argv[];
{
int hcode, hletter, n;
char buf[2];

if (argc > 3) usage(E_WNA);

buf[0] = SETHCODE;
buf[1] = HC_A;		/* default house code */

if (argc == 3)
    {
    hletter = argv[2][0];
    if (isupper(hletter)) hletter = tolower(hletter);
    for (n = 0, hcode = -1; n < 16; n++)
	if (housetab[n].h_letter == hletter)
	    {
	    buf[1] = hcode = housetab[n].h_code;
	    break;
	    }
    if (hcode == -1) error("invalid house code");
    }

sendsync();
(void) write(tty, buf, 2);
chkack();
}
