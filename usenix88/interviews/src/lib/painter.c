/*
 * Graphics implementation on top of X
 */

#include "graphics.h"

extern void bzero(void*, int);
extern void bcopy(const void*, void*, int);

Color* Painter::GetFgColor () {
    return rep->foreground;
}

Color* Painter::GetBgColor () {
    return rep->background;
}

Pattern* Painter::GetPattern () {
    return rep->pattern;
}

Brush* Painter::GetBrush () {
    return rep->br;
}

Font* Painter::GetFont () {
    return rep->font;
}

void Painter::SetTransformer (Transformer *t) {
    if (rep->matrix != t) {
	delete rep->matrix;
	rep->matrix = t;
	if (t != nil) {
	    t->Reference();
	}
    }
}

Transformer* Painter::GetTransformer () {
    return rep->matrix;
}

void Painter::MoveTo (int x, int y) {
    register Graphics* p = rep;

    p->curx = x;
    p->cury = y;
}

void Painter::GetPosition (int& x, int& y) {
    register Graphics* p = rep;

    x = p->curx;
    y = p->cury;
}

void Painter::SetOrigin (int x0, int y0) {
    rep->xoff = x0;
    rep->yoff = y0;
}

void Painter::GetOrigin (int& x0, int& y0) {
    x0 = rep->xoff;
    y0 = rep->yoff;
}

void Painter::Translate (float dx, float dy) {
    if (dx != 0.0 || dy != 0.0) {
	if (rep->matrix == nil) {
	    rep->matrix = new Transformer;
	}
	rep->matrix->Translate(dx, dy);
    }
}

void Painter::Scale (float sx, float sy) {
    if (sx != 1.0 || sy != 1.0) {
	if (rep->matrix == nil) {
	    rep->matrix = new Transformer;
	}
	rep->matrix->Scale(sx, sy);
    }
}

void Painter::Rotate (float angle) {
    if (angle - int(angle) != 0.0 || int(angle) % 360 != 0) {
	if (rep->matrix == nil) {
	    rep->matrix = new Transformer;
	}
	rep->matrix->Rotate(angle);
    }
}

#ifdef X10

Painter::Painter () {
    register Graphics* p;

    p = new Graphics;
    bzero(p, sizeof(*p));
    rep = p;
    p->foreground = black;
    p->background = white;
    p->fg = black->pixel;
    p->bg = white->pixel;
    p->raster = GXcopy;
    p->pattern = solid;
    p->pat = solid->info;
    p->tile = XMakeTile(p->fg);
    p->fillbg = true;
    p->stipple = nil;
    SetBrush(single);
    SetFont(stdfont);
}

Painter::Painter (Painter* copy) {
    register Graphics* p = rep;

    p = new Graphics;
    *p = *copy->rep;
    p->tile = XMakePixmap(p->pat, p->fg, p->bg);
    p->stipple = nil;
    rep = p;
}

Painter::~Painter () {
    if (LastRef()) {
	rep->FreeNoBgPixmaps();
	delete rep->matrix;
	delete rep;
    }
}

void Painter::FillBg (boolean b) {
    register Graphics* p = rep;

    if (p->raster != GXcopy) {
	p->fillbg = b;
	p->raster = GXcopy;
	p->Update();
    } else if (b != p->fillbg) {
	p->fillbg = b;
	if (!b && p->stipple == nil) {
	    p->MakeNoBgPixmaps();
	}
    }
}

boolean Painter::BgFilled () {
    return rep->fillbg;
}

void Painter::SetColors (Color* f, Color* b) {
    register Graphics *p = rep;
    boolean changed;

    changed = false;
    if (f != nil && p->foreground != f) {
	p->foreground = f;
	changed = true;
    }
    if (b != nil && p->background != b) {
	p->background = b;
	changed = true;
    }
    if (changed) {
	p->FreeNoBgPixmaps();
	p->Update();
    }
}

void Painter::SetPattern (Pattern* pat) {
    register Graphics *p = rep;

    if (p->pattern != pat) {
	p->pattern = pat;
	p->pat = pat->info;
	p->FreeNoBgPixmaps();
	p->Update();
    }
}

void Painter::SetBrush (Brush* b) {
    rep->br = b;
}

void Painter::SetFont (Font* f) {
    rep->font = f;
}

void Painter::Clip (
    Canvas* c, Coord left, Coord bottom, Coord right, Coord top
) {
    c->Clip(left, bottom, right, top);
    rep->curcanvas = c;
}

void Painter::NoClip () {
    register Graphics* p = rep;

    if (p->curcanvas != nil) {
	p->curcanvas->NoClip();
	p->curcanvas = nil;
    }
}

#else

Painter::Painter () {
    register Graphics* p;

    p = new Graphics;
    bzero(p, sizeof(*p));
    rep = p;
    p->gc = XCreateGC(_World_dpy, _World_root, 0, nil);
    SetPattern(solid);
    SetBrush(single);
    SetColors(black, white);
    FillBg(true);
    SetFont(stdfont);
    XSetFunction(_World_dpy, p->gc, GXcopy);
    XSetPlaneMask(_World_dpy, p->gc, XAllPlanes);
    XSetSubwindowMode(_World_dpy, p->gc, IncludeInferiors);
}

Painter::Painter (Painter* from) {
    register Graphics* p = rep;
    register Graphics* copy = from->rep;

    p = new Graphics;
    bzero(p, sizeof(*p));
    rep = p;
    p->gc = XCreateGC(_World_dpy, _World_root, 0, nil);
    SetPattern(copy->pattern);
    SetBrush(copy->br);
    SetColors(copy->foreground, copy->background);
    FillBg(copy->fillstyle == FillOpaqueStippled);
    SetFont(copy->font);
    XSetFunction(_World_dpy, p->gc, GXcopy);
    XSetPlaneMask(_World_dpy, p->gc, XAllPlanes);
    XSetSubwindowMode(_World_dpy, p->gc, IncludeInferiors);
}

Painter::~Painter () {
    if (LastRef()) {
	XFreeGC(_World_dpy, rep->gc);
	delete rep->matrix;
	delete rep;
    }
}

void Painter::FillBg (boolean b) {
    if (rep->raster == GXxor) {
	SetRaster(GXcopy);
    }
    rep->fillstyle = b ? FillOpaqueStippled : FillStippled;
    XSetFillStyle(_World_dpy, rep->gc, rep->fillstyle);
}

boolean Painter::BgFilled () {
    return rep->fillstyle == FillOpaqueStippled;
}

void Painter::SetColors (Color* f, Color* b) {
    register Graphics *p = rep;

    if (f != nil && p->foreground != f) {
	p->foreground = f;
	if (p->raster != GXxor) {
	    XSetForeground(_World_dpy, p->gc, f->PixelValue());
	}
    }
    if (b != nil && p->background != b) {
	p->background = b;
	XSetBackground(_World_dpy, p->gc, b->PixelValue());
    }
}

void Painter::SetPattern (Pattern* pat) {
    register Graphics* p = rep;

    if (p->pattern != pat) {
	p->pattern = pat;
    }
}

void Painter::SetBrush (Brush* b) {
    rep->br = b;
    XSetLineAttributes(
	_World_dpy, rep->gc, b->width, LineSolid, CapButt, JoinMiter
    );
}

void Painter::SetFont (Font* f) {
    rep->font = f;
    XSetFont(_World_dpy, rep->gc, f->id);
}

void Painter::Clip (
    Canvas* c, Coord left, Coord bottom, Coord right, Coord top
) {
    XRectangle r[1];
    Coord x, y;

    if (left > right) {
	x = right; r[0].width = left - right + 1;
    } else {
	x = left; r[0].width = right - left + 1;
    }
    if (bottom > top) {
	y = bottom; r[0].height = bottom - top + 1;
    } else {
	y = top; r[0].height = top - bottom + 1;
    }
    rep->Map(c, x, y, r[0].x, r[0].y);
    XSetClipRectangles(_World_dpy, rep->gc, 0, 0, r, 1, Unsorted);
}

void Painter::NoClip () {
    XSetClipMask(_World_dpy, rep->gc, XNone);
}

#endif
