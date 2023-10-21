/*
 * Interface to geometrical objects used to implement Graphic primitives.
 */

#ifndef geomobjs_h
#define geomobjs_h

#include "file.h"
#include "grclasses.h"
#include "objman.h"
#include "persistent.h"
#include "ref.h"
#include "reflist.h"
#include "util.h"
#include <math.h>
#include <InterViews/defs.h>

class PointObj : public Persistent {
protected:
    boolean read(File*);
    boolean write(File*);
public:
    ClassId GetClassId() { return POINTOBJ; }
    boolean IsA(ClassId id) { return GetClassId()==id || Persistent::IsA(id); }

    Coord x, y;
    PointObj(Coord x =0, Coord y =0) { this->x = x; this->y = y; }
    PointObj(PointObj* p) { x = p->x; y = p->y; }

    float Distance(PointObj& p)
	{ return sqrt(float(square(x - p.x) + square(y - p.y))); }
};

class LineObj : public Persistent {
protected:
    boolean read(File*);
    boolean write(File*);
public:
    ClassId GetClassId() { return LINEOBJ; }
    boolean IsA(ClassId id) { return GetClassId()==id || Persistent::IsA(id); }

    PointObj p1, p2;
    LineObj();
    LineObj(Coord x0, Coord y0, Coord x1, Coord y1)
        { p1.x = x0; p1.y = y0; p2.x = x1; p2.y = y1; }
    LineObj(LineObj* l)
        { p1.x = l->p1.x; p1.y = l->p1.y; p2.x = l->p2.x; p2.y = l->p2.y; }

    boolean Contains(PointObj&);
    int Same(PointObj& p1, PointObj& p2);
    boolean Intersects(LineObj&);
};

class BoxObj : public Persistent {
protected:
    boolean read(File*);
    boolean write(File*);
public:
    ClassId GetClassId() { return BOXOBJ; }
    boolean IsA(ClassId id) { return GetClassId()==id || Persistent::IsA(id); }

    Coord left, right;
    Coord bottom, top;
    BoxObj(Coord x0 =0, Coord y0 =0, Coord x1 =0, Coord y1 =0) {
	left = min(x0, x1); bottom = min(y0, y1); 
	right = max(x0, x1); top = max(y0, y1);
    }
    BoxObj(BoxObj* b)
        { left = b->left; bottom = b->bottom; right = b->right; top = b->top; }

    boolean Contains(PointObj&);
    boolean Intersects(BoxObj&);
    boolean Intersects(LineObj&);
    BoxObj operator-(BoxObj&);
    BoxObj operator+(BoxObj&);
    boolean Within(BoxObj&);
};

class FloatBoxObj {
    // very simple object to hold a box in floating point coordinates
public:
    float left, right, bottom, top;
    FloatBoxObj(float x0 =0, float y0 =0, float x1 =0, float y1 =0) {
	left = fmin(x0, x1); bottom = fmin(y0, y1); 
	right = fmax(x0, x1); top = fmax(y0, y1);
    }
};

class MultiLineObj : public Persistent {
protected:
    void GrowBuf();
    boolean CanApproxWithLine(
	double x0, double y0, double x2, double y2, double x3, double y3
    );
    void AddLine(double x0, double y0, double x1, double y1);
    void AddBezierArc(
        double x0, double y0, double x1, double y1,
        double x2, double y2, double x3, double y3
    );
    void CalcSection(
	Coord cminus1x, Coord cminus1y, Coord cx, Coord cy,
	Coord cplus1x, Coord cplus1y, Coord cplus2x, Coord cplus2y
    );
    void CreateMultiLine(Coord* cpx, Coord* cpy, int cpcount);

    boolean read(File*);
    boolean write(File*);
public:
    ClassId GetClassId() { return MULTILINEOBJ; }
    boolean IsA(ClassId id) { return GetClassId()==id || Persistent::IsA(id); }

    Coord* x, *y;
    int count;
    MultiLineObj(Coord* x =nil, Coord* y =nil, int count =0)
        { this->x = x; this->y = y; this->count = count; }
    MultiLineObj(MultiLineObj* ml)
        { x = ml->x; y = ml->y; count = ml->count; }

    void GetBox(BoxObj& b);
    boolean Contains(PointObj&);
    boolean Intersects(LineObj&);
    boolean Intersects(BoxObj&);
    boolean Within(BoxObj&);
    void SplineToMultiLine(Coord* cpx, Coord* cpy, int cpcount);
    void ClosedSplineToPolygon(Coord* cpx, Coord* cpy, int cpcount);
};

class FillPolygonObj : public MultiLineObj {
protected:
    int LowestLeft(Coord* x, Coord* y, int count);
public:
    ClassId GetClassId() { return FILLPOLYGONOBJ; }
    boolean IsA(ClassId id) {return GetClassId()==id || MultiLineObj::IsA(id);}

    FillPolygonObj();
    FillPolygonObj(Coord* x, Coord* y, int count);
    FillPolygonObj(FillPolygonObj* fp);
    ~FillPolygonObj() { delete x; delete y; }
    boolean Contains(PointObj&);
    boolean Intersects(LineObj&);
    boolean Intersects(BoxObj&);
};

#endif
