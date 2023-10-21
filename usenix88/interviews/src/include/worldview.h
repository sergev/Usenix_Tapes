/*
 * Useful for writing window managers.
 */

#ifndef worldview_h
#define worldview_h

#include <InterViews/interactor.h>

typedef void* RemoteInteractor;

class WorldView : public Interactor {
    RemoteInteractor curfocus;
protected:
    class World* world;

    void GrabMouse(Cursor*);
    void UngrabMouse();
    boolean GrabButton(unsigned button, unsigned modifiers, Cursor*);
    void UngrabButton(unsigned button, unsigned modifiers);
    void Lock();
    void Unlock();
    void ClearInput();
    void MoveMouse(Coord x, Coord y);
    void Map(RemoteInteractor);
    void MapRaised(RemoteInteractor);
    void Unmap(RemoteInteractor);
public:
    WorldView(World*, Sensor* in = nil, Painter* out = stdpaint);
    void Delete();
    RemoteInteractor Find(Coord x, Coord y);
    RemoteInteractor Choose(Cursor*, boolean waitforup = true);
    virtual void InsertRemote(RemoteInteractor);
    virtual void ChangeRemote(
	RemoteInteractor, Coord left, Coord top, int w, int h
    );
    void Move(RemoteInteractor, Coord left, Coord top);
    void Change(RemoteInteractor, Coord left, Coord top, int w, int h);
    void Raise(RemoteInteractor);
    void Lower(RemoteInteractor);
    void Focus(RemoteInteractor);
    RemoteInteractor GetFocus () { return curfocus; }
    void GetList(RemoteInteractor*&, int&);
    void FreeList(RemoteInteractor*);
    void GetInfo(RemoteInteractor, Coord& x1, Coord& y1, Coord& x2, Coord& y2);
    RemoteInteractor GetIcon(RemoteInteractor);
    void AssignIcon(RemoteInteractor i, RemoteInteractor icon);
    void UnassignIcon(RemoteInteractor i);
    char* GetName(RemoteInteractor);
    void RedrawAll();
};

#endif
