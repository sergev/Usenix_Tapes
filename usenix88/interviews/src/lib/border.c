/*
 * Border implementation.
 */

#include <InterViews/border.h>
#include <InterViews/painter.h>
#include <InterViews/shape.h>

Border::Border (Painter* out) : (stdsensor, out) {
}

void Border::Delete () {
    /* nothing to do */
}

void Border::Draw () {
    output->FillRect(canvas, 0, 0, xmax, ymax);
}

void Border::Redraw (Coord x1, Coord y1, Coord x2, Coord y2) {
    output->FillRect(canvas, x1, y1, x2, y2);
}

HBorder::HBorder (Painter* out, int t) : (out) {
    shape->height = t;
    shape->Rigid(hfil, hfil, 0, 0);
}

VBorder::VBorder (Painter* out, int t) : (out) {
    shape->width = t;
    shape->Rigid(0, 0, vfil, vfil);
}
