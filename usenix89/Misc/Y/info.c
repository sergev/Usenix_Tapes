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

extern char hc2char();

extern int
    Iloaded,
    Idays,
    Ihours,
    Iminutes;

extern unsigned char
    Ihcode;

extern char *wdays[];

/* ARGSUSED */

c_info(argc, argv)
char *argv[];
{
if (argc != 2) usage(E_2MANY);
if (Iloaded)
    (void) printf("Interface clock: %s, %2d:%02d\n",
           wdays[dowX2U(Idays)], Ihours, Iminutes);
else
    (void) printf("Interface clock not yet set\n");
(void) printf("Housecode = %c\n", hc2char(Ihcode));
}
