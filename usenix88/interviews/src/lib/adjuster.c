/*
 * Implementation of Adjuster and derived classes.
 */

#include <InterViews/adjuster.h>
#include <InterViews/bitmap.h>
#include <InterViews/painter.h>
#include <InterViews/perspective.h>
#include <InterViews/sensor.h>
#include <InterViews/shape.h>

Adjuster::Adjuster (Interactor* i, Bitmap* b, int d, Painter* p) : (nil, nil) {
    Init(i, d, p);
    bitmap = b;
    bitmap->Reference();
    shape->width = bitmap->Width();
    shape->height = bitmap->Height();
}

void Adjuster::Init (Interactor* i, int d, Painter* p) {
    output = new Painter(p);
    input = new Sensor(onoffEvents);
    input->Catch(UpEvent);
    input->Catch(DownEvent);

    view = i;
    highlighted = false;
    delay = d;
    shown = new Perspective;
}    

void Adjuster::Invert () {
    Color* fg, *bg;

    fg = bitmap->GetFgColor();
    bg = bitmap->GetBgColor();
    bitmap->SetColors(bg, fg);
    Draw();
}

void Adjuster::AutoRepeat () {
    Event e;
    
    Poll(e);	// initialize event
    do {
	if (Check()) {
	    Read(e);
	    if (e.target == this) {
		switch (e.eventType) {
		    case OnEvent:
			Highlight();
			break;
		    case OffEvent:
			UnHighlight();
			break;
		    default:
			break;
		}
	    }		    
	} else if (highlighted) {
	    Flash();
	    AdjustView(e);
	}
    } while (e.eventType != UpEvent);
}

void Adjuster::HandlePress () {
    Event e;
    
    do {
	Read(e);
	if (e.target == this) {
	    switch (e.eventType) {
		case OnEvent:
		    TimerOn();
		    Highlight();
		    break;
		case OffEvent:
		    TimerOff();
		    UnHighlight();
		    break;
		case UpEvent:
		    if (highlighted) {
			AdjustView(e);
		    }
		    break;
		case TimerEvent:
		    AutoRepeat();
		    return;
		default:
		    break;
	    }
	}
    } while (e.eventType != UpEvent);
}

void Adjuster::Flash () {
    UnHighlight();
    Highlight();
}

static const int USEC_PER_DELAY_UNIT = 100000;	    // delay unit = 1/10 secs

void Adjuster::TimerOn () {
    if (delay >= 0) {
	input->CatchTimer(0, delay * USEC_PER_DELAY_UNIT);
    }
}

void Adjuster::TimerOff () {
    input->Ignore(TimerEvent);
}

void Adjuster::Delete () {
    delete bitmap;
    delete shown;
    Interactor::Delete();
}

void Adjuster::Handle (Event& e) {
    if (e.eventType == DownEvent) {
	Highlight();
	TimerOn();
	if (delay == 0) {
	    AutoRepeat();
	} else {
	    HandlePress();
	}
	UnHighlight();
	TimerOff();
    }
}

void Adjuster::Draw () {
    bitmap->Draw(canvas);
}

void Adjuster::Redraw (Coord l, Coord b, Coord r, Coord t) {
    output->Clip(canvas, l, b, r, t);
    bitmap->Draw(canvas);
    output->NoClip();
}

void Adjuster::Resize () {
    Coord ox, oy;
    
    ox = (xmax + 1 - shape->width) / 2;
    oy = (ymax + 1 - shape->height) / 2;
    bitmap->SetOrigin(ox, oy);
}

void Adjuster::Reshape (Shape& s) {
    shape->Rigid(s.hshrink, s.hstretch, s.vshrink, s.vstretch);
}

void Adjuster::Highlight () {
    if (!highlighted) {
	Invert();
	highlighted = true;
    }
}

void Adjuster::UnHighlight () {
    if (highlighted) {
	Invert();
	highlighted = false;
    }
}

void Adjuster::AdjustView (Event&) {
    // nop default
}

Zoomer::Zoomer (
    Interactor* i, Bitmap* b, float f, Painter* p
) : (i, b, NO_AUTOREPEAT, p) {
    factor = f;
}

void Zoomer::AdjustView (Event&) {
    register Perspective* s = shown;
    Coord cx, cy;

    *s = *view->GetPerspective();
    cx = s->curx + s->curwidth/2;
    cy = s->cury + s->curheight/2;
    s->curwidth = round(float(s->curwidth) / factor);
    s->curheight = round(float(s->curheight) / factor);
    s->curx = cx - s->curwidth/2;
    s->cury = cy - s->curheight/2;
    view->Adjust(*s);    
}

static const int enlw = 25;
static const int enlh = 15;
static short enl[] = {
   0x0000, 0x0000, 0x0000, 0x0000,
   0x1000, 0x0000, 0x6c00, 0x0000,
   0x8300, 0x0001, 0x00c0, 0x0006,
   0x0030, 0x0018, 0x000c, 0x0060,
   0x00fc, 0x007e, 0x0044, 0x0044,
   0x003c, 0x0078, 0xffe0, 0x000f,
   0x0020, 0x0008, 0xffe0, 0x000f,
   0x0000, 0x0000
};

Enlarger::Enlarger (
    Interactor* i, Painter* p
) : (i, new Bitmap(enl, enlw, enlh), 2, p) {
    // nothing to do
}

static const int redw = 25;
static const int redh = 15;
static short red[] = {
   0x0000, 0x0000, 0x0000, 0x0000,
   0xfe00, 0x0000, 0x0100, 0x0001,
   0x0080, 0x0002, 0x007c, 0x007c,
   0x001c, 0x0070, 0x0064, 0x004c,
   0x0184, 0x0043, 0xc618, 0x0030,
   0x3860, 0x000c, 0x1180, 0x0003,
   0xd600, 0x0000, 0x3800, 0x0000,
   0x0000, 0x0000
};

Reducer::Reducer (
    Interactor* i, Painter* p
) : (i, new Bitmap(red, redw, redh), 0.5, p) {
    // nothing to do
}

static typedef enum MoveType { 
    MOVE_LEFT, MOVE_RIGHT, MOVE_UP, MOVE_DOWN, MOVE_UNDEF
};

Mover::Mover (
    Interactor* i, Bitmap* b, int delay, int mt, Painter* p
) : (i, b, delay, p) {
    moveType = mt;
}

void Mover::AdjustView (Event& e) {
    register Perspective* s = shown;
    int amtx, amty;

    *s = *view->GetPerspective();
    amtx = e.shift ? s->sx : s->lx;
    amty = e.shift ? s->sy : s->ly;

    switch (moveType) {
	case MOVE_LEFT:	    s->curx -= amtx; break;
	case MOVE_RIGHT:    s->curx += amtx; break;
	case MOVE_UP:	    s->cury += amty; break;
	case MOVE_DOWN:	    s->cury -= amty; break;
	default:	    break;
    }
    view->Adjust(*s);    
}

static const int lmoverw = 11;
static const int lmoverh = 11;
static short lmover[] = {
   0x0000, 0x0060, 0x0070, 0x03f8,
   0x03fc, 0x03fe, 0x03fc, 0x03f8,
   0x0070, 0x0060, 0x0000
};

LeftMover::LeftMover (Interactor* i, int delay, Painter* p) : (
    i, new Bitmap(lmover, lmoverw, lmoverh), delay, MOVE_LEFT, p
) {
    // nothing to do
};

static const int rmoverw = 11;
static const int rmoverh = 11;
static short rmover[] = {
   0x0000, 0x0030, 0x0070, 0x00fe,
   0x01fe, 0x03fe, 0x01fe, 0x00fe,
   0x0070, 0x0030, 0x0000
};

RightMover::RightMover (Interactor* i, int delay, Painter* p) : (
    i, new Bitmap(rmover, rmoverw, rmoverh), delay, MOVE_RIGHT, p
) {
    // nothing to do
};

static const int umoverw = 11;
static const int umoverh = 11;
static short umover[] = {
   0x0000, 0x0020, 0x0070, 0x00f8,
   0x01fc, 0x03fe, 0x03fe, 0x00f8,
   0x00f8, 0x00f8, 0x0000
};

UpMover::UpMover (Interactor* i, int delay, Painter* p) : (
    i, new Bitmap(umover, umoverw, umoverh), delay, MOVE_UP, p
) {
    // nothing to do
};

static const int dmoverw = 11;
static const int dmoverh = 11;
static short dmover[] = {
   0x0000, 0x00f8, 0x00f8, 0x00f8,
   0x03fe, 0x03fe, 0x01fc, 0x00f8,
   0x0070, 0x0020, 0x0000
};

DownMover::DownMover (Interactor* i, int delay, Painter* p) : (
    i, new Bitmap(dmover, dmoverw, dmoverh), delay, MOVE_DOWN, p
) {
    // nothing to do
};
