/*
 * Interface to Graphic base class and FullGraphic, a subclass of Graphic
 * for which all graphics state is defined.
 */

#ifndef base_h
#define base_h

#include "geomobjs.h"
#include "grclasses.h"
#include "ppaint.h"
#include <InterViews/painter.h>

const int UNDEF = -1;

class Canvas;
class GraphicToPainter;

class Graphic : public Persistent {
    friend class Picture;
    friend class Instance;

    Ref parent;
    Ref origin;
    int fillBg;
    Ref fg;
    Ref bg;
    Ref tag;
    Transformer* t;
    static GraphicToPainter* painters;
    static Painter* p;

    void concat(Graphic* gr, Graphic*& dest);
	// sets *dest to the concatenation of this's graphics state with
	// gr's (gr's graphics state dominates).  Creates dest if nil.
    void concatOrigin(Graphic* gr, Ref& dest);
	// concatenates this's and gr's origins, puts Ref to result in dest.
    void concatTransformer(Graphic* gr, Transformer*& dest);
	// concatenates this's and gr's transformers, assigns result to 
	// *dest.  Creates dest if nil.
    void alignBounds (Graphic*, Graphic*);
	// shorthand for finding the bounds of two graphics at once
protected:
    static Transformer* identity;   // identity matrix
    static boolean caching;	    // state of bounding box caching
    
    void cachingOn();
    void cachingOff();

    void transform(Coord& x, Coord& y);
    void transform(Coord x, Coord y, Coord& tx, Coord& ty);
    void transform(float x, float y, float& tx, float& ty);
    void transformList(Coord x[], Coord y[], int n, Coord tx[], Coord ty[]);
    void transformRect(float,float,float,float,float&,float&,float&,float&);
    void invTransform(Coord& tx, Coord& ty);
    void invTransform(Coord tx, Coord ty, Coord& x, Coord& y);
    void invTransform(float tx, float ty, float& x, float& y);
    void invTransformList(Coord tx[], Coord ty[], int n, Coord x[], Coord y[]);

    Graphic* getRoot();			    // top level parent
    void parentGS(Graphic& p);		    // parents' graphics state
    void parentXform(Transformer& t);	    // parents' transformation

    void pText(Canvas* c, char* s, int n) { p->Text(c, s, n); }
    void pPoint(Canvas* c, Coord x, Coord y) { p->Point(c, x, y); }
    void pMultiPoint(Canvas*, Coord x[], Coord y[], int);
    void pLine(Canvas*, Coord, Coord, Coord, Coord);
    void pRect(Canvas*, Coord, Coord, Coord, Coord);
    void pFillRect(Canvas*, Coord, Coord, Coord, Coord);
    void pEllipse(Canvas*, Coord, Coord, int, int);
    void pFillEllipse(Canvas*, Coord, Coord, int, int);
    void pMultiLine(Canvas*, Coord x[], Coord y[], int);
    void pPolygon(Canvas*, Coord x[], Coord y[], int);
    void pFillPolygon(Canvas*, Coord x[], Coord y[], int);
    void pBSpline(Canvas*, Coord x[], Coord y[], int);
    void pClosedBSpline(Canvas*, Coord x[], Coord y[], int);
    void pFillBSpline(Canvas*, Coord x[], Coord y[], int);
	// provide painter-equivalent rendering functions so that users
	// won't manipulate the painter directly

    boolean read(File*);
    boolean write(File*);
    boolean readObjects(File*);
    boolean writeObjects(File*);

    void update(Graphic* gs);		    // updates painter w/gs' state
    boolean parentBoundsCached();
    virtual boolean boundsCached() { return false; }
    void uncacheParentBounds();
    virtual void uncacheBounds() { uncacheParentBounds(); }

/*
 * Versions of member functions that have an extra Graphic* parameter at the
 * end take into account that graphic's graphic state information when
 * performing their function.  This is useful in hierarchical graphic objects
 * (such as Pictures) where higher-level graphics' graphics state influences
 * lower (and ultimately leaf) graphics'.
 */
    virtual void getBounds(float&, float&, float&, float&, Graphic*) { }
    void getBox(Coord&, Coord&, Coord&, Coord&, Graphic*);
    void getBox(BoxObj& b, Graphic* p)
	{ getBox(b.left, b.bottom, b.right, b.top, p); }
    virtual boolean contains(PointObj&, Graphic*) { return false; }
    virtual boolean intersects(BoxObj&, Graphic*) { return false; }

    virtual void draw(Canvas*, Graphic* gs);
	// should call update & then blast bits; shouldn't modify gs
    virtual void drawClipped(Canvas*, Coord,Coord,Coord,Coord, Graphic* gs);
	// should draw within clipping rectangle based on gs
    virtual void erase(Canvas*, Graphic*);
    virtual void eraseClipped(Canvas*, Coord,Coord,Coord,Coord, Graphic* gs);
public:
    ClassId GetClassId() { return GRAPHIC; }
    boolean IsA(ClassId id) { return GetClassId()==id || Persistent::IsA(id); }
    Persistent* GetCluster();

    Graphic(Graphic* gr =nil);
	// if gr is nil, use default values for graphics state;
	// otherwise copy pointers to graphics state objects
    ~Graphic();
    virtual Graphic* Copy () { return new Graphic(this); }

    virtual void FillBg(boolean);
    virtual int BgFilled() { return fillBg; }
    virtual void SetColors(PColor* f, PColor* b);
    virtual PColor* GetFgColor();
    virtual PColor* GetBgColor();
    virtual void SetPattern(PPattern*) { }
    virtual PPattern* GetPattern() { return nil; }
    virtual void SetBrush(PBrush*) { }
    virtual PBrush* GetBrush() { return nil; }
    virtual void SetFont(PFont*) { }
    virtual PFont* GetFont() { return nil; }

    Graphic* Parent() { return (Graphic*) parent(); }
    virtual boolean HasChildren () { return false; }

    void SetOrigin(int x0, int y0);
    void GetOrigin(int& x0, int& y0);
    void SetTag(Ref r) { tag = r; }
    Ref GetTag() { return tag; }
    virtual Graphic& operator =(Graphic&);

    void Translate(float dx, float dy);
    void Scale(float sx, float sy, float ctrx =0.0, float ctry =0.0);
    void Rotate(float angle, float ctrx =0.0, float ctry =0.0);
    void SetTransformer(Transformer*);
    Transformer* GetTransformer () { return t; }
    void TotalTransformation(Transformer&);

/*
 * Some definitions:
 * object coords:  object-relative, pre-object transformation
 * parent coords:  parent-relative, post-object transformation
 * origin coords:  origin-relative, post-parents' transformations
 * canvas coords:  canvas-relative, post-origin offset
 */
    void GetBounds(float&, float&, float&, float&);
	// bounding box in origin coordinates, floating point
    void GetBox(Coord&, Coord&, Coord&, Coord&);
    void GetBox(BoxObj& b) { GetBox(b.left, b.bottom, b.right, b.top); }
	// bounding box in origin coordinates, integer
    virtual void GetCenter(float&, float&);    
    virtual boolean Contains(PointObj&);
    virtual boolean Intersects(BoxObj&);

    virtual void Draw(Canvas*);
    virtual void Draw(Canvas*, Coord, Coord, Coord, Coord);
	// draw graphic within given area without fine clipping
    virtual void DrawClipped(Canvas*, Coord, Coord, Coord, Coord);
	// draw graphic clipped to given origin coordinates
    virtual void Erase(Canvas*);
    virtual void Erase(Canvas*, Coord, Coord, Coord, Coord);
	// erase graphic within given area without fine clipping
    virtual void EraseClipped(Canvas*, Coord, Coord, Coord, Coord);
	// erase graphic clipped to given origin coordinates

    void CenterVert(Graphic*);
    void CenterHoriz(Graphic*);
    void Center(Graphic*);
    void AlignLeft(Graphic*);
    void AlignRight(Graphic*);
    void AlignTop(Graphic*);
    void AlignBottom(Graphic*);
    void AbutLeft(Graphic*);
    void AbutRight(Graphic*);
    void AbutUp(Graphic*);
    void AbutDown(Graphic*);
};

class FullGraphic : public Graphic {
    Ref pat;
    Ref brush;
    Ref font;
protected:
    boolean read(File*);
    boolean write(File*);
public:
    ClassId GetClassId() { return FULL_GRAPHIC; }
    boolean IsA(ClassId id) { return GetClassId() == id || Graphic::IsA(id); }

    FullGraphic(Graphic* =nil);
    Graphic* Copy () { return new FullGraphic(this); }

    void SetPattern(PPattern*);
    PPattern* GetPattern();
    void SetBrush(PBrush*);
    PBrush* GetBrush();
    void SetFont(PFont*);
    PFont* GetFont();
};

inline void Graphic::pMultiPoint(Canvas* c, Coord x[], Coord y[], int n) {
    p->MultiPoint(c, x, y, n);
}

inline void Graphic::pLine(Canvas* c, Coord x1, Coord y1, Coord x2, Coord y2) {
    p->Line(c, x1, y1, x2, y2);
}

inline void Graphic::pRect(Canvas* c, Coord x1, Coord y1, Coord x2, Coord y2) {
    p->Rect(c, x1, y1, x2, y2);
}

inline void Graphic::pFillRect(Canvas* c,Coord x1,Coord y1, Coord x2,Coord y2){
    p->FillRect(c, x1, y1, x2, y2);
}

inline void Graphic::pEllipse(Canvas* c, Coord x, Coord y, int r1, int r2) {
    p->Ellipse(c, x, y, r1, r2);
}

inline void Graphic::pFillEllipse(Canvas* c, Coord x, Coord y, int r1, int r2){
    p->FillEllipse(c, x, y, r1, r2);
}

inline void Graphic::pMultiLine(Canvas* c, Coord x[], Coord y[], int n) {
    p->MultiLine(c, x, y, n);
}

inline void Graphic::pPolygon(Canvas* c, Coord x[], Coord y[], int n) {
    p->Polygon(c, x, y, n);
}

inline void Graphic::pFillPolygon(Canvas* c, Coord x[], Coord y[], int n) {
    p->FillPolygon(c, x, y, n);
}

inline void Graphic::pBSpline(Canvas* c, Coord x[], Coord y[], int n) {
    p->BSpline(c, x, y, n);
}

inline void Graphic::pClosedBSpline(Canvas* c, Coord x[], Coord y[], int n) {
    p->ClosedBSpline(c, x, y, n);
}

inline void Graphic::pFillBSpline(Canvas* c, Coord x[], Coord y[], int n) {
    p->FillBSpline(c, x, y, n);
}

#endif
