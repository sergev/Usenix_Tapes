/*
 * Interface to Ellipses and Circles, objects derived from Graphic.
 */

#ifndef ellipses_h
#define ellipses_h

#include "base.h"

class Ellipse : public Graphic {
    FloatBoxObj* box;
protected:
    Ref patbr;
    Coord x0, y0;
    int r1, r2;

    boolean read(File*);
    boolean write(File*);

    boolean boundsCached () {
	return caching && box != nil && parentBoundsCached();
    }
    void cacheBounds(float, float, float, float);
    void uncacheBounds() { delete box; box = nil; uncacheParentBounds(); }
    void getCachedBounds(float&, float&, float&, float&);
    void getBounds(float&, float&, float&, float&, Graphic*);
    boolean contains(PointObj&, Graphic*);
    boolean intersects(BoxObj&, Graphic*);
    void draw(Canvas*, Graphic*);
public:
    ClassId GetClassId() { return ELLIPSE; }
    boolean IsA(ClassId id) { return GetClassId()==id || Graphic::IsA(id); }

    Ellipse();
    Ellipse(Coord x0, Coord y0, int r1, int r2, Graphic* gr =nil);
    ~Ellipse();

    Graphic* Copy () { return new Ellipse(x0, y0, r1, r2, this); }
    void GetOriginal(Coord&, Coord&, int&, int&);

    void SetBrush(PBrush*);
    PBrush* GetBrush();
};

class FillEllipse : public Ellipse {
protected:
    void getBounds(float&, float&, float&, float&, Graphic*);
    boolean contains(PointObj&, Graphic*);
    boolean intersects(BoxObj&, Graphic*);
    void draw(Canvas*, Graphic*);
public:
    ClassId GetClassId() { return FILLELLIPSE; }
    boolean IsA(ClassId id) { return GetClassId() == id || Ellipse::IsA(id); }

    FillEllipse();
    FillEllipse(Coord x0, Coord y0, int r1, int r2, Graphic* gr =nil);

    Graphic* Copy () { return new FillEllipse(x0, y0, r1, r2, this); }
    void SetPattern(PPattern*);
    PPattern* GetPattern();
    void SetBrush(PBrush*) { }
    PBrush* GetBrush() { return nil; }
};

class Circle : public Ellipse {
public:
    ClassId GetClassId() { return CIRCLE; }
    boolean IsA(ClassId id) { return GetClassId() == id || Ellipse::IsA(id); }

    Circle();
    Circle(Coord x0, Coord y0, int radius, Graphic* gr =nil);

    Graphic* Copy () { return new Circle(x0, y0, r1, this); }
};

class FillCircle : public FillEllipse {
public:
    ClassId GetClassId() { return FILLCIRCLE; }
    boolean IsA(ClassId id) { return GetClassId()==id || FillEllipse::IsA(id);}

    FillCircle();
    FillCircle(Coord x0, Coord y0, int radius, Graphic* gr =nil);

    Graphic* Copy () { return new FillCircle(x0, y0, r1, this); }
};

#endif
