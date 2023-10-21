/*
 * Implementation of Points, Lines, and MultiLines, objects derived from
 * Graphic.
 */

#include <InterViews/Graphic/lines.h>

extern void bcopy (void*, void*, int);

boolean Point::read (File* f) {
    return Graphic::read(f) && brush.Read(f) && f->Read(x) && f->Read(y);
}

boolean Point::write (File* f) {
    return Graphic::write(f) && brush.Write(f) && f->Write(x) && f->Write(y);
}

void Point::draw (Canvas* c, Graphic* gs) {
    if (gs->GetBrush()->Width() != NO_WIDTH) {
	update(gs);
	pPoint(c, x, y);
    }
}

Point::Point () {}

Point::Point (Coord x, Coord y, Graphic* gr) : (gr) { 
    if (gr == nil) {
	SetBrush(nil);
    } else {
	SetBrush(gr->GetBrush());
    }
    this->x = x; 
    this->y = y;
}

void Point::GetOriginal (Coord& x, Coord& y) {
    x = this->x;
    y = this->y;
}

void Point::getBounds (float& x0, float& y0, float& x1, float& y1, Graphic* gs){
    float br, width;

    width = float(gs->GetBrush()->Width());
    br = (width > 1) ? width / 2 : 0;
    gs->transform(float(x), float(y), x0, y0);
    x1 = x0 + br;
    y1 = y0 + br;
    x0 -= br;
    y0 -= br;
}

boolean Point::contains (PointObj& po, Graphic* gs) {
    PointObj pt (&po);
    gs->invTransform(pt.x, pt.y);
    return (pt.x == x) && (pt.y == y);
}

boolean Point::intersects (BoxObj& b, Graphic* gs) {
    PointObj pt (this->x, this->y);
        
    gs->transform(pt.x, pt.y);
    return b.Contains(pt);
}

void Point::SetBrush (PBrush* brush) {
    this->brush = Ref(brush);
}

PBrush* Point::GetBrush () {
    return (PBrush*) brush();
}

boolean Line::read (File* f) {
    return Graphic::read(f) && brush.Read(f) &&
	f->Read(x0) && f->Read(y0) && f->Read(x1) && f->Read(y1);
}

boolean Line::write (File* f) {
    return Graphic::write(f) && brush.Write(f) &&
	f->Write(x0) && f->Write(y0) && f->Write(x1) && f->Write(y1);
}

void Line::draw (Canvas* c, Graphic* gs) {
    if (gs->GetBrush()->Width() != NO_WIDTH) {
	update(gs);
	pLine(c, x0, y0, x1, y1);
    }
}

Line::Line () {}

Line::Line (
    Coord x0, Coord y0, Coord x1, Coord y1, Graphic* gr
) : (gr) {
    if (gr == nil) {
	SetBrush(nil);
    } else {
	SetBrush(gr->GetBrush());
    }
    this->x0 = x0;
    this->y0 = y0;
    this->x1 = x1;
    this->y1 = y1;
}

void Line::GetOriginal (Coord& x0, Coord& y0, Coord& x1, Coord& y1) {
    x0 = this->x0;
    y0 = this->y0;
    x1 = this->x1;
    y1 = this->y1;
}    

void Line::getBounds (float& x0, float& y0, float& x1, float& y1, Graphic* gs) {
    float tx, ty, br, width;

    width = float(gs->GetBrush()->Width());
    br = (width > 1) ? width / 2 : 0;
    gs->transform(float(this->x0), float(this->y0), x0, y0);
    gs->transform(float(this->x1), float(this->y1), x1, y1);
    tx = x0;
    ty = y0;
    x0 = fmin(x0, x1) - br;
    y0 = fmin(y0, y1) - br;
    x1 = fmax(tx, x1) + br;
    y1 = fmax(ty, y1) + br;
}

boolean Line::contains (PointObj& po, Graphic* gs) {
    LineObj l (x0, y0, x1, y1);
    PointObj pt (&po);
    gs->invTransform(pt.x, pt.y);
    return l.Contains(pt);
}

boolean Line::intersects (BoxObj& b, Graphic* gs) {
    LineObj l (this->x0, this->y0, this->x1, this->y1);
    gs->transform(l.p1.x, l.p1.y);
    gs->transform(l.p2.x, l.p2.y);
    return b.Intersects(l);
}

void Line::SetBrush (PBrush* brush) {
    this->brush = Ref(brush);
}

PBrush* Line::GetBrush () {
    return (PBrush*) brush();
}

MultiLine::MultiLine () {
    box = nil;
    x = y = nil;
    count = 0;
}

MultiLine::MultiLine (Coord* x, Coord* y, int count, Graphic* gr) : (gr) {
    box = nil;
    if (gr == nil) {
	SetBrush(nil);
    } else {
	SetBrush(gr->GetBrush());
    }
    this->x = new Coord[count];
    this->y = new Coord[count];
    this->count = count;
    CopyArray(x, y, count, this->x, this->y);
}

MultiLine::~MultiLine () {
    uncacheBounds();
    delete x; 
    delete y;
}

boolean MultiLine::read (File* f) {
    boolean ok = Graphic::read(f) && patbr.Read(f) && f->Read(count);
    if (ok) {
	delete x;
	delete y;
	x = new Coord [count];
	y = new Coord [count];
	ok = f->Read(x, count) && f->Read(y, count);
    }
    return ok;
}

boolean MultiLine::write (File* f) {
    return Graphic::write(f) && patbr.Write(f) && f->Write(count) && 
	f->Write(x, count) && f->Write(y, count);
}

void MultiLine::draw (Canvas *c, Graphic* gs) {
    if (gs->GetBrush()->Width() != NO_WIDTH) {
	update(gs);
	pMultiLine(c, x, y, count);
    }
}

void MultiLine::cacheBounds (float x0, float y0, float x1, float y1) {
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

void MultiLine::getCachedBounds (float& x0, float& y0, float& x1, float& y1) {
    x0 = box->left;
    y0 = box->bottom;
    x1 = box->right;
    y1 = box->top;
}

void MultiLine::getBounds (
    float& x0, float& y0, float& x1, float& y1, Graphic* gs
) {
    int i;
    float tx, ty, br, width;
    
    if (boundsCached()) {
	getCachedBounds(x0, y0, x1, y1);
    } else {
	width = float(gs->GetBrush()->Width());
	br = (width > 1) ? width/2 : 0;
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
	x0 -= br;
	y0 -= br;
	x1 += br;
	y1 += br;
	cacheBounds(x0, y0, x1, y1);
    }
}

void MultiLine::GetOriginal (Coord*& x, Coord*& y, int& n) {
    n = count;
    x = new Coord[n];
    y = new Coord[n];
    CopyArray(this->x, this->y, count, x, y);
}

boolean MultiLine::contains (PointObj& po, Graphic* gs) {
    MultiLineObj ml (x, y, count);
    PointObj pt (&po);
    BoxObj b;

    getBox(b, gs);
    if (b.Contains(po)) {
	gs->invTransform(pt.x, pt.y);
	return ml.Contains(pt);
    }
    return false;
}

boolean MultiLine::intersects (BoxObj& userb, Graphic* gs) {
    Coord* convx, *convy;
    BoxObj b;
    boolean result = false;

    getBox(b, gs);
    if (b.Intersects(userb)) {
	convx = new Coord[count];
	convy = new Coord[count];
	gs->transformList(x, y, count, convx, convy);
	MultiLineObj ml (convx, convy, count);
	result = ml.Intersects(userb);
	delete convx;
	delete convy;
    }
    return result;
}

void MultiLine::SetBrush (PBrush* brush) {
    patbr = Ref(brush);
}

PBrush* MultiLine::GetBrush () {
    return (PBrush*) patbr();
}
