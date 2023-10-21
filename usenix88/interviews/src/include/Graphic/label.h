/*
 * Interface to Label, an object derived from Graphic.
 */

#ifndef label_h
#define label_h

#include "base.h"

class Label : public Graphic {
    Ref font;
protected:
    char* string;
    int count;
    boolean read(File*);
    boolean write(File*);

    void getBounds(float&, float&, float&, float&, Graphic*);
    boolean contains(PointObj&, Graphic*);
    boolean intersects(BoxObj&, Graphic*);
    void draw(Canvas*, Graphic*);
public:
    ClassId GetClassId() { return LABEL; }
    boolean IsA(ClassId id) { return GetClassId() == id || Graphic::IsA(id); }

    Label();
    Label(char* string, Graphic* pict =nil);
    Label(char* string, int count, Graphic* pict =nil);
    ~Label() { delete string; }

    Graphic* Copy () { return new Label(string, count, this); }
    void GetOriginal(char*&, int&);

    void SetFont(PFont*);
    PFont* GetFont();
};

#endif
