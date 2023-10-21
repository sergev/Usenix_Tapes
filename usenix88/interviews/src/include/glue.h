/*
 * Glue is useful for variable spacing between interactors.
 */

#ifndef glue_h
#define glue_h

#include <InterViews/interactor.h>
#include <InterViews/shape.h>

class Glue : public Interactor {
public:
    Glue(Painter* bg);
    void Draw();
    void Redraw(Coord, Coord, Coord, Coord);
};

class HGlue : public Glue {
public:
    HGlue(Painter* bg, int natural = 0, int stretch = hfil);
    HGlue(Painter* bg, int natural, int shrink, int stretch);
};

class VGlue : public Glue {
    VGlue(Painter* bg, int natural = 0, int stretch = vfil);
    VGlue(Painter* bg, int natural, int shrink, int stretch);
};

#endif
