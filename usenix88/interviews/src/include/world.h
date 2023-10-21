/*
 * Root scene for a display.
 */

#ifndef world_h
#define world_h

#include <InterViews/scene.h>

class World : public Scene {
    const char* prog;
    void* display;
    int screen;
    void* root;

    void DoInsert(Interactor*, boolean, Coord& x, Coord& y, Shape*);
    void DoChange(Interactor*, Shape*);
    void DoMove(Interactor*, Coord& x, Coord& y);
    void DoRemove(Interactor*);
public:
    World(const char* prog = nil, const char* device = nil);
    void Delete();
    int Fileno();
    void AsyncRead();
    void SyncRead();
    void SetCurrent();
    void SetRoot(void*);
    void SetScreen(int);
    int Width();
    int Height();
    int NPlanes();
    int NButtons();
    int PixelSize();
    Coord InvMapX(Coord);
    Coord InvMapY(Coord);
    char* GetDefault(const char*);
    char* GetGlobalDefault(const char*);
    unsigned ParseGeometry(char*, Coord& x, Coord& y, int& w, int& h);

    void RingBell(int);
    void SetKeyClick(int);
    void SetAutoRepeat(boolean);
    void SetFeedback(int thresh, int scale);
};

/*
 * Return bits from ParseGeometry.
 */

#define GeomNoValue 0x00
#define GeomXValue 0x01
#define GeomYValue 0x02
#define GeomWidthValue 0x04
#define GeomHeightValue 0x08
#define GeomAllValues 0x0F
#define GeomXNegative 0x10
#define GeomYNegative 0x20

#endif
