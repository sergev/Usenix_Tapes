/*
 * Input cursors.
 */

#include <InterViews/cursor.h>
#include <InterViews/paint.h>
#include <InterViews/Xcolor.h>
#include <InterViews/Xcursor.h>
#include <InterViews/Xoutput.h>

static CursorPattern defaultPat = {
    0x0000, 0x7f80, 0x7f80, 0x7e00, 0x7e00, 0x7f00, 0x7f80, 0x67c0,
    0x63e0, 0x01f0, 0x00f8, 0x007c, 0x003e, 0x001e, 0x000c, 0x0000
};

static CursorPattern defaultMask = {
    0xffc0, 0xffc0, 0xffc0, 0xff80, 0xff00, 0xff80, 0xffc0, 0xffe0,
    0xf7f0, 0xe3f8, 0x01fc, 0x00fe, 0x007f, 0x003f, 0x001e, 0x000c
};

static CursorPattern arrowPat = {
    0x0000, 0x6000, 0x7800, 0x3E00, 0x3F80, 0x1FE0, 0x1FF8, 0x0FFE, 
    0x0FF8, 0x07E0, 0x07F0, 0x03B8, 0x039C, 0x010E, 0x0106, 0x0000, 
};

static CursorPattern arrowMask = {
    0xC000, 0xF000, 0x7C00, 0x7F00, 0x3FC0, 0x3FF0, 0x1FFC, 0x1FFF, 
    0x0FFC, 0x0FF0, 0x07F8, 0x07FC, 0x03BE, 0x031F, 0x010F, 0x0106, 
};

static CursorPattern crossHairsPat = {
    0x0000, 0x0380, 0x0380, 0x0100, 0x0100, 0x0100, 0x600C, 0x7C7C, 
    0x600C, 0x0100, 0x0100, 0x0100, 0x0380, 0x0380, 0x0000, 0x0000, 
};

static CursorPattern crossHairsMask = {
    0x07C0, 0x07C0, 0x07C0, 0x0380, 0x0380, 0xE38E, 0xFC7E, 0xFC7E, 
    0xFC7E, 0xE38E, 0x0380, 0x0380, 0x07C0, 0x07C0, 0x07C0, 0x0000, 
};

static CursorPattern textPat = {
    0x0000, 0x4400, 0x2800, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 
    0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x2800, 0x4400, 0x0000
};

static CursorPattern textMask = {
    0x0000, 0xCC00, 0x7800, 0x3000, 0x3000, 0x3000, 0x3000, 0x3000, 
    0x3000, 0x3000, 0x3000, 0x3000, 0x3000, 0x7800, 0xCC00, 0x0000, 
};

static CursorPattern hourglassPat = {
    0x0000, 0x7FC0, 0x2080, 0x2080, 0x3180, 0x1F00, 0x0E00, 0x0400, 
    0x0400, 0x0E00, 0x1100, 0x2480, 0x2E80, 0x3F80, 0x7FC0, 0x0000, 
};

static CursorPattern hourglassMask = {
    0xFFE0, 0xFFE0, 0xFFE0, 0x7FC0, 0x7FC0, 0x3F80, 0x1F00, 0x0E00, 
    0x0E00, 0x1F00, 0x3F80, 0x7FC0, 0x7FC0, 0xFFE0, 0xFFE0, 0xFFE0
};

static CursorPattern ulPat = {
    0x0000, 0x7FE0, 0x7FE0, 0x6000, 0x6000, 0x6000, 0x6000, 0x6000,
    0x6000, 0x6000, 0x6000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
};

static CursorPattern ulMask = {
    0xFFF0, 0xFFF0, 0xFFF0, 0xFFF0, 0xF000, 0xF000, 0xF000, 0xF000,
    0xF000, 0xF000, 0xF000, 0xF000, 0x0000, 0x0000, 0x0000, 0x0000
};

static CursorPattern urPat = {
    0x0000, 0x07FE, 0x07FE, 0x0006, 0x0006, 0x0006, 0x0006, 0x0006,
    0x0006, 0x0006, 0x0006, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
};

static CursorPattern urMask = {
    0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x000F, 0x000F, 0x000F, 0x000F,
    0x000F, 0x000F, 0x000F, 0x000F, 0x0000, 0x0000, 0x0000, 0x0000
};

static CursorPattern llPat = {
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x6000, 0x6000, 0x6000,
    0x6000, 0x6000, 0x6000, 0x6000, 0x6000, 0x7fe0, 0x7fe0, 0x0000
};

static CursorPattern llMask = {
    0x0000, 0x0000, 0x0000, 0x0000, 0xF000, 0xF000, 0xF000, 0xF000,
    0xF000, 0xF000, 0xF000, 0xF000, 0xFFF0, 0xFFF0, 0xFFF0, 0xFFF0
};

static CursorPattern lrPat = {
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0006, 0x0006, 0x0006,
    0x0006, 0x0006, 0x0006, 0x0006, 0x0006, 0x07FE, 0x07FE, 0x0000
};

static CursorPattern lrMask = {
    0x0000, 0x0000, 0x0000, 0x0000, 0x000F, 0x000F, 0x000F, 0x000F,
    0x000F, 0x000F, 0x000F, 0x000F, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF
};

static CursorPattern noPat = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

Cursor* defaultCursor;
Cursor* arrow;
Cursor* crosshairs;
Cursor* ltextCursor;
Cursor* rtextCursor;
Cursor* hourglass;
Cursor* upperleft;
Cursor* upperright;
Cursor* lowerleft;
Cursor* lowerright;
Cursor* noCursor;

/*
 * Define the builtin cursors.
 */

void InitCursors () {
    defaultCursor = new Cursor(1, 15, defaultPat, defaultMask, black, white);
    arrow = new Cursor(1, 15, arrowPat, arrowMask, black, white);
    crosshairs = new Cursor(7, 9, crossHairsPat, crossHairsMask, black, white);
    ltextCursor = new Cursor(4, 8, textPat, textMask, black, white);
    rtextCursor = new Cursor(0, 8, textPat, textMask, black, white);
    hourglass = new Cursor(6, 8, hourglassPat, hourglassMask, black, white);
    upperleft = new Cursor(0, 15, ulPat, ulMask, black, white);
    upperright = new Cursor(15, 15, urPat, urMask, black, white);
    lowerleft = new Cursor(0, 0, llPat, llMask, black, white);
    lowerright = new Cursor(15, 0, lrPat, lrMask, black, white);
    noCursor = new Cursor(0, 0, noPat, noPat, black, white);
}

#ifdef X10

/*
 * Create a new cursor.  Each cursor has a "hot spot" position relative
 * to the lower left corner.  The first pattern specifies which pixels
 * in the cursor should be in the foreground color (1) and which in the
 * background color (0).  The second pattern specifies which pixels are
 * actually effected.  Not all devices will support this "outline" capability.
 */

Cursor::Cursor (
    short xoff, short yoff, CursorPattern p, CursorPattern m,
    Color* fg, Color* bg
) {
    typedef int* Ptr;
    register int i;
    short pattern[cursorHeight], mask[cursorHeight];
    register int* srcpat, * srcmask;
    register short* dstpat, * dstmask;
    int f, b;

    srcpat = p;
    dstpat = pattern;
    srcmask = m;
    dstmask = mask;
    for (i = 0; i < cursorHeight; i++) {
#	ifndef noswap	/* X orders bits wrong */
	    register int j;
	    register unsigned s1, s2;

	    *dstpat = 0;
	    *dstmask = 0;
	    s1 = 1;
	    s2 = (1 << 15);
	    for (j = 0; j < 16; j++) {
		if ((s1 & *srcpat) != 0) {
		    *dstpat |= s2;
		}
		if ((s1 & *srcmask) != 0) {
		    *dstmask |= s2;
		}
		s1 <<= 1;
		s2 >>= 1;
	    }
#       else /* bit-swapping not necessary */
	    *dstpat = *srcpat;
	    *dstmask = *srcmask;
#       endif
	++srcpat;
	++dstpat;
	++srcmask;
	++dstmask;
    }
    if (fg == nil) {
	f = black->PixelValue();
    } else {
	f = fg->PixelValue();
    }
    if (bg == nil) {
	b = white->PixelValue();
    } else {
	b = bg->PixelValue();
    }
    info = XCreateCursor(
	cursorWidth, cursorHeight, pattern, mask,
	xoff, cursorHeight - 1 - yoff, f, b, GXcopy
    );
}

Cursor::~Cursor () {
    XFreeCursor(info);
}

#else

static XPixmap MakeCursorPixmap (int* data) {
    XPixmap dst;
    Xgc g;
    register int i, j;
    register unsigned s1, s2;

    dst = XCreatePixmap(_World_dpy, _World_root, cursorWidth, cursorHeight, 1);
    g = XCreateGC(_World_dpy, dst, 0, nil);
    XSetForeground(_World_dpy, g, 0);
    XSetFillStyle(_World_dpy, g, FillSolid);
    XFillRectangle(_World_dpy, dst, g, 0, 0, cursorWidth, cursorHeight);
    XSetForeground(_World_dpy, g, 1);
    for (i = 0; i < cursorHeight; i++) {
	s1 = data[i];
	s2 = 1;
	for (j = 0; j < cursorWidth; j++) {
	    if ((s1 & s2) != 0) {
		XDrawPoint(_World_dpy, dst, g, cursorWidth-1-j, i);
	    }
	    s2 <<= 1;
	}
    }
    XFreeGC(_World_dpy, g);
    return dst;
}

static void MakeColor (Color* c, XColor& xc) {
    int r, g, b;

    xc.pixel = c->PixelValue();
    c->Intensities(r, g, b);
    xc.red = r;
    xc.green = g;
    xc.blue = b;
}

/*
 * Create a new cursor.  Each cursor has a "hot spot" position relative
 * to the lower left corner.  The first pattern specifies which pixels
 * in the cursor should be in the foreground color (1) and which in the
 * background color (0).  The second pattern specifies which pixels are
 * actually effected.  Not all devices will support this "outline" capability.
 */

Cursor::Cursor (
    short xoff, short yoff, CursorPattern p, CursorPattern m,
    Color* fg, Color* bg
) {
    XPixmap pat, mask;
    XColor f, b;

    pat = MakeCursorPixmap(p);
    mask = MakeCursorPixmap(m);
    MakeColor(fg, f);
    MakeColor(bg, b);
    info = XCreatePixmapCursor(
	_World_dpy, pat, mask, f, b, xoff, cursorHeight - 1 - yoff
    );
    XFreePixmap(_World_dpy, pat);
    XFreePixmap(_World_dpy, mask);
}

Cursor::~Cursor () {
    XFreeCursor(_World_dpy, info);
}

#endif
