/*
 * Support for window managers.
 */

#include <InterViews/worldview.h>
#include <InterViews/world.h>
#include <InterViews/shape.h>
#include <InterViews/sensor.h>
#include <InterViews/canvas.h>
#include <InterViews/cursor.h>
#include <InterViews/Xwindow.h>
#include <InterViews/Xinput.h>
#include <InterViews/Xutil.h>

extern void sleep(int);
extern void exit(int);

WorldView* _WorldView_this;

WorldView::WorldView (World* w, Sensor* s, Painter* p) : (s, p) {
    world = w;
    curfocus = nil;
    canvas = w->canvas;
    if (input == nil) {
	input = new Sensor;
	input->Catch(DownEvent);
	input->Catch(UpEvent);
	input->CatchRemote();
    }
    Listen(input);
    w->Listen(input);
    xmax = w->xmax;
    ymax = w->ymax;
    _WorldView_this = this;
}

void WorldView::Delete () {
}

void WorldView::InsertRemote (RemoteInteractor i) {
    Map(i);
}

void WorldView::ChangeRemote (
    RemoteInteractor i, Coord left, Coord top, int w, int h
) {
    Change(i, left, top, w, h);
}

RemoteInteractor WorldView::Choose (Cursor* c, boolean waitforup) {
    Event e;

    GrabMouse(c);
    do {
	Read(e);
    } while (e.eventType != DownEvent);
    if (waitforup) {
	do {
	    Read(e);
	} while (e.eventType != UpEvent);
    }
    UngrabMouse();
    return Find(e.x, e.y);
}

void WorldView::FreeList (RemoteInteractor* i) {
    delete i;
}

void WorldView::RedrawAll () {
    Interactor dummy;

    dummy.shape->Rect(xmax+1, ymax+1);
    world->Insert(&dummy, 0, 0);
    world->Remove(&dummy);
    Sync();
}

#ifdef X10

const int bmask = ButtonPressed|ButtonReleased;

void WorldView::GrabMouse (Cursor* c) {
    while (!XGrabMouse(world->canvas->id, c->info, bmask)) {
	sleep(1);
    }
}

void WorldView::UngrabMouse () {
    XUngrabMouse();
}

boolean WorldView::GrabButton (unsigned b, unsigned m, Cursor* c) {
    return XGrabButton(world->canvas->id, c->info, b | m, bmask);
}

void WorldView::UngrabButton (unsigned b, unsigned m) {
    XUngrabButton(b | m);
}

void WorldView::Lock () {
    XGrabServer();
}

void WorldView::Unlock () {
    XUngrabServer();
    Sync();
}

void WorldView::ClearInput () {
    XSync(1);
}

void WorldView::MoveMouse (Coord x, Coord y) {
    XWarpMouse(canvas->id, x, ymax - y);
}

void WorldView::Map (RemoteInteractor i) {
    XMapWindow(i);
}

void WorldView::MapRaised (RemoteInteractor i) {
    XMapWindow(i);
}

void WorldView::Unmap (RemoteInteractor i) {
    XUnmapWindow(i);
}

RemoteInteractor WorldView::Find (Coord x, Coord y) {
    RemoteInteractor i;
    Coord rx, ry;

    XInterpretLocator(world->canvas->id, rx, ry, i, (x << 16) | (ymax - y));
    return i;
}

void WorldView::Move (RemoteInteractor i, Coord left, Coord top) {
    XMoveWindow(i, left, ymax - top);
}

void WorldView::Change (
    RemoteInteractor i, Coord left, Coord top, int w, int h
) {
    XConfigureWindow(i, left, ymax - top, w, h);
}

void WorldView::Raise (RemoteInteractor i) {
    XRaiseWindow(i);
}

void WorldView::Lower (RemoteInteractor i) {
    XLowerWindow(i);
}

void WorldView::Focus (RemoteInteractor i) {
    if (i != curfocus) {
	curfocus = i;
	XFocusKeyboard(i == nil ? world->canvas->id : i);
    }
}

void WorldView::GetList (RemoteInteractor*& ilist, int& n) {
    XWindow parent;

    XQueryTree(canvas->id, parent, n, ilist);
}

void WorldView::GetInfo (
    RemoteInteractor i, Coord& x1, Coord& y1, Coord& x2, Coord& y2
) {
    XWindowInfo info;

    XQueryWindow(i, info);
    x1 = info.x;
    y2 = ymax - info.y;
    x2 = info.x + info.width + 2*info.bdrwidth - 1;
    y1 = y2 - info.height - 2*info.bdrwidth + 1;
}

RemoteInteractor WorldView::GetIcon (RemoteInteractor i) {
    XWindowInfo info;

    XQueryWindow(i, info);
    return info.assoc_wind;
}

void WorldView::AssignIcon (RemoteInteractor i, RemoteInteractor icon) {
    XSetIconWindow(i, icon);
}

void WorldView::UnassignIcon (RemoteInteractor i) {
    XClearIconWindow(i);
}

char* WorldView::GetName (RemoteInteractor i) {
    char* name;

    XFetchName(i, name);
    return name;
}

#endif

#ifdef X11

const int bmask = ButtonPressMask|ButtonReleaseMask|OwnerGrabButtonMask;

void WorldView::GrabMouse (Cursor* c) {
    while (
	XGrabPointer(
	    _World_dpy, world->canvas->id, XFalse, bmask,
	    GrabModeAsync, GrabModeAsync, XNone, c->info, XCurrentTime
	) != GrabSuccess
    ) {
	sleep(1);
    }
}

void WorldView::UngrabMouse () {
    XUngrabPointer(_World_dpy, XCurrentTime);
}

boolean WorldView::GrabButton (unsigned b, unsigned m, Cursor* c) {
    XGrabButton(
	_World_dpy, b, m, world->canvas->id, XTrue, bmask,
	GrabModeAsync, GrabModeAsync, XNone, c->info
    );
    return true;
}

void WorldView::UngrabButton (unsigned b, unsigned m) {
    XUngrabButton(_World_dpy, b, m, world->canvas->id);
}

void WorldView::Lock () {
    XGrabServer(_World_dpy);
}

void WorldView::Unlock () {
    XUngrabServer(_World_dpy);
    Sync();
}

void WorldView::ClearInput () {
    XSync(_World_dpy, 1);
}

void WorldView::MoveMouse (Coord x, Coord y) {
    XWarpPointer(
	_World_dpy, canvas->id, canvas->id, 0, 0, xmax, ymax, x, ymax - y
    );
}

void WorldView::Map (RemoteInteractor i) {
    XMapWindow(_World_dpy, i);
}

void WorldView::MapRaised (RemoteInteractor i) {
    XMapRaised(_World_dpy, i);
}

void WorldView::Unmap (RemoteInteractor i) {
    XUnmapWindow(_World_dpy, i);
}

RemoteInteractor WorldView::Find (Coord x, Coord y) {
    RemoteInteractor i;
    Coord rx, ry;

    XTranslateCoordinates(
	_World_dpy, world->canvas->id, world->canvas->id,
	x, ymax - y, rx, ry, i
    );
    return i;
}

void WorldView::Move (RemoteInteractor i, Coord left, Coord top) {
    XMoveWindow(_World_dpy, i, left, ymax - top);
}

void WorldView::Change (
    RemoteInteractor i, Coord left, Coord top, int w, int h
) {
    XMoveResizeWindow(_World_dpy, i, left, ymax - top, w, h);
}

void WorldView::Raise (RemoteInteractor i) {
    XRaiseWindow(_World_dpy, i);
}

void WorldView::Lower (RemoteInteractor i) {
    XLowerWindow(_World_dpy, i);
}

void WorldView::Focus (RemoteInteractor i) {
    if (i != curfocus) {
	curfocus = i;
	XSetInputFocus(
	    _World_dpy, i == nil ? (void*)XPointerRoot : i,
	    RevertToPointerRoot, XCurrentTime
	);
    }
}

void WorldView::GetList (RemoteInteractor*& ilist, int& n) {
    XWindow parent;

    XQueryTree(_World_dpy, canvas->id, parent, parent, ilist, n);
}

void WorldView::GetInfo (
    RemoteInteractor i, Coord& x1, Coord& y1, Coord& x2, Coord& y2
) {
    XWindow root;
    int x, y, w, h, bw, d;

    XGetGeometry(_World_dpy, i, root, x, y, w, h, bw, d);
    x1 = x;
    y2 = ymax - y;
    x2 = x + w + 2*bw - 1;
    y1 = y2 - h - 2*bw + 1;
}

RemoteInteractor WorldView::GetIcon (RemoteInteractor i) {
    XWMHints* h;
    RemoteInteractor r;

    h = XGetWMHints(_World_dpy, i);
    if (h == nil || (h->flags&IconWindowHint) == 0) {
	r = nil;
    } else {
	r = h->icon_window;
    }
    delete h;
    return r;
}

void WorldView::AssignIcon (RemoteInteractor i, RemoteInteractor icon) {
    XWMHints h;

    h.flags = IconWindowHint;
    h.icon_window = i;
    XSetWMHints(_World_dpy, icon, h);
    h.icon_window = icon;
    XSetWMHints(_World_dpy, i, h);
}

void WorldView::UnassignIcon (RemoteInteractor i) {
    XWMHints h;

    h.flags = IconWindowHint;
    h.icon_window = XNone;
    XSetWMHints(_World_dpy, i, h);
}

char* WorldView::GetName (RemoteInteractor i) {
    char* name;

    XFetchName(_World_dpy, i, name);
    return name;
}

#endif
