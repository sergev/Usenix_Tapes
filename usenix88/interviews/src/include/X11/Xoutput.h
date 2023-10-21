/*
 * Interface to X graphics operations.
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


#ifndef Xoutput_h
#define Xoutput_h

#include <InterViews/Xdefs.h>

/* graphics functions, as in GC.alu */

#define	GXclear			0x0		/* 0 */
#define GXand			0x1		/* src AND dst */
#define GXandReverse		0x2		/* src AND NOT dst */
#define GXcopy			0x3		/* src */
#define GXandInverted		0x4		/* NOT src AND dst */
#define	GXnoop			0x5		/* dst */
#define GXxor			0x6		/* src XOR dst */
#define GXor			0x7		/* src OR dst */
#define GXnor			0x8		/* NOT src AND NOT dst */
#define GXequiv			0x9		/* NOT src XOR dst */
#define GXinvert		0xa		/* NOT dst */
#define GXorReverse		0xb		/* src OR NOT dst */
#define GXcopyInverted		0xc		/* NOT src */
#define GXorInverted		0xd		/* NOT src OR dst */
#define GXnand			0xe		/* NOT src OR NOT dst */
#define GXset			0xf		/* 1 */

/* LineStyle */

#define LineSolid		0
#define LineOnOffDash		1
#define LineDoubleDash		2

/* capStyle */

#define CapNotLast		0
#define CapButt			1
#define CapRound		2
#define CapProjecting		3

/* joinStyle */

#define JoinMiter		0
#define JoinRound		1
#define JoinBevel		2

/* fillStyle */

#define FillSolid		0
#define FillTiled		1
#define FillStippled		2
#define FillOpaqueStippled	3

/* fillRule */

#define EvenOddRule		0
#define WindingRule		1

/* subwindow mode */

#define ClipByChildren		0
#define IncludeInferiors	1

/* SetClipRectangles ordering */

#define Unsorted		0
#define YSorted			1
#define YXSorted		2
#define YXBanded		3

/* CoordinateMode for drawing routines */

#define CoordModeOrigin		0	/* relative to the origin */
#define CoordModePrevious       1	/* relative to previous point */

/* Polygon shapes */

#define Complex			0	/* paths may intersect */
#define Nonconvex		1	/* no paths intersect, but not convex */
#define Convex			2	/* wholly convex */

/* Arc modes for PolyFillArc */

#define ArcChord		0	/* join endpoints of arc */
#define ArcPieSlice		1	/* join endpoints to center of arc */

/* GC components: masks used in CreateGC, CopyGC, ChangeGC, OR'ed into
   GC.stateChanges */

#define GCFunction              (1L<<0)
#define GCPlaneMask             (1L<<1)
#define GCForeground            (1L<<2)
#define GCBackground            (1L<<3)
#define GCLineWidth             (1L<<4)
#define GCLineStyle             (1L<<5)
#define GCCapStyle              (1L<<6)
#define GCJoinStyle		(1L<<7)
#define GCFillStyle		(1L<<8)
#define GCFillRule		(1L<<9) 
#define GCTile			(1L<<10)
#define GCStipple		(1L<<11)
#define GCTileStipXOrigin	(1L<<12)
#define GCTileStipYOrigin	(1L<<13)
#define GCFont 			(1L<<14)
#define GCSubwindowMode		(1L<<15)
#define GCGraphicsExposures     (1L<<16)
#define GCClipXOrigin		(1L<<17)
#define GCClipYOrigin		(1L<<18)
#define GCClipMask		(1L<<19)
#define GCDashOffset		(1L<<20)
#define GCDashList		(1L<<21)
#define GCArcMode		(1L<<22)

#define GCLastBit		22

/*
 * Data structure for setting graphics context.
 */
typedef struct {
	int function;		/* logical operation */
	unsigned long plane_mask;/* plane mask */
	unsigned long foreground;/* foreground pixel */
	unsigned long background;/* background pixel */
	int line_width;		/* line width */
	int line_style;	 	/* LineSolid, LineOnOffDash, LineDoubleDash */
	int cap_style;	  	/* CapNotLast, CapButt, 
				   CapRound, CapProjecting */
	int join_style;	 	/* JoinMiter, JoinRound, JoinBevel */
	int fill_style;	 	/* FillSolid, FillTiled, 
				   FillStippled, FillOpaeueStippled */
	int fill_rule;	  	/* EvenOddRule, WindingRule */
	int arc_mode;		/* ArcChord, ArcPieSlice */
	XPixmap tile;		/* tile pixmap for tiling operations */
	XPixmap stipple;	/* stipple 1 plane pixmap for stipping */
	int ts_x_origin;	/* offset for tile or stipple operations */
	int ts_y_origin;
        XFont font;	        /* default text font for text operations */
	int subwindow_mode;     /* ClipByChildren, IncludeInferiors */
	XBool graphics_exposures;/* boolean, should exposures be generated */
	int clip_x_origin;	/* origin for clipping */
	int clip_y_origin;
	XPixmap clip_mask;	/* bitmap clipping; other calls for rects */
	int dash_offset;	/* patterned/dashed line information */
	char dashes;
} XGCValues;

/*
 * Graphics context.  All Xlib routines deal in this rather than
 * in raw protocol GContext ID's.  This is so that the library can keep
 * a "shadow" set of values, and thus avoid passing values over the
 * wire which are not in fact changing. 
 */

typedef struct _XGC {
    XExtData *ext_data;	/* hook for extension to hang data */
    XGContext gid;	/* protocol ID for graphics context */
    XBool rects;	/* boolean: TRUE if clipmask is list of rectangles */
    XBool dashes;	/* boolean: TRUE if dash-list is really a list */
    unsigned long dirty;/* cache dirty bits */
    XGCValues values;	/* shadow structure of values */
} *XGC;

/*
 * Information about the screen.
 */
struct XScreen {
	XExtData *ext_data;	/* hook for extension to hang data */
	struct _XDisplay *display;/* back pointer to display structure */
	XWindow root;		/* Root window id. */
	int width, height;	/* width and height of screen */
	int mwidth, mheight;	/* width and height of  in millimeters */
	int ndepths;		/* number of depths possible */
	XDepth *depths;		/* list of allowable depths on the screen */
	int root_depth;		/* bits per pixel */
	XVisual *root_visual;	/* root visual */
	XGC default_gc;		/* GC for the root root visual */
	XColormap cmap;		/* default color map */
	unsigned long white_pixel;
	unsigned long black_pixel;	/* White and Black pixel values */
	int max_maps, min_maps;	/* max and min color maps */
	int backing_store;	/* Never, WhenMapped, Always */
	XBool save_unders;	
	long root_input_mask;	/* initial root input mask */
};

/*
 * Format structure; describes ZFormat data the screen will understand.
 */
struct XScreenFormat {
	XExtData *ext_data;	/* hook for extension to hang data */
	int depth;		/* depth of this image format */
	int bits_per_pixel;	/* bits/pixel at this depth */
	int scanline_pad;	/* scanline must padded to this multiple */
};

XGC XCreateGC(XDisplay*, XDrawable, XValueMask, XGCValues*);
void XCopyGC(XDisplay*, XGC src, XValueMask, XGC dst);
void XChangeGC(XDisplay*, XGC, XValueMask, XGCValues*);
void XFreeGC(XDisplay*, XGC);
void XSetState(XDisplay*, XGC, XPixel fg, XPixel bg, int func, XPlaneMask);
void XSetFunction(XDisplay*, XGC, int);
void XSetPlaneMask(XDisplay*, XGC, XPlaneMask);
void XSetForeground(XDisplay*, XGC, XPixel);
void XSetBackground(XDisplay*, XGC, XPixel);
void XSetLineAttributes(
    XDisplay*, XGC, XCardinal width, int style, int cap, int join
);
void XSetDashes(XDisplay*, XGC, int offset, char* dash[], int n);
void XSetFillStyle(XDisplay*, XGC, int);
void XSetFillRule(XDisplay*, XGC, int);
void XQueryBestSize(
    XDisplay*, int, XDrawable, XCardinal width, XCardinal height,
    XCardinal& rwidth, XCardinal& rheight
);
void XQueryBestTile(
    XDisplay*, XDrawable, XCardinal width, XCardinal height,
    XCardinal& rwidth, XCardinal& rheight
);
void XQueryBestStipple(
    XDisplay*, XDrawable, XCardinal width, XCardinal height,
    XCardinal& rwidth, XCardinal& rheight
);
void XSetTile(XDisplay*, XGC, XPixmap);
void XSetStipple(XDisplay*, XGC, XPixmap);
void XSetTSOrigin(XDisplay*, XGC, int x, int y);

void XSetFont(XDisplay*, XGC, XFont);
void XSetClipOrigin(XDisplay*, XGC, int x, int y);
void XSetClipMask(XDisplay*, XGC, XPixmap);
void XSetClipRectangles(
    XDisplay*, XGC, int x, int y, XRectangle[], int n, int ordering
);
void XSetArcMode(XDisplay*, XGC, int);
void XSetSubwindowMode(XDisplay*, XGC, int);
void XSetGraphicsExposures(XDisplay*, XGC, XBool);

/* 
 * Data structures for graphics operations.  On most machines, these are
 * congruent with the wire protocol structures, so reformatting the data
 * can be avoided on these architectures.
 */
typedef struct {
    short x1, y1, x2, y2;
} XSegment;

typedef struct {
    short x, y;
} XPoint;
    
typedef struct {
    short x, y;
    unsigned short width, height;
    short angle1, angle2;
} XArc;

XPixmap XCreatePixmap(
    XDisplay*, XDrawable, XCardinal width, XCardinal height, XCardinal depth
);
void XFreePixmap(XDisplay*, XPixmap);

void XClearArea(
    XDisplay*, XWindow, int x, int y, XCardinal width, XCardinal height,
    XBool exposures
);
void XClearWindow(XDisplay*, XWindow);
void XCopyArea(
    XDisplay*, XDrawable src, XDrawable dst, Xgc, int srcx, int srcy,
    XCardinal width, XCardinal height, int dstx, int dsty
);
void XCopyPlane(
    XDisplay*, XDrawable src, XDrawable dst, Xgc, int srcx, int srcy,
    XCardinal width, XCardinal height, int dstx, int dsty, XPlaneMask plane
);

void XDrawPoint(XDisplay*, XDrawable, Xgc, int x, int y);
void XDrawPoints(XDisplay*, XDrawable, Xgc, XPoint[], int, int mode);
void XDrawLine(XDisplay*, XDrawable, Xgc, int x1, int y1, int x2, int y2);
void XDrawLines(XDisplay*, XDrawable, Xgc, XPoint[], int n, int mode);
void XDrawSegments(XDisplay*, XDrawable, Xgc, XSegment[], int);
void XDrawRectangle(
    XDisplay*, XDrawable, Xgc, int x, int y, XCardinal width, XCardinal height
);
void XDrawRectangles(XDisplay*, XDrawable, Xgc, struct XRectangle[], int);
void XDrawArc(
    XDisplay*, XDrawable, Xgc, int x, int y, XCardinal width, XCardinal height,
    int angle1, int angle2
);
void XDrawArcs(XDisplay*, XDrawable, Xgc, XArc[], int);
void XFillRectangle(
    XDisplay*, XDrawable, Xgc, int x, int y, XCardinal width, XCardinal height
);
void XFillRectangles(XDisplay*, XDrawable, Xgc, XRectangle[], int);
void XFillPolygon(
    XDisplay*, XDrawable, Xgc, XPoint[], int, int shape, int mode
);
void XFillArc(
    XDisplay*, XDrawable, Xgc, int x, int y, XCardinal width, XCardinal height,
    int angle1, int angle2
);
void XFillArcs(XDisplay*, XDrawable, Xgc, XArc[], int);

void XDrawString(XDisplay*, XDrawable, Xgc, int x, int y, const char*, int);
void XDrawImageString(
    XDisplay*, XDrawable, Xgc, int x, int y, const char*, int
);

#endif
