/*
 * Deck - a Scene for stacking Interactors
 */

#ifndef deck_h
#define deck_h

#include <InterViews/scene.h>

class Deck : public Scene {
    void ComputeShape();
    void FixPerspective();
    void NewTop();
protected:
    class Card* cards;
    Interactor* top;

    void DoInsert(Interactor*, boolean, Coord&, Coord&, Shape*);
    void DoChange(Interactor*, Shape*);
    void DoRemove(Interactor*);
public:
    Deck(Painter* = stdpaint);
    void Delete();
    void Resize();
    void Draw();
    void GetComponents(Interactor**&, int&);
    void Adjust(Perspective&);

    void Flip(int =1);
    void FlipTo(int);
    void Top () { FlipTo(1); }
    void Bottom () { FlipTo(-1); }
};

#endif
