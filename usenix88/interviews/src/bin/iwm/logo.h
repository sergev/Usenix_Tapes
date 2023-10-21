#ifndef logo_h
#define logo_h

#include <InterViews/interactor.h>

class Logo : public Interactor {
public:
    Logo(Painter*);
};

class PolygonLogo : public Logo {
    float unit;
    Painter* b, * w;
    Coord rectx [4], recty [4];
    Coord trix [3], triy [4];
public:
    PolygonLogo(Painter*, Coord);
    ~PolygonLogo();
    void Redraw(Coord, Coord, Coord, Coord);
};

class Bitmap;
class BitmapLogo : public Logo {
    Painter*  framer;
    Bitmap*  bitmap;
public:
    BitmapLogo(Bitmap*, Painter*, Coord);
    ~BitmapLogo();
    void Redraw(Coord, Coord, Coord, Coord);
};

#endif
