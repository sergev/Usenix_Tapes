/*
 * Graphics interface
 */

#ifndef painter_h
#define painter_h

#include <InterViews/resource.h>
#include <InterViews/paint.h>
#include <InterViews/transformer.h>

class Canvas;

class Painter : public Resource {
    friend class Image;
    friend class Rubberband;
    class Graphics* rep;

    void SetRaster(int);
    void InvertArea(Canvas*, Coord, Coord, Coord, Coord);
    void MultiLineNoMap(Canvas* c, Coord x[], Coord y[], int n);
    void FillPolygonNoMap(Canvas* c, Coord x[], Coord y[], int n);
public:
    Painter();
    Painter(Painter*);
    ~Painter();
    void FillBg(boolean);
    boolean BgFilled();
    void SetColors(Color* f, Color* b);
    Color* GetFgColor();
    Color* GetBgColor();
    void SetPattern(Pattern*);
    Pattern* GetPattern();
    void SetBrush(Brush*);
    Brush* GetBrush();
    void SetFont(Font*);
    Font* GetFont();
    void SetTransformer(Transformer*);
    Transformer* GetTransformer();
    void MoveTo(int x, int y);
    void GetPosition(int& x, int& y);
    void SetOrigin(int x0, int y0);
    void GetOrigin(int& x0, int& y0);

    void Translate(float dx, float dy);
    void Scale(float x, float y);
    void Rotate(float angle);

    void Clip(Canvas*, Coord left, Coord bottom, Coord right, Coord top);
    void NoClip();
    void SetPlaneMask(int);

    void Text(Canvas*, const char*);
    void Text(Canvas*, const char*, int);
    void Point(Canvas*, Coord x, Coord y);
    void MultiPoint(Canvas*, Coord x[], Coord y[], int n);
    void Line(Canvas*, Coord x1, Coord y1, Coord x2, Coord y2);
    void Rect(Canvas*, Coord x1, Coord y1, Coord x2, Coord y2);
    void FillRect(Canvas*, Coord x1, Coord y1, Coord x2, Coord y2);
    void ClearRect(Canvas*, Coord x1, Coord y1, Coord x2, Coord y2);
    void Circle(Canvas*, Coord x, Coord y, int r);
    void FillCircle(Canvas*, Coord x, Coord y, int r);
    void Ellipse(Canvas*, Coord x, Coord y, int r1, int r2);
    void FillEllipse(Canvas*, Coord x, Coord y, int r1, int r2);
    void MultiLine(Canvas*, Coord x[], Coord y[], int n);
    void Polygon(Canvas*, Coord x[], Coord y[], int n);
    void FillPolygon(Canvas*, Coord x[], Coord y[], int n);
    void BSpline(Canvas*, Coord x[], Coord y[], int n);
    void ClosedBSpline(Canvas*, Coord x[], Coord y[], int n);
    void FillBSpline(Canvas*, Coord x[], Coord y[], int n);
    void Arc(Canvas*,
	Coord x0, Coord y0, Coord x1, Coord y1,
	Coord x2, Coord y2, Coord x3, Coord y3
    );
    void ArcTo(Canvas*,
	Coord x0, Coord y0, Coord x1, Coord y1, Coord x2, Coord y2
    );
    void Copy(
	Canvas* src, Coord x1, Coord y1, Coord x2, Coord y2,
	Canvas* dst, Coord x0, Coord y0
    );
    void Read(Canvas*, void*, Coord x1, Coord y1, Coord x2, Coord y2);
    void Write(Canvas*, const void*, Coord x1, Coord y1, Coord x2, Coord y2);
};

#endif
