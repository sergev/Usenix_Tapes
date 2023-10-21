/*
 * Implementation of Label, an object derived from Graphic.
 */

#include <InterViews/Graphic/label.h>

extern int strlen(const char*);
extern char* strncpy(char*, const char*, int);

void Label::draw (Canvas *c, Graphic* gs) {
    update(gs);
    pText(c, string, count);
}

Label::Label () {
    string = nil;
    count = 0;
}

Label::Label (char* string, Graphic* gr) : (gr) {
    if (gr == nil) {
	SetFont(nil);
    } else {
	SetFont(gr->GetFont());
    }
    count = strlen(string);
    this->string = new char[count];
    strncpy(this->string, string, count);
}

Label::Label (char* string, int count, Graphic* gr) : (gr) {
    if (gr == nil) {
	SetFont(nil);
    } else {
	SetFont(gr->GetFont());
    }
    this->string = new char[count];
    this->count = count;
    strncpy(this->string, string, count);
}

void Label::GetOriginal (char*& string, int& n) {
    string = new char[count];
    n = count;
    strncpy(string, this->string, count);
}

boolean Label::read (File* f) {
    boolean ok;
    ok = Graphic::read(f) && font.Read(f) && f->Read(count);
    if (ok) {
	delete string;
	string = new char [count];
	ok = f->Read(string, count);
    }
    return ok;
}

boolean Label::write (File* f) {
    return 
	Graphic::write(f) && font.Write(f) &&
	f->Write(count) && f->Write(string, count);
}

void Label::getBounds (float& x0, float& y0, float& x1, float& y1, Graphic* gs) {
    PFont* f = gs->GetFont();
    Transformer* t = gs->GetTransformer();

    if (t == nil) {
	x0 = 0;
	y0 = 0;
    } else {
	t->Transform(0, 0, x0, y0);
    }

    x1 = x0 + f->Width(string, count);
    y1 = y0 + f->Height();
}

boolean Label::contains (PointObj& p, Graphic* gs) {
    BoxObj b;

    getBox(b, gs);
    return b.Contains(p);
}

boolean Label::intersects (BoxObj& userb, Graphic* gs) {
    BoxObj b;

    getBox(b, gs);
    return b.Intersects(userb);
}

void Label::SetFont (PFont* font) {
    if (this->font != Ref(font)) {
	this->font = Ref(font);
	uncacheBounds();
    }
}

PFont* Label::GetFont () { 
    return (PFont*) font();
}
