/*
 * Scrolling implementation.
 */

#include <InterViews/scroller.h>
#include <InterViews/shape.h>
#include <InterViews/perspective.h>
#include <InterViews/sensor.h>
#include <InterViews/painter.h>
#include <InterViews/cursor.h>
#include <InterViews/rubrect.h>

const int inset = 2;	/* space between scroller canvas and bar */

static Cursor* hcursor;
static Cursor* vcursor;

static CursorPattern vertPat = {
    0x0100, 0x0380, 0x07c0, 0x0fe0, 0x0280, 0x0280, 0x0280, 0x3ff8,
    0x0280, 0x0280, 0x0280, 0x0fe0, 0x07c0, 0x0380, 0x0100, 0x0
};

static CursorPattern vertMask = {
    0x0380, 0x07c0, 0x0fe0, 0x1ff0, 0x1ff0, 0x07c0, 0x3ff8, 0x3ff8,
    0x07c0, 0x07c0, 0x1ff0, 0x1ff0, 0x0fe0, 0x07c0, 0x0380, 0x0
};

static CursorPattern horizPat = {
    0x0, 0x0, 0x100, 0x100, 0x1110, 0x3118, 0x7ffc, 0xf11e,
    0x7ffc, 0x3118, 0x1110, 0x100, 0x100, 0x0, 0x0, 0x0
};

static CursorPattern horizMask = {
    0x0, 0x0, 0x300, 0x1310, 0x3b38, 0x7ffc, 0xfffe, 0xffff,
    0xfffe, 0x7ffc, 0x3b38, 0x1310, 0x300, 0x0, 0x0, 0x0
};

Scroller::Scroller (Interactor* i, Sensor* in, Painter* out) : (in, out) {
    interactor = i;
    view = i->GetPerspective();
    view->Attach(this);
    shown = new Perspective;
    if (input == nil) {
	input = new Sensor;
	input->Catch(DownEvent);
	input->Catch(UpEvent);
    }
    output = new Painter(out);
    output->SetPattern(lightgray);
    delete out;
    tracking = new Sensor;
    tracking->Catch(UpEvent);
    tracking->Catch(MotionEvent);
    shape->Rigid();
}

void Scroller::Delete () {
    delete shown;
    delete tracking;
}

void Scroller::Resize () {
    *shown = *view;
}

inline void Scroller::Background (Coord x1, Coord y1, Coord x2, Coord y2) {
    output->FillRect(canvas, x1, y1, x2, y2);
}

HScroller::HScroller (
    Interactor* i, int size, Sensor* in, Painter* out
) : (i, in, out) {
    if (size == 0) {
	shape->height = round(0.20*inch);
	shape->vstretch = shape->height;
    } else {
	shape->height = size;
    }
    shape->width = round(1*inch);
    shape->hstretch = hfil;
    shape->hshrink = shape->width;
    if (hcursor == nil) {
	hcursor = new Cursor(7, 8, horizPat, horizMask, black, white);
    }
    cursor = hcursor;
}

VScroller::VScroller (
    Interactor* i, int size, Sensor* in, Painter* out
) : (i, in, out) {
    if (size == 0) {
	shape->width = round(0.20*inch);
	shape->hstretch = shape->width;
    } else {
	shape->width = size;
    }
    shape->height = round(1*inch);
    shape->vstretch = vfil;
    shape->vshrink = shape->height;
    if (vcursor == nil) {
	vcursor = new Cursor(7, 8, vertPat, vertMask, black, white);
    }
    cursor = vcursor;
}

void HScroller::GetBarInfo (register Perspective* s, Coord& left, int& width) {
    if (s->width == 0) {
	scale = 0.0;
    } else {
	scale = double(xmax + 1) / double(s->width);
    }
    left = round(double(s->curx - s->x0) * scale);
    width = max(round(double(s->curwidth) * scale), 10);
}

void VScroller::GetBarInfo (register Perspective* s, Coord& top, int& height) {
    if (s->height == 0) {
	scale = 0.0;
    } else {
	scale = double(ymax + 1) / double(s->height);
    }
    top = round(double(s->cury - s->y0) * scale);
    height = max(round(double(s->curheight) * scale), 10);
}

inline void HScroller::Bar (Coord x, int width) {
    output->ClearRect(canvas, x, inset+1, x+width-1, ymax-inset-1);
}

inline void VScroller::Bar (Coord y, int height) {
    output->ClearRect(canvas, inset+1, y+height-1, xmax-inset-1, y);
}

inline void HScroller::Outline (Coord x, int width) {
    output->Rect(canvas, x, inset, x+width-1, ymax-inset);
}

inline void VScroller::Outline (Coord y, int height) {
    output->Rect(canvas, inset, y+height-1, xmax-inset, y);
}

inline void HScroller::Border (Coord x) {
    output->Line(canvas, x, inset, x, ymax-inset);
}

inline void VScroller::Border (Coord y) {
    output->Line(canvas, inset, y, xmax-inset, y);
}

inline void HScroller::Sides (Coord x1, Coord x2) {
    output->Line(canvas, x1, inset, x2, inset);
    output->Line(canvas, x1, ymax-inset, x2, ymax-inset);
}

inline void VScroller::Sides (Coord y1, Coord y2) {
    output->Line(canvas, inset, y1, inset, y2);
    output->Line(canvas, xmax-inset, y1, xmax-inset, y2);
}

void HScroller::Redraw (Coord x1, Coord y1, Coord x2, Coord y2) {
    Coord left;
    int width;

    Background(x1, y1, x2, y2);
    GetBarInfo(shown, left, width);
    Bar(left, width);
    Outline(left, width);
}

void VScroller::Redraw (Coord x1, Coord y1, Coord x2, Coord y2) {
    Coord top;
    int height;

    Background(x1, y1, x2, y2);
    GetBarInfo(shown, top, height);
    Bar(top, height);
    Outline(top, height);
}

void HScroller::Handle (Event& e) {
    register Perspective* s = shown;
    register Coord nx;
    Coord lower, upper;
    int dx;

    if (e.eventType == DownEvent) {
	*shown = *view;
	nx = s->curx;
	dx = e.shift ? s->sx : s->lx;
	lower = s->x0;
	upper = lower + s->width - s->curwidth;
	switch (e.button) {
	    case 0: /* shift left */
		nx = max(nx - dx, lower);
		break;
	    case 1:
		nx = Slide(e);
		if (nx < lower) {
		    nx = lower;
		} else if (nx > upper) {
		    nx = upper;
		}
		break;
	    case 2: /* shift right */
		nx = min(nx + dx, upper);
		break;
	}
	if (s->curx != nx) {
	    s->curx = nx;
	    interactor->Adjust(*s);
	    Redraw(0, 0, xmax, ymax);
	}
    }
}

void VScroller::Handle (Event& e) {
    register Perspective* s = shown;
    register Coord ny;
    Coord lower, upper;
    int dy;

    if (e.eventType == DownEvent) {
	*shown = *view;
	ny = s->cury;
	dy = e.shift ? s->sy : s->ly;
	lower = s->y0;
	upper = lower + s->height - s->curheight;
	switch (e.button) {
	    case 0: /* scroll up */
		ny = max(ny - dy, lower);
		break;
	    case 1:
		ny = Slide(e);
		if (ny < lower) {
		    ny = lower;
		} else if (ny > upper) {
		    ny = upper;
		}
		break;
	    case 2: /* shift down */
		ny = min(ny + dy, upper);
		break;
	}
	if (s->cury != ny) {
	    s->cury = ny;
	    interactor->Adjust(*s);
	    Redraw(0, 0, xmax, ymax);
	}
    }
}

Coord HScroller::Slide (register Event& e) {
    Coord x1, y1, x2, y2;
    int width, w;

    GetBarInfo(shown, x1, width);
    if (e.x < x1 || e.x > x1+width) {
	x1 = max(0, min(e.x-width/2, xmax-width));
    }
    x2 = x1 + width - 1;
    w = e.x - x1;
    SetCursor(noCursor);
    Listen(tracking);
    SlidingRect r(output, canvas, x1+1, inset+1, x2-1, ymax-inset-1, e.x, 0);
    r.Draw();
    for (;;) {
	Read(e);
	if (e.eventType == UpEvent) {
	    break;
	} else if (e.eventType == MotionEvent) {
	    r.Track(max(w, min(e.x, xmax-width+w+1)), 0);
	}
    }
    SetCursor(hcursor);
    Listen(input);
    r.GetCurrent(x1, y1, x2, y2);
    return shown->x0 + round(double(x1-1) / scale);
}

Coord VScroller::Slide (register Event& e) {
    Coord x1, y1, x2, y2;
    int height, h;

    GetBarInfo(shown, y1, height);
    if (e.y < y1 || e.y > y1+height) {
	y1 = max(0, min(e.y-height/2, ymax-height));
    }
    y2 = y1 + height - 1;
    h = e.y - y1;
    SetCursor(noCursor);
    Listen(tracking);
    SlidingRect r(output, canvas, inset+1, y1+1, xmax-inset-1, y2-1, 0, e.y );
    r.Draw();
    for (;;) {
	Read(e);
	if (e.eventType == UpEvent) {
	    break;
	} else if (e.eventType == MotionEvent) {
	    r.Track(0, max(h, min(e.y, ymax-height+h+1)));
	}
    }
    SetCursor(vcursor);
    Listen(input);
    r.GetCurrent(x1, y1, x2, y2);
    return shown->y0 + round(double(y1-1) / scale);
}

void HScroller::Update () {
    Coord oldleft, oldright, newleft, newright;
    int oldwidth, newwidth;
    Perspective* p;

    if (canvas == nil) {
	return;
    }
    p = view;
    GetBarInfo(shown, oldleft, oldwidth);
    GetBarInfo(p, newleft, newwidth);
    if (oldleft != newleft || oldwidth != newwidth) {
	oldright = oldleft+oldwidth-1;
	newright = newleft+newwidth-1;
	if (oldright >= newleft && newright >= oldleft) {
	    if (oldright > newright) {
		Background(newright+1, inset, oldright, ymax-inset);
		Border(newright);
	    } else if (oldright < newright) {
		Bar(oldright, newright-oldright);
		Sides(oldright, newright);
		Border(newright);
	    }
	    if (oldleft > newleft) {
		Bar(newleft+1, oldleft-newleft);
		Sides(newleft, oldleft);
		Border(newleft);
	    } else if (oldleft < newleft) {
		Background(oldleft, inset, newleft-1, ymax-inset);
		Border(newleft);
	    }
	} else {
	    Background(oldleft, inset, oldright, ymax-inset);
	    Bar(newleft, newwidth);
	    Outline(newleft, newwidth);
	}
    }
    *shown = *p;
}

void VScroller::Update () {
    Coord oldbottom, oldtop, newbottom, newtop;
    int oldheight, newheight;
    Perspective* p;

    if (canvas == nil) {
	return;
    }
    p = view;
    GetBarInfo(shown, oldbottom, oldheight);
    GetBarInfo(p, newbottom, newheight);
    if (oldbottom != newbottom || oldheight != newheight) {
	oldtop = oldbottom+oldheight-1;
	newtop = newbottom+newheight-1;
	if (oldtop >= newbottom && newtop >= oldbottom) {
	    if (oldtop > newtop) {
		Background(inset, newtop+1, xmax-inset, oldtop);
		Border(newtop);
	    } else if (oldtop < newtop) {
		Bar(oldtop, newtop-oldtop);
		Sides(oldtop, newtop);
		Border(newtop);
	    }
	    if (oldbottom > newbottom) {
		Bar(newbottom+1, oldbottom-newbottom);
		Sides(newbottom, oldbottom);
		Border(newbottom);
	    } else if (oldbottom < newbottom) {
		Background(inset, oldbottom, xmax-inset, newbottom-1);
		Border(newbottom);
	    }
	} else {
	    Background(inset, oldbottom, xmax-inset, oldtop);
	    Bar(newbottom, newheight);
	    Outline(newbottom, newheight);
	}
    }
    *shown = *p;
}
