/*
 * Interface to X input events.
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

#ifndef Xinput_h
#define Xinput_h

#include <InterViews/Xdefs.h>

/* Input Event Masks. Used as event-mask window attribute and as arguments
   to Grab requests.  Not to be confused with event names.  */

#define NoEventMask			0L
#define KeyPressMask			(1L<<0)  
#define KeyReleaseMask			(1L<<1)  
#define ButtonPressMask			(1L<<2)  
#define ButtonReleaseMask		(1L<<3)  
#define EnterWindowMask			(1L<<4)  
#define LeaveWindowMask			(1L<<5)  
#define PointerMotionMask		(1L<<6)  
#define PointerMotionHintMask		(1L<<7)  
#define Button1MotionMask		(1L<<8)  
#define Button2MotionMask		(1L<<9)  
#define Button3MotionMask		(1L<<10) 
#define Button4MotionMask		(1L<<11) 
#define Button5MotionMask		(1L<<12) 
#define ButtonMotionMask		(1L<<13) 
#define KeymapStateMask			(1L<<14)
#define ExposureMask			(1L<<15) 
#define VisibilityChangeMask		(1L<<16) 
#define StructureNotifyMask		(1L<<17) 
#define ResizeRedirectMask		(1L<<18) 
#define SubstructureNotifyMask		(1L<<19) 
#define SubstructureRedirectMask	(1L<<20) 
#define FocusChangeMask			(1L<<21) 
#define PropertyChangeMask		(1L<<22) 
#define ColormapChangeMask		(1L<<23) 
#define OwnerGrabButtonMask		(1L<<24) 

/* Event names.  Used in "type" field in XEvent structures.  Not to be
confused with event masks above.  They start from 2 because 0 and 1
are reserved in the protocol for errors and replies. */

#define KeyPress		2
#define KeyRelease		3
#define ButtonPress		4
#define ButtonRelease		5
#define MotionNotify		6
#define EnterNotify		7
#define LeaveNotify		8
#define FocusIn			9
#define FocusOut		10
#define KeymapNotify		11
#define Expose			12
#define GraphicsExpose		13
#define NoExpose		14
#define VisibilityNotify	15
#define CreateNotify		16
#define DestroyNotify		17
#define UnmapNotify		18
#define MapNotify		19
#define MapRequest		20
#define ReparentNotify		21
#define ConfigureNotify		22
#define ConfigureRequest	23
#define GravityNotify		24
#define ResizeRequest		25
#define CirculateNotify		26
#define CirculateRequest	27
#define PropertyNotify		28
#define SelectionClear		29
#define SelectionRequest	30
#define SelectionNotify		31
#define ColormapNotify		32
#define ClientMessage		33
#define MappingNotify		34
#define LASTEvent		35	/* must be bigger than any event # */


/* Key masks. Used as modifiers to GrabButton and GrabKey,
   results of QueryPointer, state in various key-, mouse-, and
   button-related events. */

#define ShiftMask		(1<<0)
#define LockMask		(1<<1)
#define ControlMask		(1<<2)
#define Mod1Mask		(1<<3)
#define Mod2Mask		(1<<4)
#define Mod3Mask		(1<<5)
#define Mod4Mask		(1<<6)
#define Mod5Mask		(1<<7)

/* button masks.  Used in same manner as Key masks above. Not to be confused
   with button names below. */

#define Button1Mask		(1<<8)
#define Button2Mask		(1<<9)
#define Button3Mask		(1<<10)
#define Button4Mask		(1<<11)
#define Button5Mask		(1<<12)

#define AnyModifier		(1<<15)  /* used in GrabButton, GrabKey */


/* button names. Used as arguments to GrabButton and as detail in ButtonPress
   and ButtonRelease events.  Not to be confused with button masks above.
   Note that 0 is already defined above as "AnyButton".  */

#define Button1			1
#define Button2			2
#define Button3			3
#define Button4			4
#define Button5			5

/* Notify modes */

#define NotifyNormal		0
#define NotifyGrab		1
#define NotifyUngrab		2
#define NotifyWhileGrabbed	3

#define NotifyHint		1	/* for MotionNotify events */
		       
/* Notify detail */

#define NotifyAncestor		0
#define NotifyVirtual		1
#define NotifyInferior		2
#define NotifyNonlinear		3
#define NotifyNonlinearVirtual	4
#define NotifyPointer		5
#define NotifyPointerRoot	6
#define NotifyDetailNone	7

/* Visibility notify */

#define VisibilityUnobscured		0
#define VisibilityPartiallyObscured	1
#define VisibilityFullyObscured		2

/* Circulation request */

#define PlaceOnTop		0
#define PlaceOnBottom		1

/* protocol families */

#define FamilyInternet		0
#define FamilyDECnet		1
#define FamilyChaos		2

/* Property notification */

#define PropertyNewValue	0
#define PropertyDelete		1

/* Color Map notification */

#define ColormapUninstalled	0
#define ColormapInstalled	1

/* GrabPointer, GrabButton, GrabKeyboard, GrabKey Modes */

#define GrabModeSync		0
#define GrabModeAsync		1

/* GrabPointer, GrabKeyboard reply status */

#define GrabSuccess		0
#define AlreadyGrabbed		1
#define GrabInvalidTime		2
#define GrabNotViewable		3
#define GrabFrozen		4

/* AllowEvents modes */

#define AsyncPointer		0
#define SyncPointer		1
#define ReplayPointer		2
#define AsyncKeyboard		3
#define SyncKeyboard		4
#define ReplayKeyboard		5

/* Used in SetInputFocus, GetInputFocus */

#define RevertToNone		(int)XNone
#define RevertToPointerRoot	(int)XPointerRoot
#define RevertToParent		2

#define AutoRepeatModeOff	0
#define AutoRepeatModeOn	1
#define AutoRepeatModeDefault	2

#define LedModeOff		0
#define LedModeOn		1

/* masks for ChangeKeyboardControl */

#define KBKeyClickPercent	(1L<<0)
#define KBBellPercent		(1L<<1)
#define KBBellPitch		(1L<<2)
#define KBBellDuration		(1L<<3)
#define KBLed			(1L<<4)
#define KBLedMode		(1L<<5)
#define KBKey			(1L<<6)
#define KBAutoRepeatMode	(1L<<7)

#define MappingSuccess     	0
#define MappingBusy        	1
#define MappingFailed		2

#define MappingModifier		0
#define MappingKeyboard		1
#define MappingPointer		2

/* Data structure for XChangeKeyboardControl */

typedef struct {
        int key_click_percent;
        int bell_percent;
        int bell_pitch;
        int bell_duration;
        int led;
        int led_mode;
        int key;
        int auto_repeat_mode;   /* On, Off, Default */
} XKeyboardControl;

/* Data structure for XGetKeyboardControl */

typedef struct {
        int key_click_percent;
	int bell_percent;
	unsigned int bell_pitch, bell_duration;
	unsigned long led_mask;
	int global_auto_repeat;
	char auto_repeats[32];
} XKeyboardState;

/* Data structure for XGetMotionEvents.  */

typedef struct {
        XTime time;
	unsigned short x, y;
} XTimeCoord;

/* Data structure for X{Set,Get}ModifierMapping */

struct XModifierKeymap {
	int max_keypermod;	/* This server's max number of keys per modifier */
	XKeyCode* modifiermap;	/* an 8 by max_keypermod array of the modifiers */
};

/*
 * A "XEvent" structure always  has type as the first entry.  This 
 * uniquely identifies what  kind of event it is.  The second entry
 * is always a pointer to the display the event was read from.
 * The third entry is always a window of one type or another,
 * carefully selected to be useful to toolkit dispatchers.  (Except
 * for keymap events, which have no window.) You
 * must not change the order of the three elements or toolkits will
 * break! The pointer to the generic event must be cast before use to 
 * access any other information in the structure.
 */

/*
 * Definitions of specific events.  If new event types are defined here in
 * the future, the Xlibint.h file union for XBiggestEvent should also have
 * the event type added, to make sure that Xlib maintains enough space for
 * the largest event.  We are trying to avoid fragmentation of the malloc
 * pool by keeping the space allocated the same size for all events, even
 * if the actual information is much smaller.
 */
typedef struct {
	int type;		/* of event */
	XDisplay *display;	/* Display the event was read from */
	XWindow window;	        /* "event" window it is reported relative to */
	XWindow root;	        /* root window that the event occured on */
	XWindow subwindow;	/* child window */
	unsigned long time;	/* milliseconds */
	int x, y;		/* pointer x, y coordinates in event window */
	int x_root, y_root;	/* coordinates relative to root */
	unsigned int state;	/* key or button mask */
	unsigned int keycode;	/* detail */
	XBool same_screen;	/* same screen flag */
} XKeyEvent;
typedef XKeyEvent XKeyPressedEvent;
typedef XKeyEvent XKeyReleasedEvent;

typedef struct {
	int type;		/* of event */
	XDisplay *display;	/* Display the event was read from */
	XWindow window;	        /* "event" window it is reported relative to */
	XWindow root;	        /* root window that the event occured on */
	XWindow subwindow;	/* child window */
	unsigned long time;	/* milliseconds */
	int x, y;		/* pointer x, y coordinates in event window */
	int x_root, y_root;	/* coordinates relative to root */
	unsigned int state;	/* key or button mask */
	unsigned int button;	/* detail */
	XBool same_screen;	/* same screen flag */
} XButtonEvent;
typedef XButtonEvent XButtonPressedEvent;
typedef XButtonEvent XButtonReleasedEvent;

typedef struct {
	int type;		/* of event */
	XDisplay *display;	/* Display the event was read from */
	XWindow window;	        /* "event" window reported relative to */
	XWindow root;	        /* root window that the event occured on */
	XWindow subwindow;	/* child window */
	unsigned long time;	/* milliseconds */
	int x, y;		/* pointer x, y coordinates in event window */
	int x_root, y_root;	/* coordinates relative to root */
	unsigned int state;	/* key or button mask */
	char is_hint;		/* detail */
	XBool same_screen;	/* same screen flag */
} XMotionEvent;
typedef XMotionEvent XPointerMovedEvent;

typedef struct {
	int type;		/* of event */
	XDisplay *display;	/* Display the event was read from */
	XWindow window;	        /* "event" window reported relative to */
	XWindow root;	        /* root window that the event occured on */
	XWindow subwindow;	/* child window */
	unsigned long time;	/* milliseconds */
	int x, y;		/* pointer x, y coordinates in event window */
	int x_root, y_root;	/* coordinates relative to root */
	int mode;		/* NotifyNormal, NotifyGrab, NotifyUngrab */
	int detail;
	/*
	 * NotifyAncestor, NotifyVirtual, NotifyInferior, 
	 * NotifyNonLinear,NotifyNonLinearVirtual
	 */
	XBool same_screen;	/* same screen flag */
	XBool focus;		/* boolean focus */
	unsigned int state;	/* key or button mask */
} XCrossingEvent;
typedef XCrossingEvent XEnterWindowEvent;
typedef XCrossingEvent XLeaveWindowEvent;

typedef struct {
	int type;		/* FocusIn or FocusOut */
	XDisplay *display;	/* Display the event was read from */
	XWindow window;		/* window of event */
	int mode;		/* NotifyNormal, NotifyGrab, NotifyUngrab */
	int detail;
	/*
	 * NotifyAncestor, NotifyVirtual, NotifyInferior, 
	 * NotifyNonLinear,NotifyNonLinearVirtual, NotifyPointer,
	 * NotifyPointerRoot, NotifyDetailNone 
	 */
} XFocusChangeEvent;
typedef XFocusChangeEvent XFocusInEvent;
typedef XFocusChangeEvent XFocusOutEvent;

/* generated on EnterWindow and FocusIn  when KeyMapState selected */
typedef struct {
	int type;
	XDisplay *display;	/* Display the event was read from */
	XWindow window;
	char key_vector[32];
} XKeymapEvent;	

typedef struct {
	int type;
	XDisplay *display;	/* Display the event was read from */
	XWindow window;
	int x, y;
	int width, height;
	int count;		/* if non-zero, at least this many more */
} XExposeEvent;

typedef struct {
	int type;
	XDisplay *display;	/* Display the event was read from */
	XDrawable drawable;
	int x, y;
	int width, height;
	int count;		/* if non-zero, at least this many more */
	int major_code;		/* core is CopyArea or CopyPlane */
	int minor_code;		/* not defined in the core */
} XGraphicsExposeEvent;

typedef struct {
	int type;
	XDisplay *display;	/* Display the event was read from */
	XDrawable drawable;
	int major_code;		/* core is CopyArea or CopyPlane */
	int minor_code;		/* not defined in the core */
} XNoExposeEvent;

typedef struct {
	int type;
	XDisplay *display;	/* Display the event was read from */
	XWindow window;
	int state;		/* either Obscured or UnObscured */
} XVisibilityEvent;

typedef struct {
	int type;
	XDisplay *display;	/* Display the event was read from */
	XWindow parent;		/* parent of the window */
	XWindow window;		/* window id of window created */
	int x, y;		/* window location */
	int width, height;	/* size of window */
	int border_width;	/* border width */
	XBool override_redirect;/* creation should be overridden */
} XCreateWindowEvent;

typedef struct {
	int type;
	XDisplay *display;	/* Display the event was read from */
	XWindow event;
	XWindow window;
} XDestroyWindowEvent;

typedef struct {
	int type;
	XDisplay *display;	/* Display the event was read from */
	XWindow event;
	XWindow window;
	XBool from_configure;
} XUnmapEvent;

typedef struct {
	int type;
	XDisplay *display;	/* Display the event was read from */
	XWindow event;
	XWindow window;
	XBool override_redirect;/* boolean, is override set... */
} XMapEvent;

typedef struct {
	int type;
	XDisplay *display;	/* Display the event was read from */
	XWindow parent;
	XWindow window;
} XMapRequestEvent;

typedef struct {
	int type;
	XDisplay *display;	/* Display the event was read from */
	XWindow event;
	XWindow window;
	XWindow parent;
	int x, y;
	XBool override_redirect;
} XReparentEvent;

typedef struct {
	int type;
	XDisplay *display;	/* Display the event was read from */
	XWindow event;
	XWindow window;
	int x, y;
	int width, height;
	int border_width;
	XWindow above;
	XBool override_redirect;
} XConfigureEvent;

typedef struct {
	int type;
	XDisplay *display;	/* Display the event was read from */
	XWindow event;
	XWindow window;
	int x, y;
} XGravityEvent;

typedef struct {
	int type;
	XDisplay *display;	/* Display the event was read from */
	XWindow window;
	int width, height;
} XResizeRequestEvent;

typedef struct {
	int type;
	XDisplay *display;	/* Display the event was read from */
	XWindow parent;
	XWindow window;
	int x, y;
	int width, height;
	int border_width;
	XWindow above;
	int detail;		/* Above, Below, TopIf, BottomIf, Opposite */
	unsigned long value_mask;
} XConfigureRequestEvent;

typedef struct {
	int type;
	XDisplay *display;	/* Display the event was read from */
	XWindow event;
	XWindow window;
	int place;		/* PlaceOnTop, PlaceOnBottom */
} XCirculateEvent;

typedef struct {
	int type;
	XDisplay *display;	/* Display the event was read from */
	XWindow parent;
	XWindow window;
	int place;		/* PlaceOnTop, PlaceOnBottom */
} XCirculateRequestEvent;

typedef struct {
	int type;
	XDisplay *display;	/* Display the event was read from */
	XWindow window;
	XAtom atom;
	XTime time;
	int state;		/* NewValue, Deleted */
} XPropertyEvent;

typedef struct {
	int type;
	XDisplay *display;	/* Display the event was read from */
	XWindow window;
	XAtom selection;
	XTime time;
} XSelectionClearEvent;

typedef struct {
	int type;
	XDisplay *display;	/* Display the event was read from */
	XWindow owner;		/* must be next after type */
	XWindow requestor;
	XAtom selection;
	XAtom target;
	XAtom property;
	XTime time;
} XSelectionRequestEvent;

typedef struct {
	int type;
	XDisplay *display;	/* Display the event was read from */
	XWindow requestor;	/* must be next after type */
	XAtom selection;
	XAtom target;
	XAtom property;		/* ATOM or None */
	XTime time;
} XSelectionEvent;

typedef struct {
	int type;
	XDisplay *display;	/* Display the event was read from */
	XWindow window;
	XColormap colormap;	/* COLORMAP or None */
	XBool isnew;
	int state;		/* ColormapInstalled, ColormapUninstalled */
} XColormapEvent;

typedef struct {
	int type;
	XDisplay *display;	/* Display the event was read from */
	XWindow window;
	XAtom message_type;
	int format;
	union {
		char b[20];
		short s[10];
		int l[5];
		} data;
} XClientMessageEvent;

typedef struct {
	int type;
	XDisplay *display;	/* Display the event was read from */
	XWindow window;		/* unused */
	int request;		/* one of MappingModifier, MappingKeyboard,
				   MappingPointer */
	int first_keycode;	/* first keycode */
	int count;		/* defines range of change w. first_keycode*/
} XMappingEvent;

typedef struct {
	int type;
	XDisplay *display;	/* Display the event was read from */
	XID resourceid;		/* resource id */
	int serial;		/* serial number of failed request */
	char error_code;	/* error code of failed request */
	char request_code;	/* Major op-code of failed request */
	char minor_code;	/* Minor op-code of failed request */
} XErrorEvent;

typedef struct {
	int type;
	XDisplay *display;/* Display the event was read from */
	XWindow window;	/* window on which event was requested in event mask */
} XAnyEvent;

/*
 * this union is defined so Xlib can always use the same sized
 * event structure internally, to avoid memory fragmentation.
 */
typedef union _XEvent {
        int type;		/* must not be changed; first element */
	XAnyEvent xany;
	XKeyEvent xkey;
	XButtonEvent xbutton;
	XMotionEvent xmotion;
	XCrossingEvent xcrossing;
	XFocusChangeEvent xfocus;
	XExposeEvent xexpose;
	XGraphicsExposeEvent xgraphicsexpose;
	XNoExposeEvent xnoexpose;
	XVisibilityEvent xvisibility;
	XCreateWindowEvent xcreatewindow;
	XDestroyWindowEvent xdestroywindow;
	XUnmapEvent xunmap;
	XMapEvent xmap;
	XMapRequestEvent xmaprequest;
	XReparentEvent xreparent;
	XConfigureEvent xconfigure;
	XGravityEvent xgravity;
	XResizeRequestEvent xresizerequest;
	XConfigureRequestEvent xconfigurerequest;
	XCirculateEvent xcirculate;
	XCirculateRequestEvent xcirculaterequest;
	XPropertyEvent xproperty;
	XSelectionClearEvent xselectionclear;
	XSelectionRequestEvent xselectionrequest;
	XSelectionEvent xselection;
	XColormapEvent xcolormap;
	XClientMessageEvent xclient;
	XMappingEvent xmapping;
	XErrorEvent xerror;
	XKeymapEvent xkeymap;
} XEvent;
/*
 * _QEvent datatype for use in input queueing.
 */
typedef struct _XSQEvent {
    struct _XSQEvent *next;
    XEvent event;
} _XQEvent;

void XGrabButton(
    XDisplay*, XButton, XModifiers, XWindow, XBool,
    XEventMask, int ptrmode, int kbdmode, XWindow confined, XCursor
);
void XUngrabButton(XDisplay*, XButton, XModifiers, XWindow);
int XGrabPointer(
    XDisplay*, XWindow, XBool, XEventMask, int ptrmode, int kbdmode,
    XWindow confined, XCursor, XTime
);
void XUngrabPointer(XDisplay*, XTime);
void XChangeActivePointerGrab(XDisplay*, XEventMask, XCursor, XTime);
void XGrabKeyboard(XDisplay*, XWindow, XBool, int ptrmode, int kbdmode, XTime);
void XUngrabKeyboard(XDisplay*, XTime);
void XGrabKey(
    XDisplay*, int, XModifiers, XWindow, XBool, int ptrmode, int kbdmode
);
void XUngrabKey(XDisplay*, int key, XModifiers, XWindow);
void XAllowEvents(XDisplay*, int, XTime);

void XGrabServer(XDisplay*);
void XUngrabServer(XDisplay*);

void XWarpPointer(
    XDisplay*, XWindow src, XWindow dst, int srcx, int srcy,
    XCardinal width, XCardinal height, int dstx, int dsty
);
void XSetInputFocus(XDisplay*, XWindow, int revert, XTime);
void XGetInputFocus(XDisplay*, XWindow&, int& revert);

void XChangePointerControl(
    XDisplay*, XBool has_acc, XBool has_thresh, int acc1, int acc2, int thresh
);
void XGetPointerControl(XDisplay*, int& acc1, int& acc2, int& thresh);

void XChangeKeyboardControl(XDisplay*, XValueMask, XKeyboardControl&);
void XGetKeyboardControl(XDisplay*, XKeyboardState&);
void XAutoRepeatOn(XDisplay*);
void XAutoRepeatOff(XDisplay*);
void XBell(XDisplay*, int percent);
int XSetPointerMapping(XDisplay*, unsigned char[], int);
int XGetPointerMapping(XDisplay*, unsigned char[], int);

void XQueryPointer(
    XDisplay*, XWindow, XWindow& root, XWindow& child, int& x0, int& y0,
    int& wx, int& wy, unsigned int&
);

void XChangeKeyboardMapping(XDisplay*, int first, int percode, XKeySym[], int);
void XSetModifierMapping(XDisplay*, XModifierKeymap*);
void XGetModifierMapping(XDisplay*, XModifierKeymap*);

void XSelectInput(XDisplay*, XWindow, XEventMask);
void XFlush(XDisplay*);
void XSync(XDisplay*, int);
int XPending(XDisplay*);
void XNextEvent(XDisplay*, XEvent&);
void XPeekEvent(XDisplay*, XEvent&);

typedef XBool (*XPredicate)(XDisplay*, XEvent&, char*);

void XIfEvent(XDisplay*, XEvent&, XPredicate, char*);
int XCheckIfEvent(XDisplay*, XEvent&, XPredicate, char*);
void XPeekIfEvent(XDisplay*, XEvent&, XPredicate, char*);
void XPutBackEvent(XDisplay*, XEvent&);
void XWindowEvent(XDisplay*, XWindow, XEventMask, XEvent&);
int XCheckWindowEvent(XDisplay*, XWindow, XEventMask, XEvent&);
void XMaskEvent(XDisplay*, XEventMask, XEvent&);
int XCheckMaskEvent(XDisplay*, XEventMask, XEvent&);
XTimeCoord *XGetMotionEvents(
    XDisplay*, XWindow, XTime start, XTime stop, int& nevents
);

void XSendEvent(XDisplay*, XWindow, XBool, XEventMask, XEvent&);

XKeySym XLookupKeysym(XKeyEvent&, int);
void XRefereshKeyboardMapping(XMappingEvent&);
int XLookupString(XKeyEvent&, char*, int, XKeySym*, struct XComposeStatus*);
void XRebindKeySym(
    XDisplay*, XKeySym, XKeySym[], int, unsigned char*, int nbytes
);

#endif
