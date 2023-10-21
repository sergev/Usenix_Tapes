/*
 * Interface to BSplines, objects derived from Graphic.
 */

#ifndef splines_h
#define splines_h

#include "polygons.h"

class BSpline : public MultiLine {
    boolean contains(PointObj&, Graphic*);
    boolean intersects(BoxObj&, Graphic*);
    void draw(Canvas*, Graphic*);
public:
    ClassId GetClassId() { return BSPLINE; }
    boolean IsA(ClassId id) { return GetClassId()==id || MultiLine::IsA(id); }

    BSpline();
    BSpline(Coord* x, Coord* y, int count, Graphic* gr =nil);

    Graphic* Copy () { return new BSpline(x, y, count, this); }
};

class ClosedBSpline : public BSpline {
    boolean contains(PointObj&, Graphic*);
    boolean intersects(BoxObj&, Graphic*);
    void draw(Canvas*, Graphic*);
public:
    ClassId GetClassId() { return CLOSEDBSPLINE; }
    boolean IsA(ClassId id) { return GetClassId() == id || BSpline::IsA(id); }

    ClosedBSpline();
    ClosedBSpline(Coord* x, Coord* y, int count, Graphic* gr =nil);

    Graphic* Copy () { return new ClosedBSpline(x, y, count, this); }
};

class FillBSpline : public ClosedBSpline {
    void getBounds(float&, float&, float&, float&, Graphic*);
    boolean contains(PointObj&, Graphic*);
    boolean intersects(BoxObj&, Graphic*);
    void draw(Canvas*, Graphic*);
public:
    ClassId GetClassId() { return FILLBSPLINE; }
    boolean IsA(ClassId id) {return GetClassId()==id ||ClosedBSpline::IsA(id);}

    FillBSpline();
    FillBSpline(Coord* x, Coord* y, int count, Graphic* gr =nil);

    Graphic* Copy () { return new FillBSpline(x, y, count, this); }
    void SetPattern(PPattern*);
    PPattern* GetPattern();
    void SetBrush(PBrush*) { }
    PBrush* GetBrush() { return nil; }
};

#endif
