/*
 * Basic composite object for interaction.
 */

#ifndef scene_h
#define scene_h

#include <InterViews/interactor.h>

class Scene : public Interactor {
    void MakeWindow(Interactor*, Coord x, Coord y, int w, int h, boolean);
    void DoMap(Interactor*);
    void Assign(Interactor*, Coord x, Coord y, int w, int h);
    void InitCanvas(Interactor*);
protected:
    boolean propagate;

    Scene(Sensor*, Painter*);
    void UserPlace(Interactor*);
    void Place(Interactor*, Coord, Coord, Coord, Coord, boolean map = true);
    void Map(Interactor*, boolean raised = true);
    void Unmap(Interactor*);
    void Orphan(Interactor*);
    virtual void DoInsert(Interactor*, boolean, Coord& x, Coord& y, Shape*);
    virtual void DoChange(Interactor*, Shape*);
    virtual void DoMove(Interactor*, Coord& x, Coord& y);
    virtual void DoRemove(Interactor*);
public:
    void Insert(Interactor*);
    void Insert(Interactor*, Coord x, Coord y);
    void Change(Interactor*);
    void Move(Interactor*, Coord x, Coord y);
    void Remove(Interactor*);
    void Raise(Interactor*);
    void Lower(Interactor*);
    void Propagate(boolean);
};

/* Scene with a single component */
class MonoScene : public Scene {
protected:
    Interactor* component;

    virtual void DoInsert(Interactor*, boolean, Coord&, Coord&, Shape*);
    virtual void DoChange(Interactor*, Shape*);
public:
    MonoScene(Sensor*, Painter*);
    void Delete();
    void Resize();
    void Draw();
    void GetComponents(Interactor**&, int&);
};

#endif
