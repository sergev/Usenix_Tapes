/*
 * Rubberbanding for simple lines.
 */

#ifndef rubline_h
#define rubline_h

#include <InterViews/rubband.h>

/*
 * A RubberLine is a line with one endpoint (x0, y0) fixed while the other
 * (x1, y1) follows the tracking point (x, y).
 */

class RubberLine : public Rubberband {
protected:
    Coord fixedx, fixedy;
    Coord movingx, movingy;
public:
    RubberLine(
        Painter*, Canvas*, Coord x0, Coord y0, Coord x1, Coord y1,
	Coord offx = 0, Coord offy = 0
    );
    virtual void GetOriginal(Coord& x0, Coord& y0, Coord& x1, Coord& y1);
    virtual void GetCurrent(Coord& x0, Coord& y0, Coord& x1, Coord& y1);
    void Draw();
};

/*
 * A RubberAxis is either a horizontal or vertical line with one endpoint 
 * (x0, y0) fixed while the other (x1, y1) follows the horizontal or
 * vertical component of the tracking point (x, y).  The line is vertical if 
 * | x0 - x | < | y0 - y |; otherwise it is horizontal.
 */

class RubberAxis : public RubberLine {
public:
    RubberAxis(
        Painter*, Canvas*, Coord x0, Coord y0, Coord x1, Coord y1,
	Coord offx = 0, Coord offy = 0
    );
    void GetCurrent(Coord& x0, Coord& y0, Coord& x1, Coord& y1);
};

/*
 * A SlidingLine is a line with endpoints (x0, y0) and
 * (x1, y1) that is translated horizontally by x - rfx and vertically by 
 * y - rfy, where (x, y) is the tracking point.
 */

class SlidingLine : public RubberLine {
protected:
    Coord refx;
    Coord refy;
public:
    SlidingLine(
	Painter*, Canvas*, Coord x0, Coord y0, Coord x1, Coord y1,
	Coord rfx, Coord rfy, Coord offx = 0, Coord offy = 0
    );
    void GetCurrent(Coord& x0, Coord& y0, Coord& x1, Coord& y1);
};

/*
 * A RotatingLine is a line with endpoints (x0, y0) and (x1, y1)
 * that is rotated about (cx, cy).  Rotation is based on the angle between two
 * radii:  the one extending from (cx, cy) to (rfx, rfy), and the one from
 * (cx, cy) to the tracking point.
 */

class RotatingLine : public RubberLine {
protected:
    Coord centerx, centery, refx, refy;
    void Transform (
	Coord& x, Coord& y,
	double a0, double a1, double b0, double b1, double c0, double c1
    );
public:
    RotatingLine(
	Painter*, Canvas*, Coord x0, Coord y0, Coord x1, Coord y1, 
	Coord cx, Coord cy, Coord rfx, Coord rfy, 
	Coord offx = 0, Coord offy = 0
    );
    void GetCurrent(Coord& x0, Coord& y0, Coord& x1, Coord& y1);
    float OriginalAngle();
    float CurrentAngle();
};

#endif
