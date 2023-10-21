/*
 * TextBlock - an Interactor for Text
 */

#ifndef textblock_h
#define textblock_h

#include <InterViews/interactor.h>
#include <InterViews/shape.h>

class Layout;
class Painter;
class Sensor;
class Event;
class Perspective;
class TextPainter;

static const int BUFFERSIZE = 256;

class TextBlock : public Interactor {
friend class Layout;
protected:
    Layout* layout;
    Interactor* handler;

    int start;		    // first line
    int width, height;	    // total
    int rows, cols;	    // visible
    int left, top;	    // margins
    TextBlock* next;
    boolean draw;

    char buffer[BUFFERSIZE];
    int length;		    // of buffer
    Coord bx, by;	    // start of buffer
    Coord px, py;	    // pixels

    void Initialize(Layout*, Painter*, Sensor*);
    void ComputeShape();
    int Cols();
    int Rows();
    boolean Contains(Coord row);
    boolean PastEnd(Coord row);

    void String(const char*, int);
    void Space(int);
    void NewLine();
    void Caret();
    void Overfull();
    void Flush(TextPainter*);
    void GoTo(Coord x, Coord y);
    void EndLine();
    void StartLine();
    void EndBlock();
    void StartBlock();
    void LastLine(Coord y);

    Coord XPix(Coord);
    Coord YPix(Coord);
    Coord XChar(Coord);
    Coord YChar(Coord);
public:
    TextBlock(int width, int height, Interactor* handler = nil);
    void Delete();
    void Resize();
    void Redraw(Coord, Coord, Coord, Coord);
    void Handle(Event&);
    void Adjust(Perspective&);
    void Reshape(Shape&);
    
    void Wait();
    void Done();
};

#endif
