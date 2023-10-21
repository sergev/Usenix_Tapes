/*
 * Boxes are used to compose side-by-side.
 */

#ifndef box_h
#define box_h

#include <InterViews/scene.h>

typedef enum {
    BottomEdges, TopEdges, VertCenters
} HBoxAlignment;

typedef enum {
    LeftEdges, RightEdges, HorizCenters
} VBoxAlignment;

class BoxElement;
class BoxCanonical;

class Box : public Scene {
    int nelements;
    BoxElement* head;
    BoxElement* tail;

    void DoInsert(Interactor*, boolean, Coord& x, Coord& y, Shape*);
    void DoChange(Interactor*, Shape*);
    void DoRemove(Interactor*);
protected:
    Box();
    void ComputeShape();
    virtual void ResetShape();
    virtual void AddShape(Shape*);
    virtual void GetActual(int& major, int& minor);
    virtual void GetCanonical(Shape*, BoxCanonical&);
    virtual void PlaceElement(
	Interactor*, Coord major, int len, int thick, int minor
    );
public:
    void Delete();
    void Resize();
    void Draw();
    void GetComponents(Interactor**&, int&);
};

class HBox : public Box {
    HBoxAlignment align;

    void Init();
    void ResetShape();
    void AddShape(Shape*);
    void GetActual(int& major, int& minor);
    void GetCanonical(Shape*, BoxCanonical&);
    void PlaceElement(Interactor*, Coord, int, int, int);
public:
    HBox();
    HBox(Interactor*, Interactor*);
    HBox(Interactor*, Interactor*, Interactor*);
    HBox(Interactor*, Interactor*, Interactor*, Interactor*);
    void Align(HBoxAlignment);
};

class VBox : public Box {
    VBoxAlignment align;

    void Init();
    void ResetShape();
    void AddShape(Shape*);
    void GetActual(int& major, int& minor);
    void GetCanonical(Shape*, BoxCanonical&);
    void PlaceElement(Interactor*, Coord, int, int, int);
public:
    VBox();
    VBox(Interactor*, Interactor*);
    VBox(Interactor*, Interactor*, Interactor*);
    VBox(Interactor*, Interactor*, Interactor*, Interactor*);
    void Align(VBoxAlignment);
};

#endif
