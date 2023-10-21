/*
 * Implementation of Rectangles and Polygons, objects derived from Graphic.
 */

#include <InterViews/Graphic/polygons.h>

boolean Rect::read (File* f) {
    return Graphic::read(f) && patbr.Read(f) &&
	f->Read(x0) && f->Read(y0) && f->Read(x1) && f->Read(y1);
}

boolean Rect::write (File* f) {
    return Graphic::write(f) && patbr.Write(f) &&
	f->Write(x0) && f->Write(y0) && f->Write(x1) && f->Write(y1);
}

void Rect::draw (Canvas *c, Graphic* gs) {
    if (gs->GetBrush()->Width() != NO_WIDTH) {
	update(gs);
	pRect(c, x0, y0, x1, y1);
    }
}

Rect::Rect () {}

Rect::Rect (
    Coord x0, Coord y0, Coord x1, Coord y1, Graphic* gr
) : (gr) {
    if (gr == nil) {
	SetBrush(nil);
    } else {
	SetBrush(gr->GetBrush());
    }
    this->x0 = min(x0, x1);
    this->y0 = min(y0, y1);
    this->x1 = max(x0, x1);
    this->y1 = max(y0, y1);
}

void Rect::GetOriginal (Coord& x0, Coord& y0, Coord& x1, Coord& y1) {
    x0 = this->x0;
    y0 = this->y0;
    x1 = this->x1;
    y1 = this->y1;
}

void Rect::getBounds (float& l, float& b, float& r, float& t, Graphic* gs) {
    float br, width;

    width = float(gs->GetBrush()->Width());
    br = (width > 1) ? width/2 : 0;
    gs->transformRect(x0, y0, x1, y1, l, b, r, t);
    l -= br;
    b -= br;
    r += br;
    t += br;
}

boolean Rect::contains (PointObj& po, Graphic* gs) {
    PointObj pt (&po);
    gs->invTransform(pt.x, pt.y);
    return (
        ((pt.x == x0 || pt.x == x1) && y0 <= pt.y && pt.y <= y1) ||
	((pt.y == y0 || pt.y == y1) && x0 <= pt.x && pt.x <= x1)
    );
}

boolean Rect::intersects (BoxObj& userb, Graphic* gs) {
    Coord x[4], tx[5];
    Coord y[4], ty[5];
    
    x[0] = x[3] = this->x0;
    y[0] = y[1] = this->y0;
    x[2] = x[1] = this->x1;
    y[2] = y[3] = this->y1;
    gs->transformList(x, y, 4, tx, ty);
    tx[4] = tx[0];
    ty[4] = ty[0];
    MultiLineObj ml (tx, ty, 5);
    return ml.Intersects(userb) || ml.Within(userb);
}

void Rect::SetBrush (PBrush* brush) {
    patbr = Ref(brush);
}

PBrush* Rect::GetBrush () {
    return (PBrush*) patbr();
}

void FillRect::draw (Canvas *c, Graphic* gs) {
    update(gs);
    pFillRect(c, x0, y0, x1, y1);
}

FillRect::FillRect () {}

FillRect::FillRect (
    Coord x0, Coord y0, Coord x1, Coord y1, Graphic* gr
) : (x0, y0, x1, y1, gr) {
    if (gr == nil) {
	SetPattern(nil);
    } else {
	SetPattern(gr->GetPattern());
    }
}

void FillRect::getBounds (float& l, float& b, float& r, float& t, Graphic* gs){
    gs->transformRect(x0, y0, x1, y1, l, b, r, t);
}

boolean FillRect::contains (PointObj& po, Graphic* gs) {
    PointObj pt (&po);
    gs->invTransform(pt.x, pt.y);
    BoxObj b (x0, y0, x1, y1);
    return b.Contains(pt);
}

boolean FillRect::intersects (BoxObj& userb, Graphic* gs) {
    Transformer* t = gs->GetTransformer();
    Coord tx0, ty0, tx1, ty1;
    
    if (t != nil && t->Rotated()) {
	Coord x[4], tx[5];
	Coord y[4], ty[5];
    
	x[0] = x[3] = this->x0;
	y[0] = y[1] = this->y0;
	x[2] = x[1] = this->x1;
	y[2] = y[3] = this->y1;
	gs->transformList(x, y, 4, tx, ty);
	tx[4] = tx[0];
	ty[4] = ty[0];
	FillPolygonObj fp (tx, ty, 5);
	return fp.Intersects(userb);
    
    } else if (t != nil) {
	t->Transform(x0, y0, tx0, ty0);
	t->Transform(x1, y1, tx1, ty1);
	BoxObj b1 (tx0, ty0, tx1, ty1);
	return b1.Intersects(userb);

    } else {
	BoxObj b2 (x0, y0, x1, y1);
	return b2.Intersects(userb);
    }
}

void FillRect::SetPattern (PPattern* pat) {
    patbr = Ref(pat);
}

PPattern* FillRect::GetPattern () { 
    return (PPattern*) patbr();
}

void Polygon::draw (Canvas *c, Graphic* gs) {
    if (gs->GetBrush()->Width() != NO_WIDTH) {
	update(gs);
	pPolygon(c, x, y, count);
    }
}

Polygon::Polygon () {}

Polygon::Polygon (
    Coord* x, Coord* y, int count, Graphic* gr
) : (x, y, count, gr) {
}

boolean Polygon::contains (PointObj& pt, Graphic* gs) {
    BoxObj b;
    
    getBox(b, gs);
    if (b.Contains(pt)) {
	MultiLineObj ml (x, y, count);
	LineObj l (x[count - 1], y[count - 1], *x, *y);
	return ml.Contains(pt) || l.Contains(pt);
    }
    return false;
}

boolean Polygon::intersects (BoxObj& userb, Graphic* gs) {
    Coord* convx, *convy;
    BoxObj b;
    boolean result = false;
    
    getBox(b, gs);
    if (b.Intersects(userb)) {
	convx = new Coord[count + 1];
	convy = new Coord[count + 1];
	gs->transformList(x, y, count, convx, convy);
	convx[count] = *convx;
	convy[count] = *convy;
	MultiLineObj ml (convx, convy, count + 1);
	result = ml.Intersects(userb);
	delete convx;
	delete convy;
    }
    return result;
}

void FillPolygon::draw (Canvas *c, Graphic* gs) {
    update(gs);
    pFillPolygon(c, x, y, count);
}

FillPolygon::FillPolygon () {}

FillPolygon::FillPolygon (
    Coord* x, Coord* y, int count, Graphic* gr
) : (x, y, count, gr) {
    if (gr == nil) {
	SetPattern(nil);
    } else {
	SetPattern(gr->GetPattern());
    }
}

void FillPolygon::getBounds (
    float& x0, float& y0, float& x1, float& y1, Graphic* gs
) {
    register int i;
    float tx, ty;
	
    if (boundsCached()) {
	getCachedBounds(x0, y0, x1, y1);
    } else {
	gs->transform(float(x[0]), float(y[0]), x0, y0);
	x1 = x0;
	y1 = y0;

	for (i = 1; i < count; ++i) {
	    gs->transform(float(x[i]), float(y[i]), tx, ty);
	    x0 = fmin(x0, tx);
	    y0 = fmin(y0, ty);
	    x1 = fmax(x1, tx);
	    y1 = fmax(y1, ty);
	}
	cacheBounds(x0, y0, x1, y1);
    }
}

boolean FillPolygon::contains (PointObj& po, Graphic* gs) {
    BoxObj b;
    PointObj pt (&po);

    getBox(b, gs);
    if (b.Contains(pt)) {
	FillPolygonObj fp (x, y, count);
	gs->invTransform(pt.x, pt.y);
	return fp.Contains(pt);
    }
    return false;
}

boolean FillPolygon::intersects (BoxObj& userb, Graphic* gs) {
    Coord* convx, *convy;
    BoxObj b;
    boolean result = false;
    
    getBox(b, gs);
    if (b.Intersects(userb)) {
	convx = new Coord[count + 1];
	convy = new Coord[count + 1];
	gs->transformList(x, y, count, convx, convy);
	FillPolygonObj fp (convx, convy, count);
	result = fp.Intersects(userb);
	delete convx;
	delete convy;
    }
    return result;    
}

void FillPolygon::SetPattern (PPattern* pat) {
    patbr = Ref(pat);
}

PPattern* FillPolygon::GetPattern () { 
    return (PPattern*) patbr();
}

