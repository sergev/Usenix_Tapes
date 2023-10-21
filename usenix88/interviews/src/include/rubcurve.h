/*
 * Rubberbanding for curves.
 */

#ifndef rubcurve_h
#define rubcurve_h

#include <InterViews/rubband.h>

/*
 * A RubberEllipse is an ellipse with center (cx, cy), whose horizontal
 * and vertical radii vary as | x - cx | and | y - cy |, respectively,
 * where (x, y) is the tracking point.  (rx, ry) is the
 * value of the tracking point used to compute the initial horizontal and
 * vertical radii.
 */

class RubberEllipse : public Rubberband {
protected:
    Coord centerx, radiusx;
    Coord centery, radiusy;
public:
    RubberEllipse(
        Painter*, Canvas*, Coord cx, Coord cy, Coord rx, Coord ry,
	Coord offx = 0, Coord offy = 0
    );
    virtual void GetOriginal(Coord& cx, Coord& cy, Coord& rx, Coord& ry);
    virtual void GetCurrent(Coord& cx, Coord& cy, Coord& rx, Coord& ry);
    void Draw();
};

/*
 * A RubberCircle is a circle with center (cx, cy), whose radius varies with
 * the distance between the center and the tracking point.  (rx, ry) is the
 * value of the tracking point used to compute the initial radius.
 */

class RubberCircle : public RubberEllipse {
public:
    RubberCircle(
        Painter*, Canvas*, Coord cx, Coord cy, Coord rx, Coord ry,
	Coord offx = 0, Coord offy = 0
    );
    void Draw();
};

/*
 *  A RubberPointList manages an array of points.  Its constructor copies
 *  a given array into private storage, and its destructor deallocates 
 *  that storage.
 */

class RubberPointList : public Rubberband {
protected:
    Coord *x;
    Coord *y;
    int count;
    void Copy(Coord*, Coord*, int, Coord*&, Coord*&);
public:
    RubberPointList(
        Painter*, Canvas*, Coord px[], Coord py[], int n,
	Coord offx = 0, Coord offy = 0
    );
    void Delete();
};    

/*
 *  RubberVertex is used to derive pointlist-based rubberbands that track
 *  changes to a single vertex.
 */

class RubberVertex : public RubberPointList {
protected:
    int rubberPt;
    void DrawSplineSection (Painter*, Canvas*, Coord x[], Coord y[]);
public:
    RubberVertex(
        Painter*, Canvas*, Coord px[], Coord py[], int n, int pt,
	Coord offx = 0, Coord offy = 0
    );
    virtual void GetOriginal(Coord*& px, Coord*& py, int& n, int& pt);
    virtual void GetCurrent(Coord*& px, Coord*& py, int& n, int& pt);
};

/*
 * RubberHandles is a set of count filled rectangles (handles) each size units
 * square centered about (x[n], y[n]) for n = {0..count-1}.  The handle defined
 * by (x[rubberPt], y[rubberPt]) is translated to follow the tracking point.
 */

class RubberHandles : public RubberVertex {
protected:
    int d;	/* half of handle size */
public:
    RubberHandles(
        Painter*, Canvas*, Coord x[], Coord y[], int n, int pt, int size,
	Coord offx = 0, Coord offy = 0
    );
    void Track(Coord x, Coord y);
    void Draw();
};

/*
 * A RubberSpline is the section of the open B-spline defined by the control
 * points (x[n], y[n]) for n = {0..count-1} that changes when the control point
 * (x[rubberPt], y[rubberPt]) follows the tracking point.
 */

class RubberSpline : public RubberVertex {
public:
    RubberSpline(
        Painter*, Canvas*, Coord px[], Coord py[], int n, int pt,
	Coord offx = 0, Coord offy = 0
    );
    void Draw();
};

/*
 * A RubberClosedSpline is the section of the closed B-spline defined by the
 * control points (x[n], y[n]) for n = {0..count-1} that changes when the
 * control point (x[rubberPt], y[rubberPt]) follows the tracking point.
 */

class RubberClosedSpline : public RubberVertex {
public:
    RubberClosedSpline(
        Painter*, Canvas*, Coord px[], Coord py[], int n, int pt,
	Coord offx = 0, Coord offy = 0
    );
    void Draw();
};

/*
 *  SlidingPointList is used to derive pointlist-base sliding rubberbands.
 */

class SlidingPointList : public RubberPointList {
protected:
    Coord refx;
    Coord refy;
public:
    SlidingPointList (
        Painter*, Canvas*, Coord px[], Coord py[], int n,
	Coord rfx, Coord rfy, Coord offx = 0, Coord offy = 0
    );
    virtual void GetOriginal(Coord*& px, Coord*& py, int& n);
    virtual void GetCurrent(Coord*& px, Coord*& py, int& n);
    void Draw();
    void Track(Coord x, Coord y);
};

/*
 * SlidingLineList is a set of count-1 lines for which {x[0], y[0], x[1], y[1]}
 * defines the first line, {x[1], y[1], x[2], y[2]} defines the second, and
 * so on up to the last line {x[count-2], y[count-2], x[count-1], y[count-1]}.
 * The lines are translated horizontally by x - rfx and vertically by
 * y - rfy, where (x, y) is the tracking point.
 */

class SlidingLineList : public SlidingPointList {
public:
    SlidingLineList(
        Painter*, Canvas*, Coord x[], Coord y[], int n,
	Coord rfx, Coord rfy, Coord offx = 0, Coord offy = 0
    );
    void Draw();
};    

#endif
