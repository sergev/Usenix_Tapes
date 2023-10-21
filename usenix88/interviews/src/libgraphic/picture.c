/*
 * Picture class implementation.  A Picture is a Graphic that contains other 
 * Graphics.
 */

#include <InterViews/canvas.h>
#include <InterViews/Graphic/picture.h>

Picture::~Picture () {
    while (!refList->IsEmpty()) {
	cur = refList->First();
	refList->Remove(cur);
	delete cur;
    }
    delete refList;
    uncacheBounds();
}

void Picture::setParent (Graphic* p) {
    if (!p->parent.Valid()) {	    // a graphic can have only one parent
	p->parent = Ref(this);
    }
}

void Picture::unsetParent (Graphic* p) {
    p->parent = nil;
}

boolean Picture::read (File* f) {
    return FullGraphic::read(f) && refList->Read(f);
}

boolean Picture::readObjects (File* f) {
    return FullGraphic::readObjects(f) && refList->ReadObjects(f);
}

boolean Picture::write (File* f) {
    return FullGraphic::write(f) && refList->Write(f);
}

boolean Picture::writeObjects (File* f) {
    return FullGraphic::writeObjects(f) && refList->WriteObjects(f);
}

Picture::Picture (Graphic* gr) : (gr) {
    box = nil;
    refList = cur = new RefList(nil);
}

void Picture::Append (Graphic* p0, Graphic* p1, Graphic* p2, Graphic* p3) {
    refList->Append(new RefList(p0));
    setParent(p0);
    if (p1 != nil) {
	refList->Append(new RefList(p1));
	setParent(p1);
    }
    if (p2 != nil) {
	refList->Append(new RefList(p2));
	setParent(p2);
    }
    if (p3 != nil) {
	refList->Append(new RefList(p3));
	setParent(p3);
    }
    uncacheBounds();
}

void Picture::Prepend (Graphic* p0, Graphic* p1, Graphic* p2, Graphic* p3) {
    if (p3 != nil) {
	refList->Prepend(new RefList(p3));
	setParent(p3);
    }
    if (p2 != nil) {
	refList->Prepend(new RefList(p2));
	setParent(p2);
    }
    if (p1 != nil) {
	refList->Prepend(new RefList(p1));
	setParent(p1);
    }
    refList->Prepend(new RefList(p0));
    setParent(p0);
    uncacheBounds();
}

void Picture::InsertAfterCur (
    Graphic* p0, Graphic* p1, Graphic* p2, Graphic* p3
) {
    if (p3 != nil) {
	cur->Append(new RefList(p3));
	setParent(p3);
    }
    if (p2 != nil) {
	cur->Append(new RefList(p2));
	setParent(p2);
    }
    if (p1 != nil) {
	cur->Append(new RefList(p1));
	setParent(p1);
    }
    cur->Append(new RefList(p0));
    setParent(p0);
    uncacheBounds();
}

void Picture::InsertBeforeCur (
    Graphic* p0, Graphic* p1, Graphic* p2,Graphic* p3
) {
    cur->Prepend(new RefList(p0));
    setParent(p0);
    if (p1 != nil) {
	cur->Prepend(new RefList(p1));
	setParent(p1);
    }
    if (p2 != nil) {
	cur->Prepend(new RefList(p2));
	setParent(p2);
    }
    if (p3 != nil) {
	cur->Prepend(new RefList(p3));
	setParent(p3);
    }
    uncacheBounds();
}

Graphic* Picture::First () {
    cur = refList->First(); 
    return getGraphic(cur);
}

Graphic* Picture::Last () {
    cur = refList->Last(); 
    return getGraphic(cur);
}

Graphic* Picture::Next () {
    cur = cur->Next(); 
    return getGraphic(cur);
}

Graphic* Picture::Prev () {
    cur = cur->Prev(); 
    return getGraphic(cur);
}

void Picture::RemoveCur () {
    RefList* prev;

    if (cur != refList) {
	unsetParent((Graphic*) (*cur)());
	prev = cur;
        cur = cur->Next();
	refList->Remove(prev);
	delete prev;
	uncacheBounds();
    }
}	
    
void Picture::Remove (Graphic* p) {
    RefList* temp;

    if (getGraphic(cur) == p) {
        RemoveCur ();
    } else if ((temp = refList->Find(Ref(p))) != nil) {
	unsetParent((Graphic*) (*temp)());
	refList->Remove(temp);
	delete temp;
	uncacheBounds();
    }
}

void Picture::SetCurrent (Graphic* p) {
    RefList* temp;

    if ((temp = refList->Find(Ref(p))) != nil) {
        cur = temp;
    }
}

Graphic* Picture::FirstGraphicContaining (PointObj& pt) {
    register RefList* i;
    Graphic* subgr;

    for (i = refList->First(); i != refList->End(); i = i->Next()) {
	subgr = getGraphic(i);
	if (subgr->Contains(pt)) {
	    return subgr;
	}
    }
    return nil;
}

Graphic* Picture::LastGraphicContaining (PointObj& pt) {
    register RefList* i;
    Graphic* subgr;

    for (i = refList->Last(); i != refList->End(); i = i->Prev()) {
	subgr = getGraphic(i);
	if (subgr->Contains(pt)) {
	    return subgr;
	}
    }
    return nil;
}

Picture* Picture::GraphicsContaining (PointObj& pt) {
    register RefList* i;
    Picture* newPicture = new Picture;
    Graphic* subgr;

    for (i = refList->First(); i != refList->End(); i = i->Next()) {
	subgr = getGraphic(i);
	if (subgr->Contains(pt)) {
	    newPicture->Append(subgr);
	}
    }
    return newPicture;
}

Graphic* Picture::FirstGraphicIntersecting (BoxObj& b) {
    register RefList* i;
    Graphic* subgr;

    for (i = refList->First(); i != refList->End(); i = i->Next()) {
	subgr = getGraphic(i);
	if (subgr->Intersects(b)) {
	    return subgr;
	}
    }
    return nil;
}

Graphic* Picture::LastGraphicIntersecting (BoxObj& b) {
    register RefList* i;
    Graphic* subgr;

    for (i = refList->Last(); i != refList->End(); i = i->Prev()) {
	subgr = getGraphic(i);
	if (subgr->Intersects(b)) {
	    return subgr;
	}
    }
    return nil;
}

Picture* Picture::GraphicsIntersecting (BoxObj& b) {
    register RefList* i;
    Picture* newPicture = new Picture;
    Graphic* subgr;

    for (i = refList->First(); i != refList->End(); i = i->Next()) {
	subgr = getGraphic(i);
	if (subgr->Intersects(b)) {
	    newPicture->Append(subgr);
	}
    }
    return newPicture;
}

Graphic* Picture::FirstGraphicWithin (BoxObj& userb) {
    register RefList* i;
    Graphic* subgr;
    BoxObj b;

    for (i = refList->Last(); i != refList->End(); i = i->Prev()) {
	subgr = getGraphic(i);
	subgr->GetBox(b);
	if (b.Within(userb)) {
	    return subgr;
	}
    }
    return nil;
}

Graphic* Picture::LastGraphicWithin (BoxObj& userb) {
    register RefList* i;
    Graphic* subgr;
    BoxObj b;

    for (i = refList->Last(); i != refList->End(); i = i->Prev()) {
	subgr = getGraphic(i);
	subgr->GetBox(b);
	if (b.Within(userb)) {
	    return subgr;
	}
    }
    return nil;
}

Picture* Picture::GraphicsWithin (BoxObj& userb) {
    register RefList* i;
    Picture* newPicture = new Picture;
    Graphic* subgr;
    BoxObj b;

    for (i = refList->First(); i != refList->End(); i = i->Next()) {
	subgr = getGraphic(i);
	subgr->GetBox(b);
	if (b.Within(userb)) {
	    newPicture->Append(subgr);
	}
    }
    return newPicture;
}

Graphic* Picture::Copy () {
    register RefList* i;
    Picture* newPicture = new Picture(this);
    
    for (i = refList->First(); i != refList->End(); i = i->Next()) {
        newPicture->Append(getGraphic(i)->Copy());
    }
    return newPicture;
}

void Picture::draw (Canvas* c, Graphic* gs) {
    register RefList* i;
    Graphic* gr;
    FullGraphic gstemp;
    Transformer ttemp;

    gstemp.SetTransformer(&ttemp);
    for (i = refList->First(); i != refList->End(); i = i->Next()) {
	gr = getGraphic(i);
	gr->concat(gs, &gstemp);
	gr->draw(c, &gstemp);
    }
}

void Picture::drawClipped (
    Canvas* c, Coord l, Coord b, Coord r, Coord t, Graphic* gs
) {
    register RefList* i;
    Graphic* gr;
    FullGraphic gstemp;
    Transformer ttemp;
    BoxObj box, clipBox(l, b, r, t);
    
    getBox(box, gs);
    if (clipBox.Intersects(box)) {
	gstemp.SetTransformer(&ttemp);
	for (i = refList->First(); i != refList->End(); i = i->Next()) {
	    gr = getGraphic(i);
	    gr->concat(gs, &gstemp);
	    gr->drawClipped(c, l, b, r, t, &gstemp);
	}
    }
}

void Picture::cacheBounds (float x0, float y0, float x1, float y1) {
    Coord left, bottom, right, top;

    if (caching) {
	delete box;
	left = Coord(x0 - 1);
	bottom = Coord(y0 - 1);
	right = Coord(x1 + 1);
	top = Coord(y1 + 1);
	box = new FloatBoxObj(left, bottom, right, top);
    }
}

void Picture::getCachedBounds (float& x0, float& y0, float& x1, float& y1) {
    x0 = box->left;
    y0 = box->bottom;
    x1 = box->right;
    y1 = box->top;
}

void Picture::getBounds (float& x0, float& y0, float& x1, float& y1, Graphic* gs){
    register RefList* i;
    Graphic* gr;
    FullGraphic gstemp;
    Transformer ttemp;
    
    float tx0, ty0, tx1, ty1;
    
    if (boundsCached()) {
	getCachedBounds(x0, y0, x1, y1);
    } else {
	if (IsEmpty()) {
	    x0 = 0.0; y0 = 0.0;
	    x1 = 0.0; y1 = 0.0;
	} else {
	    gstemp.SetTransformer(&ttemp);
	    i = refList->First();
	    gr = getGraphic(i);
	    gr->concat(gs, &gstemp);
	    gr->getBounds(x0, y0, x1, y1, &gstemp);
	    for (i = i->Next(); i != refList->End(); i = i->Next()) {
		gr = getGraphic(i);
		gr->concat(gs, &gstemp);
		gr->getBounds(tx0, ty0, tx1, ty1, &gstemp);
		x0 = fmin(x0, tx0); y0 = fmin(y0, ty0);
		x1 = fmax(x1, tx1); y1 = fmax(y1, ty1);
	    }
	}
	cacheBounds(x0, y0, x1, y1);
    }
}

boolean Picture::contains (PointObj& po, Graphic* gs) {
    register RefList* i;
    Graphic* gr;
    FullGraphic gstemp;
    Transformer ttemp;
    BoxObj b;
    
    if (!IsEmpty()) {
	getBox(b, gs);
	if (b.Contains(po)) {
	    gstemp.SetTransformer(&ttemp);
	    for (i = refList->First(); i != refList->End(); i = i->Next()) {
		gr = getGraphic(i);
		gr->concat(gs, &gstemp);
		if (gr->contains(po, &gstemp)) {
		    return true;
		}
	    }
	}
    }
    return false;
}

boolean Picture::intersects (BoxObj& userb, Graphic* gs) {
    register RefList* i;
    Graphic* gr;
    FullGraphic gstemp;
    Transformer ttemp;
    BoxObj b;
    
    if (!IsEmpty()) {
	getBox(b, gs);
	if (b.Intersects(userb)) {
	    gstemp.SetTransformer(&ttemp);
	    for (i = refList->First(); i != refList->End(); i = i->Next()){
		gr = getGraphic(i);
		gr->concat(gs, &gstemp);
		if (gr->intersects(userb, &gstemp)) {
		    return true;
		}
	    }
	}
    }
    return false;
}

void Picture::Propagate () {
    register RefList* i;
    Graphic* gr;
    
    for (i = refList->First(); i != refList->End(); i = i->Next()) {
	gr = getGraphic(i);
	gr->concat(this, gr);
    }
    SetTransformer(nil);
}
