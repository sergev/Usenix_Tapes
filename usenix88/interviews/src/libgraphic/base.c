/*
 * Graphic base class implementation.
 */

#include <InterViews/canvas.h>
#include <InterViews/Graphic/base.h>
#include <InterViews/Graphic/hash.h>
#include <InterViews/Graphic/util.h>

Graphic& Graphic::operator = (Graphic& p) {
    SetColors(p.GetFgColor(), p.GetBgColor());
    origin = p.origin;
    FillBg(p.BgFilled());
    SetPattern(p.GetPattern());
    SetBrush(p.GetBrush());
    SetFont(p.GetFont());
    if (p.t == nil) {
	delete t;
	t = nil;
    } else {
	if (t == nil) {
	    t = new Transformer(p.t);
	} else {
	    *t = *p.t;
	}
    }
    uncacheBounds();
    return *this;
}

void Graphic::update (Graphic* gs) {
    Transformer* t;
    Coord x, y;

    p = painters->Find(gs);

    if (gs->origin.Valid()) {
	gs->GetOrigin(x, y);
	p->SetOrigin(x, y);
    } else {
	p->SetOrigin(0, 0);
    }
    p->MoveTo(0, 0);
    t = p->GetTransformer();
    if (t == nil) {
	if (gs->t != nil) {
	    p->SetTransformer(new Transformer(gs->t));
	}
    } else {
	if (gs->t == nil) {
	    *t = *identity;
	} else {
	    *t = *gs->t;
	}
    }
}

void Graphic::cachingOn () {
    caching = true;
}

void Graphic::cachingOff () {
    caching = false;
}

void Graphic::transform(Coord& x, Coord& y) {
    if (t != nil) {
	t->Transform(x, y);
    }
}

void Graphic::transform(Coord x, Coord y, Coord& tx, Coord& ty) {
    if (t != nil) {
	t->Transform(x, y, tx, ty);
    } else {
	tx = x;
	ty = y;
    }
}

void Graphic::transform(float x, float y, float& tx, float& ty) {
    if (t != nil) {
	t->Transform(x, y, tx, ty);
    } else {
	tx = x;
	ty = y;
    }
}

void Graphic::transformList(
    Coord x[], Coord y[], int n, Coord tx[], Coord ty[]
) {
    if (t != nil) {
	t->TransformList(x, y, n, tx, ty);
    } else {
	CopyArray(x, y, n, tx, ty);
    }
}

void Graphic::transformRect (
    float x0, float y0, float x1, float y1,
    float& nx0, float& ny0, float& nx1, float& ny1
) {
    float tx00, ty00, tx10, ty10, tx11, ty11, tx01, ty01;

    if (t != nil) {
	t->Transform(x0, y0, tx00, ty00);
	t->Transform(x1, y0, tx10, ty10);
	t->Transform(x1, y1, tx11, ty11);
	t->Transform(x0, y1, tx01, ty01);
	nx0 = fmin(tx00, fmin(tx01, fmin(tx10, tx11)));
	ny0 = fmin(ty00, fmin(ty01, fmin(ty10, ty11)));
	nx1 = fmax(tx00, fmax(tx01, fmax(tx10, tx11)));
	ny1 = fmax(ty00, fmax(ty01, fmax(ty10, ty11)));
    } else {
	nx0 = x0; ny0 = y0; nx1 = x1; ny1 = y1;
    }
}

void Graphic::invTransform(Coord& tx, Coord& ty) {
    if (t != nil) {
	t->InvTransform(tx, ty);
    }
}

void Graphic::invTransform(Coord tx, Coord ty, Coord& x, Coord& y) {
    if (t != nil) {
	t->InvTransform(tx, ty, x, y);
    } else {
	x = tx;
	y = ty;
    }
}

void Graphic::invTransform(float tx, float ty, float& x, float& y) {
    if (t != nil) {
	t->InvTransform(tx, ty, x, y);
    } else {
	x = tx;
	y = ty;
    }
}

void Graphic::invTransformList(
    Coord tx[], Coord ty[], int n, Coord x[], Coord y[]
) {
    if (t != nil) {
	t->InvTransformList(tx, ty, n, x, y);
    } else {
	CopyArray(tx, ty, n, x, y);
    }
}

Graphic* Graphic::getRoot () {
    Graphic* cur, *parent = this;
    
    do {
	cur = parent;
	parent = cur->Parent();
    } while (parent != nil);
    return cur;
}

void Graphic::parentGS (Graphic& p) {
    Graphic* parent = Parent();
    
    while (parent != nil) {
	p.concat(parent, &p);
	parent = parent->Parent();
    }
}

void Graphic::parentXform (Transformer& t) {
    Graphic* parent = Parent();
    
    while (parent != nil) {
	if (parent->t != nil) {
	    t.Postmultiply(parent->t);
	}
	parent = parent->Parent();
    }
}

boolean Graphic::read (File* f) {
    int test;
    float a[6];
    boolean ok = 
	Persistent::read(f) && parent.Read(f) && origin.Read(f) && 
	f->Read(fillBg) && fg.Read(f) && bg.Read(f) &&
	tag.Read(f) && f->Read(test);
    if (ok) {
	if (test != int(nil)) {
	    ok = f->Read(a, 6);
	    t = new Transformer(a[0], a[1], a[2], a[3], a[4], a[5]);
	}
    }
    return ok;
}

boolean Graphic::write (File* f) {
    float a[6];
    boolean ok = 
	Persistent::write(f) && parent.Write(f) && origin.Write(f) && 
	f->Write(fillBg) && fg.Write(f) && bg.Write(f) &&
	tag.Write(f) && f->Write(int(t));

    if (ok && t != nil) {
	t->GetEntries(a[0], a[1], a[2], a[3], a[4], a[5]);
	ok = f->Write(a, 6);
    }
    return ok;
}

boolean Graphic::readObjects (File* f) {
    boolean ok = Persistent::readObjects(f);
    if (origin.Valid()) {
	ok = ok && origin.ReadObjects(f);
    }
    return ok;
}

boolean Graphic::writeObjects (File* f) {
    boolean ok = Persistent::writeObjects(f);
    if (origin.Valid()) {
	ok = ok && origin.WriteObjects(f);
    }
    return ok;
}

void Graphic::concat (Graphic* gr, Graphic*& dest) {
    int fill;
    PColor* fg, *bg;
    PFont* font;
    PBrush* br;
    PPattern* pat;
    
    if (dest == nil) {
	dest = new FullGraphic;
    }
    if ((fill = gr->BgFilled()) == UNDEF) {
	fill = BgFilled();
    }
    dest->FillBg(fill);

    if ((fg = gr->GetFgColor()) == nil) {
	fg = GetFgColor();
    }
    if ((bg = gr->GetBgColor()) == nil) {
	bg = GetBgColor();
    }
    dest->SetColors(fg, bg);

    if ((pat = gr->GetPattern()) == nil) {
	pat = GetPattern();
    }
    dest->SetPattern(pat);

    if ((font = gr->GetFont()) == nil) {
	font = GetFont();
    }
    dest->SetFont(font);

    if ((br = gr->GetBrush()) == nil) {
	br = GetBrush();
    }
    dest->SetBrush(br);

    concatOrigin(gr, dest->origin);
    concatTransformer(gr, dest->t);
}

void Graphic::concatOrigin (Graphic* gr, Ref& dest) {
    dest = gr->origin;
    if (!dest.Valid()) {
	dest = origin;
    }
}

void Graphic::concatTransformer (Graphic* gr, Transformer*& dest) {
    if (dest == nil) {
	dest = new Transformer;
    }
    if (t != nil || gr->t != nil) {
	if (gr->t == nil) {
	    *dest = *t;
	} else if (t == nil) {
	    *dest = *gr->t;
	} else {	// both are nonnil
	    *dest = *t;
	    dest->Postmultiply(gr->t);
	}
    }
}

void Graphic::getBox (Coord& x0, Coord& y0, Coord& x1, Coord& y1, Graphic* gs) {
    float left, bottom, right, top;

    getBounds(left, bottom, right, top, gs);
    x0 = Coord(left - 1);
    y0 = Coord(bottom - 1);
    x1 = Coord(right + 1);
    y1 = Coord(top + 1);
}

void Graphic::GetBounds (float& x0, float& y0, float& x1, float& y1) {
    FullGraphic parents(this);
    
    parentGS(parents);
    getBounds(x0, y0, x1, y1, &parents);
}

void Graphic::GetBox (Coord& x0, Coord& y0, Coord& x1, Coord& y1) {
    float left, bottom, right, top;

    GetBounds(left, bottom, right, top);
    x0 = Coord(left - 1);
    y0 = Coord(bottom - 1);
    x1 = Coord(right + 1);
    y1 = Coord(top + 1);
}

void Graphic::draw (Canvas*, Graphic*) {
    // nop default
}

void Graphic::erase (Canvas* c, Graphic* gs) {
    PColor* fg = gs->GetFgColor();
    PColor* bg = gs->GetBgColor();
    gs->SetColors(bg, bg);
    draw(c, gs);
    gs->SetColors(fg, bg);
}

void Graphic::drawClipped (
    Canvas* c, Coord left, Coord bottom, Coord right, Coord top, Graphic* gs
) {
    BoxObj thisBox;
    BoxObj clipBox(left, bottom, right, top);

    getBox(thisBox, gs);
    if (clipBox.Intersects(thisBox)) {
	draw(c, gs);
    }
}

void Graphic::eraseClipped (
    Canvas* c, Coord left, Coord bottom, Coord right, Coord top, Graphic* gs
) {
    BoxObj thisBox;
    BoxObj clipBox(left, bottom, right, top);

    getBox(thisBox, gs);
    if (clipBox.Intersects(thisBox)) {
	erase(c, gs);
    }
}

void Graphic::Draw (Canvas* c) {
    if (Parent() == nil) {
	draw(c, this);
    } else {
	FullGraphic parents(this);
	parentGS(parents);
	draw(c, &parents);
    }
}

void Graphic::Draw (Canvas* c, Coord l, Coord b, Coord r, Coord t) {
    if (Parent() == nil) {
	drawClipped(c, l, b, r, t, this);
    } else {
	FullGraphic parents(this);
	parentGS(parents);
	drawClipped(c, l, b, r, t, &parents);
    }
}

void Graphic::DrawClipped (Canvas* c, Coord l, Coord b, Coord r, Coord t) {
    painters->Clip(c, l, b, r, t);
    if (Parent() == nil) {
	drawClipped(c, l, b, r, t, this);
    } else {
	FullGraphic parents(this);
	parentGS(parents);
	drawClipped(c, l, b, r, t, &parents);
    }
    painters->NoClip();
}

void Graphic::Erase (Canvas* c) {
    if (Parent() == nil) {
	erase(c, this);
    } else {
	FullGraphic parents(this);
	parentGS(parents);
	erase(c, &parents);
    }
}

void Graphic::Erase (Canvas* c, Coord l, Coord b, Coord r, Coord t) {
    if (Parent() == nil) {
	eraseClipped(c, l, b, r, t, this);
    } else {
	FullGraphic parents(this);
	parentGS(parents);
	eraseClipped(c, l, b, r, t, &parents);
    }
}

void Graphic::EraseClipped (Canvas* c, Coord l, Coord b, Coord r, Coord t) {
    painters->Clip(c, l, b, r, t);
    if (Parent() == nil) {
	eraseClipped(c, l, b, r, t, this);
    } else {
	FullGraphic parents(this);
	parentGS(parents);
	eraseClipped(c, l, b, r, t, &parents);
    }
    painters->NoClip();
}

boolean Graphic::parentBoundsCached () {
    Graphic* parent = Parent();
    
    if (parent == nil) {
	return true;
    } else {
	return parent->boundsCached();
    }
}

void Graphic::uncacheParentBounds () {
    Graphic* parent = Parent();
    
    if (parent != nil) {
	parent->uncacheBounds();
    }
}

Persistent* Graphic::GetCluster () {
    if (parent.Valid()) {
	return Parent()->GetCluster();
    } else {
	return this;
    }
}

static const int PAINTERS_SIZE = 100;

Graphic::Graphic (Graphic* gr) {
    PointObj* pt;
    
    parent = nil;
    tag = nil;
    t = nil;
    if (painters == nil) {
	painters = new GraphicToPainter(PAINTERS_SIZE);
	identity = new Transformer;
	cachingOn();
    }
    
    if (gr == nil) {
	origin = nil;
	FillBg(UNDEF);
	SetColors(nil, nil);
    } else {
	if (gr->origin.Valid()) {
	    pt = new PointObj((PointObj*) gr->origin());
	    origin = Ref(pt);
	    SetOrigin(pt->x, pt->y);
	} else {
	    origin = nil;
	}
	FillBg(gr->BgFilled());
	SetColors(gr->GetFgColor(), gr->GetBgColor());
	if (gr->t != nil) {
	    t = new Transformer(gr->t);
	}
    }
}

Graphic::~Graphic () {
    delete t;
}

void Graphic::GetCenter (float& x, float& y) {
    float x0, y0, x1, y1;

    GetBounds(x0, y0, x1, y1);
    x = (x0 + x1) / 2.0;
    y = (y0 + y1) / 2.0;
}    

boolean Graphic::Contains (PointObj& p) {
    if (Parent() == nil) {
        return contains(p, this);
    } else {
	FullGraphic parents(this);
	parentGS(parents);
	return contains(p, &parents);
    }
}

boolean Graphic::Intersects (BoxObj& b) {
    if (Parent() == nil) {
        return intersects(b, this);
    } else {
	FullGraphic parents(this);
	parentGS(parents);
	return intersects(b, &parents);
    }
}

void Graphic::SetOrigin (int x0, int y0) {
    PointObj* pt;
    
    if (origin.Valid()) {
	pt = (PointObj*) origin();
	if (pt->x != x0 || pt->y != y0) {
	    pt->x = x0;
	    pt->y = y0;
	    uncacheBounds();
	}
    } else {
	origin = Ref(new PointObj(x0, y0));
	uncacheBounds();
    }
}

void Graphic::GetOrigin (int& x0, int& y0) {
    Graphic parents;
    Transformer ttemp;
    PointObj* pt = nil;

    parents.SetTransformer(&ttemp);
    parentGS(parents);
    if (parents.origin.Valid()) {
	pt = (PointObj*) parents.origin();
    } else if (origin.Valid()) {
	pt = (PointObj*) origin();
    }
    if (pt == nil) {
	x0 = y0 = 0;
    } else {
	x0 = pt->x;
	y0 = pt->y;
    }
}

void Graphic::FillBg (boolean fbg) {
    fillBg = fbg;
}

void Graphic::SetColors (PColor* f, PColor* b) {
    fg = Ref(f);
    bg = Ref(b);
}

PColor* Graphic::GetFgColor () {
    return (PColor*) fg();
}

PColor* Graphic::GetBgColor () {
    return (PColor*) bg();
}

void Graphic::Translate (float dx, float dy) { 
    if (dx != 0 || dy != 0) {
	if (t == nil) {
	    t = new Transformer;
	}
	t->Translate(dx, dy);
	uncacheBounds();
    }
}

void Graphic::Scale (float sx, float sy, float cx, float cy) {
    if (sx != 1 || sy != 1) {
	if (t == nil) {
	    t = new Transformer;
	}
	if (cx != 0 || cy != 0) {
	    t->Translate(-cx, -cy);
	    t->Scale(sx, sy);
	    t->Translate(cx, cy);
	} else {
	    t->Scale(sx, sy);
	}
	uncacheBounds();
    }
}

void Graphic::Rotate (float angle, float cx, float cy) {
    float mag = (angle < 0) ? -angle : angle;

    if ((mag - int(mag)) != 0 || int(mag)%360 != 0) {
	if (t == nil) {
	    t = new Transformer;
	}
	if (cx != 0 || cy != 0) {
	    t->Translate(-cx, -cy);
	    t->Rotate(angle);
	    t->Translate(cx, cy);
	} else {
	    t->Rotate(angle);
	}
	uncacheBounds();
    }
}

void Graphic::SetTransformer (Transformer* t) {
    if (t != this->t) {
	delete this->t;
	if (t != nil) {
	    t->Reference();
	}
	this->t = t;
	uncacheBounds();
    }
}

void Graphic::TotalTransformation (Transformer& total) {
    if (t != nil) {
	total.Postmultiply(t);
    }
    parentXform(total);
}

static float fx0, fy0, fx1, fy1, mx0, my0, mx1, my1;

static void getCenters (Graphic* f, Graphic* m) {
    f->GetCenter(fx0, fy0);
    m->GetCenter(mx0, my0);
}

void Graphic::CenterVert (Graphic* p) {
    getCenters(this, p);
    Translate(0, fy0 - my0);
}

void Graphic::CenterHoriz (Graphic* p) {
    getCenters(this, p);
    Translate(fx0 - mx0, 0);
}

void Graphic::Center (Graphic* p) {
    getCenters(this, p);
    Translate(fx0 - mx0, fy0 - my0);
}

void Graphic::alignBounds (Graphic* f, Graphic* m) {
    f->getBounds(fx0, fy0, fx1, fy1, this);
    m->getBounds(mx0, my0, mx1, my1, this);
}

void Graphic::AlignLeft (Graphic* p) {
    alignBounds(this, p);
    Translate(fx0 - mx0, 0);
}

void Graphic::AlignRight (Graphic* p) {
    alignBounds(this, p);
    Translate(fx1 - mx1, 0);
}

void Graphic::AlignTop (Graphic* p) {
    alignBounds(this, p);
    Translate(0, fy1 - my1);
}

void Graphic::AlignBottom (Graphic* p) {
    alignBounds(this, p);
    Translate(0, fy0 - my0);
}

void Graphic::AbutLeft (Graphic* p) {
    alignBounds(this, p);
    Translate(fx0 - mx1, 0);
}

void Graphic::AbutRight (Graphic* p) {
    alignBounds(this, p);
    Translate(fx1 - mx0, 0);
}

void Graphic::AbutUp (Graphic* p) {
    alignBounds(this, p);
    Translate(0, fy1 - my0);
}

void Graphic::AbutDown (Graphic* p) {
    alignBounds(this, p);
    Translate(0, fy0 - my1);
}

boolean FullGraphic::read (File* f) {
    return Graphic::read(f) && pat.Read(f) && brush.Read(f) && font.Read(f);
}

boolean FullGraphic::write (File* f) {
    return Graphic::write(f) && pat.Write(f) && brush.Write(f) &&font.Write(f);
}

FullGraphic::FullGraphic (Graphic* gr) : (gr) {
    if (gr == nil) {
	SetPattern(nil);
	SetBrush(nil);
	SetFont(nil);
    } else {
	SetPattern(gr->GetPattern());
	SetBrush(gr->GetBrush());
	SetFont(gr->GetFont());
    }	
}

void FullGraphic::SetPattern (PPattern* pat) {
    this->pat = Ref(pat);
}

PPattern* FullGraphic::GetPattern () { 
    return (PPattern*) pat();
}

void FullGraphic::SetBrush (PBrush* brush) {
    this->brush = Ref(brush);
}

PBrush* FullGraphic::GetBrush () {
    return (PBrush*) brush();
}

void FullGraphic::SetFont (PFont* font) {
    if (this->font != Ref(font)) {
	this->font = Ref(font);
	uncacheBounds();
    }
}

PFont* FullGraphic::GetFont () { 
    return (PFont*) font();
}
