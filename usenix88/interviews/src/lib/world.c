/*
 * The world refers to the top-level scene.
 */

#include <InterViews/world.h>
#include <InterViews/shape.h>
#include <InterViews/canvas.h>
#include <InterViews/painter.h>
#include <InterViews/Xinput.h>
#include <InterViews/Xoutput.h>
#include <InterViews/Xutil.h>
#include <InterViews/Xwindow.h>
#include <stdio.h>
#include "table.h"

extern void InitPaint(World*), InitSensors(), InitCursors();

double inch, inches, cm, point, points;

Table* assocTable;
XDisplay* _World_dpy;
int _World_nplanes;

#ifdef X10

extern char* XGetDefault(const char*, const char*);
extern unsigned XParseGeometry(char*, Coord&, Coord&, int&, int&);

int _Workstation_planemask;

World::World (const char* s, const char* device) : (nil, nil) {
    prog = s;
    display = XOpenDisplay(device);
    if (display == nil) {
	fprintf(stderr, "fatal error: can't open display\n");
	exit(1);
    }
    root = XRootWindow;
    SetCurrent();
    assocTable = new Table(4096);
    canvas = new Canvas(root);
    canvas->width = DisplayWidth();
    canvas->height = DisplayHeight();
    _Workstation_planemask = AllPlanes;
    _World_nplanes = XDisplayPlanes();
    inch = 72;
    inches = 72;
    cm = inch/2.54;
    point = 1;
    points = 1;
    assocTable->Insert(root, this);
    xmax = canvas->width - 1;
    ymax = canvas->height - 1;
    GetDefault("anything");
    InitPaint(this);
    InitSensors();
    InitCursors();
    output = stdpaint;
    output->Reference();
}

void World::DoInsert (Interactor* i, boolean b, Coord& x, Coord& y, Shape* s) {
    XEvent xevent;

    if (b && s->width > 0 && s->height > 0) {
	Place(i, x, y, x + s->width - 1, y + s->height - 1);
    } else {
	UserPlace(i);
    }
    XWindowEvent(i->canvas->id, ExposeWindow, xevent);
    i->Draw();
}

void World::DoChange (Interactor* i, Shape* s) {
    XEvent xevent;
    Coord x, y;

    x = 0; y = 0;
    i->GetRelative(x, y, this);
    Place(i, x, y, x + s->width - 1, y + s->height - 1);
    XWindowEvent(i->canvas->id, ExposeWindow, xevent);
    i->Draw();
}

void World::DoMove (Interactor* i, Coord& x, Coord& y) {
    XMoveWindow(i->canvas->id, x, ymax - y);
}

void World::DoRemove (Interactor*) {
    XFlush();
}

int World::Fileno () {
    return Xdpyno();
}

void World::SetCurrent () {
    XSetDisplay((XDisplay*)display);
    _Workstation_planemask = AllPlanes;
    _World_nplanes = XDisplayPlanes();
}

void World::SetRoot (void*) {}
void World::SetScreen (int) {}

int World::NPlanes () {
    return XDisplayPlanes();
}

int World::NButtons () {
    return 3;
}

int World::PixelSize () {
    return XYPixmapSize(32, 1, XDisplayPlanes()) >> 2;
}

char* World::GetDefault (const char* name) {
    return (prog == nil) ? nil : XGetDefault(prog, name);
}

char* World::GetGlobalDefault (const char* name) {
    return XGetDefault("", name);
}

unsigned World::ParseGeometry (
    char* spec, Coord& x, Coord& y, int& w, int &h
) {
    return XParseGeometry(spec, x, y, w, h);
}

void World::RingBell (int v) {
    if (v > 7) {
	XFeep(7);
    } else if (v >= 0) {
	XFeep(v);
    }
}

void World::SetKeyClick (int v) {
    XKeyClickControl(v);
}

void World::SetAutoRepeat (boolean b) {
    if (b) {
	XAutoRepeatOn();
    } else {
	XAutoRepeatOff();
    }
}

void World::SetFeedback (int thresh, int scale) {
    XMouseControl(scale, thresh);
}

#endif

#ifdef X11

XColormap _World_cmap;
int _World_screen;
XWindow _World_root;

World::World (const char* s, const char* device) : (nil, nil) {
    prog = s;
    display = XOpenDisplay(device);
    if (display == nil) {
	fprintf(stderr, "fatal error: can't open display\n");
	exit(1);
    }
    screen = XDefaultScreen((XDisplay*)display);
    root = XRootWindow((XDisplay*)display, screen);
    SetCurrent();
    assocTable = new Table(4096);
    canvas = new Canvas(root);
    canvas->width = XDisplayWidth((XDisplay*)display, screen);
    canvas->height = XDisplayHeight((XDisplay*)display, screen);
    cm = 0.1*XDisplayWidthMM((XDisplay*)display, screen);
    inch = 0.254*XDisplayWidthMM((XDisplay*)display, screen);
    inches = inch;
    point = 72.07/inch;
    points = point;
    assocTable->Insert(root, this);
    xmax = canvas->width - 1;
    ymax = canvas->height - 1;
    GetDefault("anything");
    InitPaint(this);
    InitSensors();
    InitCursors();
    output = stdpaint;
    output->Reference();
}

void World::DoInsert (Interactor* i, boolean b, Coord& x, Coord& y, Shape* s) {
    if (b && s->width > 0 && s->height > 0) {
	Place(i, x, y, x + s->width - 1, y + s->height - 1);
    } else {
	UserPlace(i);
    }
}

void World::DoChange (Interactor* i, Shape* s) {
    Coord x, y;

    x = 0; y = 0;
    i->GetRelative(x, y);
    Place(i, x, y, x + s->width - 1, y + s->height - 1);
}

void World::DoMove (Interactor* i, Coord& x, Coord& y) {
    XMoveWindow((XDisplay*)display, i->canvas->id, x, ymax - y);
}

void World::DoRemove (Interactor*) {
    XFlush((XDisplay*)display);
}

int World::Fileno () {
    return XConnectionNumber((XDisplay*)display);
}

void World::SetCurrent () {
    _World_dpy = (XDisplay*)display;
    _World_root = root;
    _World_screen = screen;
    _World_cmap = XDefaultColormap((XDisplay*)display, screen);
    _World_nplanes = XDisplayPlanes((XDisplay*)display, screen);
}

void World::SetRoot (void* r) {
    root = r;
    if (_World_dpy == display) {
	_World_root = r;
    }
}

void World::SetScreen (int n) {
    screen = n;
    if (_World_dpy == display) {
	_World_screen = n;
    }
}

int World::NPlanes () {
    return XDisplayPlanes((XDisplay*)display, screen);
}

int World::NButtons () {
    return 3;
}

int World::PixelSize () {
    return XBitmapPad((XDisplay*)display);
}

char* World::GetDefault (const char* name) {
    return (prog == nil) ? nil : XGetDefault((XDisplay*)display, prog, name);
}

char* World::GetGlobalDefault (const char* name) {
    return XGetDefault((XDisplay*)display, "", name);
}

unsigned World::ParseGeometry (
    char* spec, Coord& x, Coord& y, int& w, int &h
) {
    return XParseGeometry(spec, x, y, w, h);
}

void World::RingBell (int v) {
    if (v > 100) {
	XBell((XDisplay*)display, 100);
    } else if (v >= 0) {
	XBell((XDisplay*)display, v);
    }
}

void World::SetKeyClick (int v) {
    XKeyboardControl k;

    k.key_click_percent = v;
    XChangeKeyboardControl((XDisplay*)display, KBKeyClickPercent, k);
}

void World::SetAutoRepeat (boolean b) {
    if (b) {
	XAutoRepeatOn((XDisplay*)display);
    } else {
	XAutoRepeatOff((XDisplay*)display);
    }
}

void World::SetFeedback (int t, int s) {
    XChangePointerControl((XDisplay*)display, XTrue, XTrue, s, 1, t);
}

#endif

void World::AsyncRead () {
    /* nothing to do -- X is always asynchronous */
}

void World::SyncRead () {
    /* nothing to do -- X is always asynchronous */
}

int World::Width () {
    return canvas->width;
}

int World::Height () {
    return canvas->height;
}

Coord World::InvMapX (Coord x) {
    return x;
}

Coord World::InvMapY (Coord y) {
    return canvas->height - 1 - y;
}

void World::Delete () {
    assocTable->Remove(root);
    delete assocTable;
    XCloseDisplay((XDisplay*)display);
    Scene::Delete();
}
