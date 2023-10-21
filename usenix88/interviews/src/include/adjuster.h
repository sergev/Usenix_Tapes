/*
 * Adjuster - button-like interactors for incremental adjustment of an
 * interactor's perspective.
 */

#ifndef adjuster_h
#define adjuster_h

#include <InterViews/interactor.h>

static const int NO_AUTOREPEAT = -1;

class Bitmap;
class Shape;

class Adjuster : public Interactor {
protected:
    Interactor* view;
    Bitmap* bitmap;
    int delay;
    Perspective* shown;
    boolean highlighted;

    void Init(Interactor*, int, Painter*);
    void Invert();
    void AutoRepeat();
    void HandlePress();
    void Flash();
    void TimerOn();
    void TimerOff();
    virtual void AdjustView(Event&);
public:
    Adjuster(Interactor*, Bitmap*, int = NO_AUTOREPEAT, Painter* = stdpaint);
    void Delete();			    // autorepeat delay is 1/10 secs.
    void Handle(Event&);
    void Draw();
    void Redraw(Coord, Coord, Coord, Coord);
    void Resize();
    void Reshape(Shape&);
    virtual void Highlight();
    virtual void UnHighlight();
};

class Zoomer : public Adjuster {
    float factor;
protected:
    void AdjustView(Event&);
public:
    Zoomer(Interactor*, Bitmap*, float factor, Painter* = stdpaint);
};

class Enlarger : public Zoomer {
public:
    Enlarger(Interactor*, Painter* = stdpaint);
};

class Reducer : public Zoomer {
public:
    Reducer(Interactor*, Painter* = stdpaint);
};

class Mover : public Adjuster {
protected:
    int moveType;
    void AdjustView(Event&);
public:
    Mover(Interactor*, Bitmap*, int delay, int moveType, Painter* = stdpaint);
};

class LeftMover : public Mover {
public:
    LeftMover(Interactor*, int delay = NO_AUTOREPEAT, Painter* = stdpaint);
};

class RightMover : public Mover {
public:
    RightMover(Interactor*, int delay = NO_AUTOREPEAT, Painter* = stdpaint);
};

class UpMover : public Mover {
public:
    UpMover(Interactor*, int delay = NO_AUTOREPEAT, Painter* = stdpaint);
};

class DownMover : public Mover {
public:
    DownMover(Interactor*, int delay = NO_AUTOREPEAT, Painter* = stdpaint);
};

#endif
