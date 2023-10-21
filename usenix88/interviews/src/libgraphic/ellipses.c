/*
 * Implementation of Ellipses and Circles, objects derived from Graphic.
 */

#include <InterViews/Graphic/ellipses.h>

static const float axis = 0.42;
static const float seen = 1.025;

boolean Ellipse::read (File* f) {
    return Graphic::read(f) && patbr.Read(f) &&
	f->Read(x0) && f->Read(y0) && f->Read(r1) && f->Read(r2);
}

boolean Ellipse::write (File* f) {
    return Graphic::write(f) && patbr.Write(f) &&
	f->Write(x0) && f->Write(y0) && f->Write(r1) && f->Write(r2);
}

void Ellipse::draw (Canvas *c, Graphic* gs) {
    if (gs->GetBrush()->Width() != NO_WIDTH) {
	update(gs);
	pEllipse(c, x0, y0, r1, r2);
    }
}

Ellipse::Ellipse () {
    box = nil;
}

Ellipse::Ellipse (Coord x0, Coord y0, int r1, int r2, Graphic* gr) : (gr) {
    box = nil;
    if (gr == nil) {
	SetBrush(nil);
    } else {
	SetBrush(gr->GetBrush());
    }
    this->x0 = x0;
    this->y0 = y0;
    this->r1 = r1;
    this->r2 = r2;
}    

Ellipse::~Ellipse () {
    uncacheBounds();
}

void Ellipse::GetOriginal (Coord& x0, Coord& y0, int& r1, int& r2) {
    x0 = this->x0;
    y0 = this->y0;
    r1 = this->r1;
    r2 = this->r2;
}

void Ellipse::cacheBounds (float x0, float y0, float x1, float y1) {
    Coord left, bottom, right, top;

    if (caching) {
	delete box;
	left = Coord(x0 - 1);
	bottom = Coord(y0 - 1);
	right = Coord(x1 + 1);
	top = Coord(y1 + 1);
	box = new FloatBoxObj(left, bottom, right, top);
    }
}

void Ellipse::getCachedBounds (float& x0, float& y0, float& x1, float& y1) {
    x0 = box->left;
    y0 = box->bottom;
    x1 = box->right;
    y1 = box->top;
}

void Ellipse::getBounds (float& l, float& b, float& r, float& t, Graphic* gs) {
    float br, width;

    if (boundsCached()) {
	getCachedBounds(l, b, r, t);
    } else {
	width = float(gs->GetBrush()->Width());
	br = (width > 1) ? width/2 : 0;
	gs->transformRect(x0-r1, y0-r2, x0+r1, y0+r2, l, b, r, t);
	l -= br;
	b -= br;
	r += br;
	t += br;
	cacheBounds(l, b, r, t);
    }
}

boolean Ellipse::contains (PointObj& po, Graphic* gs) {
    PointObj pt (&po);
    
    gs->invTransform(pt.x, pt.y);
    return (
	square(r2)*square(pt.x - x0) + square(r1)*square(pt.y - y0) - 
	square(r1*r2) == 0
    );
}

boolean Ellipse::intersects (BoxObj& userb, Graphic* gs) {
    Coord px1, py1, px2, py2, x[8], y[8], tx[8], ty[8];
    Transformer* t;
    MultiLineObj ml;
    BoxObj b;

    getBox(b, gs);
    if (b.Intersects(userb)) {
	px1 = round(float(r1)*axis); py1 = round(float(r2)*axis);
	px2 = round(float(r1)*seen); py2 = round(float(r2)*seen);
	x[0] = x0 + px1;    y[0] = y0 + py2;
	x[1] = x0 - px1;    y[1] = y[0];
	x[2] = x0 - px2;    y[2] = y0 + py1;
	x[3] = x[2];	    y[3] = y0 - py1;
	x[4] = x[1];	    y[4] = y0 - py2;
	x[5] = x[0];	    y[5] = y[4];
	x[6] = x0 + px2;    y[6] = y[3];
	x[7] = x[6];	    y[7] = y[2];
	
	t = gs->GetTransformer();
	if (t == nil) {
	    ml.ClosedSplineToPolygon(x, y, 8);
	} else {
	    t->TransformList(x, y, 8, tx, ty);
	    ml.ClosedSplineToPolygon(tx, ty, 8);
	}
	return ml.Intersects(userb);
    }
    return false;
}

void Ellipse::SetBrush (PBrush* brush) {
    patbr = Ref(brush);
}

PBrush* Ellipse::GetBrush () {
    return (PBrush*) patbr();
}

void FillEllipse::draw (Canvas* c, Graphic* gs) {
    update(gs);
    pFillEllipse(c, x0, y0, r1, r2);
}

FillEllipse::FillEllipse () {}

FillEllipse::FillEllipse (
    Coord x0, Coord y0, int r1, int r2, Graphic* gr
) : (x0, y0, r1, r2, gr) {
    if (gr == nil) {
	SetPattern(nil);
    } else {
	SetPattern(gr->GetPattern());
    }
}

void FillEllipse::getBounds (
    float& l, float& b, float& r, float& t, Graphic* gs
) {
    if (boundsCached()) {
	getCachedBounds(l, b, r, t);
    } else {
	gs->transformRect(x0-r1, y0-r2, x0+r1, y0+r2, l, b, r, t);
	cacheBounds(l, b, r, t);
    }
}

boolean FillEllipse::contains (PointObj& po, Graphic* gs) {
    PointObj pt (&po);
    gs->invTransform(pt.x, pt.y);
    return (
        square(r2)*square(pt.x - x0) + square(r1)*square(pt.y - y0) - 
	square(r1*r2)
    ) <= 0;
}

boolean FillEllipse::intersects (BoxObj& userb, Graphic* gs) {
    Coord px1, py1, px2, py2, x[8], y[8], tx[8], ty[8];
    Transformer* t;
    FillPolygonObj fp;
    BoxObj b;

    getBox(b, gs);
    if (b.Intersects(userb)) {
	px1 = round(float(r1)*axis); py1 = round(float(r2)*axis);
	px2 = round(float(r1)*seen); py2 = round(float(r2)*seen);
	x[0] = x0 + px1;    y[0] = y0 + py2;
	x[1] = x0 - px1;    y[1] = y[0];
	x[2] = x0 - px2;    y[2] = y0 + py1;
	x[3] = x[2];	    y[3] = y0 - py1;
	x[4] = x[1];	    y[4] = y0 - py2;
	x[5] = x[0];	    y[5] = y[4];
	x[6] = x0 + px2;    y[6] = y[3];
	x[7] = x[6];	    y[7] = y[2];
	
	t = gs->GetTransformer();
	if (t == nil) {
	    fp.ClosedSplineToPolygon(x, y, 8);
	} else {
	    t->TransformList(x, y, 8, tx, ty);
	    fp.ClosedSplineToPolygon(tx, ty, 8);
	}
	return fp.Intersects(userb);
    }
    return false;
}

void FillEllipse::SetPattern (PPattern* pat) {
    patbr = Ref(pat);
}

PPattern* FillEllipse::GetPattern () { 
    return (PPattern*) patbr();
}

Circle::Circle () {}

Circle::Circle (
    Coord x0, Coord y0, int radius, Graphic* gr
) : (x0, y0, radius, radius, gr) {
}

FillCircle::FillCircle () {}

FillCircle::FillCircle (
    Coord x0, Coord y0, int radius, Graphic* gr
) : (x0, y0, radius, radius, gr) {
}
