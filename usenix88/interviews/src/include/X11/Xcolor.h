/*
 * Interface to X color manipulation.
 */

/* 
 * Copyright 1985, 1986, 1987 by the Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific, 
 * written prior permission. M.I.T. makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * The X Window System is a Trademark of MIT.
 *
 */


#ifndef Xcolor_h
#define Xcolor_h

#include <InterViews/Xdefs.h>

/* For CreateColormap */

#define XAllocNone		0	/* create map with no entries */
#define XAllocAll		1	/* allocate entire map writeable */


/* Flags used in StoreNamedColor, StoreColors */

#define XDoRed			(1<<0)
#define XDoGreen		(1<<1)
#define XDoBlue			(1<<2)

/*
 * Data structure used by color operations
 */
struct XColor {
	unsigned long pixel;
	unsigned short red, green, blue;
	char flags;  /* do_red, do_green, do_blue */
	char pad;
};

XColormap XCreateColormap(XDisplay*, XWindow, XVisual*, int alloc);
XColormap XCopyColormapAndFree(XDisplay*, XColormap);
void XFreeColormap(XDisplay*, XColormap);
void XSetWindowColormap(XDisplay*, XWindow, XColormap);
XStatus XAllocColor(XDisplay*, XColormap, XColor*);
XStatus XAllocNamedColor(XDisplay*, XColormap, const char*, XColor&, XColor&);
XStatus XLookupColor(XDisplay*, XColormap, const char*, XColor&, XColor&);
void XStoreColors(XDisplay*, XColormap, XColor[], int);
void XStoreColor(XDisplay*, XColormap, XColor&);
XStatus XAllocColorCells(
    XDisplay*, XColormap, XBool, XPlaneMask plane[], XCardinal nplanes,
    XPixel[], XCardinal npixels
);
XStatus XAllocColorPlanes(
    XDisplay*, XColormap, XBool, XPixel[], XCardinal npixels,
    int nreds, int ngreens, int nblues,
    XPixel& rmask, XPixel& gmask, XPixel& bmask
);
void XStoreNamedColor(XDisplay*, XColormap, const char*, XPixel, int);
void XFreeColors(XDisplay*, XColormap, XPixel[], int, XCardinal nplanes);
XStatus XQueryColor(XDisplay*, XColormap, XColor&);
XStatus XQueryColors(XDisplay*, XColormap, XColor[], int ncolors);

#endif
