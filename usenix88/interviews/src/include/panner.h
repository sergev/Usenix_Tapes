/*
 * Panner - an interactor for two-dimensional scrolling and zooming.
 */

#ifndef panner_h
#define panner_h

#include <InterViews/box.h>

class Panner : public VBox {
    class Enlarger* enlarger;
    class Reducer* reducer;
    class LeftMover* lMover;
    class RightMover* rMover;
    class UpMover* uMover;
    class DownMover* dMover;
    class Slider* slider;
    void BuildAdjusters(Interactor*, Painter*);
public:
    Panner(Interactor*, int size = 0, Painter* = stdpaint);
    void Delete();    
};

class Slider : public Interactor {
    Interactor* view;
    Perspective* shown;
    Coord left, bottom, right, top;
    Coord llim, blim, rlim, tlim;	// sliding limits
    boolean constrained;
    int moveType;
    Coord origx, origy;

    Coord ViewX(Coord);
    Coord ViewY(Coord);
    Coord SliderX(Coord);
    Coord SliderY(Coord);
    void CalcLimits(Event&);		// calculate sliding limits
    void SizeKnob();			// calculate size of slider knob
    boolean Inside(Event&);		// true if inside slider knob
    void Constrain(Event&);		// constrain slider knob motion
    void Move(Coord dx, Coord dy);	// move view to reflect slider position
    void Slide(Event&);			// rubberband rect while mousing
    void Jump(Event&);			// for click outside knob
public:
    Slider(Interactor*, Painter* = stdpaint);
    void Draw();
    void Redraw(Coord, Coord, Coord, Coord);
    void Handle(Event&);
    void Update();
    void Reshape(Shape&);
    void Resize();
    void Delete();
};

#endif
