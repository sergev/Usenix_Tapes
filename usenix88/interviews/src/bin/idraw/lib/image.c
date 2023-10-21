/*
 * Backward-compatible interface for InterViews graphics.
 */

#include "image.h"
#include "table.h"
#include <InterViews/shape.h>
#include <InterViews/canvas.h>
#include <InterViews/sensor.h>
#include <InterViews/painter.h>
#include <InterViews/cursor.h>
#include <InterViews/world.h>
#include <InterViews/Xinput.h>
#include <InterViews/Xoutput.h>

static World* world;

Brush* solidLine;
Pattern* solidPattern;
Pattern* clearPattern;
Pattern* vertPattern;
Pattern* horizPattern;
Pattern* majorDiagPattern;
Pattern* minorDiagPattern;
Pattern* thickVertPattern;
Pattern* thickHorizPattern;
Pattern* thickMajorDiagPattern;
Pattern* thickMinorDiagPattern;
Pattern* stipplePattern1;
Pattern* stipplePattern2;
Pattern* stipplePattern3;
Pattern* stipplePattern4;
Pattern* stipplePattern5;
Pattern* stipplePattern6;

Cursor* arrowCursor;
Cursor* crossHairsCursor;
Cursor* hourglassCursor;

static PatternData stdpat[] = {
    /* solid */ {
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
    },
    /* clear */ {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    },
    /* vertical */ {
	0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa,
	0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa,
	0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa,
	0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa
    },
    /* horizontal */ {
	0xffffffff, 0, 0xffffffff, 0,
	0xffffffff, 0, 0xffffffff, 0,
	0xffffffff, 0, 0xffffffff, 0,
	0xffffffff, 0, 0xffffffff, 0
    },
    /* major diagonal */ {
	0x99999999, 0xcccccccc, 0x66666666, 0x33333333,
	0x99999999, 0xcccccccc, 0x66666666, 0x33333333,
	0x99999999, 0xcccccccc, 0x66666666, 0x33333333,
	0x99999999, 0xcccccccc, 0x66666666, 0x33333333
    },
    /* minor diagonal */ {
	0x33333333, 0x66666666, 0xcccccccc, 0x99999999,
	0x33333333, 0x66666666, 0xcccccccc, 0x99999999,
	0x33333333, 0x66666666, 0xcccccccc, 0x99999999,
	0x33333333, 0x66666666, 0xcccccccc, 0x99999999
    },
    /* thick vertical */ {
	0xcccccccc, 0xcccccccc, 0xcccccccc, 0xcccccccc,
	0xcccccccc, 0xcccccccc, 0xcccccccc, 0xcccccccc,
	0xcccccccc, 0xcccccccc, 0xcccccccc, 0xcccccccc,
	0xcccccccc, 0xcccccccc, 0xcccccccc, 0xcccccccc
    },
    /* thick horizontal */ {
	0xffffffff, 0xffffffff, 0, 0,
	0xffffffff, 0xffffffff, 0, 0,
	0xffffffff, 0xffffffff, 0, 0,
	0xffffffff, 0xffffffff, 0, 0
    },
    /* thick major diagonal */ {
	0xf0f0f0f0, 0x78787878, 0x3c3c3c3c, 0x1e1e1e1e,
	0x0f0f0f0f, 0x87878787, 0xc3c3c3c3, 0xe1e1e1e1,
	0xf0f0f0f0, 0x78787878, 0x3c3c3c3c, 0x1e1e1e1e,
	0x0f0f0f0f, 0x87878787, 0xc3c3c3c3, 0xe1e1e1e1
    },
    /* thick minor diagonal */ {
	0x0f0f0f0f, 0x1e1e1e1e, 0x3c3c3c3c, 0x78787878,
	0xf0f0f0f0, 0xe1e1e1e1, 0xc3c3c3c3, 0x87878787,
	0x0f0f0f0f, 0x1e1e1e1e, 0x3c3c3c3c, 0x78787878,
	0xf0f0f0f0, 0xe1e1e1e1, 0xc3c3c3c3, 0x87878787
    },
    /* stipple pattern 1 */ {
	0x88888888, 0x22222222, 0x44444444, 0x11111111,
	0x88888888, 0x22222222, 0x44444444, 0x11111111,
	0x88888888, 0x22222222, 0x44444444, 0x11111111,
	0x88888888, 0x22222222, 0x44444444, 0x11111111
    },
    /* stipple pattern 2 */ {
	0x77777777, 0x77777777, 0x22222222, 0x22222222,
	0x77777777, 0x77777777, 0x22222222, 0x22222222,
	0x77777777, 0x77777777, 0x22222222, 0x22222222,
	0x77777777, 0x77777777, 0x22222222, 0x22222222
    },
    /* stipple pattern 3 */ {
	0x06060606, 0x07070707, 0x0e0e0e0e, 0x06060606,
	0x20202020, 0x70707070, 0xd0d0d0d0, 0x60606060,
	0x06060606, 0x07070707, 0x0e0e0e0e, 0x06060606,
	0x40404040, 0x70707070, 0xe0e0e0e0, 0x60606060
    },
    /* stipple pattern 4 */ {
	0x0f0f0f0f, 0x0f0f0f0f, 0x0f0f0f0f, 0x0f0f0f0f,
	0xd0d0d0d0, 0xf0f0f0f0, 0xf0f0f0f0, 0xf0f0f0f0,
	0x0f0f0f0f, 0x0f0f0f0f, 0x0f0f0f0f, 0x0f0f0f0f,
	0xd0d0d0d0, 0xf0f0f0f0, 0xf0f0f0f0, 0xf0f0f0f0
    },
    /* stipple pattern 5 */ {
	0xffffffff, 0x8f8f8f8f, 0x1f1f1f1f, 0xffffffff,
	0xffffffff, 0xf8f8f8f8, 0xf1f1f1f1, 0xfdfdfdfd,
	0xffffffff, 0x8f8f8f8f, 0x1f1f1f1f, 0xffffffff,
	0xffffffff, 0xf8f8f8f8, 0xf1f1f1f1, 0xfdfdfdfd
    },
    /* stipple pattern 6 */ {
	0xffffffff, 0xbfbfbfbf, 0xcfcfcfcf, 0xffffffff,
	0xffffffff, 0xfcfcfcfc, 0xf9f9f9f9, 0xffffffff,
	0xffffffff, 0xbfbfbfbf, 0xcfcfcfcf, 0xffffffff,
	0xffffffff, 0xfcfcfcfc, 0xf9f9f9f9, 0xffffffff
    }
};

static void InitPatterns () {
    solidLine = single;
    solidPattern = solid;
    clearPattern = clear;
    vertPattern = new Pattern(stdpat[2]);
    horizPattern = new Pattern(stdpat[3]);
    majorDiagPattern = new Pattern(stdpat[4]);
    minorDiagPattern = new Pattern(stdpat[5]);
    thickVertPattern = new Pattern(stdpat[6]);
    thickHorizPattern = new Pattern(stdpat[7]);
    thickMajorDiagPattern = new Pattern(stdpat[8]);
    thickMinorDiagPattern = new Pattern(stdpat[9]);
    stipplePattern1 = new Pattern(stdpat[10]);
    stipplePattern2 = new Pattern(stdpat[11]);
    stipplePattern3 = new Pattern(stdpat[12]);
    stipplePattern4 = new Pattern(stdpat[13]);
    stipplePattern5 = new Pattern(stdpat[14]);
    stipplePattern6 = new Pattern(stdpat[15]);
}

void _Image_OpenWorld () {
    world = new World;
    InitPatterns();
    arrowCursor = arrow;
    crossHairsCursor = crosshairs;
    hourglassCursor = hourglass;
}

void Image::Init () {
    if (world == nil) {
	_Image_OpenWorld();
    }
    input = new Sensor(onoffEvents);
    output = new Painter;
    cursor = defaultCursor;
    foreground = white;
    background = black;
    output->SetColors(foreground, background);
    render = PAINTFGBG;
    curmask = (1 << world->NPlanes()) - 1;
    curpattern = solid;
    curlinestyle = single;
    curfont = stdfont;
    x0 = 0; y0 = 0;
    qchar = false;
    resized = true;
}

Image::Image () : (nil, nil) {
    Init();
    world->Insert(this);
    xmax = Interactor::xmax + 1;
    ymax = Interactor::ymax + 1;
    canvas->SetBackground(background);
}

Image::Image (int width, int height) {
    Init();
    shape->Rect(width, height);
    world->Insert(this);
    xmax = Interactor::xmax + 1;
    ymax = Interactor::ymax + 1;
    canvas->SetBackground(background);
}

Image::Image (XCoord left, YCoord top, int width, int height) {
    Init();
    shape->Rect(width, height);
    world->Insert(this, left, top - height + 1);
    xmax = Interactor::xmax + 1;
    ymax = Interactor::ymax + 1;
    canvas->SetBackground(background);
}

void Image::Delete () {
    world->Remove(this);
}

void Image::Sync () {
    world->Sync();
}

void Image::Clear () {
    output->ClearRect(canvas, 0, 0, Interactor::xmax, Interactor::ymax);
}

void Image::MoveTo (XCoord x, YCoord y) {
    output->MoveTo(x, y);
    x0 = x; y0 = y;
}

void Image::GetPosition (XCoord& x, YCoord& y) {
    x = x0;
    y = y0;
}

void Image::GetLocation (XCoord& x, YCoord& y) {
    x = left;
    y = bottom;
}

void Image::MapColor (int pixel, int red, int green, int blue) {
}

void Image::SetColors (Color* fg, Color* bg) {
    if (fg != foreground || bg != background) {
	output->SetColors(fg, bg);
	foreground = fg;
	background = bg;
	canvas->SetBackground(background);
    }
}

void Image::SetForegroundColor (Color* c) {
    if (c != foreground) {
	output->SetColors(c, nil);
	foreground = c;
    }
}

void Image::SetBackgroundColor (Color* c) {
    if (c != background) {
	output->SetColors(nil, c);
	background = c;
	canvas->SetBackground(c);
    }
}

void Image::SetRendering (RenderingFunction r) {
    if (r != render) {
	switch (r) {
	    case INVERT:
		output->SetRaster(GXxor);
		break;
	    case PAINTBG:
		/* unimplemented */
		break;
	    case PAINTFG:
		output->FillBg(false);
		break;
	    case PAINTFGBG:
		output->FillBg(true);
		break;
	}
	render = r;
    }
}

void Image::SetPlaneMask (int mask) {
    if (mask != curmask) {
	/* output->SetPlaneMask(mask); */
	curmask = mask;
    }
}

void Image::Writable (XCoord x1, YCoord y1, XCoord x2, YCoord y2) {
    output->Clip(canvas, x1, y1, x2, y2);
}

void Image::AllWritable () {
    output->NoClip();
}

void Image::SetLineStyle (LineStyle* ls) {
    /* unimplemented */
}

void Image::SetPattern (Pattern* p) {
    if (curpattern != p) {
	output->SetPattern(p);
	curpattern = p;
    }
}

void Image::SetPolyAlg (PolyAlg alg) {
    /* ignore */
}

void Image::Line (XCoord x1, YCoord y1, XCoord x2, YCoord y2) {
    output->Line(canvas, x1, y1, x2, y2);
    x0 = x2; y0 = y2;
}

void Image::LineTo (XCoord x, YCoord y) {
    output->Line(canvas, x0, y0, x, y);
    x0 = x; y0 = y;
}

void Image::Point (XCoord x, YCoord y) {
    output->Point(canvas, x, y);
}

void Image::PointList (XCoord* x, YCoord* y, int count) {
    output->MultiPoint(canvas, x, y, count);
}

void Image::LineList (XCoord* x, YCoord* y, int count) {
    output->MultiLine(canvas, x, y, count);
}

void Image::Circle (XCoord x, YCoord y, int r) {
    output->Circle(canvas, x, y, r);
}

void Image::FilledCircle (XCoord x, YCoord y, int r) {
    output->FillCircle(canvas, x, y, r);
}

void Image::Ellipse (XCoord x, YCoord y, int r1, int r2) {
    output->Ellipse(canvas, x, y, r1, r2);
}

void Image::FilledEllipse (XCoord x, YCoord y, int r1, int r2) {
    output->FillEllipse(canvas, x, y, r1, r2);
}

void Image::Rectangle (XCoord x1, YCoord y1, XCoord x2, YCoord y2) {
    output->Rect(canvas, x1, y1, x2, y2);
}

void Image::FilledRectangle (XCoord x1, YCoord y1, XCoord x2, YCoord y2) {
    output->FillRect(canvas, x1, y1, x2, y2);
}

void Image::InvertArea (XCoord x1, YCoord y1, XCoord x2, YCoord y2) {
    output->InvertArea(canvas, x1, y1, x2, y2);
}

void Image::Polygon (XCoord* x, YCoord* y, int count) {
    output->FillPolygon(canvas, x, y, count);
}

void Image::BSpline (XCoord* x, YCoord* y, int count) {
    output->BSpline(canvas, x, y, count);
}

void Image::ClosedBSpline (XCoord* x, YCoord* y, int count) {
    output->ClosedBSpline(canvas, x, y, count);
}

void Image::FilledBSpline (XCoord* x, YCoord* y, int count) {
    output->FillBSpline(canvas, x, y, count);
}

void Image::BezierArc (
    XCoord x0, YCoord y0, XCoord x1, YCoord y1,
    XCoord x2, YCoord y2, XCoord x3, YCoord y3
) {
    output->Arc(canvas, x0, y0, x1, y1, x2, y2, x3, y3);
}

void Image::BezierArcTo (
    XCoord x0, YCoord y0, XCoord x1, YCoord y1, XCoord x2, YCoord y2
) {
    output->ArcTo(canvas, x0, y0, x1, y1, x2, y2);
}

void Image::CopyArea (
    XCoord x1, YCoord y1, XCoord x2, YCoord y2, XCoord dstx1, YCoord dsty1
) {
    output->Copy(canvas, x1, y1, x2, y2, canvas, dstx1, dsty1);
}

void Image::ReadArea (Pixmap* p, XCoord x1, YCoord y1, XCoord x2, YCoord y2) {
    output->Copy(canvas, x1, y1, x2, y2, p, 0, 0);
}

void Image::ReadPixmap (Pixmap* p, void* dst, int) {
    output->Read(p, dst, 0, 0, p->Width(), p->Height());
}

void Image::WriteArea (Pixmap* p, XCoord x1, YCoord y1, XCoord x2, YCoord y2) {
    output->Copy(p, 0, 0, x2 - x1, y2 - y1, canvas, x1, y1);
}

void Image::WritePixmap (Pixmap* p, void* src, int) {
    output->Write(p, src, 0, 0, p->Width() - 1, p->Height() - 1);
}

int Image::PixelsToBytes (int width, int height) {
    return (width * height * world->PixelSize() + 7) >> 3;
}

void Image::Text (const char* s) {
    output->Text(canvas, s);
}

void Image::Text (const char* s, int count) {
    output->Text(canvas, s, count);
}

void Image::SetFont (Font* f) {
    if (curfont != f) {
	output->SetFont(f);
	curfont = f;
    }
}

int Image::GetHeight () {
    return curfont->Height();
}

int Image::GetFontHeight (Font* f) {
    return f == nil ? -1 : f->Height();
}

int Image::StrWidth (const char* s, int len) {
    return curfont->Width(s, len);
}

int Image::StrWidth (const char* s) {
    return curfont->Width(s);
}

int Image::FontStrWidth (Font* f, const char* s, int len) {
    return f == nil ? -1 : f->Width(s, len);
}

int Image::FontStrWidth (Font* f, const char* s) {
    return f == nil ? -1 : f->Width(s);
}

void Image::QKeyboard () {
    input->Catch(KeyEvent);
    Listen(input);
}

void Image::UnQKeyboard () {
    input->Ignore(KeyEvent);
    Listen(input);
}

void Image::QMotion () {
    input->Catch(MotionEvent);
    Listen(input);
}

void Image::UnQMotion () {
    input->Ignore(MotionEvent);
    Listen(input);
}

void Image::QButton (InputButton b) {
    if (b < 3) {
	input->CatchButton(DownEvent, b);
	input->CatchButton(UpEvent, b);
    } else {
	input->CatchButton(KeyEvent, b);
    }
    Listen(input);
}

void Image::UnQButton (InputButton b) {
    /* ignore, can't distinguish individual buttons yet */
}

void Image::QMouseButtons () {
    input->Catch(DownEvent);
    input->Catch(UpEvent);
    Listen(input);
}

void Image::UnQMouseButtons () {
    input->Ignore(DownEvent);
    input->Ignore(UpEvent);
    Listen(input);
}

void Image::QCharacter () {
    qchar = true;
    input->Catch(KeyEvent);
    Listen(input);
}

void Image::UnQCharacter () {
    qchar = false;
    input->Ignore(KeyEvent);
    Listen(input);
}

void Image::UnQAll () {
    Listen(noEvents);
}

/*
 * Get an event for an image.  We don't use Interactor.Read because
 * we want to deliver redraw/resize as explicit events.
 */

#ifdef X10

void Image::Resize () {}
void Image::Redraw (Coord, Coord, Coord, Coord) {}

boolean Image::GetEvent (Image*& target, ImageEvent& e, boolean wait) {
    XEvent xevent;
    XMouseEvent* m = (XMouseEvent*) &xevent;
    XKeyOrButtonEvent* k = (XKeyOrButtonEvent*) &xevent;
    XExposeEvent* r = (XExposeEvent*) &xevent;
    XFocusChangeEvent* f = (XFocusChangeEvent*) &xevent;
    char* mapped;
    int len;
    Image* i;
    static XWindow focus;

    for (;;) {
	if (!wait && !XPending()) {
	    return false;
	}
	XNextEvent(xevent);
	if (!assocTable->Find(i, xevent.window) || i == (Image*)world) {
	    continue;
	}
	target = i;
	switch (xevent.type) {
	    case MouseMoved:
		e.eventType = MOTION;
		e.motion.x = m->x;
		e.motion.y = i->Interactor::ymax - m->y;
		return true;
	    case KeyPressed:
		if (i->qchar) {
		    e.eventType = CHARACTER;
		    e.character.x = k->x;
		    e.character.y = i->Interactor::ymax - k->y;
		    mapped = XLookupMapping(*k, len);
		    if (mapped == nil) {
			continue;
		    }
		    e.character.ch = mapped[0];
		} else {
		    e.eventType = BUTTON;
		    e.button.x = k->x;
		    e.button.y = i->Interactor::ymax - k->y;
		    e.button.key = k->detail & 0377;
		    e.button.up = false;
		}
		return true;
	    case ButtonPressed:
		e.eventType = BUTTON;
		e.button.x = k->x;
		e.button.y = i->Interactor::ymax - k->y;
		e.button.key = 2 - (k->detail & 0377);
		e.button.up = false;
		return true;
	    case ButtonReleased:
		e.eventType = BUTTON;
		e.button.x = k->x;
		e.button.y = i->Interactor::ymax - k->y;
		e.button.key = 2 - (k->detail & 0377);
		e.button.up = true;
		return true;
	    case FocusChange:
		if ((f->detail&0377) == EnterWindow) {
		    focus = f->window;
		    e.eventType = SELECT;
		} else {
		    focus = nil;
		    e.eventType = UNSELECT;
		}
		return true;
	    case EnterWindow:
		if (f->window == focus || (f->detail&0377) == 1) {
		    continue;
		}
		e.eventType = SELECT;
		return true;
	    case LeaveWindow:
		if (f->window == focus || (f->detail&0377) == 1) {
		    continue;
		}
		e.eventType = UNSELECT;
		return true;
	    case ExposeRegion:
		e.eventType = REDRAW;
		e.redraw.left = r->x;
		e.redraw.right = r->x + r->width - 1;
		e.redraw.top = i->Interactor::ymax - r->y;
		e.redraw.bottom = e.redraw.top - r->height + 1;
		return true;
	    case ExposeWindow:
		i->SendResize(r->width, r->height);
		i->xmax = r->width;
		i->ymax = r->height;
		e.eventType = RESIZE;
		e.resize.xmax = i->xmax;
		e.resize.ymax = i->ymax;
		return true;
	    default:
		/* ignore */;
	}
    }
}

#else

static boolean ispending;	/* redraw/resize event pending */
static ImageEvent pending;	/* event information */

void Image::Resize () {
    /* ignore initial resize from Scene */
    if (!resized) {
	ispending = true;
	xmax = canvas->width;	/* backward compatible bug */
	ymax = canvas->height;	/* ... off by one */
	pending.eventType = RESIZE;
	pending.resize.xmax = xmax;
	pending.resize.ymax = ymax;
	resized = true;
    }
}

void Image::Redraw (Coord x1, Coord y1, Coord x2, Coord y2) {
    if (resized) {
	resized = false;
    } else {
	ispending = true;
	pending.eventType = REDRAW;
	pending.redraw.left = x1;
	pending.redraw.bottom = y1;
	pending.redraw.right = x2;
	pending.redraw.top = y2;
    }
}

boolean Image::GetEvent (Image*& target, ImageEvent& e, boolean wait) {
    Event ievent;
    register Image* i;

    for (;;) {
	if (!wait && !XPending(_World_dpy)) {
	    return false;
	}
	if (Interactor::GetEvent(ievent, true)) {
	    if (ievent.target == world) {
		continue;
	    }
	    i = (Image*)(ievent.target);
	    target = i;
	    switch (ievent.eventType) {
		case MotionEvent:
		    e.eventType = MOTION;
		    e.motion.x = ievent.x;
		    e.motion.y = ievent.y;
		    return true;
		case KeyEvent:
		    if (i->qchar) {
			if (ievent.len == 0) {
			    continue;
			}
			e.eventType = CHARACTER;
			e.character.x = ievent.x;
			e.character.y = ievent.y;
			e.character.ch = ievent.keystring[0];
		    } else {
			e.eventType = BUTTON;
			e.button.x = ievent.x;
			e.button.y = ievent.y;
			e.button.key = ievent.button;
			e.button.up = false;
		    }
		    return true;
		case DownEvent:
		case UpEvent:
		    e.eventType = BUTTON;
		    e.button.x = ievent.x;
		    e.button.y = ievent.y;
		    e.button.key = ievent.button;
		    e.button.up = ievent.eventType == UpEvent;
		    return true;
		case OnEvent:
		    e.eventType = SELECT;
		    return true;
		case OffEvent:
		    e.eventType = UNSELECT;
		    return true;
		case ChannelEvent:
		case TimerEvent:
		    /* not possible */
		    break;
	    }
	} else if (ispending) {
	    e = pending;
	    ispending = false;
	    return true;
	} else {
	    FlushRedraws();
	    if (ispending) {
		e = pending;
		ispending = false;
		return true;
	    }
	}
    }
}

#endif

Image* Image::QRead (ImageEvent& e) {
    Image* t;

    GetEvent(t, e, true);
    return t;
}

boolean Image::QTest (ImageEvent& e) {
    Image* t;

    return GetEvent(t, e, false);
}

void Image::StartQRead () {
    /* nop */
}

Image* Image::FinishQRead (ImageEvent& i) {
    return QRead(i);
}

void Image::QReset () {
    /* unimplemented */
}

void Image::MapKey (
    InputButton b, char c, char shift, char ctrl, char shiftctrl
) {
    /* unimplemented */
}

void Image::UnMapKey (InputButton b) {
    /* unimplemented */
}

void Image::EnableShiftLock () {
    /* unimplemented */
}

void Image::DisableShiftLock () {
    /* unimplemented */
}

void Image::EnableMetaKey () {
    /* unimplemented */
}

void Image::DisableMetaKey () {
    /* unimplemented */
}

boolean Image::KeyToChar (InputButton b, boolean down, int& c) {
#ifdef X11
    XKeyEvent k;
    Event e;

    k.type = KeyPress;
    k.keycode = b;
    if (down) {
	k.keycode |= ShiftMask;
    }
    input->Interesting(&k, e);
    if (e.len == 0) {
	return false;
    } else {
	c = e.keystring[0];
	return true;
    }
#else
    XKeyOrButtonEvent k;
    char* s;
    int len;

    k.detail = b;
    if (down) {
	k.detail |= ShiftMask;
    }
    s = XLookupMapping(k, len);
    if (s == nil) {
	return false;
    } else {
	c = s[0];
	return true;
    }
#endif
}

void Image::SetCursor (Cursor* c) {
    cursor = c;
    Interactor::SetCursor(c);
}

void Image::CursorOn () {
    /* unimplemented */
}

void Image::CursorOff () {
    /* unimplemented */
}

void Image::SetCursorPosition (XCoord x, YCoord y) {
}

void Image::GetCursorPosition (XCoord& x, YCoord& y) {
    Event e;

    world->Poll(e);
    x = e.x;
    y = e.y;
}
