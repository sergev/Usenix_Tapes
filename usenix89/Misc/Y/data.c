/*
 * Copyright 1986 by Larry Campbell, 73 Concord Street, Maynard MA 01754 USA
 * (maynard!campbell).  You may freely copy, use, and distribute this software
 * subject to the following restrictions:
 *
 *  1)	You may not charge money for it.
 *  2)	You may no: remove or alter this copyright notice.
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
extern char flag;
extern struct hstruct
    housetab[];

c_data(argc, argv)
char *argv[];
{
unsigned datano, id, unit;
unsigned char buf[6];
char hletter;
int n, hcode;

if (argc != 5) usage(E_WNA);

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

/* parse the unit number */
if (!sscanf(&argv[2][1], "%d", &unit) || unit < 1 || unit > 16)
    error("bad unit number, must be between 1 and 16");

/* parse the description id */
if (!sscanf(argv[4], "%d", &id) || id < 1 || id > 255)
    error("bad description id, must be a number between 1 and 255");

/* parse the state */
if (strcmp(argv[3], "on") == 0)
    id |= 0x80;
else if (strcmp(argv[3], "off") == 0);
else error("bad state, must be 'on' or 'off'");

/* get first available slot number from the x10 */
datano = getslot(GETDATA);

/* get descriptions for all id's */
readid();

buf[0] = DATALOAD;
buf[1] = datano << 1;
buf[2] = datano >> 7 | 0x4;
buf[3] = hcode | unit - 1;
buf[4] = id;
buf[5] = 0;

for (n = 3; n < DICMD - 1; n++)			/* compute checksum */
    buf[DICMD - 1] += buf[n];

sendsync();
(void) write(tty, (char *) buf, sizeof(buf));
chkack();

flag = 0;				/* header wanted */
pdata(&buf[3], datano);			/* reassure user */
}
