/*
 * Layout - a chain of TextBlocks
 */

#ifndef layout_h
#define layout_h

#include "textblock.h"

class Text;
class PaintMap;
class Context;
class TextPainter;
class Region;

class Layout {
    TextBlock* top, * tail;
    TextBlock* block;

    PaintMap* painters;
    Context** contexts;

    Coord x, y;
    Coord left, right;
    Coord width;

    Region* damage, * extradamage, * target;

    void* hit;
    void* lastcontext;

    boolean intarget;

    inline boolean PreTarget(Coord x, Coord y);
    inline boolean PostTarget(Coord x, Coord y);
    inline boolean InTarget();

    void Flush();
    void GoTo(Coord x, Coord y);
    void Overfull();
    void LastLine();
protected:
    Text* contents;
    TextPainter* output;
    Sensor* input;
public:
    Layout(Text*, Sensor*, TextPainter*);
    ~Layout();

    void Chain(TextBlock*);
    void Unchain(TextBlock*);
    void Rechain();

    Coord X() { return x; }
    Coord Y() { return y; }
    Coord Remaining() { return width-right-left-x; }
    boolean Drawing() { return target != nil; }

    void Margins(int left, int right);
    boolean SkipTo(Coord x, Coord y);

    void Damage(Coord x1, Coord y1, Coord x2, Coord y2);
    void ExtraDamage(Coord x, Coord y);
    void EndDamage(Coord x, Coord y);

    void String(const char* s, int length);
    void NewLine();
    void Space(int count);
    void Backspace(int count);
    void Caret();

    void Enter(void* context);
    void Leave(void* context);

    void Listen(Sensor*);
    void Paint(void* context, TextPainter* paint);
    void Unpaint(void* context);

    void Repair();
    void Touch(Text*);
    void Show(Text*);

    boolean Hit(Event&, void*& context, void*& after);
};

#endif
