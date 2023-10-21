/*
 * All that's necessary for glue is to set up the shape and
 * redraw the background.
 */

#include <InterViews/glue.h>
#include <InterViews/shape.h>
#include <InterViews/painter.h>

Glue::Glue (Painter* bg) : (nil, bg) {
    shape->width = 0;
    shape->height = 0;
    shape->Rigid(hfil, hfil, vfil, vfil);
}

void Glue::Draw () {
    output->ClearRect(canvas, 0, 0, xmax, ymax);
}

void Glue::Redraw (Coord x1, Coord y1, Coord x2, Coord y2) {
    output->ClearRect(canvas, x1, y1, x2, y2);
}

HGlue::HGlue (Painter* bg, int nat, int str) : (bg) {
    shape->width = nat;
    shape->height = 0;
    shape->Rigid(nat, str, vfil, vfil);
}

HGlue::HGlue (Painter* bg, int nat, int shr, int str) : (bg) {
    shape->width = nat;
    shape->height = 0;
    shape->Rigid(shr, str, vfil, vfil);
}

VGlue::VGlue (Painter* bg, int nat, int str) : (bg) {
    shape->width = 0;
    shape->height = nat;
    shape->Rigid(hfil, hfil, nat, str);
}

VGlue::VGlue (Painter* bg, int nat, int shr, int str) : (bg) {
    shape->width = 0;
    shape->height = nat;
    shape->Rigid(hfil, hfil, shr, str);
}
