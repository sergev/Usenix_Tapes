/*
 * Bitmap - Bit map for InterViews
 */

#ifndef bitmap_h
#define bitmap_h

#include <InterViews/defs.h>
#include <InterViews/resource.h>

class Canvas;
class Color;
class Font;
class Transformer;

class Bitmap : public Resource {
    void * bitmap;
    void * pixmap;

    short * data;
    int width;
    int height;

    Coord x0;
    Coord y0;

    Color * foreground;
    Color * background;

    void Init();
    void FreeMaps();
    int Index( int x, int y ) { return (width+15)/16 * (height-y-1) + x/16; }
    short Bit( int x, int ) { return 1 << ( x%16 ); }
    boolean Valid( int x, int y ) {
	return ! (x < 0 || x >= width || y < 0 || y >= height);
    }
public:
    Bitmap(const char * filename);		// bitmap(1) file
    Bitmap(short *, int width, int height);	// raw data
    Bitmap(Bitmap *);				// copy
    ~Bitmap();

    void Draw(Canvas *);

    int Width();
    int Height();

    void SetColors(Color * f, Color * b);
    Color * GetFgColor();
    Color * GetBgColor();

    void Transform(Transformer *);
    void Scale(float x, float y);
    void Rotate(float);

    void SetOrigin(int x0, int y0);
    void GetOrigin(int& x0, int& y0);

    void FlipHorizontal();
    void FlipVertical();
    void Rotate90();

    void Invert();
    boolean Peek(int x, int y);
    void Poke(boolean bit, int x, int y);
};

#endif
