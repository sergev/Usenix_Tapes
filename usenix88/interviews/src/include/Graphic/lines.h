/*
 * Interface to Points, Lines, and MultiLines, objects derived from Graphic.
 */

#ifndef lines_h
#define lines_h

#include "base.h"

class Point : public Graphic {
    Ref brush;
protected:
    Coord x, y;

    boolean read(File*);
    boolean write(File*);

    void getBounds(float&, float&, float&, float&, Graphic*);
    boolean contains(PointObj&, Graphic*);
    boolean intersects(BoxObj&, Graphic*);
    void draw(Canvas*, Graphic*);
public:
    ClassId GetClassId() { return POINT; }
    boolean IsA(ClassId id) { return GetClassId() == id || Graphic::IsA(id); }

    Point();
    Point(Coord x, Coord y, Graphic* gr =nil);

    Graphic* Copy () { return new Point(x, y, this); }
    void GetOriginal(Coord&, Coord&);

    void SetBrush(PBrush*);
    PBrush* GetBrush();
};

class Line : public Graphic {
    Ref brush;
protected:
    Coord x0, y0, x1, y1;

    boolean read(File*);
    boolean write(File*);

    void getBounds(float&, float&, float&, float&, Graphic*);
    boolean contains(PointObj&, Graphic*);
    boolean intersects(BoxObj&, Graphic*);
    void draw(Canvas*, Graphic*);
public:
    ClassId GetClassId() { return LINE; }
    boolean IsA(ClassId id) { return GetClassId() == id || Graphic::IsA(id); }

    Line();
    Line(Coord x0, Coord y0, Coord x1, Coord y1, Graphic* gr =nil);

    Graphic* Copy () { return new Line(x0, y0, x1, y1, this); }
    void GetOriginal(Coord& x0, Coord& y0, Coord& x1, Coord& y1);

    void SetBrush(PBrush*);
    PBrush* GetBrush();
};

class MultiLine : public Graphic {
    FloatBoxObj* box;
protected:
    Ref patbr;
    Coord* x, *y;
    int count;

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
    ClassId GetClassId() { return MULTILINE; }
    boolean IsA(ClassId id) { return GetClassId() == id || Graphic::IsA(id); }

    MultiLine();
    MultiLine(Coord* x, Coord* y, int count, Graphic* gr =nil);
    ~MultiLine();

    Graphic* Copy () { return new MultiLine(x, y, count, this); }
    void GetOriginal(Coord*& x, Coord*& y, int&);

    void SetBrush(PBrush*);
    PBrush* GetBrush();
};

#endif
