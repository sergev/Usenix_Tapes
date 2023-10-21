/*
 * Graphics state.
 */

#ifndef graphics_h
#define graphics_h

#include <InterViews/painter.h>
#include <InterViews/canvas.h>
#include <InterViews/Xoutput.h>

#ifdef X10
extern int _Workstation_planemask;

typedef XVertex XPoint;
#endif

class Graphics {
    friend class Painter;

    Color* foreground;
    Color* background;
    Pattern* pattern;
    Brush* br;
    Font* font;
    Coord curx, cury;
    int xoff, yoff;
    Transformer* matrix;
    int raster;
#ifdef X10
    int fg, bg;
    boolean fillbg;
    XBitmap pat;
    XPixmap tile;
    XPixmap clearhi, clearlo, stipple;
    Canvas* curcanvas;
#else
    Xgc gc;
    int fillstyle;
#endif
public:
    XPoint* AllocPts(int n);
    void FreePts(XPoint*);
    void Map(Canvas*, Coord x, Coord y, Coord& mx, Coord& my);
    void Map(Canvas*, Coord x, Coord y, short& mx, short& my);
    void MapList(Canvas*, Coord x[], Coord y[], int n, Coord mx[], Coord my[]);
#ifdef X10
    void DoDraw(XWindow, XPoint v[], int);
    void Update();
    void MakeNoBgPixmaps();
    void FreeNoBgPixmaps();
    void DrawTiled(void*, XPoint*, int);
#endif
};

const XPointListSize = 200;

extern XPoint _Graphics_vlist[];

inline XPoint* Graphics.AllocPts (int n) {
    return (n <= XPointListSize) ? _Graphics_vlist : new XPoint[n];
}

inline void Graphics.FreePts (XPoint* v) {
    if (v != _Graphics_vlist) {
	delete v;
    }
}

#ifdef X10

inline void Graphics.DoDraw (XWindow w, XPoint v[], int n) {
    if (br == single) {
	XDraw(w, v, n, br->width, br->width, fg,
	    raster, _Workstation_planemask
	);
    } else if (fillbg) {
	XDrawPatterned(
	    w, v, n, br->width, br->width, fg, bg,
	    (int)(br->info), raster, _Workstation_planemask
	);
    } else {
	XDrawDashed(
	    w, v, n, br->width, br->width, fg,
	    (int)(br->info), raster, _Workstation_planemask
	);
    }
}

#endif

#endif
