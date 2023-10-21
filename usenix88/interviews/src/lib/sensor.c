/*
 * Implementation of input event handling.
 */

#include <InterViews/interactor.h>
#include <InterViews/sensor.h>
#include <InterViews/world.h>
#include <InterViews/Xwindow.h>
#include <InterViews/Xinput.h>
#include <InterViews/Xutil.h>
#include <string.h>
#include "table.h"

Sensor* allEvents;
Sensor* onoffEvents;
Sensor* updownEvents;
Sensor* noEvents;
Sensor* stdsensor;

#ifdef X10

#define PointerMotionMask MouseMoved
#define KeyPressMask KeyPressed
#define EnterWindowMask EnterWindow
#define LeaveWindowMask LeaveWindow
#define FocusChangeMask FocusChange
#define SubstructureRedirectMask 0

const int upmask = ButtonPressed|ButtonReleased;
const int downmask = ButtonPressed|ButtonReleased;
const int initmask = ExposeRegion|ExposeCopy;

#endif

#ifdef X11

const int upmask = ButtonPressMask|ButtonReleaseMask|OwnerGrabButtonMask;
const int downmask = ButtonPressMask|ButtonReleaseMask|OwnerGrabButtonMask;
const int initmask = PointerMotionHintMask;

#endif

static XWindow focus;
static Sensor* grabber;

inline int ButtonIndex (unsigned b) { return (b >> 5) & 07; }
inline int ButtonFlag (unsigned b) { return 1 << (b & 0x1f); }

inline void SetButton (unsigned a[], unsigned b) {
    a[ButtonIndex(b)] |= ButtonFlag(b);
}

inline void ClearButton (unsigned a[], unsigned b) {
    a[ButtonIndex(b)] &= ~ButtonFlag(b);
}

inline boolean ButtonIsSet (unsigned a[], unsigned b) {
    return (a[ButtonIndex(b)] & ButtonFlag(b)) != 0;
}

inline void SetMouseButtons (unsigned a[]) {
    a[0] |= 0x7;
}

inline void ClearMouseButtons (unsigned a[]) {
    a[0] &= ~0x7;
}

inline boolean MouseButtons (unsigned a[]) {
    return (a[0] & 0x7) != 0;
}

Sensor::Sensor () {
    register int i;

    mask = initmask;
    channels = 0;
    maxchannel = 0;
    timer = false;
    for (i = 0; i < 8; i++) {
	down[i] = 0;
	up[i] = 0;
    }
}

Sensor::Sensor (register Sensor* s) {
    register int i;

    mask = s->mask;
    channels = s->channels;
    maxchannel = s->maxchannel;
    timer = s->timer;
    sec = s->sec;
    usec = s->usec;
    for (i = 0; i < 8; i++) {
	down[i] = s->down[i];
	up[i] = s->up[i];
    }
}

Sensor::~Sensor () {
    /* nothing to do */
}

void InitSensors () {
    allEvents = new Sensor;
    allEvents->Catch(MotionEvent);
    allEvents->Catch(DownEvent);
    allEvents->Catch(UpEvent);
    allEvents->Catch(KeyEvent);
    allEvents->Catch(OnEvent);
    allEvents->Catch(OffEvent);
    onoffEvents = new Sensor;
    onoffEvents->Catch(OnEvent);
    onoffEvents->Catch(OffEvent);
    updownEvents = new Sensor;
    updownEvents->Catch(UpEvent);
    updownEvents->Catch(DownEvent);
    noEvents = new Sensor;
    stdsensor = noEvents;
    focus = nil;
}

void Sensor::Catch (EventType t) {
    register int i;

    switch (t) {
	case MotionEvent:
	    mask |= PointerMotionMask;
	    break;
	case DownEvent:
	    mask |= downmask;
	    SetMouseButtons(down);
	    break;
	case UpEvent:
	    mask |= upmask;
	    SetMouseButtons(up);
	    break;
	case KeyEvent:
	    mask |= KeyPressMask;
	    down[0] |= 0xfffffff8;
	    for (i = 1; i < 8; i++) {
		down[i] = 0xffffffff;
	    }
	    break;
	case OnEvent:
	    mask |= EnterWindowMask|FocusChangeMask;
	    break;
	case OffEvent:
	    mask |= LeaveWindowMask|FocusChangeMask;
	    break;
	case ChannelEvent:
	    /* ignore */
	    break;
	case TimerEvent:
	    timer = true;
	    sec = 0;
	    usec = 0;
	    break;
    }
}

void Sensor::CatchButton (EventType t, int b) {
    switch (t) {
	case DownEvent:
	    mask |= downmask;
	    SetButton(down, b);
	    break;
	case UpEvent:
	    mask |= upmask;
	    SetButton(up, b);
	    break;
	case KeyEvent:
	    mask |= KeyPressMask;
	    SetButton(down, b);
	    break;
	default:
	    /* ignore */
	    break;
    }
}

void Sensor::CatchChannel (int ch) {
    channels |= (1 << ch);
    if (ch > maxchannel) {
	maxchannel = ch+1;
    }
}

void Sensor::CatchTimer (int s, int u) {
    timer = true;
    sec = s;
    usec = u;
}

void Sensor::Ignore (EventType t) {
    register int i;

    switch (t) {
	case MotionEvent:
	    mask &= ~PointerMotionMask;
	    break;
	case DownEvent:
	    ClearMouseButtons(down);
	    if (!MouseButtons(up)) {
		mask &= ~downmask;
	    }
	    break;
	case UpEvent:
	    ClearMouseButtons(up);
	    if (!MouseButtons(down)) {
		mask &= ~upmask;
	    }
	    break;
	case KeyEvent:
	    down[0] &= ~0xfffffff8;
	    for (i = 1; i < 8; i++) {
		down[i] = 0;
	    }
	    mask &= ~KeyPressMask;
	    break;
	case OnEvent:
	    mask &= ~EnterWindowMask;
	    if ((mask&LeaveWindowMask) == 0) {
		mask &= ~FocusChangeMask;
	    }
	    break;
	case OffEvent:
	    mask &= ~LeaveWindowMask;
	    if ((mask&EnterWindowMask) == 0) {
		mask &= ~FocusChangeMask;
	    }
	    break;
	case ChannelEvent:
	    channels = 0;
	    maxchannel = 0;
	    break;
	case TimerEvent:
	    timer = false;
	    break;
    }
}

void Sensor::IgnoreButton (EventType t, int b) {
    register int i;

    switch (t) {
	case DownEvent:
	    ClearButton(down, b);
	    if (!MouseButtons(down) && !MouseButtons(up)) {
		mask &= ~downmask;
	    }
	    break;
	case UpEvent:
	    ClearButton(up, b);
	    if (!MouseButtons(up) && !MouseButtons(down)) {
		mask &= ~upmask;
	    }
	    break;
	case KeyEvent:
	    ClearButton(down, b);
	    if ((down[0] & 0xfffffff8) == 0) {
		mask &= ~KeyPressMask;
		for (i = 1; i < 8; i++) {
		    if (down[i] != 0) {
			mask |= KeyPressMask;
			break;
		    }
		}
	    }
	    break;
	default:
	    /* ignore */
	    break;
    }
}

void Sensor::IgnoreChannel (int ch) {
    channels &= ~(1 << ch);
    if (channels == 0) {
	maxchannel = 0;
    } else {
	while ((channels & (1 << maxchannel)) == 0) {
	    --maxchannel;
	}
    }
}

void Sensor::CatchRemote () {
    mask |= SubstructureRedirectMask;
}

void Sensor::IgnoreRemote () {
    mask &= ~SubstructureRedirectMask;
}

#ifdef X10

boolean Sensor::Interesting (void* raw, Event& e) {
    boolean b;
    XEvent* x = (XEvent*) raw;
    register XMouseEvent* m = (XMouseEvent*)x;
    XFocusChangeEvent* f = (XFocusChangeEvent*)x;

    b = false;
    switch (x->type) {
	case MouseMoved:
	    e.eventType = MotionEvent;
	    e.x = m->x;
	    e.y = m->y;
	    e.w = nil;
	    e.wx = (m->location >> 16) & 0xffff;
	    e.wy = m->location & 0xffff;
	    b = true;
	    break;
	case KeyPressed:
	    e.eventType = KeyEvent;
	    e.GetButtonInfo(x);
	    b = ButtonIsSet(down, e.button);
	    break;
	case ButtonPressed:
	    e.eventType = DownEvent;
	    e.GetButtonInfo(x);
	    b = ButtonIsSet(down, e.button);
	    if (b && ButtonIsSet(up, e.button)) {
		grabber = this;
	    } else {
		grabber = nil;
	    }
	    break;
	case ButtonReleased:
	    e.eventType = UpEvent;
	    e.GetButtonInfo(x);
	    b = ButtonIsSet(up, e.button) || (grabber != nil);
	    break;
	case FocusChange:
	    if (f->subwindow == nil) {
		if (f->detail == EnterWindow) {
		    focus = f->window;
		    e.eventType = OnEvent;
		} else {
		    focus = nil;
		    e.eventType = OffEvent;
		}
		b = true;
	    }
	    break;
	case EnterWindow:
	    if (f->subwindow == nil && f->detail != 1 && f->window != focus) {
		e.eventType = OnEvent;
		b = true;
	    }
	    break;
	case LeaveWindow:
	    if (f->subwindow == nil && f->detail != 1 && f->window != focus) {
		e.eventType = OffEvent;
		b = true;
	    }
	    break;
	default:
	    /* ignore */;
    }
    return b;
}

void Event::GetButtonInfo (void* xe) {
    register XKeyOrButtonEvent* k = (XKeyOrButtonEvent*)xe;
    register int state;
    int n;

    timestamp = 10 * k->time;
    x = k->x;
    y = k->y;
    w = nil;
    wx = (k->location >> 16) & 0xffff;
    wy = k->location & 0xffff;
    state = k->detail;
    shift = (state & ShiftMask) != 0;
    control = (state & ControlMask) != 0;
    meta = (state & MetaMask) != 0;
    shiftlock = (state & ShiftLockMask) != 0;
    leftmouse = (state & LeftMask) != 0;
    middlemouse = (state & MiddleMask) != 0;
    rightmouse = (state & RightMask) != 0;
    button = state & 0xff;
    if (k->type == KeyPressed) {
	keystring = XLookupMapping(*k, n);
	len = n;
    } else {
	button = 2 - button;
	len = 0;
    }
    if (len == 0) {
	keystring = keydata;
	keydata[0] = '\0';
    }
}

#endif

#ifdef X11

boolean Sensor::Interesting (void* raw, Event& e) {
    boolean b;
    register XEvent* x = (XEvent*)raw;

    b = false;
    switch (x->type) {
	case MotionNotify:
	    e.eventType = MotionEvent;
	    e.x = x->xmotion.x;
	    e.y = x->xmotion.y;
	    if (!assocTable->Find(e.w, x->xmotion.root)) {
		e.w = nil;
	    }
	    e.wx = x->xmotion.x_root;
	    e.wy = x->xmotion.y_root;
	    b = true;
	    break;
	case KeyPress:
	    e.eventType = KeyEvent;
	    e.GetButtonInfo(x);
	    b = ButtonIsSet(down, e.button);
	    break;
	case ButtonPress:
	    e.eventType = DownEvent;
	    e.GetButtonInfo(x);
	    b = ButtonIsSet(down, e.button);
	    if (b && ButtonIsSet(up, e.button)) {
		grabber = this;
	    } else {
		grabber = nil;
	    }
	    break;
	case ButtonRelease:
	    e.eventType = UpEvent;
	    e.GetButtonInfo(x);
	    b = ButtonIsSet(up, e.button) || (grabber != nil);
	    break;
	case FocusIn:
	    focus = x->xfocus.window;
	    e.eventType = OnEvent;
	    b = true;
	    break;
	case FocusOut:
	    focus = nil;
	    e.eventType = OffEvent;
	    b = true;
	    break;
	case EnterNotify:
	case LeaveNotify:
	    if (x->xcrossing.detail != NotifyInferior &&
		x->xcrossing.window != focus
	    ) {
		e.eventType = x->type == EnterNotify ? OnEvent : OffEvent;
		b = true;
	    }
	    break;
	default:
	    /* ignore */;
    }
    return b;
}

void Event::GetButtonInfo (void* xe) {
    register XKeyEvent* k = &(((XEvent*)xe)->xkey);
    register int state;
    XComposeStatus status;
    char buf[4096];

    timestamp = k->time;
    x = k->x;
    y = k->y;
    if (!assocTable->Find(w, k->root)) {
	w = nil;
    }
    wx = k->x_root;
    wy = k->y_root;
    state = k->state;
    shift = (state & ShiftMask) != 0;
    control = (state & ControlMask) != 0;
    meta = (state & Mod1Mask) != 0;
    shiftlock = (state & LockMask) != 0;
    leftmouse = (state & Button1Mask) != 0;
    middlemouse = (state & Button2Mask) != 0;
    rightmouse = (state & Button3Mask) != 0;
    button = k->keycode;
    if (k->type == KeyPress) {
	len = XLookupString(*k, buf, sizeof(buf), nil, &status);
	if (len > 0) {
	    if (len <= sizeof(int)) {
		keystring = keydata;
	    } else {
		keystring = new char[len+1];
	    }
	    strcpy(keystring, buf);
	}
    } else {
	button -= 1;
	len = 0;
    }
    if (len == 0) {
	keystring = keydata;
	keydata[0] = '\0';
    }
}

#endif

void Event::GetAbsolute (Coord& absx, Coord& absy) {
    if (w == nil) {
	register Interactor* r;

	for (r = target; r->Parent() != nil; r = r->Parent());
	w = (World*)r;
    }
    absx = w->InvMapX(wx);
    absy = w->InvMapY(wy);
}

void Event::GetAbsolute (World*& s, Coord& absx, Coord& absy) {
    GetAbsolute(absx, absy);
    s = w;
}
