/*
#
# $Source: /local/projects/X/fed/RCS/scalefont.c,v $
# $Header: scalefont.c,v 1.5 87/04/09 12:34:13 jim Exp $
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


#include "font.h"			/* includes stdio.h */
#include <strings.h>

char *ProgramName;

Usage ()
{
    fprintf (stderr, 
"Usage:\n        %s inputfontname frompt outputfontdir topt\n", ProgramName);
    fprintf (stderr, 
"\nwhere inputfontname is the name of a standard fontfile of point size \n");
    fprintf (stderr,
"frompt and outputfontdir is the name of a directory to hold fed-style\n");
    fprintf (stderr,
"definitions of a topt font.  The point sizes are relative, so the output\n");
    fprintf (stderr,
"font will be topt/frompt the size of the input font.\n");
    fprintf (stderr,
"\nIt is important to note that this program does not try to scale the\n");
    fprintf (stderr,
"the font image, but just creates an empty font of the right size.\n");
    fprintf (stderr,
"\nFor example,\n");
    fprintf (stderr,
"\n                %s tr24.onx 24 tr3 3\n", ProgramName);
    fprintf (stderr,
"\nwill scale a 24pt. Times Roman font down to be a 3pt. font and put it\n");
    fprintf (stderr,
"in a subdirectory called tr3.\n");
    exit (1);
}

main (argc, argv)
    int argc;
    char *argv[];
{
    char *inputfontname, *outputfontdirname;
    int frompt, topt;
    FONT *srcfont;
    char *cp;				/* random work pointer */

    ProgramName = argv[0];
    setlinebuf (stderr);

    if (argc != 1+4)			/* sophisticated arg parsing... */
      Usage ();

    inputfontname = argv[1];
    frompt = atoi (argv[2]);
    outputfontdirname = argv[3];
    topt = atoi (argv[4]);

    srcfont = GetFont (inputfontname);
    if (!srcfont) {
	fprintf (stderr, "%s:  unable to read font %s, errno %d, %s.\n",
		 ProgramName, inputfontname, errno, SysError());
	exit (1);
    }

    /*
     * strip and trailing .onx from the output directory name to prevent
     * mishaps.
     */

    if ((cp = rindex (outputfontdirname, '.')) != NULL) {
	if (strcmp (cp, ".onx") == 0) *cp = '\0';
    }

    /*
     * make the directory and cd to it.
     */

    if (mkdir (outputfontdirname, 0775) < 0 && errno != EEXIST) {
	fprintf (stderr, "%s: unable to mkdir %s, errno %d, %s.\n",
		 ProgramName, outputfontdirname, errno, SysError());
	exit (1);
    }

    if (chdir (outputfontdirname) < 0) {
	fprintf (stderr, "%s: unable to chdir to %s, errno %d, %s\n",
		 ProgramName, outputfontdirname, errno, SysError());
	exit (1);
    }
	
    /*
     * and start doing the interesting part, we are in the target directory
     * we do this by bashing the font that we just read in and then writing
     * it back out.
     */

    bash_font (srcfont, frompt, topt);
    write_out_fontdir (srcfont, srcfont->first, srcfont->last);

    exit (0);
}

/*
 * This massages the font so that it can be used as a template for the new
 * font, which involves scaling the 
 *    o strike bitmap
 *    o baseline
 *    o height and width
 *    o width and left arrays
 */


bash_font (f, frompt, topt)
    FONT *f;
    int frompt;
    int topt;
{
    char *procname = "bash_font";
    FontPriv *fp;
    BITMAP *chars;
    int i, w;
    int newheight, newwidth;
    unsigned oldbitmapsize, newbitmapsize;
    short *widtharray, *leftarray;
    int maxwidth;

    /* make sure that everyone does the INTEGER scaling the same way */
#define scale(n) ((((n) * topt) + (frompt - 1)) / frompt)

    fp = FDATA (f);
    chars = fp->strike;

    /*
     * first, scale the width and left arrays
     */
    widtharray = fp->widths;
    leftarray = fp->leftarray;
    leftarray[0] = 0;			/* anchor on left edge */
    newwidth = 0;
    maxwidth = 0;
    for (i = 0; i <= f->last; i++) {	/* scan widths, apply to left */
	if (i < f->first) {		/* sanity check */
	    leftarray[i+1] = widtharray[i] = 0;
	    continue;
	} else {
	    w = widtharray[i] = scale (widtharray[i]);
	    leftarray[i+1] = leftarray[i] + w;
	    if (w > maxwidth) maxwidth = w;
	}
    }

    newheight = scale (chars->height);
    newwidth = leftarray [f->last+1];

    /*
     * rescale the bitmap
     */

    oldbitmapsize = BitmapSize (chars->width, chars->height);
    newbitmapsize = BitmapSize (newwidth, newheight);

    chars->data = (caddr_t) realloc (chars->data, newbitmapsize);
    if (!chars->data) {
	fprintf (stderr,
		 "%s/%s: unable to realloc %u bytes for %dx%d bitmap ",
		 ProgramName, procname, newbitmapsize, newwidth, newheight);
	fprintf (stderr, "(errno %d, %s).\n", errno, SysError());
	exit (1);
    }

    bzero (chars->data, newbitmapsize);

    chars->width = newwidth;
    f->height = chars->height = newheight;
    f->base = scale (f->base);
    f->avg_width = newwidth / (f->last - f->first + 1);
    fp->wpitch = BitmapSize (chars->width, 1);

    fp->fltable = (char **) realloc (fp->fltable, f->height * sizeof (char *));
    if (!fp->fltable) {
	fprintf (stderr,
		 "%s/%s: unable to realloc %u bytes for %d elem fltable.\n",
		 ProgramName, procname, f->height * sizeof (char *),
		 f->height);
	exit (1);
    }
    for (i = 0; i < f->height; i++) {
	fp->fltable[i] = ((caddr_t) chars->data) + i * fp->wpitch;
    }

    return;
}
