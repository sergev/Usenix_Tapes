/*
 * Implementation of the base class of interaction.
 */

#include <InterViews/canvas.h>
#include <InterViews/cursor.h>
#include <InterViews/painter.h>
#include <InterViews/scene.h>
#include <InterViews/shape.h>
#include <InterViews/sensor.h>
#include <InterViews/worldview.h>
#include <InterViews/Xcursor.h>
#include <InterViews/Xinput.h>
#include <InterViews/Xoutput.h>
#include <InterViews/Xwindow.h>
#include <InterViews/Xutil.h>
#if defined(hpux)
#include <time.h>
#else
#include <sys/time.h>
#endif
#include "table.h"

struct UnReadEvent {
    friend class Interactor;

    XWindow w;
    Event e;
    UnReadEvent* next;

    void Add(Event&);
    boolean Remove(Event&);
};

static UnReadEvent* unread;

#ifdef X10

#define Xserver() Xdpyno()
#define QLen() XQLength()
#define Unmap(w) XUnmapWindow(w)
#define Map(w) XMapWindow(w)

static boolean Mapped (XWindow w) {
    XWindowInfo info;

    XQueryWindow(w, info);
    return info.mapped;
}

static XWindow GetIcon (XWindow w) {
    XWindowInfo info;
    XQueryWindow(w, info);
    return info.assoc_wind;
}

#endif

#ifdef X11

#define Xserver() XConnectionNumber(_World_dpy)
#define QLen() XQLength(_World_dpy)
#define Unmap(w) XUnmapWindow(_World_dpy, w)
#define Map(w) XMapWindow(_World_dpy, w)

static boolean Mapped (XWindow w) {
    XWindowAttributes a;

    XGetWindowAttributes(_World_dpy, w, a);
    return a.map_state != IsUnmapped;
}

static XWindow GetIcon (XWindow w) {
    XWMHints* h = XGetWMHints(_World_dpy, w);
    return (h == nil || (h->flags&IconWindowHint) == 0) ? nil : h->icon_window;
}

extern WorldView* _WorldView_this;

#endif

Interactor::Interactor (Sensor* in, Painter* out) {
    name = nil;
    icon = nil;
    parent = nil;
    left = 0;
    bottom = 0;
    cursor = defaultCursor;
    cursensor = nil;
    input = in;
    if (in != nil) {
	in->Reference();
    }
    output = out;
    if (out != nil) {
	out->Reference();
    }
    shape = new Shape;
    canvas = nil;
    perspective = nil;
    prevmask = 0;
}

Interactor::~Interactor () {
    Delete();
    if (parent != nil) {
	parent->Remove(this);
    }
    delete canvas;
    delete shape;
    delete input;
    delete output;
}

void Interactor::Delete () {
    /* nothing to do */
}

/*
 * Check to see if an event is pending other than from an input device.
 * This check implies that no input events are waiting on the local queue,
 * that we are interested in some other event (a channel or timer), and
 * that such an event occurs before another input event arrives.
 */

inline boolean Interactor::OtherEvent (Event& e) {
    return (
	QLen() == 0 && cursensor != nil &&
	(cursensor->timer || cursensor->channels != 0) &&
	Select(e)
    );
}

/*
 * Do a select on input and desired channels, also catching timeouts
 * if desired.  Return true if the event is NOT an input event.
 */

boolean Interactor::Select (Event& e) {
#if defined(no_select)
    /* systems that do not have select */
    return false;
#else
    extern int select(int, int*, int, int, struct timeval*);
    register Sensor* s;
    unsigned d, dmask, m, rd;
    struct timeval tv, * t;
    int r;

    s = cursensor;
    FlushRedraws();
    d = Xserver();
    dmask = 1 << d;
    m = max(d+1, s->maxchannel);
    rd = s->channels | dmask;
    if (s->timer) {
	tv.tv_sec = s->sec;
	tv.tv_usec = s->usec;
	t = &tv;
    } else {
	t = nil;
    }
    for (;;) {
	e.channel = rd;
	r = select(m, &e.channel, 0, 0, t);
	if (r > 0) {
	    e.target = this;
	    e.eventType = ChannelEvent;
	    return (e.channel & dmask) == 0;
	} else if (r == 0) {
	    e.target = this;
	    e.eventType = TimerEvent;
	    return true;
	}
	/* otherwise probably interrupted system call, try again */
    }
#endif
}

void Interactor::Resize () {
    /* default is to ignore */
}

void Interactor::Draw () {
    Redraw(0, 0, xmax, ymax);
}

void Interactor::Redraw (Coord left, Coord bottom, Coord right, Coord top) {
    /* default is to ignore */
}

void Interactor::Handle (class Event& e) {
    /* default is to ignore */
}

void Interactor::Adjust (Perspective&) {
    /* default is to ignore */
}

void Interactor::Reshape (Shape&) {
    /* default is to ignore */
}

void Interactor::Update () {
    /* default is to ignore */
}

void Interactor::GetComponents (Interactor**&, int& n) {
    n = 0;
}

/*
 * Quick check of local queue to see if any events have already been read.
 */

boolean Interactor::CheckQueue () {
    return QLen() != 0;
}

void Interactor::Iconify () {
    XWindow w = nil;

    if (icon != nil && icon->canvas != nil) {
	w = icon->canvas->id;
    }
    if (w == nil) {
	w = GetIcon(canvas->id);
    }
    if (w != nil && !Mapped(w)) {
	Map(w);
	if (canvas != nil && canvas->id != nil) {
	    Unmap(canvas->id);
	}
	if (icon != nil && icon->canvas != nil && icon->canvas->id != nil) {
	    icon->Draw();
	}
    }	    
}

void Interactor::DeIconify () {
    XWindow w = nil;

    if (canvas != nil && canvas->id != nil && !Mapped(canvas->id)) {
	Map(canvas->id);
	if (icon != nil && icon->canvas != nil && icon->canvas->id != nil) {
	    w = icon->canvas->id;
	}
	if (w == nil) {
	    w = GetIcon(canvas->id);
	}
	if (w != nil) {
	    Unmap(w);
	}
	Draw();
    }
}

void Interactor::UnRead (Event& e) {
    if (e.target != nil && e.target->canvas != nil) {
	register UnReadEvent* u;

	u = new UnReadEvent;
	u->w = e.target->canvas->id;
	u->e = e;
	u->next = unread;
	unread = u;
    }
}

boolean Interactor::PrevEvent (Event& e) {
    register UnReadEvent* u;
    Interactor* i;

    for (u = unread; u != nil; u = unread) {
	unread = u->next;
	if (assocTable->Find(i, u->w) && i == u->e.target) {
	    e = u->e;
	    delete u;
	    return true;
	}
	delete u;
    }
    return false;
}

#ifdef X10

void Interactor::Listen (Sensor* s) {
    unsigned mask;

    cursensor = s;
    if (canvas != nil) {
	if ((s == nil) && (this->Parent() != nil)) {
	    prevmask = ExposeRegion|ExposeCopy;
	    XSelectInput(canvas->id, prevmask);
	} else if (prevmask != s->mask) {
	    if (this->Parent() == nil) {
		if (s->mask & (ExposeRegion|ExposeCopy)) {
		    mask = s->mask & ~(ExposeRegion|ExposeCopy);
		} else {
		    mask = s->mask;
		}
	    } else {
		mask = s->mask;
	    }
	    XSelectInput(canvas->id, mask);
	    prevmask = s->mask;
	}
    }
}

void Interactor::SetCursor (Cursor* c) {
    if (canvas != nil) {
	XDefineCursor(canvas->id, c->info);
	Sync();
    }
}

boolean Interactor::GetEvent (Event&, boolean) {
    /* not implemented for X10 */
    return false;
}

void Interactor::FlushRedraws () {
    XFlush();
}

/*
 * Read an event.
 */

void Interactor::Read (register Event& e) {
    XEvent xe;
    register XExposeEvent* r = (XExposeEvent*) &xe;
    Interactor* i;

    for (;;) {
	if (PrevEvent(e) || OtherEvent(e)) {
	    break;
	}
	XNextEvent(xe);
	if (assocTable->Find(i, xe.window)) {
	    if (i->cursensor != nil && i->cursensor->Interesting(&xe, e)) {
		e.target = i;
		e.y = i->ymax - e.y;
		break;
	    } else if (xe.type == ExposeRegion) {
		i->SendRedraw(r->x, r->y, r->width, r->height);
	    } else if (xe.type == ExposeWindow && i->parent->parent == nil) {
		i->SendResize(r->width, r->height);
	    }
	}
    }
}

/*
 * Check to see if any input events of interest are pending.
 * This routine will return true even if the event is for another interactor.
 */

boolean Interactor::Check () {
    XEvent xe;
    Event e;
    register XExposeEvent* r = (XExposeEvent*) &xe;
    Interactor* i;

    while (XPending()) {
	XNextEvent(xe);
	if (assocTable->Find(i, xe.window)) {
	    if (i->cursensor != nil && i->cursensor->Interesting(&xe, e)) {
		XPutBackEvent(xe);
		return true;
	    } else if (xe.type == ExposeRegion) {
		i->SendRedraw(r->x, r->y, r->width, r->height);
	    } else if (xe.type == ExposeWindow && i->parent->parent == nil) {
		i->SendResize(r->width, r->height);
	    }
	}
    }
    return false;
}

/*
 * Translate an ExposeRegion into the appropriate Redraw call.
 */

void Interactor::SendRedraw (Coord x, Coord iy, int w, int h) {
    register Coord y;

    y = ymax-iy;
    if (canvas->unclipped != nil) {
	canvas->Exchange(canvas->unclipped);
	Redraw(x, y-h+1, x+w-1, y);
	if (canvas->unclipped != nil) {
	    /* Redraw did a NoClip */
	    canvas->Exchange(canvas->unclipped);
	}
    } else {
	Redraw(x, y-h+1, x+w-1, y);
    }
}

/*
 * Translate an ExposeWindow event to the appropriate call.
 * ExposeWindow can imply either a full redraw
 * (as in the window moved) or a resize, so we check for a size change
 * explicitly.
 */

void Interactor::SendResize (int w, int h) {
    register Canvas* c;
    XWindowInfo info;

    if (canvas->unclipped == nil) {
	c = canvas;
    } else {
	/* set c to the CLIPPED version of the canvas */
	canvas->Exchange(canvas->unclipped);
	c = canvas->unclipped;
    }
    XQueryWindow(canvas->id, info);
    left = info.x;
    bottom = parent->ymax - info.y - h + 1;
    if (w != c->width || h != c->height) {
	c->width = w;
	c->height = h;
	/*
	 * Since Canvas::Clip can't rely on Interactor::ymax to calculate
	 * top, it must instead use height - 1.  So we have to update
	 * height here (potentially twice if there's no clipping, but
	 * it's really not worth a conditional).
	 */
	canvas->height = h;
	xmax = w - 1;
	ymax = h - 1;
	XUnmapSubwindows(c->id);
	Resize();
    }
    Draw();
    if (canvas->unclipped != nil) {
	canvas->Exchange(canvas->unclipped);
    }
}

void Interactor::Poll (Event& e) {
    int x, y;
    short state;
    register short s;
    XWindow dummy;
    register Interactor* i;
    int offx, offy;

    offx = 0;
    offy = 0;
    for (i = this; i->parent != nil; i = i->parent) {
	offx += i->left;
	offy += i->bottom;
    }
    XQueryMouseButtons(i->canvas->id, x, y, dummy, state);
    e.x = x - offx;
    e.y = (i->ymax - y) - offy;
    e.w = (World*)i;
    e.wx = x;
    e.wy = y;
    s = state;
    e.control = (s&ControlMask) != 0;
    e.meta = (s&MetaMask) != 0;
    e.shift = (s&ShiftMask) != 0;
    e.shiftlock = (s&ShiftLockMask) != 0;
    e.leftmouse = (s&LeftMask) != 0;
    e.middlemouse = (s&MiddleMask) != 0;
    e.rightmouse = (s&RightMask) != 0;
}

void Interactor::Sync () {
    XFlush();
}

void Interactor::GetRelative (Coord& x, Coord& y, Interactor* rel) {
    register Interactor* t, * r;
    Coord tx, ty, rx, ry;

    if (parent == nil) {
	if (rel == nil || rel->parent == nil) {
	    /* world relative to world -- nop */
	    return;
	}
	/* world relative to interactor is negative of interactor to world */
	rel->GetRelative(x, y);
	x = -x; y = -y;
	return;
    }
    tx = x; ty = y;
    t = this;
    for (t = this; t->parent->parent != nil; t = t->parent) {
	tx += t->left;
	ty += t->bottom;
    }
    if (rel == nil || rel->parent == nil) {
	r = nil;
    } else {
	rx = 0; ry = 0;
	for (r = rel; r->parent->parent != nil; r = r->parent) {
	    rx += r->left;
	    ry += r->bottom;
	}
    }
    if (r == t) {
	/* this and rel are within same top-level interactor */
	x = tx - rx;
	y = ty - ry;
    } else {
	XWindowInfo info;
	
	XQueryWindow(t->canvas->id, info);
	x = tx + info.x;
	y = ty + t->parent->ymax - (info.y + info.height - 1);
	if (r != nil) {
	    XQueryWindow(r->canvas->id, info);
	    x -= info.x;
	    y -= t->parent->ymax - (info.y + info.height - 1);
	}
    }
}

void Interactor::SetName (const char* s) {
    name = s;
    if (s != nil && canvas != nil) {
	XStoreName(canvas->id, s);
    }
}

void Interactor::SetIcon (Interactor* i) {
    if (icon != nil && icon->canvas != nil && icon->canvas->id != nil) {
	icon->icon = nil;
	XClearIconWindow(icon->canvas->id);
    }
    icon = i;
    if ((canvas != nil) && canvas->id != nil) {
	if (i == nil) {
	    XClearIconWindow(canvas->id);
	} else if ((i->canvas != nil) && i->canvas->id != nil) {
	    XSetIconWindow(canvas->id, i->canvas->id);
	}
    }
}

#endif

#ifdef X11

void Interactor::Listen (Sensor* s) {
    register unsigned int m;

    cursensor = s;
    if (canvas == nil) {
	/* can't set input interest without window */
	return;
    }
    m = (s == nil) ? 0 : s->mask;
    if (m != 0 && m == prevmask) {
	/* already set */
	return;
    }
    prevmask = m;
    if (parent != nil) {
	/* exposure on everyone but root window */
	m |= ExposureMask;
	if (parent->parent == nil) {
	    /* configure events to top-level windows only */
	    m |= StructureNotifyMask;
	}
    }
    XSelectInput(_World_dpy, canvas->id, m);
}

void Interactor::SetCursor (Cursor* c) {
    if (canvas != nil) {
	XDefineCursor(_World_dpy, canvas->id, c->info);
	Sync();
    }
}

/*
 * Read an event.  Along the way, we have to handle expose/configure events.
 * To avoid a "flush-draw window" inner loop for when a window hierarchy
 * is mapped, we batch together redraw events and process them all at once.
 */

class PendingRedraw {
    friend class Interactor;

    Interactor* dest;
    int x, y, width, height;
};

const int redrawBufSize = 200;

static PendingRedraw redraw[redrawBufSize];
static int nredraws;

void Interactor::Read (Event& e) {
    for (;;) {
	if (PrevEvent(e) || OtherEvent(e)) {
	    if (nredraws > 0) {
		FlushRedraws();
	    }
	    break;
	}
	if (nredraws > 0 && QLen() == 0) {
	    FlushRedraws();
	}
	if (GetEvent(e, true)) {
	    break;
	}
    }
}

/*
 * Read a single event from the event stream.  If it is an input event,
 * is associated with an interactor, and the interactor is interested in it,
 * then return true.  In all other cases, return false.
 */

boolean Interactor::GetEvent (register Event& e, boolean remove) {
    XEvent xe;
    XWindow w;
    Interactor* i;
    XExposeEvent* r = &xe.xexpose;
    XConfigureEvent* c = &xe.xconfigure;
    XConfigureRequestEvent* cr = &xe.xconfigurerequest;

    XNextEvent(_World_dpy, xe);
    switch (xe.type) {
	case Expose:
	    if (assocTable->Find(i, r->window)) {
		i->SendRedraw(r->x, r->y, r->width, r->height);
	    }
	    return false;
	case ConfigureNotify:
	    if (assocTable->Find(i, c->window)) {
		i->left = c->x;
		i->bottom = i->parent->ymax - c->y - c->height + 1;
		i->SendResize(c->width, c->height);
	    }
	    return false;
	case MotionNotify:
	    w = xe.xmotion.window;
	    break;
	case KeyPress:
	    w = xe.xkey.window;
	    break;
	case ButtonPress:
	case ButtonRelease:
	    w = xe.xbutton.window;
	    break;
	case FocusIn:
	case FocusOut:
	    w = xe.xfocus.window;
	    break;
	case EnterNotify:
	case LeaveNotify:
	    w = xe.xcrossing.window;
	    break;
	case MapRequest:
	    if (_WorldView_this != nil &&
		_WorldView_this->canvas->id == xe.xmaprequest.parent
	    ) {
		_WorldView_this->InsertRemote(xe.xmaprequest.window);
	    }
	    return false;
	case ConfigureRequest:
	    if (_WorldView_this != nil &&
		_WorldView_this->canvas->id == cr->parent
	    ) {
		_WorldView_this->ChangeRemote(
		    cr->window, cr->x, ymax - cr->y, cr->width, cr->height
		);
	    }
	    return false;
	default:
	    /* ignore */
	    return false;
    }
    /* only input events should get here */
    if (nredraws > 0) {
	FlushRedraws();
    }
    if (!assocTable->Find(i, w) ||
	i->cursensor == nil || !i->cursensor->Interesting(&xe, e)
    ) {
	return false;
    }
    e.target = i;

    /*
     * Can't do the QueryPointer before checking to make sure the window
     * is valid because a motion event might have been sent just before
     * we destroyed the target window.
     */
    if (xe.type == MotionNotify) {
	XWindow dummy;
	int x, y, wx, wy;
	unsigned int state;

	XQueryPointer(_World_dpy, w, dummy, dummy, x, y, wx, wy, state);
	e.x = wx;
	e.y = wy;
    }
    e.y = i->ymax - e.y;
    if (!remove) {
	XPutBackEvent(_World_dpy, xe);
    }
    return true;
}

/*
 * Force out all the redraws that were batched while getting events.
 */

void Interactor::FlushRedraws () {
    register PendingRedraw* r, * s;
    register Interactor* i;
    register Coord top;

    s = &redraw[nredraws];
    nredraws = 0;
    for (r = &redraw[0]; r < s; r++) {
	i = r->dest;
	top = i->ymax - r->y;
	i->Redraw(r->x, top - r->height + 1, r->x + r->width - 1, top);
    }
    Sync();
}

/*
 * Check to see if any input events of interest are pending.
 * This routine will return true even if the event is for another interactor.
 */

boolean Interactor::Check () {
    Event e;

    while (XPending(_World_dpy)) {
	if (GetEvent(e, false)) {
	    return true;
	}
    }
    if (nredraws > 0) {
	FlushRedraws();
    }
    return false;
}

void Interactor::SendRedraw (Coord x, Coord y, int w, int h) {
    register PendingRedraw* p;

    if (nredraws >= redrawBufSize) {
	FlushRedraws();
    }
    p = &redraw[nredraws];
    p->dest = this;
    p->x = x;
    p->y = y;
    p->width = w;
    p->height = h;
    ++nredraws;
}

void Interactor::SendResize (int w, int h) {
    canvas->width = w;
    canvas->height = h;
    xmax = w - 1;
    ymax = h - 1;
    Resize();
}

void Interactor::Poll (Event& e) {
    XWindow root, child;
    int x, y, root_x, root_y;
    unsigned int state;
    register unsigned int s;

    XQueryPointer(
	_World_dpy, canvas->id, root, child, root_x, root_y, x, y, state
    );
    e.x = x;
    e.y = ymax - y;
    if (!assocTable->Find(e.w, root)) {
	e.w = nil;
    }
    e.wx = root_x;
    e.wy = root_y;
    s = state;
    e.control = (s&ControlMask) != 0;
    e.meta = (s&Mod1Mask) != 0;
    e.shift = (s&ShiftMask) != 0;
    e.shiftlock = (s&LockMask) != 0;
    e.leftmouse = (s&Button1Mask) != 0;
    e.middlemouse = (s&Button2Mask) != 0;
    e.rightmouse = (s&Button3Mask) != 0;
}

void Interactor::Sync () {
    XFlush(_World_dpy);
}

void Interactor::GetRelative (Coord& x, Coord& y, Interactor* rel) {
    register Interactor* t, * r;
    Coord tx, ty, rx, ry;

    if (parent == nil) {
	if (rel == nil || rel->parent == nil) {
	    /* world relative to world -- nop */
	    return;
	}
	/* world relative to interactor is negative of interactor to world */
	rel->GetRelative(x, y);
	x = -x; y = -y;
	return;
    }
    tx = x; ty = y;
    t = this;
    for (t = this; t->parent->parent != nil; t = t->parent) {
	tx += t->left;
	ty += t->bottom;
    }
    if (rel == nil || rel->parent == nil) {
	r = nil;
    } else {
	rx = 0; ry = 0;
	for (r = rel; r->parent->parent != nil; r = r->parent) {
	    rx += r->left;
	    ry += r->bottom;
	}
    }
    if (r == t) {
	/* this and rel are within same top-level interactor */
	x = tx - rx;
	y = ty - ry;
    } else {
	Interactor* w;
	XWindow child;

	w = t->parent;
	XTranslateCoordinates(
	    _World_dpy, canvas->id, w->canvas->id, x, ymax - y, rx, ry, child
	);
	x = rx;
	y = w->ymax - ry;
    }
}

void Interactor::SetName (const char* s) {
    name = s;
    if (s != nil && canvas != nil) {
	XStoreName(_World_dpy, canvas->id, s);
    }
}

void Interactor::SetIcon (Interactor* i) {
    XWMHints h;

    icon = i;
    if (i != nil && canvas != nil && i->canvas->id != nil) {
	h.flags = IconWindowHint;
	h.icon_window = i->canvas->id;
	XSetWMHints(_World_dpy, canvas->id, h);
    }
}

#endif
