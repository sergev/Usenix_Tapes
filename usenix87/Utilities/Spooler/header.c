#ifndef lint
static char rcsid[] = "$Header: /usr/enee/chris/sys/spool/RCS/header.c,v 1.2 84/01/24 21:16:26 chris Exp $";
#endif

/* Copyright (c) 1984 Chris Torek / University of Maryland */

/*
 * $Header: /usr/enee/chris/sys/spool/RCS/header.c,v 1.2 84/01/24 21:16:26 chris Exp $
 *
 * Generate a printout header
 *
 * Usage: header (stdin from control file, stdout to output device)
 *
 * $Log:	header.c,v $
 * Revision 1.2  84/01/24  21:16:26  chris
 * Added ARGSUSED comment to main() to keep lint quieter.
 * 
 * Revision 1.1  83/12/10  01:03:36  chris
 * Initial revision
 * 
 */

#include <stdio.h>
#include "spool.h"
#include <sys/types.h>

char *ProgName;
char  user[BUFSIZ],
      title[BUFSIZ];
struct SpoolDevice *sd;
int   devline;

extern char chrtab[][16];

time_t  time ();
char   *ctime (),
       *strcpy ();

/* ARGSUSED */
main (argc, argv) char **argv; {
    register char  *cp;
    time_t tm;
    char    buf[BUFSIZ];

    ProgName = *argv;
    GetDevices ();
    while (fgets (buf, sizeof buf, stdin)) {
	cp = &buf[strlen (buf)];
	if (*--cp == '\n')
	    *cp = 0;
	switch (*buf) {
	    case 'D': 
		if (sd)
		    error ("bad format control file\n");
		cp = buf + 1;
		while (*cp == ' ')
		    cp++;
		sd = FindDevice (cp);
		if (sd == 0)
		    error ("invalid device in control file\n");
		break;
	    case 'U': 
		cp = buf + 1;
		while (*cp == ' ')
		    cp++;
		strcpy (user, cp);
		break;
	    case 'T': 
		cp = buf + 1;
		while (*cp == ' ')
		    cp++;
		strcpy (title, cp);
	}
    }
    if (sd == 0)
	error ("no device in control file\n");
    if (*user == 0)
	error ("missing user name\n");
    SkipTo (sd -> sd_pagelen / 2 - 16);
    GenTitle (user);
    SkipTo (sd -> sd_pagelen / 2);
    Column (sd -> sd_linelen / 2 - 13);
    (void) time (&tm);
    printf ("%.25s\n", ctime (&tm));
    devline++;
    SkipTo (sd -> sd_pagelen / 2 + 2);
    if (*title)
	GenTitle (title);
    if (sd -> sd_ff)
	fputs (sd -> sd_ff, stdout);
    else {
	if (devline > sd -> sd_pagelen)
	    devline %= sd -> sd_pagelen;
	SkipTo (sd -> sd_pagelen);
    }
    exit (0);
}

error (s)
char *s;
{
    fprintf (stderr, "%s: %s", ProgName, s);
    exit (1);
}

SkipTo (row) {
    if (row <= devline || row > sd -> sd_pagelen)
	return;
    while (devline < row) {
	putchar ('\n');
	devline++;
    }
}

Column (n) register n; {
    if (n < 1 || n >= sd -> sd_linelen - 13)
	return;
    while (--n >= 0)
	putchar (' ');
}

GenTitle (str)
char *str;
{
    register int    c,
		    i,
		    j,
		    t;
    register char  *s,
		   *p;
    int		len,
		nblank;
    char       *pend;
    static char buf[BUFSIZ];

    *buf = 'x';
    len = strlen (str);
    if (sd -> sd_linelen) {
	nblank = (sd -> sd_linelen - 8 * len) / 2;
	pend = &buf[sd -> sd_linelen - 7];
    }
    else
	nblank = 0, pend = &buf[sizeof buf];
    for (i = 0; i < 16; i++) {
	s = str;
	p = &buf[1];
	t = nblank;
	while (--t >= 0)
	    *p++ = ' ';
	while (c = *s++ & 0177) {
	    if (p >= pend)
		break;
	    t = c < ' ' || c >= 0177 ? 0 : chrtab[c - ' '][i];
	    j = 8;
	    while (--j >= 0) {
		*p++ = t & 128 ? c : ' ';
		t <<= 1;
	    }
	}
	while (*--p == ' ')
	    ;
	p++;
	*p++ = '\n';
	*p = 0;
	fputs (&buf[1], stdout);
	devline++;
    }
}
