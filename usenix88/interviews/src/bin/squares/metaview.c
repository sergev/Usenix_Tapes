/*
 * Implementation of squares metaview.
 */

#include "metaview.h"
#include "sframe.h"
#include <InterViews/box.h>
#include <InterViews/button.h>
#include <InterViews/glue.h>
#include <InterViews/painter.h>
#include <InterViews/sensor.h>
#include <InterViews/world.h>

static RadioButton* MakeButton (
    const char* str, ButtonState* s, Button* control, int n, Painter* p
) {
    RadioButton* rb = new RadioButton(str, s, n, p);
    rb->SetDimensions(80*pixels, 30*pixels);
    control->Attach(rb);
    return rb;
}

HBox* WidthChoices (ButtonState* s, Button* control, Painter* p) {
    register HBox* h = new HBox;
    h->Insert(new HGlue(p, 20*pixels, 0, 0));
    h->Insert(MakeButton("wide", s, control, 18*pixels, p));
    h->Insert(MakeButton("normal", s, control, 14*pixels, p));
    h->Insert(MakeButton("narrow", s, control, 10*pixels, p));
    h->Insert(new HGlue(p, 30*pixels));
    return h;
}

HBox* SideChoices (
    ButtonState* s, Button* control,
    const char* first, const char* second, Painter* p
) {
    register HBox* h = new HBox;
    h->Insert(new HGlue(p, 20*pixels, 0, 0));
    h->Insert(MakeButton(first, s, control, false, p));
    h->Insert(MakeButton(second, s, control, true, p));
    h->Insert(new HGlue(p, 30*pixels));
    return h;
}

SquaresMetaView::SquaresMetaView (SquaresFrame* v, Painter* out) : (out) {
    view = v;

    vwidth = new ButtonState(view->vwidth);
    below = new ButtonState(view->below);
    hwidth = new ButtonState(view->hwidth);
    right = new ButtonState(view->right);
    accept = new ButtonState(false);
    cancel = new ButtonState(false);

    Painter* small = new Painter(output);
    Painter* large = new Painter(output);
    small->SetFont(new Font("8x13"));
    large->SetFont(new Font("9x15"));

    Button* cb = new CheckBox("Horizontal Scroller", hwidth, 0, large);
    VBox* hcontrol = new VBox(
	cb,
	new VGlue(output, 10*pixels, 12*pixels, 100*pixels),
	WidthChoices(hwidth, cb, small),
	SideChoices(below, cb, "above", "below", small)
    );

    cb = new CheckBox("Vertical Scroller", vwidth, 0, large);
    VBox* vcontrol = new VBox(
	cb,
	new VGlue(output, 10*pixels, 12*pixels, 100*pixels),
	WidthChoices(vwidth, cb, small),
	SideChoices(right, cb, "left", "right", small)
    );

    HBox* ok = new HBox;
    ok->Insert(new HGlue(output));
    ok->Insert(new PushButton("Accept", accept, true, output));
    ok->Insert(new HGlue(output, 20*pixels));
    ok->Insert(new PushButton("Cancel", cancel, true, output));
    ok->Insert(new HGlue(output, 30*pixels));

    VBox* dialog = new VBox;
    dialog->Insert(
	new HBox(
	    new HGlue(output, 30*pixels),
	    hcontrol,
	    new HGlue(output, 30*pixels)
	)
    );
    dialog->Insert(new VGlue(output, 10*pixels, 8*pixels, vfil/2));
    dialog->Insert(
	new HBox(
	    new HGlue(output, 30*pixels),
	    vcontrol,
	    new HGlue(output, 30*pixels)
	)
    );
    dialog->Insert(new VGlue(output, 10*pixels, 8*pixels, vfil/2));
    dialog->Insert(ok);

    Insert(
	new VBox(
	    new VGlue(output, 20*pixels),
	    dialog,
	    new VGlue(output, 20*pixels)
	)
    );
    /* remove references to Painters */
    delete small;
    delete large;
}

void SquaresMetaView::Dialog (Event& e) {
    World* w;
    Coord wx, wy;
    int a, b, c;

    vwidth->SetValue(view->vwidth);
    below->SetValue(view->below);
    hwidth->SetValue(view->hwidth);
    right->SetValue(view->right);
    accept->SetValue(false);
    cancel->SetValue(false);
    e.GetAbsolute(w, wx, wy);
    w->Insert(this);
    do {
	w->Read(e);
	e.target->Handle(e);
	accept->GetValue(a);
	cancel->GetValue(c);
    } while (!a && !c);
    w->Remove(this);
    if (a) {
	vwidth->GetValue(view->vwidth);
	below->GetValue(b);
	view->below = b;
	hwidth->GetValue(view->hwidth);
	right->GetValue(b);
	view->right = b;
	view->MakeFrame();
	view->Parent()->Change(view);
    }
}

void SquaresMetaView::Update () {
    vwidth->SetValue(view->vwidth);
    below->SetValue(view->below);
    hwidth->SetValue(view->hwidth);
    right->SetValue(view->right);
    Draw();
}
