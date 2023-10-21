/*
 * InterViews squares demo program.
 */

#include "subject.h"
#include "view.h"
#include "sframe.h"
#include <InterViews/sensor.h>
#include <InterViews/world.h>

static Squares* MakeInitialSquares () {
    const initnumsquares = 3;
    Squares* s;
    int i;

    s = new Squares;
    for (i = 0; i < initnumsquares; i++) {
	s->Add();
    }
    return s;
}

static SquaresView* MakeInitialView () {
    return new SquaresView(MakeInitialSquares());
}

void main () {
    Event e;

    World* w = new World;
    w->Insert(new SquaresFrame(MakeInitialView(), stdpaint));
    w->Read(e);
    while (e.target != nil) {
	e.target->Handle(e);
	w->Read(e);
    }
}
