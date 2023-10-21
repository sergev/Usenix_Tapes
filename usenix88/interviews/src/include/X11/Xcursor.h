/*
 * Interface to X cursor operations.
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


#ifndef Xcursor_h
#define Xcursor_h

#include <InterViews/Xdefs.h>

struct XColor;

XCursor XCreateFontCursor(XDisplay*, int shape);
XCursor XCreatePixmapCursor(
    XDisplay*, XPixmap src, XPixmap mask, XColor& fg, XColor& bg, int x, int y
);
XCursor XCreateGlyphCursor(
    XDisplay*, XFont src, XFont mask, unsigned int schar, unsigned int mchar,
    XColor& fg, XColor& bg
);
void XRecolorCursor(XDisplay*, XCursor, XColor& fg, XColor& bg);
void XFreeCursor(XDisplay*, XCursor);
void XQueryBestCursor(
    XDisplay*, XDrawable, XCardinal width, XCardinal height,
    XCardinal& rwidth, XCardinal& rheight
);
void XDefineCursor(XDisplay*, XWindow, XCursor);
void XUndefineCursor(XDisplay*, XWindow);

#endif
