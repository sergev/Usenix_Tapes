/*
 * Banner implementation.
 */

#include <InterViews/banner.h>
#include <InterViews/shape.h>
#include <InterViews/painter.h>

const int pad = 2*pixels;	/* space around banner text */

Banner::Banner (Painter* out, char* lt, char* m, char* rt) : (stdsensor, out) {
    left = lt;
    middle = m;
    right = rt;
    highlight = false;
    inverse = new Painter(output);
    Init();
}

void Banner::Delete () {
    delete inverse;
}

void Banner::Init () {
    register Font* f;
    register int w;

    f = output->GetFont();
    lw = left == nil ? 0 : f->Width(left);
    mw = middle == nil ? 0 : f->Width(middle);
    rw = right == nil ? 0 : f->Width(right);
    if (mw > 0) {
	w = mw + 2*max(lw, rw);
    } else {
	w = lw + rw;
    }
    shape->width = 2*pad + w + f->Width("    ");
    shape->height = f->Height() + 2*pad;
    shape->Rigid(0, hfil, 0, 0);
    inverse->SetColors(output->GetBgColor(), output->GetFgColor());
}

void Banner::Draw () {
    Redraw(0, 0, xmax, ymax);
}

void Banner::Redraw (Coord x1, Coord y1, Coord x2, Coord y2) {
    register Painter* p;

    p = highlight ? inverse : output;
    p->ClearRect(canvas, x1, y1, x2, y2);
    if (right != nil && rx <= x2) {
	p->MoveTo(rx, pad);
	p->Text(canvas, right);
    }
    if (middle != nil && mx + mw >= x1 && mx <= x2) {
	p->MoveTo(mx, pad);
	p->Text(canvas, middle);
    }
    if (left != nil && lx + lw >= x1) {
	p->MoveTo(lx, pad);
	p->Text(canvas, left);
    }
}

void Banner::Resize () {
    lx = pad;
    mx = (xmax - mw) / 2;
    rx = xmax - rw + 1 - pad;
}

void Banner::Update () {
    Init();
    Resize();
    Redraw(0, 0, xmax, ymax);
}
