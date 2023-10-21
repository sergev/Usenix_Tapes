/*
 * Graphics attributes
 */

#ifndef paint_h
#define paint_h

#include <InterViews/defs.h>
#include <InterViews/resource.h>

/* A color is defined by rgb intensity; it can optionally be indexed. */
class Color : public Resource {
    friend class Painter;
    int pixel;
    short red, green, blue;
public:
    Color(int r, int g, int b);
    Color(int index, int r, int g, int b);
    Color(int index);
    Color(const char*);
    ~Color();
    PixelValue () { return pixel; }
    void Intensities (int& r, int& g, int& b) { r = red; g = green; b = blue; }
    boolean Valid();
};

/* A pattern is a bit array describing where to fill. */
static const int patternHeight = 16;
static const int patternWidth = 16;
class Pattern : public Resource {
    friend class Painter;
    void* info;
public:
    Pattern(int p[patternHeight]);
    Pattern(int dither);
    Pattern(class Bitmap*);
    ~Pattern();
};

/* A brush specifies how lines should be drawn. */
class Brush : public Resource {
    friend class Painter;
    friend class Graphics;
    int width;
    void* info;
public:
    Brush(int p, int w = 1);
    ~Brush();
    Width () { return width; }
};

/*
 * Fonts are accessed by a name, which is mapped through a table
 * to workstation-specific font files and formats.
 */
class Font : public Resource {
    friend class Painter;
    friend class Bitmap;
    void* id;
    void* info;
    int height;

    void Init();
public:
    Font(const char*);
    Font(const char*, int);
    ~Font();
    int Baseline();
    int Height();
    int Width(const char*);
    int Width(const char*, int);
    int Index(const char*, int offset, boolean between);
    int Index(const char*, int, int offset, boolean between);
    boolean Valid();
    boolean FixedWidth();
};

/* Standard attributes */
extern Color* black;
extern Color* white;
extern Pattern* solid;
extern Pattern* clear;
extern Pattern* lightgray;
extern Pattern* gray;
extern Pattern* darkgray;
extern Brush* single;
extern Font* stdfont;

#endif
