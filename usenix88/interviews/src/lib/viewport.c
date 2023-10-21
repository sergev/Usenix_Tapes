/*
 * A viewport contains another interactor whose position is determined
 * by the viewport's perspective.
 */

#include <InterViews/canvas.h>
#include <InterViews/painter.h>
#include <InterViews/perspective.h>
#include <InterViews/shape.h>
#include <InterViews/viewport.h>

/*
 * Create a viewport on another interactor with a particular alignment.
 */

Viewport::Viewport (
    Sensor* in, Painter* out, Interactor* i, ViewAlignment a
) : (in, out) {
    align = a;
    shape->Rigid(hfil, hfil, vfil, vfil);
    perspective = new Perspective;
    perspective->sx = 1;
    perspective->sy = 1;
    perspective->lx = 10;
    perspective->ly = 10;
    Propagate(false);
    if (i != nil) {
	Insert(i);
    }
}

void Viewport::Delete () {
    delete perspective;
    MonoScene::Delete();
}

/*
 * When the component changes, replace it according to the new shape.
 * The viewport's shape does not change; thus, the change is not
 * propagated further.
 */

void Viewport::DoChange (Interactor*, Shape* s) {
    shape->width = s->width;
    shape->height = s->height;
}

void Viewport::DoMove (Interactor*, Coord& x, Coord& y) {
    perspective->curx = x;
    perspective->cury = y;
}

/*
 * Always give the component its natural size and position
 * it according to our perspective and alignment.
 */

void Viewport::Resize () {
    register Perspective* p = perspective;
    Coord x, y;
    int w, h;

    canvas->SetBackground(output->GetBgColor());
    w = component->shape->width;
    h = component->shape->height;
    switch (align) {
	case LowerLeftView:
	    x = 0;
	    y = 0;
	    break;
	case LowerRightView:
	    x = xmax - w + 1;
	    y = 0;
	    break;
	case UpperLeftView:
	    x = 0;
	    y = ymax - h + 1;
	    break;
	case UpperRightView:
	    x = xmax - w + 1;
	    y = ymax - h + 1;
	    break;
	case MiddleRightView:
	    x = xmax - w + 1;
	    y = (ymax - h + 1)/2;
	    break;
	case MiddleLeftView:
	    x = 0;
	    y = (ymax - h + 1)/2;
	    break;
	case UpperMiddleView:
	    x = (xmax - w + 1)/2;
	    y = ymax - h + 1;
	    break;
	case LowerMiddleView:
	    x = (xmax - w + 1)/2;
	    y = 0;
	    break;
	case CenterView:
	    x = (xmax - w + 1)/2;
	    y = (ymax - h + 1)/2;
	    break;
    }
    p->width = w;
    p->curwidth = xmax+1;
    p->curx = -x;
    p->height = h;
    p->curheight = ymax+1;
    p->cury = -y;
    Place(component, x, y, x + w - 1, y + h - 1);
}

void Viewport::Redraw (Coord x1, Coord y1, Coord x2, Coord y2) {
    output->ClearRect(canvas, x1, y1, x2, y2);
}

/*
 * Adjust places the component according to the requested perspective.
 */

void Viewport::Adjust (Perspective& np) {
    register Perspective* p = perspective;
    register Shape* s = component->shape;
    Coord x, y;

    if (p->width > p->curwidth) {
	p->curx = np.curx;
	p->cury = np.cury;
	p->Update();
	x = -(p->curx);
	y = -(p->cury);
	Place(component, x, y, x + s->width - 1, y + s->height - 1);
#       ifdef X10
	    component->Draw();
#       endif
    }
}
