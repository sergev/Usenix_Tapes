/*
 * A frame surrounds another interactor.
 */

#ifndef frame_h
#define frame_h

#include <InterViews/scene.h>

class Frame : public MonoScene {
protected:
    int border;

    virtual void DoChange(Interactor*, Shape*);
public:
    Frame(Painter*, Interactor* = nil, int width = 1);
    virtual void Resize();
    virtual void Redraw(Coord left, Coord bottom, Coord right, Coord top);
    virtual void Handle(Event&);
    virtual void Highlight(boolean);
};

class TitleFrame : public Frame {
protected:
    class Banner* banner;

    TitleFrame(Painter*, int width = 1);
    virtual void DoInsert(Interactor*, boolean, Coord& x, Coord& y, Shape*);
public:
    TitleFrame(Painter*, Banner*, Interactor*, int width = 1);
    void Highlight(boolean);
};

class BorderFrame : public Frame {
    Painter* outline;
public:
    BorderFrame(Painter*, Interactor* = nil, int width = 1);
    void Delete();
    void Highlight(boolean);
};

#endif
