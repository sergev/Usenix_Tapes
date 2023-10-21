#ifndef desktop_h
#define desktop_h

#include <InterViews/worldview.h>
#include <InterViews/frame.h>
#include "lock.h"
#include "logo.h"

typedef enum {
    FOCUS = 0, LOWER = 1, RAISE = 2, REDRAW = 3, ICONIFY = 4, MOVE = 5,
    RESIZE = 6, SCREENLOCK = 7, EXIT = 8, NULL_OP = 9
} OperationCode;

const int Operations = 10;

class Icon : public Interactor {
    char* s;
    Coord xpos, ypos;
public:
    Icon(Painter*, const char*, int pad = 4);
    ~Icon();

    boolean IsOld(const char*);

    void Resize();
    void Redraw(Coord, Coord, Coord, Coord);
};

struct IconList {
    RemoteInteractor ri;
    Icon* icon;	
    IconList* next;
};

class DesktopDispatcher;

class Desktop : public WorldView {
    DesktopDispatcher* dispatch;
    IconList* icons;
    boolean running, lock_server, ignorecaps;
    boolean snap_resize, constrain_resize;
    Bitmap* logo_bitmap;
    ScreenLock* lock;
    Logo* logo;
    Coord logosize, logo_x, logo_y;

    boolean GrabEm();
    void UnGrabEm();
    void Config();

    void AddIcon(Icon*);
    void RemoveIcon(RemoteInteractor);
    boolean IconIsOld(RemoteInteractor, const char*);
    Icon* FindIcon(RemoteInteractor);
public:
    Desktop(World*);
    ~Desktop();

    void Run();

    void DoFocus(Event&);
    void DoLower(Event&);
    void DoRaise(Event&);
    void DoRedraw(Event&);
    void DoIconify(Event&);
    void DoMove(Event&);
    void DoResize(Event&);
    void DoLock(Event&);
    void DoExit(Event&);
    void InsertRemote(void*);
};

#endif
