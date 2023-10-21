/*
 * Interface to Picture, a composite of one or more Graphics.
 */

#ifndef picture_h
#define picture_h

#include "base.h"

class Picture : public FullGraphic {
    FloatBoxObj* box;
    void setParent(Graphic*);
    void unsetParent(Graphic*);
protected:
    RefList* refList, *cur;
    Graphic* getGraphic(RefList* r) { return (Graphic*) (*r)(); }

    boolean read(File*);
    boolean write(File*);
    boolean readObjects(File*);
    boolean writeObjects(File*);
    
    boolean boundsCached () {
	return caching && box != nil && parentBoundsCached();
    }
    void cacheBounds(float, float, float, float);
    void uncacheBounds() { delete box; box = nil; uncacheParentBounds(); }
    void getCachedBounds(float&, float&, float&, float&);

    void getBounds(float&, float&, float&, float&, Graphic*);
    boolean contains(PointObj&, Graphic*);
    boolean intersects(BoxObj&, Graphic*);
    void draw(Canvas*, Graphic*);
    void drawClipped(Canvas*, Coord, Coord, Coord, Coord, Graphic*);
public:
    ClassId GetClassId () { return PICTURE; }
    boolean IsA (ClassId id) {return GetClassId()==id || FullGraphic::IsA(id);}

    Picture(Graphic* gr =nil);
    ~Picture();

    void Append(Graphic*, Graphic* =nil, Graphic* =nil, Graphic* =nil);
    void Prepend(Graphic*, Graphic* =nil, Graphic* =nil, Graphic* =nil);
    void InsertAfterCur(Graphic*, Graphic* =nil, Graphic* =nil, Graphic* =nil);
    void InsertBeforeCur(Graphic*, Graphic* =nil, Graphic* =nil,Graphic* =nil);
    void Remove(Graphic* p);
    void RemoveCur();
    void SetCurrent(Graphic* p);
    Graphic* GetCurrent () { return getGraphic(cur); }
    Graphic* First();
    Graphic* Last();
    Graphic* Next();
    Graphic* Prev();
    boolean IsEmpty () { return refList->IsEmpty(); }
    boolean AtEnd () { return cur == refList; }

    Graphic* FirstGraphicContaining(PointObj&);
    Graphic* LastGraphicContaining(PointObj&);
    Picture* GraphicsContaining(PointObj&);

    Graphic* FirstGraphicIntersecting(BoxObj&);
    Graphic* LastGraphicIntersecting(BoxObj&);
    Picture* GraphicsIntersecting(BoxObj&);

    Graphic* FirstGraphicWithin(BoxObj&);
    Graphic* LastGraphicWithin(BoxObj&);
    Picture* GraphicsWithin(BoxObj&);

    Graphic* Copy();
    boolean HasChildren () { return !IsEmpty(); }
    void Propagate();
};

#endif
