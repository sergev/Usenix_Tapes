/*
 * Persistent graphics attributes.
 */

#ifndef ppaint_h
#define ppaint_h

#include "file.h"
#include "grclasses.h"
#include "objman.h"
#include "persistent.h"
#include "ref.h"
#include "reflist.h"
#include <InterViews/defs.h>
#include <InterViews/paint.h>

class PColor : public Persistent {
    typedef Color* ColorPtr;
protected:
    Color* value;
    boolean read(File*);
    boolean write(File*);
public:
    ClassId GetClassId() { return PCOLOR; }
    boolean IsA(ClassId id) { return GetClassId()==id || Persistent::IsA(id); }

    PColor();
    PColor(int r, int g, int b);
    PColor(int index, int r, int g, int b);
    PColor(int index);
    PColor(const char*);
    ~PColor();
    int PixelValue () { return value->PixelValue(); }
    void Intensities(int& r, int& g, int& b) { value->Intensities(r, g, b); }
    boolean Valid () { return value->Valid(); }
    operator ColorPtr() { return value; }
};

class PPattern : public Persistent {
    typedef Pattern* PatternPtr;
protected:
    int data [patternHeight];
    Pattern* value;
    boolean read(File*);
    boolean write(File*);
public:
    ClassId GetClassId() { return PPATTERN; }
    boolean IsA(ClassId id) { return GetClassId()==id || Persistent::IsA(id); }

    PPattern();
    PPattern(int p[patternHeight]);
    PPattern(int dither);
    ~PPattern();
    operator PatternPtr() { return value; }
};

const int NO_WIDTH = -1;

class PBrush : public Persistent {
    typedef Brush* BrushPtr;
protected:
    int p;
    Brush* value;
    boolean read(File*);
    boolean write(File*);
public:
    ClassId GetClassId () { return PBRUSH; }
    boolean IsA(ClassId id) { return GetClassId()==id || Persistent::IsA(id); }

    PBrush();
    PBrush(int p, int w =1);
    ~PBrush();
    int Width () { return (value == nil) ? NO_WIDTH : value->Width(); }
    operator BrushPtr() { return value; }
};

class PFont : public Persistent {
    typedef Font* FontPtr;
protected:
    char* name;
    int count;
    Font* value;
    boolean read(File*);
    boolean write(File*);
public:
    ClassId GetClassId() { return PFONT; }
    boolean IsA(ClassId id) { return GetClassId()==id || Persistent::IsA(id); }

    PFont();
    PFont(const char*);
    PFont(const char*, int);
    ~PFont();
    int Baseline () { return value->Baseline(); }
    int Height() { return value->Height(); }
    int Width(const char* s) { return value->Width(s); }
    int Width(const char* s, int len) { return value->Width(s, len); }
    int Index(const char*, int offset, boolean between);
    int Index(const char*, int, int offset, boolean between);
    boolean Valid () { return value->Valid(); }
    boolean FixedWidth() { return value->FixedWidth(); }
    operator FontPtr() { return value; }
};

/*
 * Standard attributes.  These must be initialized by calling
 * InitPPaint.  If the persistence features of paints are not required,
 * then InitPPaint must be called explicitly before any of the standard
 * attributes are used.  Otherwise the call to InitPPaint should be
 * made in the persistent object initialization routine passed to the
 * object manager constructor.  See Persistent(3I) for more information.
 */
extern PColor* pblack;
extern PColor* pwhite;
extern PPattern* psolid;
extern PPattern* pclear;
extern PBrush* psingle;
extern PBrush* pnone;
extern PFont* pstdfont;
extern void InitPPaint();

#endif
