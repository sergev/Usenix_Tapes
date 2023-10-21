/*
 * A menu is a list of items to select from using the mouse.
 */

#ifndef menu_h
#define menu_h

#include <InterViews/scene.h>

class Menu : public MonoScene {
    boolean persistent;
protected:
    Scene* layout;
    Interactor* cur;
    Coord xoff, yoff;

    void DoInsert(Interactor*, boolean, Coord&, Coord&, Shape*);
public:
    Menu(Sensor*, Painter*, boolean persistent = true);
    void Compose();
    void Popup(Event&, Interactor*&);
    void GetSelection (Interactor*& i) { i = cur; }
    void Handle(Event&);
};

class HMenu : public Menu {
    void DoInsert(Interactor*, boolean, Coord&, Coord&, Shape*);
public:
    HMenu(Sensor*, Painter*);
};

class VMenu : public Menu {
public:
    VMenu(Sensor*, Painter*);
};

/*
 * A menu item is an interactor that highlights itself when
 * it gets focus, and unhighlights itself when it loses focus.
 */

class MenuItem : public Interactor {
protected:
    class Painter* highlight;
    class Painter* normal;
public:
    MenuItem(Painter* out);
    void Delete();
    void Handle(Event&);
    virtual void Highlight();
    virtual void UnHighlight();
};

class TextItem : public MenuItem {
    const char* text;
public:
    TextItem(Painter*, const char*);
    void Draw();
    void Redraw(Coord, Coord, Coord, Coord);
};

#endif
