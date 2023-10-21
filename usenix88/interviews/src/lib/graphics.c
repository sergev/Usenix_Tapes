/*
 * Graphics implementation on top of X.
 */

#include "graphics.h"
#include <InterViews/Xfont.h>
#include <string.h>

extern int strlen(const char*);

#ifdef X10

extern int _World_nplanes;

#endif

XPoint _Graphics_vlist[XPointListSize];

void Graphics.Map (Canvas* c, Coord x, Coord y, Coord& mx, Coord& my) {
    if (matrix == nil) {
	mx = x; my = y;
    } else {
	matrix->Transform(x, y, mx, my);
    }
#ifdef X10
    mx += xoff - c->left;
    my = c->height - 1 - (my + yoff - c->bottom);
#else
    mx += xoff;
    my = c->height - 1 - (my + yoff);
#endif
}

void Graphics.Map (Canvas* c, Coord x, Coord y, short& sx, short& sy) {
    Coord mx, my;

    if (matrix == nil) {
	mx = x; my = y;
    } else {
	matrix->Transform(x, y, mx, my);
    }
#ifdef X10
    mx += xoff - c->left;
    my = c->height - 1 - (my + yoff - c->bottom);
#else
    mx += xoff;
    my = c->height - 1 - (my + yoff);
#endif
    sx = short(mx);
    sy = short(my);
}

void Graphics.MapList (
    Canvas* c, Coord x[], Coord y[], int n, Coord mx[], Coord my[]
) {
    register Coord* xp, * yp, * mxp, * myp;
    Coord* lim;

    xp = x; yp = y;
    mxp = mx; myp = my;
    lim = &x[n];
    if (matrix == nil) {
	for (; xp < lim; xp++, yp++, mxp++, myp++) {
#ifdef X10
	    *mxp = *xp + xoff - c->left;
	    *myp = c->height - 1 - (*yp + yoff - c->bottom);
#else
	    *mxp = *xp + xoff;
	    *myp = c->height - 1 - (*yp + yoff);
#endif
	}
    } else {
	for (; xp < lim; xp++, yp++, mxp++, myp++) {
	    matrix->Transform(*xp, *yp, *mxp, *myp);
#ifdef X10
	    *mxp += xoff - c->left;
	    *myp = c->height - 1 - (*myp + yoff - c->bottom);
#else
	    *mxp += xoff;
	    *myp = c->height - 1 - (*myp + yoff);
#endif
	}
    }
}

const float axis = 0.42;
const float seen = 1.025;

void Painter::Ellipse (Canvas* c, Coord cx, Coord cy, int r1, int r2) {
    Coord px1, py1, px2, py2, x[8], y[8];

    px1 = round(float(r1)*axis); py1 = round(float(r2)*axis);
    px2 = round(float(r1)*seen); py2 = round(float(r2)*seen);
    x[0] = cx + px1;	y[0] = cy + py2;
    x[1] = cx - px1;	y[1] = y[0];
    x[2] = cx - px2;	y[2] = cy + py1;
    x[3] = x[2];	y[3] = cy - py1;
    x[4] = x[1];	y[4] = cy - py2;
    x[5] = x[0];	y[5] = y[4];
    x[6] = cx + px2;	y[6] = y[3];
    x[7] = x[6];	y[7] = y[2];
    ClosedBSpline(c, x, y, 8);
}

void Painter::FillEllipse (Canvas* c, Coord cx, Coord cy, int r1, int r2) {
    Coord px1, py1, px2, py2, x[8], y[8];

    px1 = round(float(r1)*axis); py1 = round(float(r2)*axis);
    px2 = round(float(r1)*seen); py2 = round(float(r2)*seen);
    x[0] = cx + px1;	y[0] = cy + py2;
    x[1] = cx - px1;	y[1] = y[0];
    x[2] = cx - px2;	y[2] = cy + py1;
    x[3] = x[2];	y[3] = cy - py1;
    x[4] = x[1];	y[4] = cy - py2;
    x[5] = x[0];	y[5] = y[4];
    x[6] = cx + px2;	y[6] = y[3];
    x[7] = x[6];	y[7] = y[2];
    FillBSpline(c, x, y, 8);
}

void Painter::ArcTo (Canvas* c,
    Coord x0, Coord y0, Coord x1, Coord y1, Coord x2, Coord y2
) {
    register Graphics* p = rep;

    Arc(c, p->curx, p->cury, x0, y0, x1, y1, x2, y2);
    p->curx = x2;
    p->cury = y2;
}

#ifdef X10

void Graphics.Update () {
    if (raster == GXxor) {
	tile = XWhitePixmap;
	fg = 1;
	bg = 0;
	fillbg = true;
    } else {
	fg = foreground->PixelValue();
	bg = background->PixelValue();
	if (tile != XWhitePixmap) {
	    XFreePixmap(tile);
	}
	tile = XMakePixmap(pat, fg, bg);
	if (!fillbg && stipple == nil) {
	    MakeNoBgPixmaps();
	}
    }
}

void Graphics.MakeNoBgPixmaps () {
    if (_World_nplanes == 1) {
	/* first turn off fg bits, leave bg as is */
	clearhi = XMakePixmap(pat, 0, 1);
    } else {
	int mask = 1 << _World_nplanes - 1;
	clearhi = XMakePixmap(pat, mask-1, mask);
	clearlo = XMakePixmap(pat, mask, mask-1);
    }
    /* set fg bits to foreground color */
    stipple = XMakePixmap(pat, fg, 0);
}

void Graphics.FreeNoBgPixmaps () {
    if (stipple != nil) {
	XFreePixmap(stipple);
	XFreePixmap(clearhi);
	if (_World_nplanes > 1) {
	    XFreePixmap(clearlo);
	}
	stipple = nil;
    }
}

void Graphics.DrawTiled (void* id, XPoint* v, int n) {
    if (fillbg) {
	XDrawTiled(id, v, n, tile, raster, _Workstation_planemask);
    } else if (_World_nplanes == 1) {
	/* erase fg bits */
	XDrawTiled(id, v, n, clearhi, GXand, _Workstation_planemask);
	/* fill in fg bits */
	XDrawTiled(id, v, n, stipple, GXor, _Workstation_planemask);
    } else {
	/*
	 * First turn off fg bits, leave bg as is.  Then set fg
	 * bits to foreground color.
	 * To avoid using bogus colors, we'll erase the bits in
	 * two passes with two legal colors.
	 */
	int mask = 1 << _World_nplanes - 1;
	XDrawTiled(id, v, n, clearhi, GXand, mask);
	XDrawTiled(id, v, n, clearlo, GXand, mask-1);
	XDrawTiled(id, v, n, stipple, GXor, _Workstation_planemask);
    }
}

void Painter::SetRaster (int r) {
    register Graphics* p = rep;

    if (r != p->raster) {
	p->raster = r;
	p->Update();
    }
}

void Painter::InvertArea (Canvas* c, Coord x1, Coord y1, Coord x2, Coord y2) {
    register Graphics* p = rep;
    Coord left, bottom, right, top, tmp;

    p->Map(c, x1, y1, left, bottom);
    p->Map(c, x2, y2, right, top);
    if (left > right) {
	tmp = left; left = right; right = tmp;
    }
    if (top > bottom) {
	tmp = bottom; bottom = top; top = tmp;
    }
    XPixFill(
	c->id, left, top, right - left + 1, bottom - top + 1, 1, 0,
	GXxor, _Workstation_planemask
    );
}

void Painter::Text (Canvas* c, const char* s) {
    register Graphics* p = rep;
    Coord x0, y0;
    register int len;

    p->Map(c, p->curx, p->cury + p->font->height - 1, x0, y0);
    len = strlen(s);
    if (p->fillbg) {
	XTextPad(
	    c->id, x0, y0, s, len, p->font->id, 0, 0,
	    p->fg, p->bg, p->raster, _Workstation_planemask
	);
    } else {
	XTextMaskPad(
	    c->id, x0, y0, s, len, p->font->id, 0, 0,
	    p->fg, GXcopy, _Workstation_planemask
	);
    }
    p->curx += p->font->Width(s, len);
}

void Painter::Text (Canvas* c, const char* s, int len) {
    register Graphics* p = rep;
    Coord x0, y0;

    p->Map(c, p->curx, p->cury + p->font->height - 1, x0, y0);
    if (p->fillbg) {
	XTextPad(
	    c->id, x0, y0, s, len, p->font->id, 0, 0,
	    p->fg, p->bg, p->raster, _Workstation_planemask
	);
    } else {
	XTextMaskPad(
	    c->id, x0, y0, s, len, p->font->id, 0, 0,
	    p->fg, GXcopy, _Workstation_planemask
	);
    }
    p->curx += p->font->Width(s, len);
}

void Painter::Point (Canvas* c, Coord x, Coord y) {
    register Graphics* p = rep;
    Coord mx, my;

    p->Map(c, x, y, mx, my);
    XLine(
	c->id, mx, my, mx, my,
	p->br->width, p->br->width, p->fg, p->raster, _Workstation_planemask
    );
}

void Painter::MultiPoint (Canvas* c, Coord x[], Coord y[], int n) {
    register Graphics* p = rep;
    register XPoint* v;
    register int i;

    v = p->AllocPts(n);
    for (i = 0; i < n; i++) {
	p->Map(c, x[i], y[i], v[i].x, v[i].y);
	v[i].flags = VertexDontDraw;
    }
    p->DoDraw(c->id, v, n);
    p->FreePts(v);
}

/*
 * X behaves anomalously, at best, for single lines (in other words,
 * VertexDrawLastPoint doesn't seem to work right), so we simply add
 * the end point on twice.
 */

void Painter::Line (Canvas* c, Coord x1, Coord y1, Coord x2, Coord y2) {
    register Graphics* p = rep;
    XPoint v[3];

    p->Map(c, x1, y1, v[0].x, v[0].y);
    v[0].flags = 0;
    p->Map(c, x2, y2, v[1].x, v[1].y);
    v[1].flags = 0;
    v[2] = v[1];
    v[2].flags = VertexDrawLastPoint;
    p->DoDraw(c->id, v, 3);
}

void Painter::Rect (Canvas* c, Coord x1, Coord y1, Coord x2, Coord y2) {
    register Graphics* p = rep;
    XPoint v[5];

    p->Map(c, x1, y1, v[0].x, v[0].y);	v[0].flags = 0;
    p->Map(c, x2, y1, v[1].x, v[1].y);	v[1].flags = 0;
    p->Map(c, x2, y2, v[2].x, v[2].y);	v[2].flags = 0;
    p->Map(c, x1, y2, v[3].x, v[3].y);  v[3].flags = 0;
    p->Map(c, x1, y1, v[4].x, v[4].y);	v[4].flags = 0;
    p->DoDraw(c->id, v, 5);
}

void Painter::FillRect (Canvas* c, Coord x1, Coord y1, Coord x2, Coord y2) {
    register Graphics* p = rep;
    Coord left, bottom, right, top, tmp, x[4], y[4];
    int w, h;

    if (p->matrix != nil && p->matrix->Rotated() && !p->matrix->Rotated90()) {
	x[0] = x[3] = x1;
	x[1] = x[2] = x2;
	y[0] = y[1] = y1;
	y[2] = y[3] = y2;
	FillPolygon(c, x, y, 4);
    } else {
	p->Map(c, x1, y1, left, bottom); 
	p->Map(c, x2, y2, right, top);
	if (left > right) {
	    tmp = left; left = right; right = tmp;
	}
	if (top > bottom) {
	    tmp = bottom; bottom = top; top = tmp;
	}
	w = right - left + 1;
	h = bottom - top + 1;
	if (p->fillbg) {
	    XTileFill(
		c->id, left, top, w, h,
		p->tile, 0, p->raster, _Workstation_planemask
	    );
	} else if (_World_nplanes == 1) {
	    /* erase fg bits */
	    XTileFill(
		c->id, left, top, w, h, p->clearhi, 0, 
		GXand, _Workstation_planemask
	    );
	    /* fill in fg bits */
	    XTileFill(
		c->id, left, top, w, h, p->stipple, 0, 
		GXor, _Workstation_planemask
	    );
	} else {
	    /*
	     * First turn off fg bits, leave bg as is.  Then set fg
	     * bits to foreground color.
	     * To avoid using bogus colors, we'll erase the bits in
	     * two passes with two legal colors.
	     */
	    int mask = 1 << _World_nplanes - 1;
	    XTileFill(c->id, left, top, w, h, p->clearhi, 0, GXand, mask);
	    XTileFill(c->id, left, top, w, h, p->clearlo, 0, GXand, mask-1);
	    XTileFill(c->id, left, top, w, h, p->stipple, 0, GXor, mask-1);
	}
    }
}

void Painter::ClearRect (Canvas* c, Coord x1, Coord y1, Coord x2, Coord y2) {
    register Graphics* p = rep;
    Coord left, bottom, right, top, tmp;

    p->Map(c, x1, y1, left, bottom);
    p->Map(c, x2, y2, right, top);
    if (left > right) {
	tmp = left; left = right; right = tmp;
    }
    if (top > bottom) {
	tmp = bottom; bottom = top; top = tmp;
    }
    XPixFill(
	c->id, left, top, right - left + 1, bottom - top + 1,
	p->bg, 0, p->raster, _Workstation_planemask
    );
}

void Painter::Circle (Canvas* c, Coord x, Coord y, int r) {
    register Graphics* p = rep;
    XPoint v[5];

    if (p->matrix != nil && p->matrix->Stretched()) {
	Ellipse(c, x, y, r, r);
    } else {
	p->Map(c, x-r, y, v[0].x, v[0].y);
	v[0].flags = VertexStartClosed+VertexCurved;
	p->Map(c, x, y+r, v[1].x, v[1].y);
	v[1].flags = VertexCurved;
	p->Map(c, x+r, y, v[2].x, v[2].y);
	v[2].flags = VertexCurved;
	p->Map(c, x, y-r, v[3].x, v[3].y);
	v[3].flags = VertexCurved;
	p->Map(c, x-r, y, v[4].x, v[4].y);
	v[4].flags = VertexEndClosed+VertexCurved;
	p->DoDraw(c->id, v, 5);
    }
}

void Painter::FillCircle (Canvas* c, Coord x, Coord y, int r) {
    register Graphics* p = rep;
    XPoint v[5];

    if (p->matrix != nil && p->matrix->Stretched()) {
	FillEllipse(c, x, y, r, r);
    } else {
	p->Map(c, x-r, y, v[0].x, v[0].y);
	v[0].flags = VertexStartClosed+VertexCurved;
	p->Map(c, x, y+r, v[1].x, v[1].y);
	v[1].flags = VertexCurved;
	p->Map(c, x+r, y, v[2].x, v[2].y);
	v[2].flags = VertexCurved;
	p->Map(c, x, y-r, v[3].x, v[3].y);
	v[3].flags = VertexCurved;
	p->Map(c, x-r, y, v[4].x, v[4].y);
	v[4].flags = VertexEndClosed+VertexCurved;
	p->DrawTiled(c->id, v, 5);
    }
}

void Painter::MultiLine (Canvas* c, Coord x[], Coord y[], int n) {
    register Graphics* p = rep;
    register XPoint* v;
    register int i;

    v = p->AllocPts(n);
    for (i = 0; i < n; i++) {
	p->Map(c, x[i], y[i], v[i].x, v[i].y);
	v[i].flags = 0;
    }
    p->DoDraw(c->id, v, n);
    p->FreePts(v);
}

void Painter::MultiLineNoMap (Canvas* c, Coord x[], Coord y[], int n) {
    register Graphics* p = rep;
    register XPoint* v;
    register int i;

    v = p->AllocPts(n);
    for (i = 0; i < n; i++) {
	v[i].x = x[i];
	v[i].y = y[i];
	v[i].flags = 0;
    }
    p->DoDraw(c->id, v, n);
    p->FreePts(v);
}

void Painter::Polygon (Canvas* c, Coord x[], Coord y[], int n) {
    register Graphics* p = rep;
    register XPoint* v;
    register int i;

    v = p->AllocPts(n+1);
    for (i = 0; i < n; i++) {
	p->Map(c, x[i], y[i], v[i].x, v[i].y);
	v[i].flags = 0;
    }
    if (x[i-1] != x[0] || y[i-1] != y[0]) {
	v[i] = v[0];
	++i;
    }
    v[0].flags |= VertexStartClosed;
    v[i-1].flags |= VertexEndClosed;
    p->DoDraw(c->id, v, i);
    p->FreePts(v);
}

void Painter::FillPolygonNoMap (Canvas* c, Coord x[], Coord y[], int n) {
    register Graphics* p = rep;
    register XPoint* v;
    register int i;

    v = p->AllocPts(n);
    for (i = 0; i < n; i++) {
	v[i].x = x[i];
	v[i].y = y[i];
	v[i].flags = 0;
    }
    if (x[i-1] != x[0] || y[i-1] != y[0]) {
	v[i] = v[0];
	++i;
    }
    v[0].flags |= VertexStartClosed;
    v[i-1].flags |= VertexEndClosed;
    p->DrawTiled(c->id, v, i);
    p->FreePts(v);
}

void Painter::FillPolygon (Canvas* c, Coord x[], Coord y[], int n) {
    register Graphics* p = rep;
    register XPoint* v;
    register int i;

    v = p->AllocPts(n+1);
    for (i = 0; i < n; i++) {
	p->Map(c, x[i], y[i], v[i].x, v[i].y);
	v[i].flags = 0;
    }
    if (x[i-1] != x[0] || y[i-1] != y[0]) {
	v[i] = v[0];
	++i;
    }
    v[0].flags |= VertexStartClosed;
    v[i-1].flags |= VertexEndClosed;
    p->DrawTiled(c->id, v, i);
    p->FreePts(v);
}

void Painter::Copy (
    Canvas* src, Coord x1, Coord y1, Coord x2, Coord y2,
    Canvas* dst, Coord x0, Coord y0
) {
    register Graphics* p = rep;
    register int w, h;
    Coord sx1, sy1, dx1, dy1;

    w = x2 - x1 + 1;
    h = y2 - y1 + 1;
    p->Map(src, x1, y2, sx1, sy1);
    p->Map(dst, x0, y0+h-1, dx1, dy1);
    if (src->offscreen) {
	if (dst->offscreen) {
	    /* offscreen-to-offscreen unimplemented */
	} else {
	    XPixmapPut(
		dst->id, sx1, sy1, dx1, dy1, w, h,
		src->id, p->raster, _Workstation_planemask
	    );
	}
    } else {
	if (dst->offscreen) {
	    if (dst->id != nil) {
		XFreePixmap(dst->id);
	    }
	    dst->id = XPixmapSave(src->id, sx1, sy1, w, h);
	    dst->width = w;
	    dst->height = h;
	} else {
	    if (src->id == dst->id) {
		XCopyArea(
		    src->id, sx1, sy1, dx1, dy1, w, h,
		    p->raster, _Workstation_planemask
		);
		src->WaitForCopy();
	    } else {
		/* cross-window unimplemented */
	    }
	}
    }
}

void Painter::Read (
    Canvas* c, void* dst, Coord x1, Coord y1, Coord x2, Coord y2
) {
    XPixmapGetXY(c->id, x1, c->height-y2-1, x2-x1+1, y2-y1+1, dst);
}

void Painter::Write (
    Canvas* c, const void* src, Coord x1, Coord y1, Coord x2, Coord y2
) {
    XPixmapBitsPutXY(
	c->id, x1, c->height-y2-1, x2-x1+1, y2-y1+1,
	(short*)src, nil, rep->raster, _Workstation_planemask
    );
}

#else

void Painter::SetRaster (int r) {
    register Graphics* p = rep;

    if (p->raster != r) {
	p->raster = r;
	XSetFunction(_World_dpy, p->gc, r);
	if (r == GXxor) {
	    XSetForeground(_World_dpy, p->gc, 1);
	    XSetFillStyle(_World_dpy, p->gc, FillSolid);
	} else {
	    XSetForeground(_World_dpy, p->gc, p->foreground->PixelValue());
	    XSetFillStyle(_World_dpy, p->gc, p->fillstyle);
	}
    }
}

/*
 * InvertArea is here for backward compatibility with the 1.0 library.
 * It should not be used in general, as it will not work with transformations.
 */

void Painter::InvertArea (Canvas* c, Coord x1, Coord y1, Coord x2, Coord y2) {
    register Graphics* p = rep;
    Coord left, bottom, right, top, tmp;

    p->Map(c, x1, y1, left, bottom);
    p->Map(c, x2, y2, right, top);
    if (left > right) {
	tmp = left; left = right; right = tmp;
    }
    if (top > bottom) {
	tmp = bottom; bottom = top; top = tmp;
    }
    if (p->raster == GXxor) {
	XFillRectangle(
	    _World_dpy, c->id, p->gc, left, top, right-left+1, bottom-top+1
	);
    } else {
	XSetFunction(_World_dpy, p->gc, GXxor);
	XSetForeground(_World_dpy, p->gc, 1);
	XSetFillStyle(_World_dpy, p->gc, FillSolid);
	XFillRectangle(
	    _World_dpy, c->id, p->gc, left, top, right-left+1, bottom-top+1
	);
	XSetFunction(_World_dpy, p->gc, GXcopy);
	XSetForeground(_World_dpy, p->gc, p->foreground->PixelValue());
	XSetFillStyle(_World_dpy, p->gc, p->fillstyle);
    }
}

void Painter::Text (Canvas* c, const char* s) {
    Text(c, s, strlen(s));
}

void Painter::Text (Canvas* c, const char* s, int len) {
    register Graphics* p = rep;
    register XFontStruct* f = (XFontStruct*)(p->font->info);
    Coord x0, y0;

    p->Map(c, p->curx, p->cury + f->descent - 1, x0, y0);
    if (p->fillstyle == FillOpaqueStippled) {
	XDrawImageString(_World_dpy, c->id, p->gc, x0, y0, s, len);
    } else {
	XDrawString(_World_dpy, c->id, p->gc, x0, y0, s, len);
    }
    p->curx += p->font->Width(s, len);
}

void Painter::Point (Canvas* c, Coord x, Coord y) {
    register Graphics* p = rep;
    Coord mx, my;

    p->Map(c, x, y, mx, my);
    XDrawPoint(_World_dpy, c->id, p->gc, mx, my);
}

void Painter::MultiPoint (Canvas* c, Coord x[], Coord y[], int n) {
    register Graphics* p = rep;
    register XPoint* v;
    register int i;

    v = p->AllocPts(n);
    for (i = 0; i < n; i++) {
	p->Map(c, x[i], y[i], v[i].x, v[i].y);
    }
    XDrawPoints(_World_dpy, c->id, p->gc, v, n, CoordModeOrigin);
    p->FreePts(v);
}

void Painter::Line (Canvas* c, Coord x1, Coord y1, Coord x2, Coord y2) {
    register Graphics* p = rep;
    int mx1, my1, mx2, my2;

    p->Map(c, x1, y1, mx1, my1);
    p->Map(c, x2, y2, mx2, my2);
    XSetStipple(_World_dpy, p->gc, p->br->info);
    XDrawLine(_World_dpy, c->id, p->gc, mx1, my1, mx2, my2);
}

void Painter::Rect (Canvas* c, Coord x1, Coord y1, Coord x2, Coord y2) {
    register Graphics* p = rep;
    
    if (p->matrix != nil && p->matrix->Rotated() && !p->matrix->Rotated90()) {
	Coord x[4], y[4];

	x[0] = x[3] = x1;
	x[1] = x[2] = x2;
	y[0] = y[1] = y1;
	y[2] = y[3] = y2;
	Polygon(c, x, y, 4);
    } else {
	Coord left, bottom, right, top, tmp;
	int w, h;

	p->Map(c, x1, y1, left, bottom);
	p->Map(c, x2, y2, right, top);
	if (left > right) {
	    tmp = left; left = right; right = tmp;
	}
	if (top > bottom) {
	    tmp = bottom; bottom = top; top = tmp;
	}
	w = right - left;
	h = bottom - top;
	XSetStipple(_World_dpy, p->gc, p->br->info);
	XDrawRectangle(_World_dpy, c->id, p->gc, left, top, w, h);
    }
}

void Painter::FillRect (Canvas* c, Coord x1, Coord y1, Coord x2, Coord y2) {
    register Graphics* p = rep;

    if (p->matrix != nil && p->matrix->Rotated() && !p->matrix->Rotated90()) {
	Coord x[4], y[4];

	x[0] = x[3] = x1;
	x[1] = x[2] = x2;
	y[0] = y[1] = y1;
	y[2] = y[3] = y2;
	FillPolygon(c, x, y, 4);
    } else {
	Coord left, bottom, right, top, tmp;
	int w, h;

	p->Map(c, x1, y1, left, bottom);
	p->Map(c, x2, y2, right, top);
	if (left > right) {
	    tmp = left; left = right; right = tmp;
	}
	if (top > bottom) {
	    tmp = bottom; bottom = top; top = tmp;
	}
	w = right - left + 1;
	h = bottom - top + 1;
	XSetStipple(_World_dpy, p->gc, p->pattern->info);
	XFillRectangle(_World_dpy, c->id, p->gc, left, top, w, h);
    }
}

void Painter::ClearRect (Canvas* c, Coord x1, Coord y1, Coord x2, Coord y2) {
    register Graphics* p = rep;
    Pattern* curpat;

    XSetForeground(_World_dpy, p->gc, p->background->PixelValue());
    XSetBackground(_World_dpy, p->gc, p->foreground->PixelValue());
    curpat = p->pattern;
    p->pattern = solid;
    FillRect(c, x1, y1, x2, y2);
    XSetForeground(_World_dpy, p->gc, p->foreground->PixelValue());
    XSetBackground(_World_dpy, p->gc, p->background->PixelValue());
    p->pattern = curpat;
}

void Painter::Circle (Canvas* c, Coord x, Coord y, int r) {
    register Graphics* p = rep;
    int left, top, d;

    if (p->matrix != nil && p->matrix->Stretched()) {
	Ellipse(c, x, y, r, r);
    } else {
	p->Map(c, x-r, y+r, left, top);
	d = r+r;
	XSetStipple(_World_dpy, p->gc, p->br->info);
	XDrawArc(_World_dpy, c->id, p->gc, left, top, d, d, 0, 360*64);
    }
}

void Painter::FillCircle (Canvas* c, Coord x, Coord y, int r) {
    register Graphics* p = rep;
    int left, top, d;

    if (p->matrix != nil && p->matrix->Stretched()) {
	FillEllipse(c, x, y, r, r);
    } else {
	p->Map(c, x-r, y+r, left, top);
	d = r+r;
	XSetStipple(_World_dpy, p->gc, p->pattern->info);
	XFillArc(_World_dpy, c->id, p->gc, left, top, d, d, 0, 360*64);
    }
}

void Painter::MultiLine (Canvas* c, Coord x[], Coord y[], int n) {
    register Graphics* p = rep;
    register XPoint* v;
    register int i;

    v = p->AllocPts(n);
    for (i = 0; i < n; i++) {
	p->Map(c, x[i], y[i], v[i].x, v[i].y);
    }
    XSetStipple(_World_dpy, p->gc, p->br->info);
    XDrawLines(_World_dpy, c->id, p->gc, v, n, CoordModeOrigin);
    p->FreePts(v);
}

void Painter::MultiLineNoMap (Canvas* c, Coord x[], Coord y[], int n) {
    register Graphics* p = rep;
    register XPoint* v;
    register int i;

    v = p->AllocPts(n);
    for (i = 0; i < n; i++) {
	v[i].x = x[i];
	v[i].y = y[i];
    }
    XSetStipple(_World_dpy, p->gc, p->br->info);
    XDrawLines(_World_dpy, c->id, p->gc, v, n, CoordModeOrigin);
    p->FreePts(v);
}

void Painter::Polygon (Canvas* c, Coord x[], Coord y[], int n) {
    register Graphics* p = rep;
    register XPoint* v;
    register int i;

    v = p->AllocPts(n+1);
    for (i = 0; i < n; i++) {
	p->Map(c, x[i], y[i], v[i].x, v[i].y);
    }
    if (x[i-1] != x[0] || y[i-1] != y[0]) {
	v[i] = v[0];
	++i;
    }
    XSetStipple(_World_dpy, p->gc, p->br->info);
    XDrawLines(_World_dpy, c->id, p->gc, v, i, CoordModeOrigin);
    p->FreePts(v);
}

void Painter::FillPolygonNoMap (Canvas* c, Coord x[], Coord y[], int n) {
    register Graphics* p = rep;
    register XPoint* v;
    register int i;

    v = p->AllocPts(n);
    for (i = 0; i < n; i++) {
	v[i].x = x[i];
	v[i].y = y[i];
    }
    XSetStipple(_World_dpy, p->gc, p->pattern->info);
    XFillPolygon(_World_dpy, c->id, p->gc, v, n, Complex, CoordModeOrigin);
    p->FreePts(v);
}

void Painter::FillPolygon (Canvas* c, Coord x[], Coord y[], int n) {
    register Graphics* p = rep;
    register XPoint* v;
    register int i;

    v = p->AllocPts(n+1);
    for (i = 0; i < n; i++) {
	p->Map(c, x[i], y[i], v[i].x, v[i].y);
    }
    XSetStipple(_World_dpy, p->gc, p->pattern->info);
    XFillPolygon(_World_dpy, c->id, p->gc, v, n, Complex, CoordModeOrigin);
    p->FreePts(v);
}

void Painter::Copy (
    Canvas* src, Coord x1, Coord y1, Coord x2, Coord y2,
    Canvas* dst, Coord x0, Coord y0
) {
    register Graphics* p = rep;
    register int w, h;
    Coord sx1, sy1, dx1, dy1;

    w = x2 - x1 + 1;
    h = y2 - y1 + 1;
    p->Map(src, x1, y2, sx1, sy1);
    p->Map(dst, x0, y0+h-1, dx1, dy1);
    XCopyArea(_World_dpy, src->id, dst->id, p->gc, sx1, sy1, w, h, dx1, dy1);
    dst->WaitForCopy();
}

void Painter::Read (
    Canvas* c, void* dst, Coord x1, Coord y1, Coord x2, Coord y2
) {
    /* unimplemented */
}

void Painter::Write (
    Canvas* c, const void* src, Coord x1, Coord y1, Coord x2, Coord y2
) {
    /* unimplemented */
}

#endif
