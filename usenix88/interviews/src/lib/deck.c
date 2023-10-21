/*
 * Deck - a Scene for stacking Interactors
 */

#include <InterViews/deck.h>
#include <InterViews/perspective.h>
#include <InterViews/glue.h>

class Card {
public:
    Interactor* i;
    Card * next;
    Card * prev;
    Card(Interactor * ii = nil) { i = ii; next = prev = this; }
    ~Card() { next->prev = prev; prev->next = next; }
};

Deck::Deck(Painter *out) : (nil,out) {
    cards = new Card;
    Perspective * p = perspective = new Perspective;
    p->sx = p->lx = 1;
    p->sy = p->ly = 1;
    p->curwidth = p->curheight = 1;
    p->x0 = p->y0 = 1;
}

void Deck::Delete() {
    while ( cards->next != cards ) {
	delete cards->next->i;
	delete cards->next;
    }
    delete cards;
    delete perspective;
}
    
void Deck::ComputeShape() {
    int hnat = 0, hmin = 0, hmax = hfil;
    int vnat = 0, vmin = 0, vmax = vfil;
    int count = 0;
    Card * card = cards->next;
    while ( card != cards ) {
	Shape * s = card->i->shape;
	hnat = max(hnat, s->width);
	hmin = min(hmin, s->width - s->hshrink);
	hmax = max(hmax, s->width + s->hstretch);
	vnat = max(vnat, s->height);
	vmin = min(vmin, s->height - s->vshrink);
	vmax = max(vmax, s->height + s->vstretch);
	card = card->next;
	count += 1;
    }
    perspective->width = perspective->height = count;
    FixPerspective();
    shape->width = hnat;
    shape->hshrink = shape->width - hmin;
    shape->hstretch = hmax - shape->width;
    shape->height = vnat;
    shape->vshrink = shape->height - vmin;
    shape->vstretch = vmax - shape->height;
}

void Deck::FixPerspective() {
    register Perspective * p = perspective;
    if ( p->curx > p->width ) {
	p->curx = p->width;
    } else if ( p->curx < p->x0 ) {
	p->curx = p->x0;
    }
    if ( p->cury > p->height ) {
	p->cury = p->height;
    }
    p->Update();
}

void Deck::NewTop() {
    Card * card = cards;
    for ( int i = perspective->curx; i > 0; --i ) {
	card = card->next;
    }
    if ( top != nil && card->i != top ) {
	Map(card->i);
	Unmap(top);
	top = card->i;
    }
}

void Deck::DoInsert(Interactor *i, boolean, Coord&, Coord&, Shape*) {
    if ( i != nil ) {
	Card * c = new Card(i);
	c->prev = cards->prev;
	c->next = cards;
	cards->prev->next = c;
	cards->prev = c;
	++perspective->width;
	++perspective->height;
	ComputeShape();
    }
}

void Deck::DoRemove(Interactor *i) {
    Card * card = cards->next;
    while ( card != cards ) {
	if ( card->i == i ) {
	    delete card;
	    --perspective->width;
	    --perspective->height;
	    ComputeShape();
	    break;
	} else {
	    card = card->next;
	}
    }
}

void Deck::DoChange(Interactor *, Shape*) {
    ComputeShape();
}

void Deck::Resize() {
    int pos = 1;
    Card * card = cards->next;
    while ( card != cards ) {
	Interactor * i = card->i;
	Shape * s = i->shape;
	int l, r, b, t;
	int width = xmax+1;
	width = max(width, s->width - s->hshrink);
	width = min(width, s->width + s->hstretch);
	int height = ymax+1;
	height = max(height, s->height - s->vshrink);
	height = min(height, s->height + s->vstretch);
	l = (xmax+1-width)/2; r = xmax - l;
	b = (ymax+1-height)/2; t = ymax - b;
	if ( pos == perspective->curx ) {
	    top = i;
	    Place(i, l, b, r, t, true);
	} else {
	    Place(i, l, b, r, t, false);
	}
	card = card->next;
	pos += 1;
    }
}

void Deck::Draw() {
    if ( top != nil ) {
	top->Draw();
    }
}

void Deck::GetComponents (Interactor**& a, int& n) {
    Card* card;

    n = perspective->width;
    if (n > 0) {
	a = new Interactor*[n];
	register Interactor** c = a;
	for ( card = cards->next; card != cards; card = card->next ) {
	    *c++ = card->i;
	}
    }
}

void Deck::Adjust(Perspective& p) {
    *perspective = p;
    FixPerspective();
    NewTop();
}

void Deck::Flip(int count) {
    perspective->curx += count;
    perspective->cury += count;
    FixPerspective();
    NewTop();
}

void Deck::FlipTo(int position) {
    if ( position > 0 ) {
	perspective->curx = position;
	perspective->cury = position;
    } else if ( position < 0 ) {
	perspective->curx = perspective->width+1+position;
	perspective->cury = perspective->height+1+position;
    }
    FixPerspective();
    NewTop();
}
