/*
 * Spline drawing.
 */

#include <InterViews/painter.h>
#include "graphics.h"

inline int abs (int a) {
    return a > 0 ? a : -a;
}

const int INITBUFSIZE = 100;
const double SMOOTHNESS = 1.0;

static int llsize = 0;
static int llcount = 0;
static Coord* llx;
static Coord* lly;

extern void bcopy(void*, void*, int);

static void GrowBufs (Coord*& b1, Coord*& b2, int& cur) {
    Coord* newb1;
    Coord* newb2;
    int newsize;

    if (cur == 0) {
        cur = INITBUFSIZE;
	b1 = new Coord[INITBUFSIZE];
	b2 = new Coord[INITBUFSIZE];
    } else {
	newsize = cur * 2;
	newb1 = new Coord[newsize];
	newb2 = new Coord[newsize];
	bcopy(b1, newb1, newsize * sizeof(Coord));
	bcopy(b2, newb2, newsize * sizeof(Coord));
	delete b1;
	delete b2;
	b1 = newb1;
	b2 = newb2;
	cur = newsize;
    }
}

inline void Midpoint (
    double x0, double y0, double x1, double y1, double& mx, double& my
) {
    mx = (x0 + x1) / 2.0;
    my = (y0 + y1) / 2.0;
}

inline void ThirdPoint (
    double x0, double y0, double x1, double y1, double& tx, double& ty
) {
    tx = (2*x0 + x1) / 3.0;
    ty = (2*y0 + y1) / 3.0;
}

inline boolean CanApproxWithLine (
    double x0, double y0, double x2, double y2, double x3, double y3
) {
    double triangleArea, sideSquared, dx, dy;
    
    triangleArea = x0*y2 - x2*y0 + x2*y3 - x3*y2 + x3*y0 - x0*y3;
    triangleArea *= triangleArea;	// actually 4 times the area
    dx = x3 - x0;
    dy = y3 - y0;
    sideSquared = dx*dx + dy*dy;
    return triangleArea <= SMOOTHNESS * sideSquared;
}

inline void AddLine (double x0, double y0, double x1, double y1) {
    if (llcount >= llsize) {
	GrowBufs(llx, lly, llsize);
    }

    if (llcount == 0) {
	llx[llcount] = round(x0);
	lly[llcount] = round(y0);
	++llcount;
    }
    llx[llcount] = round(x1);
    lly[llcount] = round(y1);
    ++llcount;
}

static void AddBezierArc (
    double x0, double y0, double x1, double y1,
    double x2, double y2, double x3, double y3
) {
    double midx01, midx12, midx23, midlsegx, midrsegx, cx;
    double midy01, midy12, midy23, midlsegy, midrsegy, cy;
    
    Midpoint(x0, y0, x1, y1, midx01, midy01);
    Midpoint(x1, y1, x2, y2, midx12, midy12);
    Midpoint(x2, y2, x3, y3, midx23, midy23);
    Midpoint(midx01, midy01, midx12, midy12, midlsegx, midlsegy);
    Midpoint(midx12, midy12, midx23, midy23, midrsegx, midrsegy);
    Midpoint(midlsegx, midlsegy, midrsegx, midrsegy, cx, cy);    

    if (CanApproxWithLine(x0, y0, midlsegx, midlsegy, cx, cy)) {
        AddLine(x0, y0, cx, cy);
    } else if (
        (midx01 != x1) || (midy01 != y1) ||
	(midlsegx != x2) || (midlsegy != y2) ||
	(cx != x3) || (cy != y3)
    ) {    
        AddBezierArc(
	    x0, y0, midx01, midy01, midlsegx, midlsegy, cx, cy
	);
    }
    if (CanApproxWithLine(cx, cy, midx23, midy23, x3, y3)) {
        AddLine(cx, cy, x3, y3);
    } else if (
        (cx != x0) || (cy != y0) ||
	(midrsegx != x1) || (midrsegy != y1) ||
	(midx23 != x2) || (midy23 != y2)
    ) {        
        AddBezierArc(
	    cx, cy, midrsegx, midrsegy, midx23, midy23, x3, y3
	);
    }
}

static void CalcBSpline (
    Coord cminus1x, Coord cminus1y, Coord cx, Coord cy,
    Coord cplus1x, Coord cplus1y, Coord cplus2x, Coord cplus2y
) {
    double p0x, p1x, p2x, p3x, tempx,
	   p0y, p1y, p2y, p3y, tempy;
    
    ThirdPoint(
        double(cx), double(cy), double(cplus1x), double(cplus1y), p1x, p1y
    );
    ThirdPoint(
        double(cplus1x), double(cplus1y), double(cx), double(cy), p2x, p2y
    );
    ThirdPoint(
        double(cx), double(cy), double(cminus1x), double(cminus1y),
	tempx, tempy
    );
    Midpoint(tempx, tempy, p1x, p1y, p0x, p0y);
    ThirdPoint(
        double(cplus1x), double(cplus1y), double(cplus2x), double(cplus2y),
	tempx, tempy
    );
    Midpoint(tempx, tempy, p2x, p2y, p3x, p3y);
    
    AddBezierArc(p0x, p0y, p1x, p1y, p2x, p2y, p3x, p3y);
}

inline void CreateOpenLineList (Coord *cpx, Coord *cpy, int cpcount) {
    int cpi;
    
    llcount = 0;
    CalcBSpline(
	cpx[0], cpy[0], cpx[0], cpy[0], cpx[0], cpy[0], cpx[1], cpy[1]
    );
    CalcBSpline(
	cpx[0], cpy[0], cpx[0], cpy[0], cpx[1], cpy[1], cpx[2], cpy[2]
    );

    for (cpi = 1; cpi < cpcount - 2; ++cpi) {
	CalcBSpline(
	    cpx[cpi - 1], cpy[cpi - 1], cpx[cpi], cpy[cpi],
	    cpx[cpi + 1], cpy[cpi + 1], cpx[cpi + 2], cpy[cpi + 2]
        );
    }
    CalcBSpline(
	cpx[cpi - 1], cpy[cpi - 1], cpx[cpi], cpy[cpi],
	cpx[cpi + 1], cpy[cpi + 1], cpx[cpi + 1], cpy[cpi + 1]
    );
    CalcBSpline(
	cpx[cpi], cpy[cpi], cpx[cpi + 1], cpy[cpi + 1],
	cpx[cpi + 1], cpy[cpi + 1], cpx[cpi + 1], cpy[cpi + 1]
    );
}

inline void CreateClosedLineList (Coord *cpx, Coord *cpy, int cpcount) {
    int cpi;
    
    llcount = 0;
    CalcBSpline(
	cpx[cpcount - 1], cpy[cpcount - 1], cpx[0], cpy[0], 
	cpx[1], cpy[1], cpx[2], cpy[2]
    );

    for (cpi = 1; cpi < cpcount - 2; ++cpi) {
	CalcBSpline(
	    cpx[cpi - 1], cpy[cpi - 1], cpx[cpi], cpy[cpi],
	    cpx[cpi + 1], cpy[cpi + 1], cpx[cpi + 2], cpy[cpi + 2]
        );
    }
    CalcBSpline(
	cpx[cpi - 1], cpy[cpi - 1], cpx[cpi], cpy[cpi],
	cpx[cpi + 1], cpy[cpi + 1], cpx[0], cpy[0]
    );
    CalcBSpline(
	cpx[cpi], cpy[cpi], cpx[cpi + 1], cpy[cpi + 1],
	cpx[0], cpy[0], cpx[1], cpy[1]
    );
}

static int bufsize = 0;
static Coord* bufx, * bufy;

static void CheckBufs (Coord*& b1, Coord*& b2, int& cur, int desired) {
    if (cur < desired) {
	if (cur == 0) {
	    cur = max(INITBUFSIZE, desired);
	} else {
	    delete b1;
	    delete b2;
	    cur = max(cur * 2, desired);
	}
	b1 = new Coord[cur];
	b2 = new Coord[cur];
    }
}

void Painter::Arc (
    Canvas* c, Coord x0, Coord y0, Coord x1, Coord y1,
    Coord x2, Coord y2, Coord x3, Coord y3
) {
    register Graphics* p = rep;
    Coord tx0, ty0, tx1, ty1, tx2, ty2, tx3, ty3;
    
    llcount = 0;
    p->Map(c, x0, y0, tx0, ty0);
    p->Map(c, x1, y1, tx1, ty1);
    p->Map(c, x2, y2, tx2, ty2);
    p->Map(c, x3, y3, tx3, ty3);
    AddBezierArc(tx0, ty0, tx1, ty1, tx2, ty2, tx3, ty3);
    MultiLineNoMap(c, llx, lly, llcount);
}

void Painter::BSpline (Canvas* c, Coord x[], Coord y[], int count) {
    register Graphics* p = rep;

    CheckBufs(bufx, bufy, bufsize, count);
    p->MapList(c, x, y, count, bufx, bufy);
    CreateOpenLineList(bufx, bufy, count);
    MultiLineNoMap(c, llx, lly, llcount);
}

void Painter::ClosedBSpline (Canvas* c, Coord x[], Coord y[], int count) {
    register Graphics* p = rep;

    CheckBufs(bufx, bufy, bufsize, count);
    p->MapList(c, x, y, count, bufx, bufy);
    if (count < 3) {
        MultiLineNoMap(c, bufx, bufy, count);
    } else {
        CreateClosedLineList(bufx, bufy, count);
        MultiLineNoMap(c, llx, lly, llcount);
    }
}

void Painter::FillBSpline (Canvas* c, Coord x[], Coord y[], int count) {
    register Graphics* p = rep;

    CheckBufs(bufx, bufy, bufsize, count);
    p->MapList(c, x, y, count, bufx, bufy);
    if (count < 3) {
        FillPolygonNoMap(c, bufx, bufy, count);
    } else {
        CreateClosedLineList(bufx, bufy, count);
        FillPolygonNoMap(c, llx, lly, llcount);
    }
}
