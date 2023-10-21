/*
 * Old-style graphics interface for backward compatibility,
 * defined in terms of images.
 */

#include "image.h"
#include <InterViews/canvas.h>
#include <InterViews/cursor.h>

XCoord vdi_XMAXSCREEN;
YCoord vdi_YMAXSCREEN;

static Image* curimage = (Image*) -1;

inline int abs (int a) {
    return a < 0 ? -a : a;
}

inline void SetSizes () {
    vdi_XMAXSCREEN = curimage->Width();
    vdi_YMAXSCREEN = curimage->Height();
}

void vdi_Initialize () {
    curimage = new Image;
    SetSizes();
}

void vdi_Reset () {
    /* nop */
}

void vdi_InitPlace (XCoord left, YCoord top, int width, int height) {
    curimage = new Image(left, top, width, height);
    SetSizes();
}

void vdi_InitSize (int width, int height) {
    curimage = new Image(width, height);
    SetSizes();
}

void vdi_Finishup () {
    delete curimage;
}

void vdi_Sync () {
    curimage->Sync();
}

Image* vdi_GetImage () {
    return curimage;
}

void vdi_SetImage (Image *i) {
    curimage = i;
}

void vdi_Clear () {
    curimage->Clear();
}

void vdi_MoveTo (XCoord x, YCoord y) {
    curimage->MoveTo(x, y);
}

void vdi_GetPosition (XCoord& x, YCoord& y) {
    curimage->GetPosition(x, y);
}

void vdi_GetLocation (XCoord& x, YCoord& y) {
    curimage->GetLocation(x, y);
}

void vdi_MapColor (int pixel, int red, int green, int blue) {
    curimage->MapColor(pixel, red, green, blue);
}

static Color* ColorPixelToPtr (int c) {
    if (c == 0) {
	return black;
    } else if (c == 1 || c == 15) {
	return white;
    } else {
	return new Color(c);
    }
}

void vdi_SetColors (int fg, int bg) {
    curimage->SetColors(ColorPixelToPtr(fg), ColorPixelToPtr(bg));
}

void vdi_GetColors (int& fg, int& bg) {
    Color* c1;
    Color* c2;

    curimage->GetColors(c1, c2);
    fg = c1->PixelValue();
    bg = c2->PixelValue();
}

void vdi_SetForegroundColor (int c) {
    curimage->SetForegroundColor(ColorPixelToPtr(c));
}

int vdi_GetForegroundColor () {
    Color* c;

    c = curimage->GetForegroundColor();
    return c->PixelValue();
}

void vdi_SetBackgroundColor (int c) {
    curimage->SetBackgroundColor(ColorPixelToPtr(c));
}

int vdi_GetBackgroundColor () {
    Color* c;

    c = curimage->GetBackgroundColor();
    return c->PixelValue();
}

void vdi_SetRendering (RenderingFunction r) {
    curimage->SetRendering(r);
}

RenderingFunction vdi_GetRendering () {
    return curimage->GetRendering();
}

void vdi_SetPlaneMask (int mask) {
    curimage->SetPlaneMask(mask);
}

int vdi_GetPlaneMask () {
    return curimage->GetPlaneMask();
}

void vdi_Writable (XCoord x1, YCoord y1, XCoord x2, YCoord y2) {
    curimage->Writable(x1, y1, x2, y2);
}

void vdi_AllWritable () {
    curimage->AllWritable();
}

int vdi_DefLineStyle (int ls) {
    return (int) new LineStyle(ls);
}

void vdi_SetLineStyle (int n) {
    curimage->SetLineStyle((LineStyle*) n);
}

int vdi_GetLineStyle () {
    return (int) (curimage->GetLineStyle());
}

/*
 * These magic tables are necessary to deal with idraw storing
 * old-style pattern ids in picture files (sigh).
 */

static Pattern** stdpat[] = {
    &solidPattern,
    &solidPattern, &clearPattern, &vertPattern, &horizPattern,
    &majorDiagPattern, &minorDiagPattern,
    &thickVertPattern, &thickHorizPattern,
    &thickMajorDiagPattern, &thickMinorDiagPattern,
    &stipplePattern1, &stipplePattern2, &stipplePattern3,
    &stipplePattern4, &stipplePattern5, &stipplePattern6
};

static Pattern* defpat[100];

static int defined = 36;

int vdi_AssocPattern (Pattern* p) {
    register int cur;

    cur = defined;
    defpat[cur] = p;
    defined = cur+1;
    return cur;
}

int vdi_DefPattern (int* data) {
    register int cur;

    cur = defined;
    defpat[cur] = new Pattern(data);
    defined = cur+1;
    return cur;
}

void vdi_SetPattern (int n) {
    if (n < 16) {
	curimage->SetPattern(*stdpat[n]);
    } else if (n < 100) {
	curimage->SetPattern(defpat[n]);
    } else {
	curimage->SetPattern((Pattern*) n);
    }
}

int vdi_GetPattern () {
    return (int) (curimage->GetPattern());
}

void vdi_SetPolygonAlg (PolyAlg alg) {
    curimage->SetPolyAlg(alg);
}

void vdi_Line (XCoord x1, YCoord y1, XCoord x2, YCoord y2) {
    curimage->Line(x1, y1, x2, y2);
}

void vdi_LineTo (XCoord x, YCoord y) {
    curimage->LineTo(x, y);
}

void vdi_Point (XCoord x, YCoord y) {
    curimage->Point(x, y);
}

static void Convert (short sx[], short sy[], XCoord x[], YCoord y[], int n) {
    register int i;

    for (i = 0; i < n; i++) {
	x[i] = sx[i];
	y[i] = sy[i];
    }
}

void vdi_PointList (short sx[], int nx, short sy[], int) {
    XCoord* x;
    YCoord* y;

    x = new XCoord[nx];
    y = new YCoord[nx];
    Convert(sx, sy, x, y, nx);
    curimage->PointList(x, y, nx);
}

void vdi_LineList (short sx[], int nx, short sy[], int) {
    XCoord* x;
    YCoord* y;

    x = new XCoord[nx];
    y = new YCoord[nx];
    Convert(sx, sy, x, y, nx);
    curimage->LineList(x, y, nx);
}

void vdi_Circle (XCoord x, YCoord y, int r) {
    curimage->Circle(x, y, r);
}

void vdi_FilledCircle (XCoord x, YCoord y, int r) {
    curimage->FilledCircle(x, y, r);
}

void vdi_Ellipse (XCoord x, YCoord y, int r1, int r2) {
    curimage->Ellipse(x, y, r1, r2);
}

void vdi_FilledEllipse (XCoord x, YCoord y, int r1, int r2) {
    curimage->FilledEllipse(x, y, r1, r2);
}

void vdi_Rectangle (XCoord x1, YCoord y1, XCoord x2, YCoord y2) {
    curimage->Rectangle(x1, y1, x2, y2);
}

void vdi_FilledRectangle (XCoord x1, YCoord y1, XCoord x2, YCoord y2) {
    curimage->FilledRectangle(x1, y1, x2, y2);
}

void vdi_InvertArea (XCoord x1, YCoord y1, XCoord x2, YCoord y2) {
    curimage->InvertArea(x1, y1, x2, y2);
}

void vdi_Polygon (short sx[], int nx, short sy[], int) {
    XCoord* x;
    YCoord* y;

    x = new XCoord[nx];
    y = new YCoord[nx];
    Convert(sx, sy, x, y, nx);
    curimage->Polygon(x, y, nx);
}

void vdi_BezierArc (
    XCoord x0, YCoord y0, XCoord x1, YCoord y1,
    XCoord x2, YCoord y2, XCoord x3, YCoord y3
) {
    curimage->BezierArc(x0, y0, x1, y1, x2, y2, x3, y3);
}

void vdi_BezierArcTo (
    XCoord x0, YCoord y0, XCoord x1, YCoord y1, XCoord x2, YCoord y2
) {
    curimage->BezierArcTo(x0, y0, x1, y1, x2, y2);
}

void vdi_BSpline (short sx[], int nx, short sy[], int) {
    XCoord* x;
    YCoord* y;

    x = new XCoord[nx];
    y = new YCoord[nx];
    Convert(sx, sy, x, y, nx);
    curimage->BSpline(x, y, nx);
}

void vdi_ClosedBSpline (short sx[], int nx, short sy[], int) {
    XCoord* x;
    YCoord* y;

    x = new XCoord[nx];
    y = new YCoord[nx];
    Convert(sx, sy, x, y, nx);
    curimage->ClosedBSpline(x, y, nx);
}

void vdi_FilledBSpline (short sx[], int nx, short sy[], int) {
    XCoord* x;
    YCoord* y;

    x = new XCoord[nx];
    y = new YCoord[nx];
    Convert(sx, sy, x, y, nx);
    curimage->FilledBSpline(x, y, nx);
}

void vdi_CopyArea (
    XCoord x1, YCoord y1, XCoord x2, YCoord y2, XCoord dstx1, YCoord dsty1
) {
    curimage->CopyArea(x1, y1, x2, y2, dstx1, dsty1);
}

void vdi_ReadArea (
    XCoord x1, YCoord y1, XCoord x2, YCoord y2, void* dst, int count
) {
    Pixmap* p;
    
    p = new Pixmap(abs(x2 - x1) + 1, abs(y2 - y1) + 1);
    curimage->ReadArea(p, x1, y1, x2, y2);
    curimage->ReadPixmap(p, dst, count);
    delete p;
}

void vdi_WriteArea (
    XCoord x1, YCoord y1, XCoord x2, YCoord y2, void* src, int count
) {
    Pixmap* p;

    p = new Pixmap(abs(x2 - x1) + 1, abs(y2 - y1) + 1);
    curimage->WritePixmap(p, src, count);
    curimage->WriteArea(p, x1, y1, x2, y2);
    delete p;
}

Pixmap* vdi_CreatePixmap (int w, int h) {
    return new Pixmap(w, h);
}

void vdi_DestroyPixmap (Pixmap* p) {
    delete p;
}

void vdi_SaveArea (Pixmap* p, XCoord x1, YCoord y1, XCoord x2, YCoord y2) {
    curimage->ReadArea(p, x1, y1, x2, y2);
}

void vdi_RestoreArea (Pixmap* p, XCoord x1, YCoord y1, XCoord x2, YCoord y2) {
    curimage->WriteArea(p, x1, y1, x2, y2);
}

void vdi_ReadPixmap (Pixmap* p, void* dst, int count) {
    curimage->ReadPixmap(p, dst, count);
}

void vdi_WritePixmap (Pixmap* p, const void* src, int count) {
    curimage->WritePixmap(p, src, count);
}

int vdi_PixelsToBytes (int width, int height) {
    return curimage->PixelsToBytes(width, height);
}

void vdi_CharStr (char* s, int count) {
    register char* p;
    register int i;

    for (p = s, i = 0; i < count && *p != '\0'; p++, i++);
    curimage->Text(s, i);
}

int vdi_LoadFont (const char* s, int len) {
    return (int) new Font(s, len);
}

void vdi_SetFont (int n) {
    curimage->SetFont((Font*) n);
}

int vdi_GetFont () {
    return (int) (curimage->GetFont());
}

int vdi_GetHeight () {
    return curimage->GetHeight();
}

int vdi_GetFontHeight (int n) {
    return curimage->GetFontHeight((Font*) n);
}

int vdi_StrWidth (const char* s, int len) {
    return curimage->StrWidth(s, len);
}

int vdi_FontStrWidth (int n, const char* s, int len) {
    return curimage->FontStrWidth((Font*) n, s, len);
}

void input_QKeyboard () {
    curimage->QKeyboard();
}

void input_UnQKeyboard () {
    curimage->UnQKeyboard();
}

void input_QMotion () {
    curimage->QMotion();
}

void input_UnQMotion () {
    curimage->UnQMotion();
}

void input_QButton (InputButton b) {
    curimage->QButton(b);
}

void input_UnQButton (InputButton b) {
    curimage->UnQButton(b);
}

void input_QMouseButtons () {
    curimage->QMouseButtons();
}

void input_UnQMouseButtons () {
    curimage->UnQMouseButtons();
}

void input_QCharacter () {
    curimage->QCharacter();
}

void input_UnQCharacter () {
    curimage->UnQCharacter();
}

void input_UnQAll () {
    curimage->UnQAll();
}

void input_QRead (ImageEvent& e) {
    curimage->QRead(e);
    SetSizes();
}

boolean input_QTest (ImageEvent& e) {
    boolean b;

    b = curimage->QTest(e);
    SetSizes();
    return b;
}

void input_StartQRead () {
    curimage->StartQRead();
}

void input_FinishQRead (ImageEvent& e) {
    curimage->FinishQRead(e);
}

void input_QReset () {
    curimage->QReset();
}

void input_MapKey (
    InputButton b, char c, char shift, char ctrl, char shiftctrl
) {
    curimage->MapKey(b, c, shift, ctrl, shiftctrl);
}

void input_UnMapKey (InputButton b) {
    curimage->UnMapKey(b);
}

void input_EnableShiftLock () {
    curimage->EnableShiftLock();
}

void input_DisableShiftLock () {
    curimage->DisableShiftLock();
}

void input_EnableMetaKey () {
    curimage->EnableMetaKey();
}

void input_DisableMetaKey () {
    curimage->DisableMetaKey();
}

boolean input_KeyToChar (InputButton b, boolean down, int& c) {
    return curimage->KeyToChar(b, down, c);
}

int input_DefCursor (int xoff, int yoff, short* c) {
    CursorPattern p;
    register int i;

    for (i = 0; i < cursorHeight; i++) {
	p[i] = c[i];
    }
    return (int) new Cursor(xoff, yoff, p, p, black, white);
}

void input_SetCursor (int n) {
    curimage->SetCursor((Cursor*) n);
}

int input_GetCursor () {
    return (int) (curimage->GetCursor());
}

void input_CursorOn () {
    curimage->CursorOn();
}

void input_CursorOff () {
    curimage->CursorOff();
}

void input_SetCursorPosition (XCoord x, YCoord y) {
    curimage->SetCursorPosition(x, y);
}

void input_GetCursorPosition (XCoord& x, YCoord& y) {
    curimage->GetCursorPosition(x, y);
}

/*
 * For Modula-2.
 */

int buttons_state;
int cursors_curxoff, cursors_curyoff;

void vdi__init () {}
void input__init () {}
void config__init () {}
void rawproto__init () {}
void client__init () {}
void inputq__init () {}
void buttons__init () {}
void cursors__init () {}
