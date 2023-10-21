/*
 * Implementation of the frame around a squares view.
 */

#include "sframe.h"
#include "subject.h"
#include "view.h"
#include "metaview.h"
#include <InterViews/shape.h>
#include <InterViews/sensor.h>
#include <InterViews/banner.h>
#include <InterViews/border.h>
#include <InterViews/box.h>
#include <InterViews/glue.h>
#include <InterViews/scroller.h>
#include <InterViews/world.h>

extern void exit(int);

SquaresFrame::SquaresFrame (SquaresView* v, Painter* out) : (out) {
    hwidth = 14*pixels;
    vwidth = 14*pixels;
    below = true;
    right = true;
    Init(v);
    MakeFrame();
}

SquaresFrame::SquaresFrame (SquaresFrame* f, Painter* out) : (out) {
    hwidth = f->hwidth;
    vwidth = f->vwidth;
    below = f->below;
    right = f->right;
    Init(new SquaresView(f->view->subject));
    MakeFrame();
}

void SquaresFrame::Init (SquaresView* v) {
    view = v;
    meta = new SquaresMetaView(this, output);
    menu = new Menu(nil, output);
    Item(menu, "add square", NEWSQ);
    Item(menu, "zoom in", ZOOMIN);
    Item(menu, "zoom out", ZOOMOUT);
    Item(menu, "new view", NEWVIEW);
    Item(menu, "view setup", SETUP);
    Item(menu, "quit", QUIT);
    menu->Compose();
    quit = new Menu(nil, output);
    Item(quit, "yes, quit", QUIT);
    Item(quit, "no, don't quit", SETUP);
    quit->Compose();
    input->Catch(DownEvent);
    input->Catch(UpEvent);
}

void SquaresFrame::MakeFrame () {
    Interactor* v = view;
    Interactor* s;
    Border* b;

    if (canvas != nil) {
	v->shape->width = v->xmax + 1;
	v->shape->height = v->ymax + 1;
    }
    if (vwidth > 0) {
	s = new VScroller(view, vwidth, nil, output);
	b = new VBorder(output);
	v = right ? new HBox(v, b, s) : new HBox(s, b, v);
    }
    if (hwidth > 0) {
	s = new HScroller(view, hwidth, nil, output);
	if (vwidth > 0) {
	    Glue* g = new HGlue(output, vwidth, 0);
	    b = new VBorder(output);
	    s = right ? new HBox(s, b, g) : new HBox(g, b, s);
	}
	b = new HBorder(output);
	v = below ? new VBox(v, b, s) : new VBox(s, b, v);
    }
    Insert(
	new VBox(
	    new Banner(output, "squares demo", "InterViews 2.3", "2/8/88"),
	    new HBorder(output),
	    v
	)
    );
}

void SquaresFrame::Handle (Event& e) {
    SquaresMenuItem* i;
    World* w;
    Coord wx, wy;

    if (e.eventType == DownEvent && e.button == RIGHTMOUSE) {
	menu->Popup(e, i);
	if (i != nil) {
	    switch (i->choice) {
		case NEWSQ:
		    view->subject->Add();
		    break;
		case ZOOMIN:
		    view->Zoom(-1);
		    break;
		case ZOOMOUT:
		    view->Zoom(1);
		    break;
		case NEWVIEW:
		    e.GetAbsolute(w, wx, wy);
		    w->Insert(new SquaresFrame(this, output));
		    break;
		case SETUP:
		    meta->Dialog(e);
		    break;
		case QUIT:
		    quit->Popup(e, i);
		    if (i != nil && i->choice == QUIT) {
			exit(0);
		    }
		    break;
	    }
	}
    } else {
	TitleFrame::Handle(e);
    }
}
