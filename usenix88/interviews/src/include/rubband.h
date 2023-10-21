/*
 * Interface to Rubberbands, objects that vary their appearance in response to
 * changes in a parameter, which for this class is a coordinate pair.
 * Rubberbands are typically used to provide graphical feedback to a user
 * performing some interactive operation.  For example, a rubberband can track
 * the motion of the mouse and draw an outline of a window as it is resized.
 * 
 * To use a rubberband, first specify its initial appearance by passing the
 * appropriate information to its constructor.  Then, the rubberband will
 * automatically erase and redraw itself whenever its Track function is
 * called with the tracking point (x, y) as its argument.  The
 * exact change in the rubberband's appearance depends on the rubberband
 * chosen.  The rubberband may be explicity drawn or erased by calling its
 * Draw or Erase functions.  Its original and current appearance
 * can be obtained at any time by calling its GetOriginal and
 * GetCurrent functions.  The rubberband is automatically erased when its
 * destructor is called.
 *
 * If a rubberband's constructor is passed a non-nil painter, a copy of that
 * painter will be used to render the rubberband when it is drawn.  The
 * user can specify a particular painter to use by passing a nil painter
 * to the constructor and later calling the SetPainter function.  N.B.:
 * the rubberband may alter the painter specified by SetPainter.
 */

#ifndef rubband_h
#define rubband_h

#include <InterViews/defs.h>
#include <InterViews/resource.h>

typedef enum { Left, Right, Bottom, Top } Side;

/*
 * The primordial class from which all other rubberbands are derived.
 */

class Rubberband : public Resource {
protected:
    class Painter* output;
    class Canvas* canvas;
    boolean drawn;
    Coord trackx, offx;
    Coord tracky, offy;
    float Angle(Coord, Coord, Coord, Coord);	// angle line makes w/horiz
    float Distance(Coord, Coord, Coord, Coord); // distance between 2 points
public:
    Rubberband(Painter*, Canvas*, Coord offx, Coord offy);
    virtual void Draw();
    void Redraw();
    virtual void Erase();
    virtual void Track(Coord x, Coord y);
    virtual void Delete();
    ~Rubberband();

    virtual void SetPainter(Painter*);
    virtual void SetCanvas(Canvas*);
    Painter* GetPainter() { return output; }
    Canvas* GetCanvas() { return canvas; }
};

#endif
