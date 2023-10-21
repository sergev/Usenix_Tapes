/*
 * A border is simply a line surrounded by glue on both sides.
 */

#ifndef border_h
#define border_h

#include <InterViews/interactor.h>

class Border : public Interactor {
public:
    Border(Painter*);
    void Delete();
    void Draw();
    void Redraw(Coord, Coord, Coord, Coord);
};

class HBorder : public Border {
public:
    HBorder(Painter* out = stdpaint, int thick = 1);
};

class VBorder : public Border {
public:
    VBorder(Painter* out = stdpaint, int thick = 1);
};

#endif
