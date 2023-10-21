#include <stdio.h>
#include "spool.h"

/* Copyright (c) 1984 Chris Torek / University of Maryland */

/*
 * $Header: /usr/enee/chris/sys/spool/RCS/readdev.c,v 1.2 83/12/10 02:36:48 chris Exp $
 *
 * Routines for reading /etc/spool_devices and looking up device names
 *
 * $Log:	readdev.c,v $
 * Revision 1.2  83/12/10  02:36:48  chris
 * Fixed \ decoding
 * 
 * Revision 1.1  83/12/10  01:03:44  chris
 * Initial revision
 * 
 */

char *malloc (),
     *strcpy ();

char *
strsave (s) char *s; {
    register int len;
    static char *pool;
    static int   size;

    len = strlen (s) + 1;
    if (size < len) {		/* sigh, potentially wasting space here */
	size = len > BUFSIZ/2 ? len + BUFSIZ : BUFSIZ;
	pool = malloc ((unsigned) size);
	if (pool == 0)
	    error ("out of memory\n");
    }
    strcpy (pool, s);
    size -= len;
    pool += len;
    return pool - len;
}

/* classify string s according to list l */
/* (i.e., l[class(s)-1] matches s) */
static
class (s, l)
register char *s;
char **l;
{
    register char *p, **lp = l;

    while (p = *lp++) {
	if (*p == *s && strcmp (p, s) == 0)
	    return lp - l;
    }
    return 0;
}

/* Make an arg vector, interpreting ' " and \ along the way */
static
MakeArgV (s, v, b)
register char	*s,		/* source */
		**v,		/* vector base */
		*b;		/* canon buf */
{
    register quote = 0;

    while (*s) {
	while (*s == ' ' || *s == '\t')
	    s++;
	if (*s == 0)
	    break;
	*v++ = b;
	while (*s) {
	    if (*s == quote) {	/* out of quotes */
		quote = 0;
		s++;
		continue;
	    }
	    if (quote == 0 && (*s == '\'' || *s == '"')) {
		quote = *s++;	/* into quotes */
		continue;
	    }
	    if (*s == '\\') {	/* \ escapes */
		if (*++s == 0)
		    break;
		switch (*s) {
		case '0': case '1': case '2': case '3':
		case '4': case '5': case '6': case '7':
		    *b = *s++ - '0';
		    if (*s >= '0' && *s <= '7') {
			*b <<= 3, *b += *s++ - '0';
			if (*s >= '0' && *s <= '7')
			    *b <<= 3, *b += *s++ - '0';
		    }
		    b++;
		    break;
		case 'r':
		    *b++ = '\r';
		    break;
		case 'n':
		    *b++ = '\n';
		    break;
		case 'f':
		    *b++ = '\014';
		    break;
		case 't':
		    *b++ = '\t';
		    break;
		case 'b':
		    *b++ = '\b';
		    break;
		default:
		    *b++ = *s++;
		    break;
		}
		continue;
	    }
	    if (quote || (*s != ' ' && *s != '\t'))
		*b++ = *s++;	/* keep adding it on... */
	    else {
		*b++ = 0;	/* end of this arg */
		break;
	    }
	}
    }
    *b = 0;
    *v = 0;			/* end of argv list */
}

/*
 * Read the DevFile
 */

GetDevices () {
    register struct SpoolDevice *s;
    register char  *cp,
		  **vp;
    static char     line[BUFSIZ],
		   *vec[BUFSIZ],
		    canonbuf[BUFSIZ];
    register FILE  *sf;
    int		    c;

 /* Types for "class" to pick out: device=1, exec=2, ... */
    static char *ty[] = {
	"device", "exec", "restricted", "pagelen",
	"linelen", "header", "formfeed", 0
    };

    s = 0;
    if ((sf = fopen (DevFile, "r")) == NULL)
	error ("panic: open devfile\n");
    for (;;) {
	(void) fgets (line, sizeof line, sf);
	if (ferror (sf))
	    error ("panic: devfile read\n");
	if (feof (sf))
	    break;
	cp = line + strlen (line);
	if (cp > line && *--cp == '\n')
	    *cp = 0;
	cp = line;
	while (*cp == ' ' || *cp == '\t')
	    cp++;
	if (*cp == '#' || *cp == 0)
	    continue;
	MakeArgV (cp, vec, canonbuf);
	vp = vec;		/* for fast access... */
	c = class (*vec, ty);
	if (c != 1 && s == 0)
	    error ("panic: devfile bad format\n");
	switch (c) {
	case 0: 		/* not in ty[] */
	    continue;
	case 1: 		/* device */
	    if (vp[1] == (char *) 0 || vp[2] == (char *) 0)
		error ("panic: device name\n");
	    s = (struct SpoolDevice *) malloc (sizeof (*s));
	    if (s == 0)
		error ("out of memory\n");
	    s -> sd_dev = strsave (vp[1]);
	    s -> sd_namelist.nl_name = strsave (vp[2]);
	    s -> sd_namelist.nl_next = 0;
	    s -> sd_proglist = 0;
	    s -> sd_linelen = 0;
	    s -> sd_pagelen = 0;
	    s -> sd_header = 0;
	    s -> sd_ff = 0;
	    s -> sd_next = DevList;
	    DevList = s;
	    for (vp = &vec[3]; *vp; vp++) {
		register struct namelist *n;

		n = (struct namelist *) malloc (sizeof *n);
		if (n == 0)
		    error ("out of memory\n");
		n -> nl_name = strsave (*vp);
		n -> nl_next = s -> sd_namelist.nl_next;
		s -> sd_namelist.nl_next = n;
	    }
	    break;
	case 2: 		/* exec */
	    if (vp[1] == 0)
		error ("panic: exec name\n");
	    {
		register struct proglist *p;
		register char  *lp;

		p = (struct proglist *) malloc (sizeof *p);
		if (p == 0)
		    error ("out of memory\n");
		if (vp[2]) {
		    p -> pr_shortname = strsave (vp[1]);
		    vp = &vec[2];
		    lp = line;
		    while (cp = *vp++) {
			while (*lp++ = *cp++)
			    ;
			lp[-1] = ' ';
		    }
		    *--lp = 0;	/* ASSUMES AT LEAST ONE *vp NONEMPTY */
		    if ((p -> pr_longname = strsave (line)) == 0)
			error ("out of memory\n");
		}
		else {
		    p -> pr_shortname = 0;
		    p -> pr_longname = strsave (vp[1]);
		}
		p -> pr_next = s -> sd_proglist;
		s -> sd_proglist = p;
	    }
	    break;
	case 3: 		/* restricted */
	    s -> sd_restricted++;
	    break;
	case 4: 		/* pagelen */
	    if (vp[1] == 0)
		error ("panic: bad pagelen\n");
	    s -> sd_pagelen = atoi (vp[1]);
	    break;
	case 5: 		/* linelen */
	    if (vp[1] == 0)
		error ("panic: bad linelen\n");
	    s -> sd_linelen = atoi (vp[1]);
	    break;
	case 6: 		/* header */
	    if (vp[1])
		s -> sd_header = strsave (vp[1]);
	    break;
	case 7: 		/* formfeed */
	    if (vp[1])
		s -> sd_ff = strsave (vp[1]);
	    else
		s -> sd_ff = "\014";/* control-L */
	    break;
	default: 
	    error ("panic: bad case in switch\n");
	    break;
	}
    }
    (void) fclose (sf);
}

/* Given a name, return the device for it, or 0 for no such device.  Any of
   the aliases are legal, and so is the physical device (why not?). */
struct SpoolDevice *
FindDevice (name)
register char *name;
{
    register struct SpoolDevice *s;
    register struct namelist *n;

    for (s = DevList; s; s = s -> sd_next) {
	if (*s -> sd_dev == *name && strcmp (s -> sd_dev, name) == 0)
	    return s;
	n = &s -> sd_namelist;
	while (n) {
	    if (*n -> nl_name == *name && strcmp (n -> nl_name, name) == 0)
		return s;
	    n = n -> nl_next;
	}
    }
    return 0;
}
