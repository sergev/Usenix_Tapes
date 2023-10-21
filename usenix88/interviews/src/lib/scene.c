/*
 * Base composite interactor.
 */

#include <InterViews/scene.h>
#include <InterViews/shape.h>
#include <InterViews/canvas.h>
#include <InterViews/cursor.h>
#include <InterViews/sensor.h>
#include <InterViews/painter.h>
#include <InterViews/rubrect.h>
#include <InterViews/Xcursor.h>
#include <InterViews/Xinput.h>
#include <InterViews/Xoutput.h>
#include <InterViews/Xwindow.h>
#include "table.h"

Scene.Scene (Sensor* in, Painter* out) : (in, out) {
    propagate = true;
}

#ifdef X10

extern void sleep(int);

typedef struct ExposeList {
    XEvent* event;
    ExposeList* next;
};

void Scene::UserPlace (Interactor* i) {
    Event e;
    Coord x1, y1, x2, y2, tmp;
    RubberRect* r;
    const int mask = ButtonPressed|ButtonReleased|MouseMoved;
    register XEvent* xe;
    register ExposeList* list = nil;
    register ExposeList* el;

    do {
	Poll(e);
    } while (e.leftmouse || e.middlemouse || e.rightmouse);
    while (!XGrabMouse(canvas->id, upperleft->info, mask)) {
	sleep(1);
    }
    do {
	Poll(e);
    } while (!e.leftmouse && !e.middlemouse && !e.rightmouse);
    e.target = this;
    e.GetAbsolute(x1, y1);
    if (e.leftmouse && i->shape->width != 0) {
	x2 = x1 + i->shape->width - 1;
	y2 = y1 + i->shape->height - 1;
	r = new SlidingRect(output, canvas, x1, y1, x2, y2, x1, y2);
	r->Track(x1, y1);
    } else {
	do {
	    Poll(e);
	    e.target = this;
	    e.GetAbsolute(x2, y2);
	    x2 = e.x; y2 = e.y;
	} while (x2 == x1 && y2 == y1 &&
	     (e.leftmouse || e.middlemouse || e.rightmouse)
	);
	r = new RubberRect(output, canvas, x1, y1, x2, y1);
	r->Track(x2, y1);
	XGrabMouse(canvas->id, lowerright->info, mask);
    }
    XGrabServer();
    while (e.leftmouse || e.middlemouse || e.rightmouse) {
	Poll(e);
	e.target = this;
	e.GetAbsolute(x1, y1);
	x1 = e.x; y1 = e.y;
	r->Track(x1, y1);
    }

    xe = new XEvent;
    while (XPending()) {
	XNextEvent(*xe);
	if (xe->type == ExposeRegion || xe->type == ExposeWindow ||
	    xe->type == ExposeCopy
	) {
	    el = new ExposeList;
	    el->event = xe;
	    el->next = list;
	    list = el;
	    xe = new XEvent;
	}
    }
    while (list != nil) {
	XPutBackEvent(*list->event);
	delete(list->event);
	list = list->next;
    }

    XUngrabServer();
    XUngrabMouse();
    r->GetCurrent(x1, y1, x2, y2);
    delete r;

    if (x1 > x2) {
	tmp = x1; x1 = x2; x2 = tmp;
    } else if (x1 == x2) {
	x2 = x1 + 10;
    }
    if (y1 > y2) {
	tmp = y1; y1 = y2; y2 = tmp;
    } else if (y1 == y2) {
	y2 = y1 + 10;
    }
    Place(i, x1, y1, x2, y2);
}

void Scene::Place (
    Interactor* i, Coord x1, Coord y1, Coord x2, Coord y2, boolean map
) {
    register XWindow id;
    register int w, h;
    Coord top;

    if (i->parent != nil && i->parent != this) {
	i->parent->Remove(i);
    }
    i->parent = this;
    top = ymax - y2;
    w = x2 - x1 + 1;
    h = y2 - y1 + 1;
    if (i->canvas == nil) {
	id = XCreateWindow(canvas->id, x1, top, w, h, 0, 0, 0);
	i->canvas = new Canvas(id);
	Assign(i, x1, y1, w, h);
	InitCanvas(i);
    } else {
	id = i->canvas->id;
	XUnmapSubwindows(id);
	Assign(i, x1, y1, w, h);
	XConfigureWindow(id, x1, top, w, h);
    }
    if (map) {
	XMapWindow(id);
    }
}

void Scene::Map (register Interactor* i, boolean) {
    XMapWindow(i->canvas->id);
    if (parent != nil) {
	i->Draw();
    }
}

void Scene::Unmap (register Interactor* i) {
    XUnmapWindow(i->canvas->id);
}

void Scene::InitCanvas (register Interactor* i) {
    register XWindow id = i->canvas->id;
    register Interactor* ic = i->icon;
    Coord x,y;

    i->SetName(i->name);
    if (ic != nil && ic->canvas == nil) {
	ic->parent = this;
	ic->icon = nil;
	x = i->left + ((i->xmax - ic->shape->width) >> 1) + 1;
	y = i->bottom + ((i->ymax - ic->shape->height) >> 1) + 1;
	ic->canvas = new Canvas(
	    XCreateWindow(
		canvas->id,
		x, ymax - (y + ic->shape->height - 1),
		ic->shape->width, ic->shape->height,
		0, 0, 0
	    )
	);
	Assign(ic, x, y, ic->shape->width, ic->shape->height);
	InitCanvas(ic);
	i->SetIcon(ic);
    } else if (ic != nil) {
	i->SetIcon(ic);
    }

    if (i->cursensor == nil) {
	if (i->input == nil) {
	    XSelectInput(id, ExposeRegion|ExposeCopy);
	} else {
	    i->cursensor = i->input;
	    XSelectInput(id, i->cursensor->mask);
	}
    } else {
	XSelectInput(id, i->cursensor->mask);
    }
    XDefineCursor(id, i->cursor->info);
    assocTable->Insert(id, i);
}

void Scene::Raise (Interactor* i) {
    if (i->canvas != nil && !i->canvas->offscreen) {
	XRaiseWindow(i->canvas->id);
    }
}

void Scene::Lower (Interactor* i) {
    if (i->canvas != nil && !i->canvas->offscreen) {
	XLowerWindow(i->canvas->id);
    }
}

#endif

#ifdef X11

/*
 * Place an interactor according to user preferences.
 */

void Scene::UserPlace (register Interactor* i) {
    if (i->parent != nil && i->parent != this) {
	i->parent->Remove(i);
    }
    i->parent = this;
    MakeWindow(i, 0, 0, i->shape->width, i->shape->height, false);
    DoMap(i);
}

/*
 * Place an interactor at a particular position.
 */

void Scene::Place (
    Interactor* i, Coord x1, Coord y1, Coord x2, Coord y2, boolean map
) {
    register int width, height;
    Coord top;

    if (i->parent != nil && i->parent != this) {
	i->parent->Remove(i);
    }
    i->parent = this;
    top = ymax - y2;
    width = x2 - x1 + 1;
    height = y2 - y1 + 1;
    if (i->canvas == nil) {
	MakeWindow(i, x1, top, width, height, true);
    } else {
	if (parent == nil) {
	    XSetWindowAttributes a;

	    a.override_redirect = XTrue;
	    XChangeWindowAttributes(
		_World_dpy, i->canvas->id, CWOverrideRedirect, &a
	    );
	}
	XMoveResizeWindow(_World_dpy, i->canvas->id, x1, top, width, height);
    }
    Assign(i, x1, ymax - top - height + 1, width, height);
    if (map) {
	XMapRaised(_World_dpy, i->canvas->id);
    }
}

/*
 * Create and map a window for an interactor and initialize
 * the interactor's canvas appropriately.  If the "force" parameter
 * is true, then give the window the position and size requested.
 * If "force" is false, then potentially let the user place and size
 * the window.
 */

void Scene::MakeWindow (
    register Interactor* i, Coord x, Coord y, int width, int height,
    boolean force
) {
    XWindow w;
    register XValueMask m;
    XSetWindowAttributes a;
    int rwidth, rheight;

    m = CWDontPropagate;
    if (force) {
	m |= CWOverrideRedirect;
	a.override_redirect = XTrue;
    }
    if (parent != nil) {
	m |= CWBackPixmap|CWWinGravity;
	a.background_pixmap = (XPixmap)XParentRelative;
	a.win_gravity = UnmapGravity;
    }
    a.do_not_propagate_mask = (
	FocusChangeMask | EnterWindowMask | LeaveWindowMask
    );
    rwidth = (width == 0) ? 100 : width;
    rheight = (height == 0) ? 100 : height;
    w = XCreateWindow(
	_World_dpy, canvas->id, x, y, rwidth, rheight, 0, 0,
	XInputOutput, XCopyFromParent, m, &a
    );
    i->canvas = new Canvas(w);
    assocTable->Insert(w, i);
    XDefineCursor(_World_dpy, w, i->cursor->info);
    i->Listen(i->cursensor == nil ? i->input : i->cursensor);
}

/*
 * Map an interactor and wait for confirmation of its position.
 * The interactor is either being placed by the user or inserted
 * in the world (in which case a window manager might choose to allocate
 * it a different size and placement).
 */

struct EventInfo {
    XWindow w;
    Interactor* i;
};

static XBool MapOrRedraw (XDisplay*, XEvent& xe, char* w) {
    register EventInfo* x = (EventInfo*)w;

    return (
	(xe.type == MapNotify && xe.xmap.window == x->w) ||
	(xe.type == ConfigureNotify && xe.xconfigure.window == x->w) ||
	(xe.type == Expose && assocTable->Find(x->i, xe.xexpose.window))
    );
}

void Scene::DoMap (Interactor* i) {
    EventInfo info;
    XEvent xe;
    XConfigureEvent* c = &xe.xconfigure;
    XExposeEvent* x = &xe.xexpose;
    boolean assigned;

    info.w = i->canvas->id;
    XMapRaised(_World_dpy, info.w);
    assigned = false;
    for (;;) {
	XIfEvent(_World_dpy, xe, MapOrRedraw, (char*)&info);
	if (xe.type == MapNotify) {
	    if (!assigned) {
		Assign(i, 0, 0, i->shape->width, i->shape->height);
	    }
	    break;
	} else if (xe.type == ConfigureNotify) {
	    Assign(i, c->x, ymax - c->y - c->height + 1, c->width, c->height);
	    assigned = true;
	} else if (xe.type == Expose) {
	    info.i->SendRedraw(x->x, x->y, x->width, x->height);
	    FlushRedraws();
	}
    }
}

void Scene::Map (register Interactor* i, boolean raised) {
    if (raised) {
	XMapRaised(_World_dpy, i->canvas->id);
    } else {
	XMapWindow(_World_dpy, i->canvas->id);
    }
}

void Scene::Unmap (register Interactor* i) {
    XUnmapWindow(_World_dpy, i->canvas->id);
}

void Scene::Raise (Interactor* i) {
    if (i->canvas != nil && !i->canvas->offscreen) {
	XRaiseWindow(_World_dpy, i->canvas->id);
    }
}

void Scene::Lower (Interactor* i) {
    if (i->canvas != nil && !i->canvas->offscreen) {
	XLowerWindow(_World_dpy, i->canvas->id);
    }
}

#endif

/*
 * Assign the actual location of an interactor and call its Resize operation
 * so that if it is a scene it can place its components.
 */

void Scene::Assign (register Interactor* i, Coord x, Coord y, int w, int h) {
    i->left = x;
    i->bottom = y;
    i->xmax = w - 1;
    i->ymax = h - 1;
    i->canvas->width = w;
    i->canvas->height = h;
    i->Resize();
}

void Scene::Insert (Interactor* i) {
    DoInsert(i, false, i->left, i->bottom, i->shape);
}

void Scene::Insert (register Interactor* i, Coord x, Coord y) {
    DoInsert(i, true, x, y, i->shape);
}

void Scene::Change (Interactor* i) {
    if (propagate) {
	DoChange(i, i->shape);
	if (parent != nil) {
	    parent->Change(this);
	}
    } else {
	Resize();
#       ifdef X10
	    Draw();
#       endif
    }
}

void Scene::Move (Interactor* i, Coord x, Coord y) {
    DoMove(i, x, y);
}

void Scene::Remove (Interactor* i) {
    DoRemove(i);
    i->parent = nil;
    if (i->canvas != nil) {
	Unmap(i);
	Orphan(i);
    }
}

void Scene::Orphan (register Interactor* i) {
    Interactor** a;
    int n;
    register int index;

    i->GetComponents(a, n);
    if (n > 0) {
	for (index = 0; index < n; index++) {
	    Orphan(a[index]);
	}
	delete a;
    }
    delete i->canvas;
    i->canvas = nil;
    i->prevmask = 0;
}

void Scene::DoInsert (Interactor* i, boolean, Coord&, Coord&, Shape*) {
    /* default is to ignore */
}

void Scene::DoChange (Interactor*, Shape*) {
    /* default is to ignore */
}

void Scene::DoMove (Interactor*, Coord&, Coord&) {
    /* default is to ignore */
}

void Scene::DoRemove (Interactor*) {
    /* default is to ignore */
}

void Scene::Propagate (boolean b) {
    propagate = b;
}

/*
 * A common case is a scene with a single subcomponent.  This construct
 * occurs when one interactor is defined in terms of another, e.g.,
 * a menu is built out of a frame around a box.  The reason a MonoScene
 * is preferred over subclassing is that it simplies implementing the virtuals.
 * In the menu example, menus can handle events independently of frames.
 */

MonoScene::MonoScene (Sensor* in, Painter* out) : (in, out) {
    component = nil;
}

void MonoScene::Delete () {
    delete component;
}

void MonoScene::DoInsert (Interactor* i, boolean, Coord&, Coord&, Shape* s) {
    component = i;
    DoChange(i, s);
}

void MonoScene::DoChange (Interactor*, Shape* s) {
    *shape = *s;
}

void MonoScene::Resize () {
    canvas->SetBackground(output->GetBgColor());
    Place(component, 0, 0, xmax, ymax);
}

void MonoScene::Draw () {
    Scene::Draw();
    component->Draw();
}

void MonoScene::GetComponents (Interactor**& a, int& n) {
    n = 1;
    a = new Interactor*[1];
    a[0] = component;
}
