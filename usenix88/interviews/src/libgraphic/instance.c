/*
 * Instance class implementation.  An Instance is a Graphic that is a
 * variation of another Graphic.
 */

#include <InterViews/canvas.h>
#include <InterViews/Graphic/instance.h>

boolean Instance::read (File* f) {
    return FullGraphic::read(f) && refGr.Read(f);
}

boolean Instance::write (File* f) {
    return FullGraphic::write(f) && refGr.Write(f);
}

Instance::Instance (Graphic* obj, Graphic* gr) : (gr) {
    refGr = Ref(obj);
}

void Instance::GetOriginal (Graphic*& gr) {
    gr = getGraphic();
}

void Instance::draw (Canvas* c, Graphic* gs) {
    Graphic* gr = getGraphic();
    FullGraphic gstemp;
    Transformer ttemp;

    gstemp.SetTransformer(&ttemp);
    gr->concat(gs, &gstemp);
    gr->draw(c, &gstemp);
}

void Instance::drawClipped (
    Canvas* c, Coord l, Coord b, Coord r, Coord t, Graphic* gs
) {
    Graphic* gr = getGraphic();
    FullGraphic gstemp;
    Transformer ttemp;
    BoxObj box, clipBox(l, b, r, t);

    cachingOff();
    getBox(box, gs);
    if (clipBox.Intersects(box)) {
	gstemp.SetTransformer(&ttemp);
	gr->concat(gs, &gstemp);
	gr->drawClipped(c, l, b, r, t, &gstemp);
    }
    cachingOn();
}

void Instance::getBounds (
    float& x0, float& y0, float& x1, float& y1, Graphic* gs
) {
    Graphic* gr = getGraphic();
    FullGraphic gstemp;
    Transformer ttemp;

    gstemp.SetTransformer(&ttemp);
    gr->concat(gs, &gstemp);
    cachingOff();
    gr->getBounds(x0, y0, x1, y1, &gstemp);
    cachingOn();
}

boolean Instance::contains (PointObj& po, Graphic* gs) {
    Graphic* gr = getGraphic();
    FullGraphic gstemp;
    Transformer ttemp;
    boolean result;

    gstemp.SetTransformer(&ttemp);
    gr->concat(gs, &gstemp);
    cachingOff();
    result = gr->contains(po, &gstemp);
    cachingOn();
    return result;
}

boolean Instance::intersects (BoxObj& b, Graphic* gs) {
    Graphic* gr = getGraphic();
    FullGraphic gstemp;
    Transformer ttemp;
    boolean result;

    gstemp.SetTransformer(&ttemp);
    gr->concat(gs, &gstemp);
    cachingOff();
    result = gr->intersects(b, &gstemp);
    cachingOn();
    return result;
}
