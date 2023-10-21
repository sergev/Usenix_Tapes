/*
 * A banner is a one line title bar.
 */

#ifndef banner_h
#define banner_h

#include <InterViews/interactor.h>

class Banner : public Interactor {
    int lw, mw, rw;
    Coord lx, mx, rx;
    Painter* inverse;

    void Init();
public:
    char* left;
    char* middle;
    char* right;
    boolean highlight;

    Banner(Painter* out, char* lt, char* mid = nil, char* rt = nil);
    void Delete();
    void Draw();
    void Redraw(Coord, Coord, Coord, Coord);
    void Resize();
    void Update();
};

#endif
