/*
 * Panner implementation.
 */

#include <InterViews/adjuster.h>
#include <InterViews/border.h>
#include <InterViews/glue.h>
#include <InterViews/painter.h>
#include <InterViews/panner.h>
#include <InterViews/perspective.h>
#include <InterViews/rubrect.h>
#include <InterViews/sensor.h>
#include <InterViews/shape.h>

extern int abs (int);

Panner::Panner (Interactor* i, int size, Painter* p) {
    HBox* adjusters;
    VBox* zoomers;
    VBox* movers;
    Shape adjs;

    BuildAdjusters(i, p);
    
    movers = new VBox(
	uMover, new HBox(lMover, new HGlue(stdpaint), rMover), dMover
    );
    zoomers = new VBox(enlarger, reducer);
    adjusters = new HBox(movers, zoomers);
    adjusters->shape->vstretch = 0;
    adjusters->shape->vshrink = adjusters->shape->height/3;
    
    slider = new Slider(i, p);
    adjs = *adjusters->shape;
    slider->Reshape(adjs);
    Insert(adjusters);
    Insert(new HBorder(stdpaint));
    Insert(slider);
    if (size != 0) {
	shape->width = size;
    }
}

void Panner::Delete () {
    delete enlarger;
    delete reducer;
    delete lMover;
    delete rMover;
    delete uMover;
    delete dMover;
    delete slider;
    VBox::Delete();
}

static int DELAY = 3;	    // 0.3 second delay

void Panner::BuildAdjusters (Interactor* i, Painter* p) {
    enlarger = new Enlarger(i, p);
    reducer = new Reducer(i, p);

    lMover = new LeftMover(i, DELAY, p);
    rMover = new RightMover(i, DELAY, p);
    uMover = new UpMover(i, DELAY, p);
    dMover = new DownMover(i, DELAY, p);
}

static const int MIN_SLIDER_HT = 20;
static typedef enum MoveType { MOVE_HORIZ, MOVE_VERT, MOVE_UNDEF };

Slider::Slider (Interactor* i, Painter* p) : (nil, nil) {
    input = new Sensor(updownEvents);
    output = new Painter(p);
    view = i;
    view->GetPerspective()->Attach(this);
    shown = new Perspective;
    constrained = false;
    moveType = MOVE_UNDEF;
    *shown = *view->GetPerspective();
    shape->vstretch = shape->vshrink = 0;
}

void Slider::Reshape (Shape& ns) {
    register Perspective* s = shown;
    Scene* parent = Parent();
    float aspect;
    int height;
    
    shape->width = ns.width;
    aspect = float(s->height) / float(s->width);
    height = round(aspect * float(shape->width));
    if (shape->height != height) {
	shape->height = height;
	if (parent != nil) {
	    parent->Change(this);
	}
    }
    ns = *shape;
}

void Slider::Draw () {
    if (canvas != nil) {
	output->SetPattern(lightgray);
	output->FillRect(canvas, 0, 0, xmax, ymax);
	output->SetPattern(clear);
	output->FillRect(canvas, left, bottom, right, top);
	output->SetPattern(solid);
	output->Rect(canvas, left, bottom, right, top);
	output->Line(canvas, left+1, bottom-1, right+1, bottom-1);
	output->Line(canvas, right+1, bottom-1, right+1, top-1);
    }
}

void Slider::Redraw (Coord left, Coord bottom, Coord right, Coord top) {
    output->Clip(canvas, left, bottom, right, top);
    Draw();
    output->NoClip();
}

inline Coord Slider::ViewX (Coord x) {
    return round(float(x) * float(shown->width) / float(xmax));
}

inline Coord Slider::ViewY (Coord y) {
    return round(float(y) * float(shown->height) / float(ymax));
}

inline Coord Slider::SliderX (Coord x) {
    return round(float(x) * float(xmax) / float(shown->width));
}

inline Coord Slider::SliderY (Coord y) {
    return round(float(y) * float(ymax) / float(shown->height));
}

void Slider::Move (Coord dx, Coord dy) {
    shown->curx += dx;
    shown->cury += dy;
}

boolean Slider::Inside (Event& e) {
    return e.x > left && e.x < right && e.y > bottom && e.y < top;
}

void Slider::CalcLimits (Event& e) {
    llim = e.x - left;
    blim = e.y - bottom;
    rlim = e.x + xmax - right;
    tlim = e.y + ymax - top;
    constrained = e.shift;
    moveType = MOVE_UNDEF;
    origx = e.x;
    origy = e.y;
}

static int CONSTRAIN_THRESH = 2;    
    // difference between x and y movement needed to decide which direction
    // is constrained

void Slider::Constrain (Event& e) {
    Coord dx, dy;

    if (constrained && moveType == MOVE_UNDEF) {
	dx = abs(e.x - origx);
	dy = abs(e.y - origy);
	if (abs(dx - dy) < CONSTRAIN_THRESH) {
	    e.x = origx;
	    e.y = origy;
	} else if (dx > dy) {
	    moveType = MOVE_HORIZ;
	} else {
	    moveType = MOVE_VERT;
	}
    }

    if (!constrained) {
	e.x = min(max(e.x, llim), rlim);
	e.y = min(max(e.y, blim), tlim);

    } else if (moveType == MOVE_HORIZ) {
	e.x = min(max(e.x, llim), rlim);
	e.y = origy;

    } else if (moveType == MOVE_VERT) {
	e.x = origx;
	e.y = min(max(e.y, blim), tlim);

    }
}

void Slider::Slide (Event& e) {
    Coord newleft, newbot, dummy;

    Listen(allEvents);
    SlidingRect r(output, canvas, left, bottom, right, top, e.x, e.y);
    CalcLimits(e);
    do {
	switch (e.eventType) {
	    case MotionEvent:
		Constrain(e);
		r.Track(e.x, e.y);
		break;
	    default:
		break;
	}
	Read(e);
    } while (e.eventType != UpEvent);

    r.GetCurrent(newleft, newbot, dummy, dummy);
    Move(ViewX(newleft - left), ViewY(newbot - bottom));
    Listen(input);
}

void Slider::Jump (Event& e) {
    Coord dx, dy;
    
    if (e.button == LEFTMOUSE) {
	dx = shown->sx;
	dy = shown->sy;
    } else if (e.button == MIDDLEMOUSE) {
	dx = shown->lx;
	dy = shown->ly;
    } else {
	dx = shown->width;
	dy = shown->height;
    }

    if (e.x < left) {
	dx = -dx;
    } else if (e.x < right) {
	dx = 0;
    }
    if (e.y < bottom) {
	dy = -dy;
    } else if (e.y < top) {
	dy = 0;
    }
    Move(dx, dy);
}	

void Slider::Handle (Event& e) {
    if (e.eventType == DownEvent) {
	*shown = *view->GetPerspective();
	if (Inside(e)) {
	    Slide(e);
	} else {
	    Jump(e);
	}
	view->Adjust(*shown);
    }
}
    
static const int MIN_SIZE = 8;

void Slider::SizeKnob () {
    register Perspective* s;
    int sleft, sbottom, swidth, sheight;
    
    s = shown;
    *s = *view->GetPerspective();
    sleft = min(SliderX(s->curx - s->x0), xmax-MIN_SIZE);
    sbottom = min(SliderY(s->cury - s->y0), ymax-MIN_SIZE);
    swidth = max(SliderX(s->curwidth), MIN_SIZE);
    sheight = max(SliderY(s->curheight), MIN_SIZE);

    left = max(0, sleft);
    bottom = max(0, sbottom);
    right = min(xmax, sleft+swidth);
    top = min(ymax, sbottom+sheight);
}    

void Slider::Update () {
    Coord oldxmax, l, b, r, t;
    Shape s;

    if (canvas != nil) {
	oldxmax = xmax; 
	l = left; b = bottom; r = right; t = top;
	SizeKnob();
	s.width = xmax + 1;
	Reshape(s);
	if (
	    oldxmax != xmax ||
	    l != left || b != bottom || r != right || t != top
	) {
	    Draw();
	}
    }
}

void Slider::Resize () {
    SizeKnob();
    shape->hstretch = 0;
}

void Slider::Delete () {
    delete shown;
    Interactor::Delete();
}
