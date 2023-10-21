/*
 * Implementation of squares subject.
 */

#include "subject.h"
#include "view.h"

Squares::Squares () {
    head = nil;
    views = nil;
}

Squares::~Squares () {
    register SquareData* p, * next;
    register SquaresViewList* v, * nextv;

    for (p = head; p != nil; p = next) {
	next = p->next;
	delete p;
    }
    for (v = views; v != nil; v = nextv) {
	nextv = v->next;
	delete v;
    }
}

void Squares::Add (float cx, float cy, float size) {
    register SquareData* s = new SquareData;
    s->cx = cx;
    s->cy = cy;
    s->size = size;
    s->next = head;
    head = s;
    Notify();
}

static float Random () {
#ifdef hpux
    extern int rand();
    const int bigint = 1<<14;
    int r = rand();
#else
    extern int random();
    const int bigint = 1<<30;
    int r = random();
#endif
    if (r < 0) {
	r = -r;
    }
    return float(r % bigint) / float(bigint);
}

void Squares::Add () {
    Add(Random()/2 + .25, Random()/2 + .25, Random()/2);
}

void Squares::Attach (SquaresView* v) {
    SquaresViewList* vl;

    vl = new SquaresViewList;
    vl->view = v;
    vl->next = views;
    views = vl;
}

void Squares::Detach (SquaresView* v) {
    SquaresViewList* vl;
    SquaresViewList* prev;

    prev = nil;
    for (vl = views; vl != nil; vl = vl->next) {
	if (vl->view == v) {
	    if (prev == nil) {
		views = vl->next;
	    } else {
		prev->next = vl->next;
	    }
	    delete vl;
	    return;
	}
    }
}

void Squares::Notify () {
    SquaresViewList* vl;

    for (vl = views; vl != nil; vl = vl->next) {
	vl->view->Update();
    }
}
