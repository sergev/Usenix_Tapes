/*
 * A viewport contains another interactor.  Unlike a MonoScene or Frame,
 * a viewport always gives the interactor its desired shape.  However,
 * the interactor may not be entirely viewable through the viewport.
 * The viewport's perspective can be used to adjust what portion is visible.
 */

#ifndef viewport_h
#define viewport_h

#include <InterViews/scene.h>

typedef enum {
    LowerLeftView, LowerRightView, UpperLeftView, UpperRightView,
    MiddleRightView, MiddleLeftView, UpperMiddleView, LowerMiddleView,
    CenterView
} ViewAlignment;

class Viewport : public MonoScene {
protected:
    ViewAlignment align;

    virtual void DoChange(Interactor*, Shape*);
    virtual void DoMove(Interactor*, Coord& x, Coord& y);
public:
    Viewport(Sensor*, Painter*, Interactor* = nil, ViewAlignment = CenterView);
    void Delete();
    void Resize();
    void Redraw(Coord, Coord, Coord, Coord);
    void Adjust(Perspective&);
};

#endif
