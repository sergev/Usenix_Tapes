/*
 * Base class for interactive objects.
 */

#ifndef interactor_h
#define interactor_h

#include <InterViews/defs.h>

extern class Sensor* stdsensor;		/* default input */
extern class Painter* stdpaint;		/* default output */

struct Event;

class Interactor {
    friend class Scene;
    friend class Image;
    
    const char* name;			/* associated string name */
    Interactor* icon;			/* icon to use, if any */
    Scene* parent;			/* where inserted */
    Coord left;				/* relative to parent */
    Coord bottom;			/* relative to parent */
    Sensor* cursensor;			/* current input interest */
    unsigned prevmask;			/* previous interest */

    boolean PrevEvent(Event&);
    boolean OtherEvent(Event&);
    boolean Select(Event&);
    boolean GetEvent(Event&, boolean);
    void FlushRedraws();
    void SendRedraw(Coord x, Coord y, int width, int height);
    void SendResize(int width, int height);
protected:
    class Shape* shape;			/* desired shape */
    class Canvas* canvas;		/* actual display area */
    class Perspective* perspective;	/* portion displayed */
    Coord xmax;				/* canvas->Width() - 1 */
    Coord ymax;				/* canvas->Height() - 1 */
    Sensor* input;			/* normal input event interest */
    Painter* output;			/* normal output parameters */
    class Cursor* cursor;		/* cursor when in canvas */
public:
    Interactor(Sensor* in = stdsensor, Painter* out = stdpaint);
    ~Interactor();
    Scene* Parent () { return parent; }
    void GetRelative (Coord& x, Coord &y, Interactor* = nil);
    Perspective* GetPerspective () { return perspective; }
    void Listen(Sensor*);
    void SetCursor(Cursor*);
    void SetName(const char*);
    void SetIcon(Interactor*);
    void Iconify();
    void DeIconify();
    void Read(Event&);
    void UnRead(Event&);
    boolean Check();
    boolean CheckQueue();
    void Poll(Event&);
    void Sync();
    virtual void Delete();
    virtual void Resize();
    virtual void Draw();
    virtual void Redraw(Coord left, Coord bottom, Coord right, Coord top);
    virtual void Handle(Event&);
    virtual void Adjust(Perspective&);
    virtual void Reshape(Shape&);
    virtual void Update();
    virtual void GetComponents(Interactor**&, int&);
};

#endif
