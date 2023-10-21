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

extern char flag;
extern int tty, timeout;
extern int pevent();
extern int pdata();
extern struct id id[];

c_dump(argc, argv)
char *argv[];
{
if (argc != 3) usage(E_WNA);

if (strcmp(argv[2], EVENTS) == 0)
    dumpcontents(pevent, GETEVENTS);
else if (strcmp(argv[2], DATA) == 0)
    dumpcontents(pdata, GETDATA);
else if (strcmp(argv[2], "all") == 0)
    {
    dumpcontents(pdata, GETDATA);
    (void) putchar('\n');
    dumpcontents(pevent, GETEVENTS);
    }
else error("unknown dump request");
}

dumpcontents(handler, type)
int (*handler) ();
{
register unsigned i, total;
register scnt, dcnt;
char *msgstr;
unsigned char buf[8];
unsigned n, cksum, count;

flag = 0;		/* set print header indicator */

if (type == GETEVENTS)
    {
    scnt = ETOTAL;
    dcnt = ESIZE;
    msgstr = EVENTS;
    }
else
    {
    readid();
    scnt = DTOTAL;
    dcnt = DSIZE;
    msgstr = DATA;
    }

sendsync();
buf[0] = type;
(void) write(tty, (char *) buf, 1);
getsync();
n = xread(tty, buf, 1, timeout);
if (n != 1) error("UPLOAD: no response");
cksum = 0;
count = 0;
total = 0;
for (i = 0; i < scnt; i++)
    {
    n = xread(tty, buf, 1, timeout);
    if (n != 1) error("UPLOAD: reply truncated");
    total++;
    if (buf[0] == 0xFF) continue;
    n = xread(tty, &buf[1], dcnt - 1, timeout);
    if (n != dcnt - 1) error("UPLOAD: information truncated");
    for (n = 0; n < dcnt; n++)
    	cksum += buf[n];
    (*handler) (buf, i, dcnt);
    count++;
    }
n = xread(tty, buf, 1, timeout);
if (n != 1) error("UPLOAD: checksum not received");
if (count && (cksum & 0xFF) != buf[0]) error("UPLOAD: bad checksum received");
if (! count)
(void) printf("No %s currently stored in interface (%d replies)\n",
    msgstr, total);
}
