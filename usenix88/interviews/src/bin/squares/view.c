/*
 * Implementation of squares view.
 */

#include "subject.h"
#include "view.h"
#include <InterViews/shape.h>
#include <InterViews/perspective.h>
#include <InterViews/painter.h>

SquaresView::SquaresView (Squares* s, Painter* out) : (nil, out) {
    subject = s;
    subject->Attach(this);
    perspective = new Perspective;
    perspective->Init(0, 0, 100, 100);
    perspective->curwidth = 100;
    perspective->curheight = 100;
    perspective->sx = 1;
    perspective->sy = 1;
    perspective->lx = 5;
    perspective->ly = 5;
    shape->width = 200;
    shape->height = 200;
}

void SquaresView::Delete () {
    subject->Detach(this);
}

void SquaresView::Redraw (Coord x1, Coord y1, Coord x2, Coord y2) {
    register SquareData* s;
    register int size;
    register Perspective* p;
    Coord x0c, y0c, xstart, ystart;
    int pwidth, pheight;
    int pxmax, pymax;

    p = perspective;
    pwidth = p->width - p->x0;
    pheight = p->height - p->y0;
    pxmax = round(xmax*(float(pwidth)/p->curwidth));
    pymax = round(ymax*(float(pheight)/p->curheight));
    xstart = round((float(p->curx - p->x0)/pwidth)*pxmax);
    ystart = round((float(p->cury - p->y0)/pheight)*pymax);
    output->ClearRect(canvas, x1, y1, x2, y2);
    for (s = subject->Head(); s != nil; s = s->next) {
	size = min(round(pxmax*s->size), round(pymax*s->size))/2;
	x0c = round(pxmax*s->cx - xstart);
	y0c = round(pymax*s->cy - ystart);
	output->Rect(canvas, x0c - size, y0c - size, x0c + size, y0c + size);
    }
}


void SquaresView::Zoom (int dir) {
    register Perspective* p = perspective;
    int xscrollamt = round(p->width/6.0);
    int yscrollamt = round(p->height/6.0);
    int xscrollinc = round(xscrollamt/2.0);
    int yscrollinc = round(yscrollamt/2.0);

    p->curwidth += dir*xscrollamt;
    p->curx -= dir*xscrollinc;
    p->curheight += dir*yscrollamt;
    p->cury -= dir*yscrollinc;

    if (p->curx < 0) {
	p->curwidth += p->curx;
	p->curx = 0;
    }
    if (p->curwidth < xscrollamt) {
	p->curx += ((p->curwidth - xscrollamt)/2);
	p->curwidth =  xscrollamt;
    }
    if (p->curx +  p->curwidth > p->width) {
	p->curwidth = p->width - p->curx;
    }
    if (p->cury < 0) {
	p->curheight += p->cury;
	p->cury = 0;
    }
    if (p->curheight < yscrollamt) {
	p->cury += ((p->curheight - yscrollamt)/2);
	p->curheight =  yscrollamt;
    }
    if (p->cury +  p->curheight > p->height) {
	p->curheight = p->height - p->cury;
    }
    p->Update();
    Update();
}

void SquaresView::Adjust (register Perspective& n) {
    register Perspective* p = perspective;

    if (p->curx != n.curx || p->cury != n.cury ||
	p->curwidth != n.curwidth || p->curheight != n.curheight
    ) {
	*p = n;
	Draw();
    }
}

void SquaresView::Update () {
    Draw();
}
