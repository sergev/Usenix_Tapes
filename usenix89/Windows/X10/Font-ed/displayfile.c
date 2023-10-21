/*
#
# $Source: /local/projects/X/fed/RCS/displayfile.c,v $
# $Header: displayfile.c,v 1.10 87/04/08 08:48:56 jim Exp $
#
#                     Copyright (c) 1987 Cognition Inc.
#
# Permission to use, copy, modify and distribute this software and its
# documentation for any purpose and without fee is hereby granted, provided
# that the above copyright notice appear in all copies, and that both
# that copyright notice and this permission notice appear in supporting
# documentation, and that the name of Cognition Inc. not be used in
# advertising or publicity pertaining to distribution of the software
# without specific, written prior permission.  Cognition Inc. makes no
# representations about the suitability of this software for any purpose.
# It is provided "as-is" without express or implied warranty.
#
#							  Jim Fulton
#							  Cognition Inc.
#                                                         900 Tech Park Drive
# uucp:  ...!{mit-eddie,talcott,necntc}!ci-dandelion!jim  Billerica, MA
# arpa:  jim@athena.mit.edu, fulton@eddie.mit.edu         (617) 667-4800
#
*/


#include <stdio.h>
#include <X/Xlib.h>

char *ProgramName;

static struct helplist {
    char *what;
    char *desc;
} help[] = {
    {"-r",         "reverse forward and background color"},
    {"-p padding", "padding between characters and border"},
    {"-b bcolor",  "background color or %num number"},
    {"-f fcolor",  "foreground color or %num number"},
    {"-h hpad",    "horizontal intercharacter padding"},
    {"-v vpad",    "vertical intercharacter padding"},
    {"-n",         "print newline symbols at end of line"},
    {"-V",         "verbose, print messages when no character present"},
    {"-w width",   "border width"},
    {"=+X+Y",      "is an X location specification for the window"},
    {"file",       "is a text file whose lines are displayed"},
    {"fontdir",    "is the FED font directory"},
    {"host:dpyno", "where to display"},
    /* insert new items above this line. */
    {NULL,         NULL},
};

Usage ()
{
    struct helplist *h;

    fprintf (stderr, "Usage:\n        ");
    fprintf (stderr, "%s [-flags] [=+X+Y] file fontdir [host:dpyno]\n\n", 
	     ProgramName);
    fprintf (stderr, "where -flags and args are:\n");

    for (h = help; h->what; h++) {
	fprintf (stderr, "    %-14s%s.\n", h->what, h->desc);
    }

    exit (1);
}

main (argc, argv)
    int argc;
    char *argv[];
{
    int geomask, x, y, width, height, bdr;
    int background, foreground, horzpadding, vertpadding;
    int clearargcount;
    int printnewlines, verbose;
    char *filename = NULL;
    char *fontdirname = NULL;
    char *displayname = NULL;
    char *arg;
    char *backgroundname = NULL, *foregroundname = NULL;
    Display *dpy;
    ColorDef cdef;
    int reverse;
    int padding;

    ProgramName = argv[0];
    setlinebuf (stderr);

#define NextArg() (argc--, arg = *++argv)
#define MoreArgs() (argc > 0)

    x = y = width = height = 0;
    bdr = 2;
    background = BlackPixel;
    foreground = WhitePixel;
    horzpadding = vertpadding = 0;
    clearargcount = 0;
    printnewlines = 0;
    verbose = 0;
    reverse = 0;
    for (NextArg(); MoreArgs(); NextArg()) {
	if (*arg == '=') {
	    geomask = XParseGeometry (arg, &x, &y, &width, &height);
	} else if (*arg == '-') {
	    switch (arg[1]) {
	      case 'r':
		reverse = 1;
		break;
	      case 'p':
		if (!MoreArgs()) Usage();
		NextArg();
		padding = atoi (arg);
		break;
	      case 'b':
		if (!MoreArgs()) Usage();
		NextArg();
		backgroundname = arg;
		break;
	      case 'f':
		if (!MoreArgs()) Usage();
		NextArg();
		foregroundname = arg;
		break;
	      case 'h':
		if (!MoreArgs()) Usage();
		NextArg();
		horzpadding = atoi (arg);
		break;
	      case 'v':
		if (!MoreArgs()) Usage();
		NextArg();
		vertpadding = atoi (arg);
		break;
	      case 'n':
		printnewlines = 1;
		break;
	      case 'V':
		verbose++;
		break;
		/* insert new flags above this line */
	      case 'w':
		if (!MoreArgs()) Usage();
		NextArg();
		bdr = atoi (arg);
		break;
	      default:
		Usage ();
	    }
	} else {
	    switch (++clearargcount) {
	      case 1:
		filename = arg;
		break;
	      case 2:
		fontdirname = arg;
		break;
	      case 3:
		displayname = arg;
		break;
	      default:
		Usage ();
	    }
	}
    }

    if (!(dpy = XOpenDisplay (displayname))) {
	fprintf (stderr, "%s:  unable to open display.\n", ProgramName);
	exit (1);
    }

    if (!(filename && fontdirname)) Usage ();

    if (reverse) {
	char *tmpcp;
	int tmpi;

	tmpcp = backgroundname;
	backgroundname = foregroundname;
	foregroundname = tmpcp;

	tmpi = background;
	background = foreground;
	foreground = tmpi;
    }

    if (backgroundname) {
	if (backgroundname[0] == '%')
	  background = atoi (backgroundname + 1);
	else if (DisplayCells() > 2) {
	    if (XParseColor (backgroundname, &cdef) &&
		XGetHardwareColor (&cdef))
	      background = cdef.pixel;
	    else 
	      fprintf (stderr, "%s: unable to parse background color %s.\n",
		       ProgramName, background);
	}
    }

    if (foregroundname) {
	if (foregroundname[0] == '%')
	  foreground = atoi (foregroundname + 1);
	else if (DisplayCells() > 2) {
	    if (XParseColor (foregroundname, &cdef) &&
		XGetHardwareColor (&cdef))
	      foreground = cdef.pixel;
	    else 
	      fprintf (stderr, "%s: unable to parse foreground color %s.\n",
		       ProgramName, foreground);
	}
    }

    do_font_display (filename, fontdirname, geomask, x, y, bdr,
		     background, foreground, horzpadding, vertpadding,
		     padding, printnewlines, verbose);
    XCloseDisplay (dpy);
    exit (0);
}
