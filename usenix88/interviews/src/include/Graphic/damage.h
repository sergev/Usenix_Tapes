/*
 * Damage - maintains an array of non-overlapping rectangles representing
 * damaged areas of of a canvas, used for smart redraw.
 */

#ifndef damage_h
#define damage_h

#include <InterViews/defs.h>

class BoxList;
class BoxObj;
class Canvas;
class Graphic;
class Painter;
class RefList;

class Damage {
    BoxList* areas;
    RefList* additions;
    Canvas* canvas;
    Painter* output;
    Graphic* graphic;
    
    int Area(BoxObj&);
    void DrawAreas();
    void DrawAdditions();
    void Merge(BoxObj&);
public:
    Damage(Canvas*, Painter*, Graphic*);
    ~Damage();
    
    boolean Incurred();
    void Added(Graphic*);
    void Incur(Graphic*);
    void Incur(BoxObj&);
    void Repair();
    void Reset();
};

#endif
