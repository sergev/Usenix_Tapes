/*
 * Interface to X window operations.
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


#ifndef Xwindow_h
#define Xwindow_h

#include <InterViews/Xdefs.h>

/* Window classes used by CreateWindow */

#define XInputOutput		1
#define XInputOnly		2

/* Window attributes for CreateWindow and ChangeWindowAttributes */

#define CWBackPixmap		(1L<<0)
#define CWBackPixel		(1L<<1)
#define CWBorderPixmap		(1L<<2)
#define CWBorderPixel           (1L<<3)
#define CWBitGravity		(1L<<4)
#define CWWinGravity		(1L<<5)
#define CWBackingStore          (1L<<6)
#define CWBackingPlanes	        (1L<<7)
#define CWBackingPixel	        (1L<<8)
#define CWOverrideRedirect	(1L<<9)
#define CWSaveUnder		(1L<<10)
#define CWEventMask		(1L<<11)
#define CWDontPropagate	        (1L<<12)
#define CWColormap		(1L<<13)
#define CWCursor	        (1L<<14)

/* ConfigureWindow structure */

#define CWX			(1<<0)
#define CWY			(1<<1)
#define CWWidth			(1<<2)
#define CWHeight		(1<<3)
#define CWBorderWidth		(1<<4)
#define CWSibling		(1<<5)
#define CWStackMode		(1<<6)


/* Bit Gravity */

#define ForgetGravity		0
#define NorthWestGravity	1
#define NorthGravity		2
#define NorthEastGravity	3
#define WestGravity		4
#define CenterGravity		5
#define EastGravity		6
#define SouthWestGravity	7
#define SouthGravity		8
#define SouthEastGravity	9
#define StaticGravity		10

/* Window gravity + bit gravity above */

#define UnmapGravity		0

/* Used in CreateWindow for backing-store hint */

#define NotUseful               0
#define WhenMapped              1
#define Always                  2

/* Used in GetWindowAttributes reply */

#define IsUnmapped		0
#define IsUnviewable		1
#define IsViewable		2

/* Used in ChangeSaveSet */

#define SetModeInsert           0
#define SetModeDelete           1

/* Used in ChangeCloseDownMode */

#define DestroyAll              0
#define RetainPermanent         1
#define RetainTemporary         2

/* Window stacking method (in configureWindow) */

#define Above                   0
#define Below                   1
#define TopIf                   2
#define BottomIf                3
#define Opposite                4

/* Circulation direction */

#define RaiseLowest             0
#define LowerHighest            1

/* Property modes */

#define PropModeReplace         0
#define PropModePrepend         1
#define PropModeAppend          2

struct XScreen;

/*
 * Data structure for setting window attributes.
 */
typedef struct {
    XPixmap background_pixmap;	/* background or None or ParentRelative */
    XPixel background_pixel;	/* background pixel */
    XPixmap border_pixmap;	/* border of the window */
    XPixel border_pixel;	/* border pixel value */
    int bit_gravity;		/* one of bitgravity values */
    int win_gravity;		/* one of the window gravity values */
    int backing_store;		/* NotUseful, WhenMapped, Always */
    XPlaneMask backing_planes;	/* planes to be preserved if possible */
    XPixel backing_pixel;	/* value to use in restoring planes */
    XBool save_under;		/* should bits under be saved? (popups) */
    long event_mask;		/* set of events that should be saved */
    long do_not_propagate_mask;	/* set of events that should not propagate */
    XBool override_redirect;	/* boolave value for override-redirect */
    XColormap colormap;		/* color map to be associated with window */
    XCursor cursor;		/* cursor to be displayed (or None) */
} XSetWindowAttributes;

typedef struct {
    int x, y;			/* location of window */
    int width, height;		/* width and height of window */
    int border_width;		/* border width of window */
    int depth;          	/* depth of window */
    XVisual *visual;		/* the associated visual structure */
    XWindow root;        	/* root of screen containing window */
    int window_class;		/* InputOutput, InputOnly*/
    int bit_gravity;		/* one of bit gravity values */
    int win_gravity;		/* one of the window gravity values */
    int backing_store;		/* NotUseful, WhenMapped, Always */
    XPlaneMask backing_planes;	/* planes to be preserved if possible */
    XPixel backing_pixel;	/* value to be used when restoring planes */
    XBool save_under;		/* boolean, should bits under be saved? */
    XColormap colormap;		/* color map to be associated with window */
    XBool map_installed;	/* boolean, is color map currently installed*/
    int map_state;		/* IsUnmapped, IsUnviewable, IsViewable */
    long all_event_masks;	/* set of events all people have interest in*/
    long your_event_mask;	/* my event mask */
    long do_not_propagate_mask; /* set of events that should not propagate */
    XBool override_redirect;	/* boolean value for override-redirect */
    XScreen *screen;		/* back pointer to correct screen */
} XWindowAttributes;

/* 
 * Data structure for XReconfigureWindow
 */
typedef struct {
    int x, y;
    int width, height;
    int border_width;
    XWindow sibling;
    int stack_mode;
} XWindowChanges;

XWindow XCreateWindow(
    XDisplay*, XWindow parent, int x, int y,
    XCardinal width, XCardinal height, XCardinal bwidth, XCardinal depth,
    XCardinal window_class, XVisual*, XValueMask, XSetWindowAttributes*
);
XWindow XCreateSimpleWindow(
    XDisplay*, XWindow, int x, int y,
    XCardinal width, XCardinal height, XCardinal bwidth,
    XPixel border, XPixel background
);
void XDestroyWindow(XDisplay*, XWindow);
void XDestroySubwindows(XDisplay*, XWindow);
void XMapWindow(XDisplay*, XWindow);
void XMapRaised(XDisplay*, XWindow);
void XMapSubwindows(XDisplay*, XWindow);
void XUnmapWindow(XDisplay*, XWindow);
void XUnmapSubwindows(XDisplay*, XWindow);
void XConfigureWindow(XDisplay*, XWindow, XValueMask, XWindowChanges*);
void XMoveWindow(XDisplay*, XWindow, int x, int y);
void XResizeWindow(XDisplay*, XWindow, XCardinal width, XCardinal height);
void XMoveResizeWindow(
    XDisplay*, XWindow, int x, int y, XCardinal width, XCardinal height
);
void XSetWindowBorderWidth(XDisplay*, XWindow, XCardinal width);
void XRaiseWindow(XDisplay*, XWindow);
void XLowerWindow(XDisplay*, XWindow);
void XCirculateSubwindows(XDisplay*, XWindow, int direction);
void XCirculateSubwindowsUp(XDisplay*, XWindow);
void XCirculateSubwindwosDown(XDisplay*, XWindow);
void XRestackWindows(XDisplay*, XWindow[], int nwindows);
void XChangeWindowAttributes(
    XDisplay*, XWindow, XValueMask, XSetWindowAttributes*
);
void XSetWindowBackground(XDisplay*, XWindow, XPixel);
void XSetWindowBackgroundPixmap(XDisplay*, XWindow, XPixmap);
void XSetWindowBorder(XDisplay*, XWindow, XPixel);
void XSetWindowBorderPixmap(XDisplay*, XWindow, XPixmap);

XStatus XQueryTree(
    XDisplay*, XWindow, XWindow& root, XWindow& parent,
    XWindow*& children, int& nchildren
);
XStatus XGetWindowAttributes(XDisplay*, XWindow, XWindowAttributes&);
XStatus XGetGeometry(
    XDisplay*, XDrawable, XDrawable& root, int& x, int& y,
    XCardinal& width, XCardinal& height, XCardinal& bwidth, XCardinal& depth
);
int XTranslateCoordinates(
    XDisplay*, XWindow src, XWindow dst, int srcx, int srcy,
    int& dstx, int& dsty, XWindow& child
);

int XGetWindowProperty(
    XDisplay*, XWindow, XAtom, long offset, long len, XBool del,
    XAtom req_type, XAtom& actual_type, int& actual_format, unsigned long& n,
    long& bytes_after, unsigned char*& prop
);
XAtom* XListProperties(XDisplay*, XWindow, int& nprop);
void XChangeProperty(
    XDisplay*, XWindow, XAtom prop, XAtom type, int format, int mode,
    unsigned char* data, int nelements
);
void XRotateWindowProperties(XDisplay*, XWindow, XAtom[], int n, int npos);
void XDeleteProperty(XDisplay*, XWindow, XAtom);

void XSetSelectionOwner(XDisplay*, XAtom, XWindow, XTime);
XWindow XGetSelectionOwner(XDisplay*, XAtom);
void XConvertSelection(
    XDisplay*, XAtom selection, XAtom target, XAtom prop,
    XWindow requestor, XTime
);

void XStoreName(XDisplay*, XWindow, const char*);
XStatus XFetchName(XDisplay*, XWindow, char*&);
void XSetIconName(XDisplay*, XWindow, char*);
XStatus XGetIconName(XDisplay*, XWindow, char*&);
void XSetCommand(XDisplay*, XWindow, char* argv[], int argc);

#endif
