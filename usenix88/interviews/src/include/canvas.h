/*
 * A canvas is an area of graphics output.
 */

#ifndef canvas_h
#define canvas_h

#include <InterViews/defs.h>

class Canvas {
    friend class Interactor;
    friend class Scene;
    friend class World;
    friend class WorldView;
    friend class Image;
    friend class Painter;
    friend class Graphics;
    friend class Bitmap;

    void* id;
    boolean offscreen;
    int width, height;

    Canvas(void*);
    void WaitForCopy();
#ifdef X10
    /* implement clipping with a transparent window */
    Coord left, bottom;
    Canvas* unclipped;

    void Clip(Coord left, Coord bottom, Coord right, Coord top);
    void NoClip();
    void Exchange(Canvas*);
#endif
public:
    Canvas(int w, int h);
    ~Canvas();
    void* Id () { return id; }
    int Width () { return width; }
    int Height () { return height; }
    void SetBackground(class Color*);
};

#endif
