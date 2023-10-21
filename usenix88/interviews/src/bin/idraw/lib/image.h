/*
 * Old-fashioned interface for low-level graphics windows.
 */

#ifndef image_h
#define image_h

#include <InterViews/interactor.h>
#include <InterViews/paint.h>
#include <InterViews/cursor.h>

#define WHITE white
#define BLACK black

extern Brush* solidLine;
extern Pattern* solidPattern;
extern Pattern* clearPattern;
extern Pattern* vertPattern;
extern Pattern* horizPattern;
extern Pattern* majorDiagPattern;
extern Pattern* minorDiagPattern;
extern Pattern* thickVertPattern;
extern Pattern* thickHorizPattern;
extern Pattern* thickMajorDiagPattern;
extern Pattern* thickMinorDiagPattern;
extern Pattern* stipplePattern1;
extern Pattern* stipplePattern2;
extern Pattern* stipplePattern3;
extern Pattern* stipplePattern4;
extern Pattern* stipplePattern5;
extern Pattern* stipplePattern6;

extern Cursor* arrowCursor;
extern Cursor* crossHairsCursor;
extern Cursor* hourglassCursor;

typedef int PatternData[patternHeight];

typedef int XCoord;
typedef int YCoord;

typedef enum { INVERT, PAINTFG, PAINTBG, PAINTFGBG } RenderingFunction;
typedef enum { COMPLEXPOLY, CONVEXPOLY } PolyAlg;
typedef void UncoverProc(void*, XCoord, YCoord, XCoord, YCoord);

typedef int InputButton;

typedef Brush LineStyle;
typedef Canvas Pixmap;

typedef enum {
    BUTTON,		/* key pressed or released */
    MOTION,		/* mouse moved */
    CHARACTER,		/* key pressed, intepreted as ascii */
    CHANNELREADY,	/* not implemented */
    TIMER,		/* not implemented */
    ERROR,		/* interrupted read */
    REDRAW,		/* image damaged */
    RESIZE,		/* image changed size */
    SELECT,		/* image is now input focus */
    UNSELECT,		/* image is no longer input focus */
} ImageEventType;

struct ImageEvent {
    int time;
    ImageEventType eventType;
    union {
	struct {
	    short x;
	    short y;
	} motion;
	struct {
	    short x;
	    short y;
	    InputButton key;
	    boolean up;
	} button;
	struct {
	    short x;
	    short y;
	    char ch;
	} character;
	struct {
	    short left;
	    short bottom;
	    short right;
	    short top;
	} redraw;
	struct {
	    int xmax;
	    int ymax;
	} resize;
    };
};

class Image : public Interactor {
    Color* foreground;
    Color* background;
    RenderingFunction render;
    int curmask;
    Pattern* curpattern;
    LineStyle* curlinestyle;
    Font* curfont;
    Coord x0, y0;
    boolean qchar;
    boolean resized;

    void Init();
    boolean GetEvent(Image*&, ImageEvent&, boolean wait);
public:
    Coord xmax, ymax;

    Image();
    Image(int width, int height);
    Image(XCoord, YCoord, int width, int height);
    void Delete();
    void Resize();
    void Redraw(Coord, Coord, Coord, Coord);

    int Width() { return xmax; }
    int Height() { return ymax; }
    void Sync();
    void MoveTo(XCoord, YCoord);
    void GetPosition(XCoord&, YCoord&);
    void GetLocation(XCoord&, YCoord&);
    void MapColor(int pixel, int red, int green, int blue);
    void SetColors(Color* fg, Color* bg);
    void GetColors (Color*& fg, Color*& bg) {
	fg = foreground; bg = background;
    }
    void SetForegroundColor(Color*);
    Color* GetForegroundColor () { return foreground; }
    void SetBackgroundColor(Color*);
    Color* GetBackgroundColor () { return background; }
    void SetRendering(RenderingFunction);
    RenderingFunction GetRendering () { return render; }
    void SetPlaneMask(int);
    int GetPlaneMask () { return curmask; }
    void Writable(XCoord, YCoord, XCoord, YCoord);
    void AllWritable();
    void SetLineStyle(LineStyle*);
    LineStyle *GetLineStyle () { return curlinestyle; }
    void SetPattern(Pattern*);
    Pattern *GetPattern () { return curpattern; }
    void SetPolyAlg(PolyAlg);
    void Clear();
    void Line(XCoord x1, YCoord y1, XCoord x2, YCoord y2);
    void LineTo(XCoord x, YCoord y);
    void Point(XCoord x, YCoord y);
    void PointList(XCoord* x, YCoord* y, int count);
    void LineList(XCoord* x, YCoord* y, int count);
    void Circle(XCoord x, YCoord y, int r);
    void FilledCircle(XCoord x, YCoord y, int r);
    void Ellipse(XCoord x, YCoord y, int r1, int r2);
    void FilledEllipse(XCoord x, YCoord y, int r1, int r2);
    void Rectangle(XCoord x1, YCoord y1, XCoord x2, YCoord y2);
    void FilledRectangle(XCoord x1, YCoord y1, XCoord x2, YCoord y2);
    void InvertArea(XCoord x1, YCoord y1, XCoord x2, YCoord y2);
    void Polygon(XCoord* x, YCoord* y, int count);
    void BSpline(XCoord* x, YCoord* y, int count);
    void ClosedBSpline(XCoord* x, YCoord* y, int count);
    void FilledBSpline(XCoord* x, YCoord* y, int count);
    void BezierArc (
	XCoord x0, YCoord y0, XCoord x1, YCoord y1,
	XCoord x2, YCoord y2, XCoord x3, YCoord y3
    );
    void BezierArcTo (
	XCoord x0, YCoord y0, XCoord x1, YCoord y1, XCoord x2, YCoord y2
    );
    void CopyArea(
	XCoord x1, YCoord y1, XCoord x2, YCoord y2, XCoord dstx1, YCoord dsty1
    );
    void ReadArea(Pixmap*, XCoord x1, YCoord y1, XCoord x2, YCoord y2);
    void WriteArea(Pixmap*, XCoord x1, YCoord y1, XCoord x2, YCoord y2);
    void ReadPixmap(Pixmap*, void *dst, int);
    void WritePixmap(Pixmap*, void *src, int);
    int PixelsToBytes(int width, int height);
    void SetFont(Font*);
    Font* GetFont () { return curfont; }
    int GetHeight();
    int GetFontHeight(Font* f);
    int StrWidth(const char* s, int len);
    int StrWidth(const char* s);
    int FontStrWidth(Font* f, const char* s, int len);
    int FontStrWidth(Font* f, const char* s);
    void Text(const char* s, int len);
    void Text(const char* s);

    void QKeyboard();
    void UnQKeyboard();
    void QMotion();
    void UnQMotion();
    void QButton(InputButton);
    void UnQButton(InputButton);
    void QMouseButtons();
    void UnQMouseButtons();
    void QCharacter();
    void UnQCharacter();
    void UnQAll();
    Image* QRead(ImageEvent&);
    boolean QTest(ImageEvent&);
    void StartQRead();
    Image* FinishQRead(ImageEvent&);
    void QReset();
    void MapKey(InputButton, char, char, char, char);
    void UnMapKey(InputButton);
    boolean KeyToChar(InputButton, boolean down, int&);
    void EnableShiftLock();
    void DisableShiftLock();
    void EnableMetaKey();
    void DisableMetaKey();

    void CursorOn();
    void CursorOff();
    void SetCursor(Cursor*);
    Cursor* GetCursor () { return cursor; }
    void SetCursorPosition(XCoord x, YCoord y);
    void GetCursorPosition(XCoord& x, YCoord& y);
};

#endif
