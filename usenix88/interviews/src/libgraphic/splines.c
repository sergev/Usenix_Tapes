/*
 * Implementation of BSplines and ClosedBSplines, objects derived from
 * Graphic.
 */

#include <InterViews/Graphic/splines.h>

void BSpline::draw (Canvas *c, Graphic* gs) {
    if (gs->GetBrush()->Width() != NO_WIDTH) {
	update(gs);
	pBSpline(c, x, y, count);
    }
}

BSpline::BSpline () {}

BSpline::BSpline (
    Coord* x, Coord* y, int count, Graphic* gr
) : (x, y, count, gr) {
}

boolean BSpline::contains (PointObj& po, Graphic* gs) {
    PointObj pt (&po);
    BoxObj b;

    getBox(b, gs);
    if (b.Contains(pt)) {
	gs->invTransform(pt.x, pt.y);
	MultiLineObj ml;
	ml.SplineToMultiLine(x, y, count);
	return ml.Contains(pt);
    }
    return false;
}

boolean BSpline::intersects (BoxObj& userb, Graphic* gs) {
    Coord* convx, *convy;
    BoxObj b;
    boolean result = false;
    
    getBox(b, gs);
    if (b.Intersects(userb)) {
	convx = new Coord[count];
	convy = new Coord[count];
	gs->transformList(x, y, count, convx, convy);
	MultiLineObj ml;
	ml.SplineToMultiLine(convx, convy, count);
	result = ml.Intersects(userb);
	delete convx;
	delete convy;
    }
    return result;
}

void ClosedBSpline::draw (Canvas *c, Graphic* gs) {
    if (gs->GetBrush()->Width() != NO_WIDTH) {
	update(gs);
	pClosedBSpline(c, x, y, count);
    }
}

ClosedBSpline::ClosedBSpline () {}

ClosedBSpline::ClosedBSpline (
    Coord* x, Coord* y, int count, Graphic* gr
) : (x, y, count, gr) {
}

boolean ClosedBSpline::contains (PointObj& po, Graphic* gs) {
    PointObj pt (&po);
    BoxObj b;

    getBox(b, gs);
    if (b.Contains(pt)) {
	gs->invTransform(pt.x, pt.y);
	MultiLineObj ml;
	ml.ClosedSplineToPolygon(x, y, count);
	return ml.Contains(pt);
    }
    return false;
}

boolean ClosedBSpline::intersects (BoxObj& userb, Graphic* gs) {
    Coord* convx, *convy;
    BoxObj b;
    boolean result = false;
    
    getBox(b, gs);
    if (b.Intersects(userb)) {
	convx = new Coord[count];
	convy = new Coord[count];
	gs->transformList(x, y, count, convx, convy);
	MultiLineObj ml;
	ml.ClosedSplineToPolygon(convx, convy, count);
	result = ml.Intersects(userb);
	delete convx;
	delete convy;
    }
    return result;
}

void FillBSpline::draw (Canvas *c, Graphic* gs) {
    update(gs);
    pFillBSpline(c, x, y, count);
}

FillBSpline::FillBSpline () {}

FillBSpline::FillBSpline (
    Coord* x, Coord* y, int count, Graphic* gr
) : (x, y, count, gr) {
    if (gr == nil) {
	SetPattern(nil);
    } else {
	SetPattern(gr->GetPattern());
    }
}

void FillBSpline::getBounds (
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

boolean FillBSpline::contains (PointObj& po, Graphic* gs) {
    PointObj pt (&po);
    BoxObj b;

    getBox(b, gs);
    if (b.Contains(pt)) {
	gs->invTransform(pt.x, pt.y);
	FillPolygonObj fp;
	fp.ClosedSplineToPolygon(x, y, count);
	return fp.Contains(pt);
    }
    return false;
}

boolean FillBSpline::intersects (BoxObj& userb, Graphic* gs) {
    Coord* convx, *convy;
    BoxObj b;
    boolean result = false;
    
    getBox(b, gs);
    if (b.Intersects(userb)) {
	convx = new Coord[count];
	convy = new Coord[count];
	gs->transformList(x, y, count, convx, convy);
	FillPolygonObj fp;
	fp.ClosedSplineToPolygon(convx, convy, count);
	result = fp.Intersects(userb);
	delete convx;
	delete convy;
    }
    return result;
}

void FillBSpline::SetPattern (PPattern* pat) {
    patbr = Ref(pat);
}

PPattern* FillBSpline::GetPattern () { 
    return (PPattern*) patbr();
}
