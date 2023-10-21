/*
 * General scrolling interface.
 */

#ifndef scroller_h
#define scroller_h

#include <InterViews/interactor.h>

class Scroller : public Interactor {
protected:
    Interactor* interactor;
    Perspective* view;
    Perspective* shown;
    double scale;
    Sensor* tracking;

    void Background(Coord, Coord, Coord, Coord);
    Scroller(Interactor*, Sensor* in, Painter* out);
    void Delete();
    void Resize();
public:
};

class HScroller : public Scroller {
    void GetBarInfo(Perspective*, Coord&, int&);
    void Bar(Coord, int);
    void Outline(Coord, int);
    void Border(Coord);
    void Sides(Coord, Coord);
    Coord Slide(Event&);
public:
    HScroller(
	Interactor*, int size = 0, Sensor* in = nil, Painter* out = stdpaint
    );
    void Redraw(Coord, Coord, Coord, Coord);
    void Handle(Event&);
    void Update();
};

class VScroller : public Scroller {
    void GetBarInfo(Perspective*, Coord&, int&);
    void Bar(Coord, int);
    void Outline(Coord, int);
    void Border(Coord);
    void Sides(Coord, Coord);
    Coord Slide(Event&);
public:
    VScroller(
	Interactor*, int size = 0, Sensor* in = nil, Painter* out = stdpaint
    );
    void Redraw(Coord, Coord, Coord, Coord);
    void Handle(Event&);
    void Update();
};

#endif
