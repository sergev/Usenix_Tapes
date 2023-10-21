/*
 * View of a list of squares.
 */

#ifndef view_h
#define view_h

#include <InterViews/interactor.h>

class SquaresView : public Interactor {
    friend class SquaresFrame;

    class Squares* subject;
public:
    SquaresView(Squares*, Painter* out = stdpaint);
    void Delete();
    void Zoom(int);
    void Redraw(Coord, Coord, Coord, Coord);
    void Update();
    void Adjust(Perspective&);
};

#endif
