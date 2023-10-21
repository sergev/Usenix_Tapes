/*
 * Interface to Rects and Polygons, objects derived from Graphic.
 */

#ifndef polygons_h
#define polygons_h

#include "lines.h"

class Rect : public Graphic {
protected:
    Ref patbr;
    Coord x0, y0, x1, y1;

    boolean read(File*);
    boolean write(File*);

    void getBounds(float&, float&, float&, float&, Graphic*);
    boolean contains(PointObj&, Graphic*);
    boolean intersects(BoxObj&, Graphic*);
    void draw(Canvas*, Graphic*);
public:
    ClassId GetClassId() { return RECT; }
    boolean IsA(ClassId id) { return GetClassId() == id || Graphic::IsA(id); }

    Rect();
    Rect(Coord x0, Coord y0, Coord x1, Coord y1, Graphic* gr =nil);

    Graphic* Copy () { return new Rect(x0, y0, x1, y1, this); }
    void GetOriginal(Coord&, Coord&, Coord&, Coord&);

    void SetBrush(PBrush*);
    PBrush* GetBrush();
};

class FillRect : public Rect {
protected:
    void getBounds(float&, float&, float&, float&, Graphic*);
    boolean contains(PointObj&, Graphic*);
    boolean intersects(BoxObj&, Graphic*);
    void draw(Canvas*, Graphic*);
public:
    ClassId GetClassId() { return FILLRECT; }
    boolean IsA(ClassId id) { return GetClassId() == id || Rect::IsA(id); }

    FillRect();
    FillRect(Coord x0, Coord y0, Coord x1, Coord y1, Graphic* gr =nil);

    Graphic* Copy () { return new FillRect(x0, y0, x1, y1, this); }
    void SetPattern(PPattern*);
    PPattern* GetPattern();
    void SetBrush(PBrush*) { }
    PBrush* GetBrush() { return nil; }
};

class Polygon : public MultiLine {
    boolean contains(PointObj&, Graphic*);
    boolean intersects(BoxObj&, Graphic*);
    void draw(Canvas*, Graphic*);
public:
    ClassId GetClassId() { return POLYGON; }
    boolean IsA(ClassId id) { return GetClassId()==id || MultiLine::IsA(id); }

    Polygon();
    Polygon(Coord* x, Coord* y, int count, Graphic* gr =nil);

    Graphic* Copy () { return new Polygon(x, y, count, this); }
};

class FillPolygon : public Polygon {
    void getBounds(float&, float&, float&, float&, Graphic*);
    boolean contains(PointObj&, Graphic*);
    boolean intersects(BoxObj&, Graphic*);
    void draw(Canvas*, Graphic*);
public:
    ClassId GetClassId() { return FILLPOLYGON; }
    boolean IsA(ClassId id) { return GetClassId() == id || Polygon::IsA(id); }

    FillPolygon();
    FillPolygon(Coord* x, Coord* y, int count, Graphic* gr =nil);

    Graphic* Copy () { return new FillPolygon(x, y, count, this); }
    void SetPattern(PPattern*);
    PPattern* GetPattern();
    void SetBrush(PBrush*) { }
    PBrush* GetBrush() { return nil; }
};

#endif
