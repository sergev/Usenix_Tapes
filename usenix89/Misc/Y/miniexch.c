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
#include <sgtty.h>
#include "x10.h"

externint tty;

/*
 * mxconnect(port)
 *
 *	Connect to specified port on DEC Mini-Exchange
 */

#ifdef MINIEXCH
mxconnect(port)
{
static char
    mx1[] = "//",
    mx2[] = " P",		/* port number goes here before P */
    mx3[] = "\r";

#define PORTID mx2[0]
#define MXLEN sizeof(mxmsg)
#define MRLEN 5
#define MRACKOFFSET (MRLEN-1)

unsigned char
    mxrply[MRLEN];

int n;

sleep(SMALLPAUSE);
PORTID = '0' + port;
(void) write(tty, mx1, 2);
sleep(SMALLPAUSE);
(void) write(tty, mx2, 2);
sleep(SMALLPAUSE);
(void) write(tty, mx3, 1);
n = xread(tty, mxrply, MRLEN, 3);		/* 2-second timeout */
if (n != MRLEN)
    {
    (void) fprintf(stderr, "Mini-exchange replied with %d bytes\n", n);
    for (n = 0; n < MRLEN; n++)
    	(void) fprintf(stderr, "reply[%2d] = 0x%x\n", n, mxrply[n]);
    error("Mini-exchange timeout");
    }
if (mxrply[MRACKOFFSET] != 'A') error("Port 3 busy or disconnected");
}
#endif
