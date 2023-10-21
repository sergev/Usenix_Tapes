/*
 * GraphicBlock - an interactor that contains a picture.
 */

#ifndef grblock_h
#define grblock_h

#include <InterViews/interactor.h>

typedef enum HAlign { LEFT, HCTR, RIGHT };
typedef enum VAlign { TOP, VCTR, BOTTOM };
typedef enum Zooming { CONTINUOUS, BINARY };

class Graphic;
class Perspective;

class GraphicBlock : public Interactor {
protected:
    Graphic* graphic;
    Coord pad;
    HAlign halign;
    VAlign valign;
    Zooming zooming;
    Coord x0, y0;                   // graphic offset
    float mag;			    // total magnification

    Painter* alt;		    // alternate painter for drawing
    void SwapPainters();	    // background: a hack to avoid SetColors
    Painter* GetPainter();	    // in menu highlighting.
    void InvertPainter();
    void Init();

    void Normalize(Perspective&);   // normalize units
    void Align();		    // align graphic
    void Fix();			    // keep alignment fixed during resize
    float NearestPow2(float);	    // convert to nearest power of 2
    float ScaleFactor(Perspective&);
    void Zoom(Perspective&);
    void Scroll(Perspective&);
public:
    GraphicBlock(
	Sensor*, Graphic*, 
	Coord pad = 0, HAlign = HCTR, VAlign = VCTR, Zooming = CONTINUOUS
    );
    void Resize();
    void Update();
    void Draw();
    void Redraw(Coord, Coord, Coord, Coord);
    void Adjust(Perspective&);
    void Delete();
    void Invert();

    Graphic* GetGraphic() { return graphic; }
    float GetMagnification() { return mag; }
    void SetMagnification(float);
};

#endif
