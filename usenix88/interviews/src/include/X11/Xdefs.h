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

#ifndef Xdefs_h
#define Xdefs_h

#define X_PROTOCOL	11		/* current protocol version */
#define X_PROTOCOL_REVISION 65533	/* current minor version */

typedef void* XID;

typedef XID XWindow;
typedef XID XDrawable;
typedef XID XFont;
typedef XID XPixmap;
typedef XID XCursor;
typedef XID XColormap;
typedef XID XGContext;
typedef XID XKeySym;

typedef unsigned long XMask;
typedef unsigned long XAtom;
typedef unsigned long XVisualID;
typedef unsigned long XTime;
typedef unsigned char XKeyCode;

/* new for procedure interfaces */
typedef unsigned long XValueMask;
typedef unsigned int XCardinal;
typedef unsigned long XPixel;
typedef unsigned long XPlaneMask;
typedef unsigned int XButton;
typedef unsigned int XModifiers;
typedef unsigned int XEventMask;

struct XRectangle {
    short x, y;
    unsigned short width, height;
};

struct XExtData;
struct _XExtension;
struct XScreen;
struct XScreenFormat;
typedef struct _XGC* Xgc;

#define XNone                0L	/* universal null resource or null atom */
#define XParentRelative      1L	/* background pixmap in CreateWindow
				    and ChangeWindowAttributes */
#define XCopyFromParent      0L	/* border pixmap in CreateWindow
				       and ChangeWindowAttributes
				   special VisualID and special window
				       class passed to CreateWindow */
#define XPointerWindow       0L	/* destination window in SendEvent */
#define XInputFocus          1L	/* destination window in SendEvent */
#define XPointerRoot         1L	/* focus window in SetInputFocus */
#define XAnyPropertyType     0L	/* special Atom, passed to GetProperty */
#define XAnyKey		     0L	/* special Key Code, passed to GrabKey */
#define XAnyButton           0L	/* special Button Code, passed to GrabButton */
#define XAllTemporary        0L	/* special Resource ID passed to KillClient */
#define XCurrentTime         0L	/* special Time */
#define XNoSymbol	     0L	/* special KeySym */

/* QueryBestSize Class */

#define XCursorShape		0	/* largest size that can be displayed */
#define XTileShape		1	/* size tiled fastest */
#define XStippleShape		2	/* size stippled fastest */

#define XDontPreferBlanking	0
#define XPreferBlanking		1
#define XDefaultBlanking	2

#define XDisableScreenSaver	0
#define XDisableScreenInterval	0

#define XDontAllowExposures	0
#define XAllowExposures		1
#define XDefaultExposures	2

#define XScreenSaverReset 0
#define XScreenSaverActive 1

/* for ChangeHosts */

#define XHostInsert		0
#define XHostDelete		1

/* for ChangeAccessControl */

#define XEnableAccess		1      
#define XDisableAccess		0

/* Display classes  used in opening the connection 
 * Note that the statically allocated ones are even numbered and the
 * dynamically changeable ones are odd numbered */

#define XStaticGray		0
#define XGrayScale		1
#define XStaticColor		2
#define XPseudoColor		3
#define XTrueColor		4
#define XDirectColor		5

/* Byte order  used in imageByteOrder and bitmapBitOrder */

#define XLSBFirst		0
#define XMSBFirst		1

#define XBool int
#define XStatus int
#define XTrue 1
#define XFalse 0

#define XConnectionNumber(dpy) ((dpy)->fd)
#define XRootWindow(dpy, scr) (((dpy)->screens[(scr)]).root)
#define XDefaultScreen(dpy) ((dpy)->default_screen)
#define XDefaultRootWindow(dpy) (((dpy)->screens[(dpy)->default_screen]).root)
#define XDefaultVisual(dpy, scr) (((dpy)->screens[(scr)]).root_visual)
#define XDefaultGC(dpy, scr) (((dpy)->screens[(scr)]).default_gc)
#define XBlackPixel(dpy, scr) (((dpy)->screens[(scr)]).black_pixel)
#define XWhitePixel(dpy, scr) (((dpy)->screens[(scr)]).white_pixel)
#define XAllPlanes (~0)
#define XQLength(dpy) ((dpy)->qlen)
#define XDisplayWidth(dpy, scr) (((dpy)->screens[(scr)]).width)
#define XDisplayHeight(dpy, scr) (((dpy)->screens[(scr)]).height)
#define XDisplayWidthMM(dpy, scr) (((dpy)->screens[(scr)]).mwidth)
#define XDisplayHeightMM(dpy, scr) (((dpy)->screens[(scr)]).mheight)
#define XDisplayPlanes(dpy, scr) (((dpy)->screens[(scr)]).root_depth)
#define XDisplayCells(dpy, scr) (DefaultVisual((dpy), (scr))->map_entries)
#define XScreenCount(dpy) ((dpy)->nscreens)
#define XServerVendor(dpy) ((dpy)->vendor)
#define XProtocolVersion(dpy) ((dpy)->proto_major_version)
#define XProtocolRevision(dpy) ((dpy)->proto_minor_version)
#define XVendorRelease(dpy) ((dpy)->vnumber)
#define XDisplayString(dpy) ((dpy)->display_name)
#define XDefaultDepth(dpy, scr) (((dpy)->screens[(scr)]).root_depth)
#define XDefaultColormap(dpy, scr) (((dpy)->screens[(scr)]).cmap)
#define XBitmapUnit(dpy) ((dpy)->bitmap_unit)
#define XBitmapBitOrder(dpy) ((dpy)->bitmap_bit_order)
#define XBitmapPad(dpy) ((dpy)->bitmap_pad)
#define XImageByteOrder(dpy) ((dpy)->byte_order)

/*
 * Visual structure; contains information about colormapping possible.
 */
typedef struct {
	XExtData *ext_data;	/* hook for extension to hang data */
	XVisualID visualid;	/* visual id of this visual */
	int screen_class;	/* class of screen (monochrome, etc.) */
	unsigned long red_mask, green_mask, blue_mask;	/* mask values */
	int bits_per_rgb;	/* log base 2 of distinct color values */
	int map_entries;	/* color map entries */
} XVisual;

/*
 * Depth structure; contains information for each possible depth.
 */	
typedef struct {
	int depth;		/* this depth (Z) of the depth */
	int nvisuals;		/* number of Visual types at this depth */
	XVisual *visuals;	/* list of visuals possible at this depth */
} XDepth;

/*
 * Data structure for host setting; getting routines.
 *
 */

typedef struct {
	int family;		/* for example AF_DNET */
	int length;		/* length of address, in bytes */
	char *address;		/* pointer to where to find the bytes */
} XHostAddress;

/*
 * Display datatype maintaining display specific data.
 */
typedef struct _XDisplay {
	XExtData *ext_data;	/* hook for extension to hang data */
	struct _XDisplay *next;	/* next open Display on list */
	int fd;			/* Network socket. */
	int lock;		/* is someone in critical section? */
	int proto_major_version;/* maj. version of server's X protocol */
	int proto_minor_version;/* minor version of servers X protocol */
	char *vendor;		/* vendor of the server hardware */
        long resource_base;	/* resource ID base */
	long resource_mask;	/* resource ID mask bits */
	long resource_id;	/* allocator current ID */
	int resource_shift;	/* allocator shift to correct bits */
	XID (*resource_alloc)(); /* allocator function */
	int byte_order;		/* screen byte order, LSBFirst, MSBFirst */
	int bitmap_unit;	/* padding and data requirements */
	int bitmap_pad;		/* padding requirements on bitmaps */
	int bitmap_bit_order;	/* LeastSignificant or MostSignificant */
	int nformats;		/* number of pixmap formats in list */
	struct XScreenFormat *pixmap_format;	/* pixmap format list */
	int vnumber;		/* X protocol version number. */
	int release;		/* release of the server */
	struct _XSQEvent *head, *tail;	/* Input event queue. */
	int qlen;		/* Length of input event queue */
	int last_request_read;	/* sequence number of last event read NI */
	int request;		/* sequence number of last request. */
	char *last_req;		/* beginning of last request, or dummy */
	char *buffer;		/* Output buffer starting address. */
	char *bufptr;		/* Output buffer index pointer. */
	char *bufmax;		/* Output buffer maximum+1 address. */
	unsigned max_request_size; /* maximum number 32 bit words in request*/
	struct _XrmResourceDataBase *db;
	int (*synchandler)();	/* Synchronization handler */
	char *display_name;	/* "host:display" string used on this connect*/
	int default_screen;	/* default screen for operations */
	int nscreens;		/* number of screens on this server*/
	XScreen *screens;	/* pointer to list of screens */
	int motion_buffer;	/* size of motion buffer */
	XWindow current;	/* for use internally for Keymap notify */
	int min_keycode;	/* minimum defined keycode */
	int max_keycode;	/* maximum defined keycode */
	XKeySym *keysyms;	/* This server's keysyms */
	struct XModifierKeymap *modifiermap;/* This server's modifier keymap */
	int keysyms_per_keycode;/* number of rows */
	char *xdefaults;	/* contents of defaults from server */
	char *scratch_buffer;	/* place to hang scratch buffer */
	unsigned long scratch_length;	/* length of scratch buffer */
	int ext_number;		/* extension number on this display */
	_XExtension* ext_procs;	/* extensions initialized on this display */
	/*
	 * the following can be fixed size, as the protocol defines how
	 * much address space is available. 
	 * While this could be done using the extension vector, there
	 * may be MANY events processed, so a search through the extension
	 * list to find the right procedure for each event might be
	 * expensive if many extensions are being used.
	 */
	int (*event_vec[128])();/* vector for wire to event */
	int (*wire_vec[128])();	/* vector for event to wire */
} XDisplay;

#define XAllocID(dpy) ((*(dpy)->resource_alloc)((dpy)))

extern XDisplay* _World_dpy;
extern XColormap _World_cmap;
extern int _World_screen;
extern XWindow _World_root;

XDisplay* XOpenDisplay(const char*);
char* XDisplayName(XDisplay*);
void XCloseDisplay(XDisplay*);

char* XGetDefault(XDisplay*, const char* prog, const char* param);
XAtom XInternAtom(XDisplay*, char*, int);
XColormap* XListInstalledColormaps();
char* XGetAtomName(XDisplay*, XAtom);

void XSetScreenSaver(
    XDisplay*, int timeout, int interval, int blanking, int exposures
);
void XForceScreenSaver(XDisplay*, int mode);
void XActivateScreenSaver(XDisplay*);
void XResetScreenSaver(XDisplay*);
void XGetScreenSaver(
    XDisplay*, int& timeout, int& interval, int& blanking, int& exposures
);

void XAddHost(XDisplay*, XHostAddress&);
void XAddHosts(XDisplay*, XHostAddress[], int);
XHostAddress* XListHosts(XDisplay*, int& n, XBool&);
void XRemoveHost(XDisplay*, XHostAddress&);
void XRemoveHosts(XDisplay*, XHostAddress[], int);

#endif
