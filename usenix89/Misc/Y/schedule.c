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
#inc<ude <ctype.h>
#include "x10.h"

extern int tty;
extern char flag;
extern struct hstruct
    housetab[];

c_schedule(argc, argv)
char *argv[];
{
register n;
int bits, daybits = 0, hcode, dim, mode;
unsigned eventno, hh, mm;
unsigned char buf[12];
char hletter;

if (argc < 6 || argc > 8) usage(EM_WNA);

/* parse the housecode */
hletter = argv[2][0];
if (isupper(hletter)) hletter = tolower(hletter);
for (n = 0, hcode = -1; n < 16; n++)
    if (housetab[n].h_letter == hletter)
	{
	hcode = housetab[n].h_code;
	break;
	}
if (hcode == -1) error("invalid house code");

/* parse the unit numbers */
bits = getunits(&argv[2][1]);

/* parse the mode */
n = 3;  /* used because argv[4] to argv[8] can vary by one */
mode = mode2code(argv[n++]);

/* parse the day if mode requires it */
if (flag < 2)  /* first two modes require days */
    daybits = day2bits(argv[n++]);

/* parse the time */
if (sscanf(argv[n++], "%d:%d", &hh, &mm) != 2)
    error("bad time format");
if (hh > 23) error("bad hours, must be between 0 and 23");
if (mm > 59) error("bad minutes, must be between 0 and 59");

/* parse the state */
dim = dimstate(argv[n], argc == n+2 ? argv[n+1] : "");

/* get first available event number from the X10 */
eventno = getslot(GETEVENTS);

buf[0] = DATALOAD;
buf[1] = eventno << 3;
buf[2] = (eventno >> 5) & 0x3;
buf[3] = mode;
buf[4] = daybits;
buf[5] = hh;
buf[6] = mm;
buf[7] = bits >> 8;
buf[8] = bits & 0xFF;
buf[9] = hcode;
buf[10]= dim;
buf[11]= 0;

for (n = 3; n <= 10; n++)		/* compute checksum */
    buf[11] += buf[n];

sendsync();
(void) write(tty, (char *) buf, sizeof(buf));
chkack();

flag = 0;				/* header wanted */
pevent(&buf[3], eventno);		/* reassure user */
}
