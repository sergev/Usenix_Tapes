/* $Header: /interviews/new/include/RCS/Xlib.h,v 1.3 87/07/17 16:48:32 hegarty Exp $ */
/* Copyright    Massachusetts Institute of Technology    1985	*/

/*
 *	Xlib.h - Header definition and support file for the C subroutine
 *	interface library (Xlib) to the X Window System Protocol.
 *
 */

#include <InterViews/X.h>

#define XStatus int
#define XId long
#define XClearVertexFlag() (_XlibCurrentDisplay->lastdraw = NULL)
#define XMakePattern(pattern, patlen, patmul)\
	((Pattern)(((patmul) << 20) | (((patlen) - 1) << 16) | (pattern) ))
#define Xdpyno() (_XlibCurrentDisplay->fd)
#define XRootWindow (_XlibCurrentDisplay->root)
#define XBlackPixmap (_XlibCurrentDisplay->black)
#define XWhitePixmap (_XlibCurrentDisplay->white)
#define AllPlanes (~0)
#define XQLength() (_XlibCurrentDisplay->qlen)
#define XDisplayType() (_XlibCurrentDisplay->dtype)
#define XDisplayPlanes() (_XlibCurrentDisplay->dplanes)
#define XDisplayCells() (_XlibCurrentDisplay->dcells)
#define XProtocolVersion() (_XlibCurrentDisplay->vnumber)
#define XDisplayName() (_XlibCurrentDisplay->displayname)

/* Bitmask returned by XParseGeometry().  Each bit tells if the corresponding
   value (x, y, width, height) was found in the parsed string. */

#define NoValue	0x0000
#define XValue  0x0001
#define YValue	0x0002
#define WidthValue  0x0004
#define HeightValue  0x0008
#define AllValues 0x000F
#define XNegative 0x0010
#define YNegative 0x0020

/* Definition of a generic event.  It must be cast to a specific event
 * type before one can read event-specific data */

typedef struct _XEvent {
    	unsigned long type;   /* of event (KeyPressed, ExposeWindow, etc.) */
	XWindow window;	      /* which selected this event */
	long pad_l1, pad_l2;  /* event-specific data */
	XWindow subwindow;    /* child window (if any) where event happened */
	long pad_l4; 	      /* event-specific data */
} XEvent;


/*
 * _QEvent datatype for use in input queueing.
 */
typedef struct _qevent {
	struct _qevent *next;
	XEvent event;
} _QEvent;


/*
 * Display datatype maintaining display specific data.
 */
typedef struct _display {
	int fd;			/* Network socket. */
	XWindow root;		/* Root window id. */
	int vnumber;		/* X protocol version number. */
	int dtype;		/* X server display device type. */
	int dplanes;		/* Number of display bit planes. */
	int dcells;		/* Number of display color map cells. */
	_QEvent *head, *tail;	/* Input event queue. */
	int qlen;		/* Length of input event queue */
	int request;		/* Id of last request. */
	char * lastdraw;	/* Last draw request. */
	char *buffer;		/* Output buffer starting address. */
	char *bufptr;		/* Output buffer index pointer. */
	char *bufmax;		/* Output buffer maximum+1 address. */
	int squish;		/* Squish MouseMoved events? */
	XPixmap black, white;   /* Constant tile pixmaps */
	char *displayname;	/* "host:display" string used on this connect*/
	int width, height;	/* width and height of display */
} XDisplay;


/*
 * XAssoc - Associations used in the XAssocTable data structure.  The 
 * associations are used as circular queue entries in the association table
 * which is contains an array of circular queues (buckets).
 */
typedef struct _x_assoc {
	struct _x_assoc *next;	/* Next object in this bucket. */
	struct _x_assoc *prev;	/* Previous obejct in this bucket. */
	XDisplay *display;	/* Display which ownes the id. */
	XId x_id;		/* X Window System id. */
	char *data;		/* Pointer to untyped memory. */
} XAssoc;

/* 
 * XAssocTable - X Window System id to data structure pointer association
 * table. An XAssocTable consists of a pointer to an array of XAssoc's
 * (table) which are circular queue (bucket) headers and an integer (size)
 * representing the number of circular queues (buckets) in the array.
 */

typedef struct _x_assoc _assocarray[];	/* C++ can't handle it all together */

typedef struct _x_assoc_table {
	_assocarray *table;		/* Pointer to array of queues */
	int size;			/* Table size. */
} XAssocTable;


/* 
 * Data returned by XQueryWindow.
 */
typedef struct _WindowInfo {
	short width, height;	/* Width and height. */
	short x, y;		/* X and y coordinates. */
	short bdrwidth;		/* Border width. */
	short mapped;		/* IsUnmapped, IsMapped or IsInvisible.*/
	short type;		/* IsTransparent, IsOpaque or IsIcon. */
	XWindow assoc_wind;	/* Associated icon or opaque Window. */
} XWindowInfo;


/* 
 * Data returned by XQueryFont.
 */
typedef struct _FontInfo {
	XFont id;
	short height, width, baseline, fixedwidth;
	unsigned char firstchar, lastchar;
	short *widths;		/* pointer to width array in OpenFont */
} XFontInfo;


/*
 * Data structure used by color operations; ints rather than shorts
 * to keep 16 bit protocol limitation out of the library.
 */
typedef struct _Color {
	int pixel;
	unsigned short red, green, blue;
} XColor;


/*
 * Data structure use by XCreateTiles.
 */
typedef struct _TileFrame {
	int pixel;		/* Pixel color for constructing the tile. */
	XPixmap pixmap;		/* Pixmap id of the pixmap, filled in later. */
} XTileFrame;


/*
 * Data structures used by XCreateWindows XCreateTransparencies and
 * XCreateWindowBatch.
 */
typedef struct _OpaqueFrame {
	XWindow self;		/* window id of the window, filled in later */
	short x, y;		/* where to create the window */
	short width, height;	/* width and height */
	short bdrwidth;		/* border width */
	XPixmap border;		/* border pixmap */
	XPixmap background;	/* background */
} XOpaqueFrame;

typedef struct _TransparentFrame {
	XWindow self;		/* window id of the window, filled in later */
	short x, y;		/* where to create the window */
	short width, height;	/* width and height */
} XTransparentFrame;

typedef struct _BatchFrame {
	short type;		/* One of (IsOpaque, IsTransparent). */
	XWindow parent;		/* Window if of the window's parent. */
	XWindow self;		/* Window id of the window, filled in later. */
	short x, y;		/* Where to create the window. */
	short width, height;	/* Window width and height. */
	short bdrwidth;		/* Window border width. */
	XPixmap border;		/* Window border pixmap */
	XPixmap background;	/* Window background pixmap. */
} XBatchFrame;


/*
 * Definitions of specific events
 * In all of the following, fields whose names begin with "pad" contain
 * no meaningful value.
 */

struct _XKeyOrButtonEvent {
	unsigned long type;   /* of event (KeyPressed, ButtonReleased, etc.) */
	XWindow window;	      /* which selected this event */
	unsigned short time;  /* in 10 millisecond ticks */
	short detail;	      /* event-dependent data (key state, etc.) */
	short x;    	      /* mouse x coordinate within event window */
	short y;    	      /* mouse y coordinate within event window */
	XWindow subwindow;    /* child window (if any) mouse was in */
	XLocator location;    /* absolute coordinates of mouse */
};

typedef struct _XKeyOrButtonEvent XKeyOrButtonEvent;

typedef struct _XKeyOrButtonEvent XKeyEvent;
typedef struct _XKeyOrButtonEvent XKeyPressedEvent;
typedef struct _XKeyOrButtonEvent XKeyReleasedEvent;

typedef struct _XKeyOrButtonEvent XButtonEvent;
typedef struct _XKeyOrButtonEvent XButtonPressedEvent;
typedef struct _XKeyOrButtonEvent XButtonReleasedEvent;

struct _XMouseOrCrossingEvent {
	unsigned long type;	/* EnterWindow, LeaveWindow, or MouseMoved */
	XWindow window;		/* which selected this event */
	short pad_s2;
	short detail;		/* event-dependent data (key state, etc. ) */
	short x;		/* mouse x coordinate within event window */
	short y;		/* mouse y coordinate within event window */
	XWindow subwindow;	/* child window (if any) mouse was in */
	XLocator location;	/* absolute coordinates of mouse */
};

typedef struct _XMouseOrCrossingEvent XMouseOrCrossingEvent;

typedef struct _XMouseOrCrossingEvent XMouseEvent;
typedef struct _XMouseOrCrossingEvent XMouseMovedEvent;

typedef struct _XMouseOrCrossingEvent XCrossingEvent;
typedef struct _XMouseOrCrossingEvent XEnterWindowEvent;
typedef struct _XMouseOrCrossingEvent XLeaveWindowEvent;

struct _XExposeEvent {
	unsigned long type;	/* ExposeWindow or ExposeRegion */
	XWindow window;		/* that selected this event */
	short pad_s2; 	      
	short detail;		/* 0 or ExposeCopy */
	short width;		/* width of exposed area */
	short height;		/* height of exposed area */
	XWindow subwindow;	/* child window (if any) actually exposed */
	short y;		/* top of exposed area (0 for ExposeWindow) */
	short x;		/* left edge of exposed area */
};

typedef struct _XExposeEvent XExposeEvent;
typedef struct _XExposeEvent XExposeWindowEvent;
typedef struct _XExposeEvent XExposeRegionEvent;

typedef struct _XExposeCopyEvent {
    	unsigned long type;	/* ExposeCopy */
	XWindow window;		/* that selected this event */
	long pad_l1;
	long pad_l2;	      
	XWindow subwindow;	/* child window (if any) actually exposed */
	long pad_l4;	      
} XExposeCopyEvent;
	
typedef struct _XUnmapEvent {
	unsigned long type;	/* UnmapWindow */
	XWindow window;		/* that selected this event */
	long pad_l1;
	long pad_l2;	      
	XWindow subwindow;	/* child window (if any) actually unmapped */
	long pad_l4;	      
} XUnmapEvent;

typedef struct _XFocusChangeEvent {
	unsigned long type;	/* FocusChange */
	XWindow window;		/* that selected this event */
	short pad_s2;
	short detail;		/* EnterWindow or LeaveWindow */
	long pad_l2;	      
	XWindow subwindow;	/* child (if any) of actual focus change */
	long pad_l4;	      
} XFocusChangeEvent;

typedef struct _XErrorEvent {
	long pad;
	long serial;		/* serial number of failed request */
	char error_code;    	/* error code of failed request */
	char request_code;	/* request code of failed request */
	char func;  	        /* function field of failed request */
	char pad_b7;
	XWindow window;	    	/* Window of failed request */
	long pad_l3;
	long pad_l4;
} XErrorEvent;

/*
 * Line pattern related definitions for the library.
 */
typedef long XPattern;

#define DashedLine XMakePattern(0xf0f0, 16, 1)
#define DottedLine XMakePattern(0xaaaa, 16, 1)
#define DotDashLine XMakePattern(0xf4f4, 16, 1)
#define SolidLine  XMakePattern(1,1,1)

typedef short KeyMapEntry [8];

/* define values for keyboard map table */
/* these values will vanish in the next version; DO NOT USE THEM! */
#define SHFT	(short) -2
#define CNTL	(short) -3
#define LOCK	(short) -4
#define SYMBOL	(short) -5
#define KEYPAD	(short) -6
#define CURSOR	(short) -7
#define PFX	(short) -8
#define FUNC1	(short) -9
#define FUNC2	(short) -10
#define FUNC3	(short) -11
#define FUNC4	(short) -12
#define FUNC5	(short) -13
#define FUNC6	(short) -14
#define FUNC7	(short) -15
#define FUNC8	(short) -16
#define FUNC9	(short) -17
#define FUNC10	(short) -18
#define FUNC11	(short) -19
#define FUNC12	(short) -20
#define FUNC13	(short) -21
#define FUNC14	(short) -22
#define FUNC15	(short) -23
#define FUNC16	(short) -24
#define FUNC17	(short) -25
#define FUNC18	(short) -26
#define FUNC19	(short) -27
#define FUNC20	(short) -28
#define E1	(short) -29
#define E2	(short) -30
#define E3	(short) -31
#define E4	(short) -32
#define E5	(short) -33
#define E6	(short) -34

extern XDisplay *_XlibCurrentDisplay;

XDisplay *XOpenDisplay(const char *);
void XSetDisplay(XDisplay *);
void XCloseDisplay(XDisplay *);

XWindow XCreateWindow(
    XWindow, int x, int y, int width, int height, int borderwidth,
    XPixmap border, XPixmap bg
);
XWindow XCreateTransparency(XWindow, int x, int y, int width, int height);
void XDestroyWindow(XWindow);
void XDestroySubwindows(XWindow);
int XCreateWindows(XWindow, XOpaqueFrame defs[], int);
int XCreateTransparencies(XWindow, XTransparentFrame defs[], int);
int XCreateWindowBatch(XWindow, XBatchFrame defs, int);
XWindow XCreate(
    const char *prompt, const char *program, const char *geometry,
    const char *deflt, XOpaqueFrame *frame, int minwidth, int minheight
);
XWindow XCreateTerm(
    const char *prompt, const char *program, const char *geometry,
    const char *deflt, XOpaqueFrame *frame, int minwidth, int minheight,
    int xadder, int yadder, int *cwidth, int *cheight, XFontInfo *f,
    int fwidth, int fheight
);

void XMapWindow(XWindow);
void XMapSubwindows(XWindow);
void XUnmapWindow(XWindow);
void XUnmapTransparent(XWindow);
void XUnmapSubwindows(XWindow);
void XMoveWindow(int x, int y);
void XChangeWindow(XWindow, int width, int height);
void XConfigureWindow(XWindow, int x, int y, int width, int height);
void XRaiseWindow(XWindow);
void XLowerWindow(XWindow);
void XCircWindowUp(XWindow);
void XCircWindowDown(XWindow);

XStatus XQueryWindow(XWindow, XWindowInfo *info);
XStatus XQueryTree(XWindow, XWindow *, int *, XWindow **);

void XChangeBackground(XWindow, XPixmap);
void XChangeBorder(XWindow, XPixmap);
void XTileAbsolute(XWindow);
void XClipDrawThrough(XWindow);
void XClipClipped(XWindow);

void XStoreName(XWindow, const char *);
XStatus XFetchName(XWindow, const char **);

void XSetResizeHint(XWindow, int w0, int h0, int winc, int hinc);
void XGetResizeHint(XWindow, int *w0, int *h0, int *winc, int *hinc);
void XSetIconWindow(XWindow, XWindow);
void XClearIconWindow(XWindow);

XStatus XQueryMouse(XWindow, int *x, int *y, XWindow *sub);
XStatus XQueryMouseButtons(XWindow, int *x, int *y, XWindow *sub, short *state);
XStatus XUpdateMouse(XWindow, int *x, int *y, XWindow *sub);
void XWarpMouse(XWindow, int x, int y);
void XCondWarmpMouse(
    XWindow, XWindow, int dx, int dy, int sx, int sy, int swidth, int sheight
);
XStatus XInterpretLocator(XWindow, int *x, int *y, XWindow *sub, XLocator loc);

void XLine(
    XWindow, int x1, int y1, int x2, int y2, int width, int height,
    int pixel, int func, int planes
);
void XDraw(
    XWindow, XVertex v[], int, int width, int height,
    int pixel, int func, int planes
);
void XDrawPatterned(
    XWindow, XVertex v[], int, int width, int height,
    int pixel, int altpix, XPattern pat, int func, int planes
);
void XDrawDashed(
    XWindow, XVertex v[], int, int width, int height,
    int pixel, XPattern pat, int func, int planes
);
void XDrawTiled(
    XWindow, XVertex v[], int, XPixmap, int func, int planes
);
void XDrawFilled(
    XWindow, XVertex v[], int, int pixel, int func, int planes
);
void XQueryBrushShape (int width, int height, int *cwidth, int *cheight);

void XClear(XWindow);
void XPixSet(XWindow, int x, int y, int width, int height, int pixel);
void XPixFill(
    XWindow, int x, int y, int width, int height, int pixel,
    XBitmap clipmask, int func, int planes
);
void XPixmapPut(
    XWindow, int sx, int sy, int dx, int dy, int width, int height,
    XPixmap, int func, int planes
);
void XQueryTileShape(int width, int height, int *rwidth, int *rheight);
void XTileSet(XWindow, int x, int y, int width, int height, XPixmap);
void XTileFill(
    XWindow, int x, int y, int width, int height, XPixmap,
    XBitmap clipmask, int func, int planes
);

void XMoveArea(XWindow, int sx, int sy, int dx, int dy, int width, int height);
void XCopyArea(
    XWindow, int sx, int sy, int dx, int dy, int width, int height,
    int func, int planes
);

XCursor XStoreCursor(
    XBitmap cursor, XBitmap mask, int xoff, int yoff, int fg, int bg, int func
);
void XQueryCursorShape(int width, int height, int *rwidth, int *rheight);
void XFreeCursor(XCursor);
XCursor XCreateCursor(
    int width, int height, short *cursor, short *mask, int xoff, int yoff,
    int fg, int bg, int func
);
void XDefineCursor(XWindow, XCursor);
void XUndefineCursor(XWindow);

XStatus XGetHardwareColor(XColor *);
XStatus XGetColorCells(
    int contig, int ncolors, int nplanes, int *planes, int pixels[]
);
XStatus XGetColor(const char *, XColor *hard_def, XColor *exact_def);
void XFreeColors(int pixels[], int);
void XStoreColors(int, XColor *);
void XStoreColor(XColor *);
XStatus XQueryColor(XColor *);
void XQueryColors(XColor defs[], int);
XStatus XParseColor(const char *, XColor *);

XFontInfo *XOpenFont(const char *);
void XCloseFont(XFontInfo *);
XFont XGetFont(const char *);
XStatus XQueryFont(XFont, XFontInfo *);
void XFreeFont(XFont);
XStatus XCharWidths(const char *, int, XFont, short *);
short *XFontWidths(XFont);
int XQueryWidth(const char *, XFont);
int XStringWidth(const char *, XFontInfo *, int charpad, int spacepad);

void XText(XWindow, int x, int y, const char *, int, XFont, int fg, int bg);
void XTextPad(
    XWindow, int x, int y, const char *, int, XFont,
    int charpad, int spacepad, int fg, int bg, int func, int planes
);
void XTextMask(XWindow, int x, int y, const char *, int, XFont, int fg);
void XTextMaskPad(
    XWindow, int x, int y, const char *, int, XFont,
    int charpad, int spacepad, int fg, int func, int planes
);

void XMouseControl(int acc, int thresh);
void XFeepControl(int);
void XFeep(int);
void XKeyClickControl(int);
void XAutoRepeatOn();
void XAutoRepeatOff();
void XLockUpDown();
void XLockToggle();
void XScreenSaver(int time, int pattime, int video);

void XSelectInput(XWindow, int);
void XFlush();
void XSync();
int XPending();
void XNextEvent(XEvent *);
void XPutBackEvent(XEvent *);
void XPeekEvent(XEvent *);
void XWindowEvent(XWindow, int, XEvent *);
void XMaskEvent(int, XEvent *);
