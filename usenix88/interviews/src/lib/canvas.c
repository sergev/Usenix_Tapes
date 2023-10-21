/*
 * A canvas is implemented as either an X window or pixmap.
 */

#include <InterViews/canvas.h>
#include <InterViews/interactor.h>
#include <InterViews/paint.h>
#include <InterViews/Xwindow.h>
#include <InterViews/Xinput.h>
#include <InterViews/Xoutput.h>
#include "table.h"

#ifdef X10

Canvas::Canvas (void* c) {
    id = c;
    width = 0;
    height = 0;
    offscreen = false;
    left = 0;
    bottom = 0;
    unclipped = nil;
}

Canvas::Canvas (int w, int h) {
    id = nil;
    left = 0;
    bottom = 0;
    unclipped = nil;
    width = w;
    height = h;
    offscreen = true;
}

Canvas::~Canvas () {
    if (id != nil) {
	if (offscreen) {
	    XFreePixmap(id);
	} else {
	    XDestroyWindow(id);
	    assocTable->Remove(id);
	}
	id = nil;
    }
    if (unclipped != nil) {
	delete unclipped;
    }
}

void Canvas::WaitForCopy () {
    XEvent xe;
    register XExposeEvent* r = (XExposeEvent*)&xe;
    Interactor* i;

    for (;;) {
	XWindowEvent(id, ExposeCopy|ExposeRegion, xe);
	if (xe.type == ExposeCopy) {
	    break;
	}
	/* must be ExposeRegion */
	if (assocTable->Find(i, r->window)) {
	    i->Redraw(
		r->x, height - r->y - r->height,
		r->x + r->width - 1, height - 1 - r->y
	    );
	}
    }
}

void Canvas::SetBackground (Color* c) {
    XPixmap p;
    void* id;
    
    id = (unclipped == nil) ? this->id : unclipped->id;
    if (id != nil && !offscreen) {
	if (c == white) {
	    XChangeBackground(id, XWhitePixmap);
	} else if (c == black) {
	    XChangeBackground(id, XBlackPixmap);
	} else {
	    p = XMakeTile(c->PixelValue());
	    XChangeBackground(id, p);
	    XFreePixmap(p);
	}
    }
}

void Canvas::Clip (Coord x1, Coord y1, Coord x2, Coord y2) {
    Coord left, bottom, right, top;
    int w, h;

    if (x1 <= x2) {
	left = x1; right = x2;
    } else {
	left = x2; right = x1;
    }
    if (y1 <= y2) {
	bottom = y1; top = y2;
    } else {
	bottom = y2; top = y1;
    }
    w = right - left + 1;
    h = top - bottom + 1;
    if (unclipped != nil) {
	top = unclipped->height - 1 - top;
	XConfigureWindow(id, x1, top, w, h);
    } else {
	top = height - 1 - top;
	unclipped = new Canvas(XCreateTransparency(id, x1, top, w, h));
	Exchange(unclipped);
	XSelectInput(id, EnterWindow|LeaveWindow);
	XMapWindow(id);
    }
    this->left = x1;
    this->bottom = y1;
    width = w;
    height = h;
}

void Canvas::NoClip () {
    if (unclipped != nil) {
	Exchange(unclipped);
	delete unclipped;
	unclipped = nil;
    }
}

/*
 * Exchange clipping-related information between this and another canvas.
 */

void Canvas::Exchange (Canvas* c) {
    void* tv;
    Coord tc;
    int ti;
    
    tv = c->id; c->id = id; id = tv;
    tc = c->left; c->left = left; left = tc;
    tc = c->bottom; c->bottom = bottom; bottom = tc;
    ti = c->width; c->width = width; width = ti;
    ti = c->height; c->height = height; height = ti;
}

#endif

#ifdef X11

Canvas::Canvas (void* c) {
    id = c;
    width = 0;
    height = 0;
    offscreen = false;
}

Canvas::Canvas (int w, int h) {
    id = XCreatePixmap(
	_World_dpy, _World_root, w, h, XDefaultDepth(_World_dpy, _World_screen)
    );
    width = w;
    height = h;
    offscreen = true;
}

Canvas::~Canvas () {
    if (id != nil) {
	if (offscreen) {
	    XFreePixmap(_World_dpy, id);
	} else {
	    XDestroyWindow(_World_dpy, id);
	    assocTable->Remove(id);
	}
	id = nil;
    }
}

void Canvas::WaitForCopy () {
    XEvent xe;
    register int t;
    register XExposeEvent* r = &xe.xexpose;
    register XGraphicsExposeEvent* g = &xe.xgraphicsexpose;
    Interactor* i;

    for (;;) {
	XWindowEvent(_World_dpy, id, ExposureMask, xe);
	t = xe.type;
	if (t == NoExpose) {
	    break;
	}
	if (t == Expose && assocTable->Find(i, r->window)) {
	    i->Redraw(
		r->x, height - r->y - r->height,
		r->x + r->width - 1, height - 1 - r->y
	    );
	} else if (t == GraphicsExpose && assocTable->Find(i, g->drawable)) {
	    i->Redraw(
		g->x, height - g->y - g->height,
		g->x + g->width - 1, height - 1 - g->y
	    );
	    if (g->count == 0) {
		break;
	    }
	}
    }
}

/*
 * Associate a background color with a canvas.
 */

void Canvas::SetBackground (Color* c) {
    if (!offscreen) {
	XSetWindowBackground(_World_dpy, id, c->PixelValue());
    }
}

#endif
