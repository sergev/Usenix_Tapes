/*
 * Rubberbanding for rectangles.
 */

#ifndef rubrect_h
#define rubrect_h

#include <InterViews/rubband.h>

/*
 * A RubberRect is a rectangle with one corner (x0, y0) fixed while the other
 * (x1, y1) follows the tracking point (x, y).
 */

class RubberRect : public Rubberband {
protected:
    Coord fixedx, fixedy;
    Coord movingx, movingy;
public:
    RubberRect(
        Painter*, Canvas*, Coord x0, Coord y0, Coord x1, Coord y1,
	Coord offx = 0, Coord offy = 0
    );
    virtual void GetOriginal(Coord& x0, Coord& y0, Coord& x1, Coord& y1);
    virtual void GetCurrent(Coord& x0, Coord& y0, Coord& x1, Coord& y1);
    void Draw();
};

/*
 * A SlidingRect is a rectangle with opposing corners (x0, y0) and (x1, y1)
 * that is translated horizontally by x - rfx and vertically by y - rfy, 
 * where (x, y) is the tracking point.
 */

class SlidingRect : public RubberRect {
protected:
    Coord refx;
    Coord refy;
public:
    SlidingRect(
        Painter*, Canvas*, Coord x0, Coord y0, Coord x1, Coord y1, 
	Coord rfx, Coord rfy, Coord offx = 0, Coord offy = 0
    );
    void GetCurrent(Coord& x0, Coord& y0, Coord& x1, Coord& y1);
};

/*
 * A StretchingRect is a rectangle with opposing corners (x0, y0) and (x1, y1)
 * whose side s follows the tracking point.
 */

class StretchingRect : public RubberRect {
protected:
    Side side;
public:
    StretchingRect (
        Painter*, Canvas*, Coord x0, Coord y0, Coord x1, Coord y1, Side s,
	Coord offx = 0, Coord offy = 0
    );
    void GetCurrent(Coord& x0, Coord& y0, Coord& x1, Coord& y1);
};


/* 
 * A ScalingRect is a rectangle with opposing corners (x0, y0) and (x1, y1)
 * that is scaled equally in x and y about (cx, cy) so that an edge intersects
 * the tracking point.
 */

class ScalingRect : public RubberRect {
protected:
    Coord centerx, centery;
    int width, height;
public:
    ScalingRect(
        Painter*, Canvas*, Coord x0, Coord y0, Coord x1, Coord y1,
	Coord cx, Coord cy, Coord offx = 0, Coord offy = 0
    );
    void GetCurrent(Coord& x0, Coord& y0, Coord& x1, Coord& y1);
    float CurrentScaling();
};

/*
 * A RotatingRect is a rectangle with opposing corners (x0, y0) and (x1, y1)
 * that is rotated about (cx, cy).  Rotation is based on the angle between two
 * radii:  the one extending from (cx, cy) to (rfx, rfy), and the one from
 * (cx, cy) to the tracking point.
 */

class RotatingRect : public Rubberband {
protected:
    Coord left, right, centerx, refx;
    Coord bottom, top, centery, refy;
    void Transform (
	Coord& x, Coord& y,
	double a0, double a1, double b0, double b1, double c0, double c1
    );
public:
    RotatingRect(
        Painter*, Canvas*, Coord x0, Coord y0, Coord x1, Coord y1, 
	Coord cx, Coord cy, Coord rfx, Coord rfy, 
	Coord offx = 0, Coord offy = 0
    );
    virtual void GetOriginal(Coord& x0, Coord& y0, Coord& x1, Coord& y1);
    virtual void GetCurrent(
        Coord& leftbotx, Coord& leftboty,
	Coord& rightbotx, Coord& rightboty,
	Coord& righttopx, Coord& righttopy,
	Coord& lefttopx, Coord& lefttopy
    );
    float CurrentAngle();
    void Draw();
};

#endif
