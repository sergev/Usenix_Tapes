/*
 * GraphicBlock implementation.
 */

#include <InterViews/canvas.h>
#include <InterViews/painter.h>
#include <InterViews/perspective.h>
#include <InterViews/scene.h>
#include <InterViews/shape.h>
#include <InterViews/Graphic/base.h>
#include <InterViews/Graphic/grblock.h>
#include <InterViews/Graphic/util.h>

extern double frexp(double, int*);  // a = frexp(x,n) where a*2^n = x
extern double ldexp(double, int);   // x = ldexp(a,n) where a*2^n = x

GraphicBlock::GraphicBlock (
    Sensor* in, Graphic* gr, Coord padding, HAlign h, VAlign v, Zooming z
) : (in, nil) {
    graphic = gr;
    pad = padding;
    halign = h;
    valign = v;
    zooming = z;
    mag = 1;
    perspective = new Perspective;
    output = new Painter(stdpaint);
    alt = new Painter(stdpaint);
    alt->SetColors(nil, stdpaint->GetFgColor());
    Init();
}

void GraphicBlock::Init () {
    Coord left, bottom, right, top;

    graphic->GetBox(left, bottom, right, top);
    shape->width = right - left + 2*pad;
    shape->height = top - bottom + 2*pad;
    x0 = left;
    y0 = bottom;
    perspective->width = shape->width;
    perspective->height = shape->height;
    Align();
}

void GraphicBlock::Align () {
    register Perspective* p = perspective;
    Coord l, b, dummy;

    switch (halign) {
	case LEFT:
	    p->curx = 0;
	    break;	
	case HCTR:
	    p->curx = (p->width - p->curwidth)/2;
	    break;
	case RIGHT:
	    p->curx = p->width - p->curwidth;
	    break;
    }

    switch (valign) {
	case BOTTOM:
	    p->cury = 0;
	    break;	
	case VCTR:
	    p->cury = (p->height - p->curheight)/2;
	    break;
	case TOP:
	    p->cury = p->height - p->curheight;
	    break;
    }
    graphic->GetBox(l, b, dummy, dummy);
    l = pad - l - p->curx;
    b = pad - b - p->cury;
    graphic->Translate(l, b);
    x0 += l;
    y0 += b;
}

void GraphicBlock::Fix () {
    register Perspective* p = perspective;
    Coord l, b, dummy;

    graphic->GetBox(l, b, dummy, dummy);
    l = pad - l;
    b = pad - b;

    switch (halign) {
	case LEFT:
	    p->curx = l;
	    break;
	case HCTR:
	    p->curx -= (xmax + 1 - p->curwidth)/2;
	    break;
	case RIGHT:
	    p->curx -= (xmax + 1 - p->curwidth);
	    break;
    }

    switch (valign) {
	case BOTTOM:
	    p->cury = b;
	    break;
	case VCTR:
	    p->cury -= (ymax + 1 - p->curheight)/2;
	    break;
	case TOP:
	    p->cury -= (ymax + 1 - p->curheight);
	    break;
    }
    l -= p->curx;
    b -= p->cury;
    graphic->Translate(l, b);
    x0 += l;
    y0 += b;
}

void GraphicBlock::Resize () {
    register Perspective* p = perspective;
    
    Fix();
    p->curwidth = xmax + 1;
    p->curheight = ymax + 1;
    p->sx = p->curwidth/4;
    p->sy = p->curheight/4;
    p->lx = p->curwidth;
    p->ly = p->curheight;
    p->Update();
}

void GraphicBlock::Update () {
    register Perspective* p = perspective;
    Coord left, bottom, right, top;

    graphic->GetBox(left, bottom, right, top);
    if (x0 != left) {
	p->curx += x0 - left;
	x0 = left;
    }
    if (y0 != bottom) {
	p->cury += y0 - bottom;
	y0 = bottom;
    }
    p->width = right - left + 2*pad;
    p->height = top - bottom + 2*pad;
    p->Update();
    Draw();
}

void GraphicBlock::SwapPainters () {
    Painter* p;
    
    p = alt;
    alt = output;
    output = p;
}

Painter* GraphicBlock::GetPainter () {
    PColor* pbg;
    Color* bg;
    Painter* p = output;

    if (graphic != nil && (pbg = graphic->GetBgColor()) != nil) {
	bg = *pbg;
	if (bg == output->GetBgColor()) {
	    // p equals output
	} else if (bg == alt->GetBgColor()) {
	    p = alt;
	} else {
	    SwapPainters();
	    output->SetColors(nil, bg);
	    p = output;
	}
    }
    return p;
}

void GraphicBlock::InvertPainter () {
    Color* fg, *bg, *afg, *abg;
    
    fg = output->GetFgColor();
    bg = output->GetBgColor();
    afg = alt->GetFgColor();
    abg = alt->GetBgColor();
    SwapPainters();
    if (fg != abg || bg != afg) {
	output->SetColors(bg, fg);
    }
}

void GraphicBlock::Draw () {
    if (canvas != nil) {
	GetPainter()->ClearRect(canvas, 0, 0, xmax, ymax);
	if (graphic != nil) {
	    graphic->Draw(canvas, 0, 0, xmax, ymax);
	}
    }
}

void GraphicBlock::Redraw (Coord l, Coord b, Coord r, Coord t) {
    if (canvas != nil) {
	GetPainter()->ClearRect(canvas, l, b, r, t);
	if (graphic != nil) {
	    graphic->DrawClipped(canvas, l, b, r, t);
	}
    }
}

void GraphicBlock::Normalize (Perspective& np) {
    register Perspective* p = perspective;
    float hfactor, vfactor;

    if (p->width != np.width) {
	hfactor = float(p->width) / float(np.width);
	np.x0 = round(hfactor * float(np.x0));
	np.width = p->width;
	np.curx = round(hfactor * float(np.curx));
	np.curwidth = round(hfactor * float(np.curwidth));
	np.sx = round(hfactor * float(np.sx));
    }
    if (p->height != np.height) {
	vfactor = float(p->height) / float(np.height);
	np.y0 = round(hfactor * float(np.y0));
	np.height = p->height;
	np.cury = round(hfactor * float(np.cury));
	np.curheight = round(hfactor * float(np.curheight));
	np.sy = round(hfactor * float(np.sy));
    }
}

float GraphicBlock::NearestPow2 (float factor) {
    double mant;
    int pow2;

    mant = frexp(factor, &pow2);
    if (mant < 0.95) {
	--pow2;
    }
    return ldexp(1.0, pow2);
}

float GraphicBlock::ScaleFactor (Perspective& np) {
    register Perspective* p = perspective;
    float factor = 1;
    Coord dx, dy;

    dx = abs(p->curwidth - np.curwidth);
    dy = abs(p->curheight - np.curheight);
    if (dx < dy) {
	factor = float(p->curwidth) / float(np.curwidth);
    } else {
	factor = float(p->curheight) / float(np.curheight);
    }
    if (zooming == BINARY) {
	factor = NearestPow2(factor);
    }
    return factor;
}

void GraphicBlock::Zoom (Perspective& np) {
    register Perspective* p = perspective;
    Coord cx, cy, halfw, halfh, dx, dy;
    float factor = ScaleFactor(np);

    if (factor != 1.0) {
	cx = np.curx + np.curwidth/2;
	cy = np.cury + np.curheight/2;
	halfw = p->curwidth/2;
	halfh = p->curheight/2;
	dx = (p->curx + halfw) - cx;
	dy = (p->cury + halfh) - cy;
	graphic->Translate(dx, dy);
	graphic->Scale(factor, factor, float(halfw), float(halfh));

	x0 = round((x0 + dx - halfw)*factor + halfw);
	y0 = round((y0 + dy - halfh)*factor + halfh);
	p->width = round(p->width * factor);
	p->height = round(p->height * factor);
	p->curx = round(float(cx) * factor) - halfw;
	p->cury = round(float(cy) * factor) - halfh;
	mag *= factor;
    }
}

void GraphicBlock::Scroll (Perspective& np) {
    register Perspective* p = perspective;
    Coord dx, dy;

    np.curx = min(max(0, np.curx), max(p->curx, p->width - p->curwidth));
    np.cury = min(max(0, np.cury), max(p->cury, p->height - p->curheight));
    dx = p->curx - np.curx;
    dy = p->cury - np.cury;
    graphic->Translate(dx, dy);
    x0 += dx;
    y0 += dy;
    p->curx = np.curx;
    p->cury = np.cury;
}

void GraphicBlock::Adjust (Perspective& np) {
    register Perspective* p = perspective;
    Perspective ptmp;
    
    if (*p != np) {
	Normalize(np);
	ptmp = *p;
	if (np.curwidth != canvas->Width() || np.curheight!=canvas->Height()) {
	    Zoom(np);
	} else {
	    Scroll(np);
	}
	p->Update();
	if (ptmp != *p) {
	    Draw();
	}
    }
}

void GraphicBlock::Delete () {
    delete alt;
    delete perspective;
    Interactor::Delete();
}

void GraphicBlock::Invert () {	    // just switches colors
    PColor* fg, *bg;
    
    if (graphic == nil) {
	InvertPainter();
    } else {
	fg = graphic->GetFgColor();
	bg = graphic->GetBgColor();
	graphic->SetColors(bg, fg);
    }
    Draw();
}

void GraphicBlock::SetMagnification (float m) {
    register Perspective* p = perspective;
    float factor;
    Coord cx, cy, halfw, halfh;

    if (zooming == BINARY) {
	m = NearestPow2(m);
    }
    factor = m/mag;

    if (factor != 1.0) {
	halfw = p->curwidth/2;
	halfh = p->curheight/2;
	cx = p->curx + halfw;
	cy = p->cury + halfh;
	graphic->Scale(factor, factor, float(halfw), float(halfh));

	x0 = round((x0 - halfw)*factor + halfw);
	y0 = round((y0 - halfh)*factor + halfh);
	p->width = round(p->width * factor);
	p->height = round(p->height * factor);
	p->curx = round(float(cx) * factor) - halfw;
	p->cury = round(float(cy) * factor) - halfh;

	mag = m;
	p->Update();
	Draw();
    }
}
