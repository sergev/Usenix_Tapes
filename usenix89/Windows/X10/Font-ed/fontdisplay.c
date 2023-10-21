/*
#
# $Source: /local/projects/X/fed/RCS/fontdisplay.c,v $
# $Header: fontdisplay.c,v 1.12 87/04/08 08:49:12 jim Exp $
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
#define _DONT_INCLUDE_X_
#include "font.h"
#include "elem.h"
#include "mapfile.h"
#include <ctype.h>

extern int errno;

static short _NORASTER_ = 0;
#define NORASTER &_NORASTER_;

do_font_display (filename, fontdirname, 
		 geomask, x, y, bdr, background, foreground,
		 horzpadding, vertpadding,
		 padding, printnewlines, verbose)
    char *filename;			/* input file to display */
    char *fontdirname;			/* FED fontdir name */
    int geomask;			/* XParseGeometry return mask */
    int x, y, bdr;			/* upper left of window */
    int background, foreground;		/* colors in which to paint */
    int horzpadding, vertpadding;	/* intercharacter padding */
    int padding;			/* padding betwen border and chars */
    int printnewlines;			/* bool, t:print \n font symbols */
    int verbose;			/* bool, t:complain */
{
    FONT f;
    FontElementList *elemlist;
    FontElement *elemarray[CHARSPERFONT];    /* array of pointers */
    FontElement *elem;
    struct mapfile *textlines;
    int ntextlines;
    int i, j;
    int winwidth, winheight;
    Window w;
    WindowInfo wininfo;
    XEvent event;
    XExposeEvent *exposep;

    ntextlines = mapfilein (filename, &textlines);
    if (ntextlines < 0) {
	Error ("unable to read text file", filename);	 /* exits */
    }

    elemlist = read_directory (fontdirname);	/* exits */

    if (chdir (fontdirname) < 0) {
	Error ("unable to chdir to", fontdirname);    /* exits */
    }

    read_profile (&f);			/* exits on error */

    /*
     * set the elem array so that we can just index into the array to locate
     * the character information.  We can then use the macros given below.
     * We can then be a little clever and only read in the bitmaps that we
     * need, ala demand-paging.
     */

#define ElemOf(c) (elemarray[(unsigned) (c)])
#define ElemPresent(c) (ElemOf(c) != NULL)
#define RasterOf(c) (ElemOf(c)->raster)
#define RasterPOf(c) &RasterOf(c)
#define BitmapReadin(c) (RasterOf(c) != NULL)

    bzero ((char *) elemarray, sizeof elemarray);
    for (elem = elemlist->head; elem; elem = elem->next) {
	elem->width = elem->size = 0;			 /* clear for safety */
	elem->raster = NULL;					    /* ditto */
	elem->height = f.height;	    /* set font height for all chars */
	elemarray[elem->num] = elem;
    }

    /*
     * Iter over the lines of text paging in the elements that are needed
     * and computing spacing.  Once we know the size of the window that will
     * be needed to display the output we make it, map it, and handle expose
     * events to paint the information.
     */

#define relop(a,b,op) ((a) op (b) ? (a) : (b))
#define min(a,b) relop ((a), (b), <)
#define max(a,b) relop ((a), (b), >)

    winheight = (ntextlines * (f.height + vertpadding) - vertpadding +
		 2*padding);
    winwidth = 0;

    /*
     * Padding is defined to be between characters, not between text and
     * window border.
     */

    for (i = 0; i < ntextlines; i++) {
	int linelen = textlines[i].len;
	char *line = textlines[i].line;
	int linewidth = 2*padding;

	for (j = 0; j < linelen; j++) {
	    unsigned char c = (line[j] & 0xff);
	    unsigned uc = (unsigned) c;
	    int charwidth, charheight;
	    int width, height, x_hot, y_hot, rastersize;
	    short *raster;

	    elem = ElemOf (uc);
	    raster = NULL;
	    rastersize = 0;

	    if (!elem) {		/* then char not in font */
		if (verbose) {
		    errno = 0;
		    MessageHeader (NULL, NULL, NULL);
		    fprintf (stderr,
			     "file %s, line %d, pos %d, char %c (0%03o)",
			     filename, i+1, j+1,
			     isprint(c) ? c : '?', c);
		    fprintf (stderr, "not in font %s.\n", fontdirname);
		}
		continue;		/* goto next element */
	    }

	    if (!elem->raster) {	/* then [try to] read it in */
		char bmfname[4];
		FILE *bmfilep;

		bmfname[0] = bmfname[1] = bmfname[2] = bmfname[3] = '\0';
		sprintf (bmfname, "%03o", (int) c);

		bmfilep = fopen (bmfname, "r");
		if (!bmfilep) {
		    if (verbose) {
			Error ("unable to open bitmap file", bmfname);
		    }
		    width = height = x_hot = y_hot = rastersize = 0;
		    raster = NORASTER;
		    goto got_char;
		}

		/*
		 * Now try to read in the file.  First assume that it is
		 * in standard binary format, otherwise try ASCII before
		 * barfing.
		 */

		raster = ReadBitmapFile (bmfilep, &width, &height,
					 &x_hot, &y_hot, &rastersize);
		if (!raster) {
		    rewind (bmfilep);
		    raster = ReadAsciiBitmapFile (bmfilep, &width, &height,
						  &x_hot, &y_hot, &rastersize);
		    if (!raster) {
			Warning ("unable to read bitmap file", bmfname);
			width = height = x_hot = y_hot = rastersize = 0;
			raster = NORASTER;
		    } /*fi*/
		} /*fi*/

		fclose (bmfilep);

		/*
		 * now add the width of this character to the line width
		 */
	      got_char:
		elem->width = width;
		elem->size = rastersize;
		elem->raster = raster;
	    } else {
		width = elem->width;
	    }
	    linewidth += width + horzpadding;
	    if (width == 0 && c == f.space)
	      linewidth += f.avg_width >> 1;
	} /*rof - for j in chars in line*/

	/*
	 * and keep track of the maximum width
	 */

	linewidth -= horzpadding;
	winwidth = max (winwidth, linewidth);
    } /*rof - for i in lines of text*/

    /*
     * If everything went okay we should now know the proper size of the
     * window, and we should have read in the characters that we will need
     * (and only those characters).  We can then got and make the window
     * and display the characters.
     */

    if (!XQueryWindow (RootWindow, &wininfo)) {
	Error ("unable to query root window on display", DisplayName());
					/* exits */
    }

    if (geomask & XNegative) {
	x = wininfo.width - 1 - bdr - x - bdr;
    }
    if (geomask & YNegative) {
	y = wininfo.height - 1 - bdr - y - bdr;
    }

    w = XCreateWindow (RootWindow, x, y, winwidth, winheight, bdr, 
		       BlackPixmap, XMakeTile (background));
    if (!w) {
	MessageHeader (NULL, NULL, NULL);
	fprintf (stderr,
		 "unable to create =%dx%d+%d+%d window on display %s.\n",
		 winwidth, winheight, x, y, DisplayName());
	exit (1);
    }

    XSelectInput (w, ButtonPressed | ExposeWindow | ExposeRegion);
    XMapWindow (w);
    XFlush ();

    exposep = (XExposeEvent *) &event;
    while (1) {				/* look until user presses or ^C */
	XNextEvent (&event);

	switch (event.type) {
	  case ButtonPressed:
	    goto done;
	  case ExposeWindow:
	  case ExposeRegion:
	    refresh_window (w, textlines, ntextlines, elemarray, &f,
			    exposep->x, exposep->y,
			    exposep->width, exposep->height, foreground,
			    background, horzpadding, vertpadding, padding,
			    printnewlines);
	    break;
	  default:
	    errno = 0;
	    MessageHeader (NULL, NULL, NULL);
	    fprintf (stderr, "unknown event type 0x%04lx\n", event.type);
	    break;
	}
    }

  done:
    XDestroyWindow (w);
    XFlush ();
    return;
}

/*
 * Repaint the screen on refresh events.  Clear the damaged area and then 
 * repaint all of the characters that would fall into that area.
 */

static refresh_window (w, textlines, nlines, elemarray, f, x, y, width, height,
		       foreground, background, horzpadding, vertpadding,
		       padding, printnewlines)
    Window w;				/* X window in which to scribble */
    struct mapfile textlines[];		/* text to be displayed */
    int nlines;				/* number of lines in textlines */
    FontElement *elemarray[];		/* font elements */
    FONT *f;				/* font */
    int x, y, width, height;		/* area to refresh */
    int background, foreground;		/* colors to use */
    int horzpadding, vertpadding;	/* padding to use in between chars */
    int padding;			/* padding between border and chars */
    int printnewlines;			/* bool, t:don't eat \n */
{
    int startline, stopline;		/* where to start and stop */
    FontElement *elem;			/* the font elem for a character */
    int dstx, dsty;			/* dst coords for each char bitmap */

    startline = y / (f->height + vertpadding);	  /* 0 is top line */
    stopline = (y + height) / (f->height + vertpadding);
    if (stopline >= nlines) stopline = nlines - 1;

			       /* clear the damaged area to get rid of trash */
    XPixFill (w, x, y, width, height, background, NULL, GXcopy, AllPlanes);

    dsty = startline * (f->height + vertpadding);
    for (; startline <= stopline; startline++) {       /* iter over affected */
	int i, len;			/* work vars for scanning lines */
	char *chars;			/* the chars in  a line */
	unsigned c;			/* a char in a line */

        dstx = 0;
	len = textlines[startline].len;
	chars = textlines[startline].line;
	for (i = 0; i < len; i++, chars++) {	/* iter over chars in line */
	    unsigned cwidth;

	    if (*chars == '\n' && !printnewlines) continue;
	    c = (((unsigned) *chars) & 0xff);
	    elem = ElemOf (c);
	    if (elem->width == 0 && c == f->space)
	      cwidth = f->avg_width >> 1;
	    else if (!elem || elem->width < 1 || elem->size < 1) {
		cwidth = 0;
	    } else {
		cwidth = elem->width;
					/* if inside damaged area */
		if ((dstx + cwidth >= x) && (dstx <= (x + width))) {
		    XBitmapBitsPut (w, dstx + padding, dsty + padding,
				    elem->width, elem->height, elem->raster,
				    foreground, background, NULL, GXcopy,
				    AllPlanes);
		}
	    }

	    dstx += cwidth + horzpadding;
	    if (dstx >= x + width) break;    /* skip rest of loop */
	} /*rof - for each char in a line*/
	dsty += f->height + vertpadding;
    } /*rof - for each affected line*/
    XFlush ();
    return;
}
