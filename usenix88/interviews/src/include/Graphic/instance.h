/*
 * Interface to Instance, a Graphic that refers to another Graphic.
 */

#ifndef instance_h
#define instance_h

#include "base.h"

class Instance : public FullGraphic {
protected:
    Ref refGr;
    Graphic* getGraphic() { return (Graphic*) refGr(); }

    boolean read(File*);
    boolean write(File*);

    void getBounds(float&, float&, float&, float&, Graphic*);
    boolean contains(PointObj&, Graphic*);
    boolean intersects(BoxObj&, Graphic*);
    void draw(Canvas*, Graphic*);
    void drawClipped(Canvas*, Coord, Coord, Coord, Coord, Graphic*);
public:
    ClassId GetClassId() { return INSTANCE; }
    boolean IsA(ClassId id) {return GetClassId()==id || FullGraphic::IsA(id);}

    Instance(Graphic* obj =nil, Graphic* gr =nil);

    Graphic* Copy () { return new Instance(getGraphic(), this); }
    void GetOriginal(Graphic*&);
};

#endif
