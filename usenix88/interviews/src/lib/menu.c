/*
 * Menu implementation.
 */

#include <InterViews/box.h>
#include <InterViews/frame.h>
#include <InterViews/menu.h>
#include <InterViews/painter.h>
#include <InterViews/sensor.h>
#include <InterViews/shape.h>
#include <InterViews/world.h>

Menu::Menu (Sensor* in, Painter* out, boolean persist = true) : (in, out) {
    persistent = persist;
    layout = nil;
    cur = nil;
    xoff = 0;
    yoff = 0;
    if (input == nil) {
	input = new Sensor(onoffEvents);
	input->Catch(DownEvent);
	input->Catch(UpEvent);
    }
}

HMenu::HMenu (Sensor* in, Painter* out) : (in, out) {
    /* nothing to do */
}

VMenu::VMenu (Sensor* in, Painter* out) : (in, out) {
    /* nothing to do */
}

void Menu::DoInsert (Interactor* i, boolean, Coord&, Coord&, Shape*) {
    if (layout == nil) {
	layout = new VBox;
    }
    layout->Insert(i);
}

void HMenu::DoInsert (Interactor* i, boolean, Coord&, Coord&, Shape*) {
    if (layout == nil) {
	layout = new HBox;
    }
    layout->Insert(i);
}

void Menu::Compose () {
    Coord x, y;
    Frame* f;

    f = new Frame(output, layout);
    MonoScene::DoInsert(f, false, x, y, f->shape);
    xoff = shape->width / 2;
    yoff = shape->height / 2;
}

/*
 * Pop up a menu in the world at the coordinates specified
 * by the given event.  The event is what caused the popup request --
 * set it to what causes the menu to close.  Store the selected item
 * in the final parameter as well as the current selection.
 */

void Menu::Popup (register Event& e, Interactor*& item) {
    register Interactor* i;
    World* w;
    Coord wx, wy;

    e.GetAbsolute(w, wx, wy);
    w->Insert(this, wx - xoff, wy - yoff);
    i = nil;
    for (;;) {
	Read(e);
	if (e.eventType == UpEvent) {
	    break;
	}
	if (e.target->Parent() == layout) {
	    i = e.target;
	    i->Handle(e);
	    if (e.eventType == UpEvent) {
		break;
	    }
	    if (!persistent) {
		Poll(e);
		if (e.x < 0 || e.x > xmax || e.y < 0 || e.y > ymax) {
		    i = nil;
		    break;
		}
	    }
	} else if (e.target == this && e.eventType == OffEvent) {
	    i = nil;
	    if (!persistent) {
		break;
	    }
	}
    }
    if (i != nil) {
	xoff = i->xmax/2;
	yoff = i->ymax/2;
	i->GetRelative(xoff, yoff, this);
    }
    cur = i;
    item = i;
    e.GetAbsolute(wx, wy);
    e.target = w;
    e.x = wx;
    e.y = wy;
    w->Remove(this);
    Sync();
}

void Menu::Handle (Event& e) {
    for (;;) {
	if (e.eventType == UpEvent) {
	    break;
	} else if (e.target->Parent() == layout) {
	    cur = e.target;
	    cur->Handle(e);
	    if (e.eventType == UpEvent) {
		break;
	    }
	} else if (e.target != this) {
	    Parent()->Handle(e);
	} else if (e.eventType != DownEvent) {
	    cur = nil;
	}
	Read(e);
    }
}

MenuItem::MenuItem (Painter* out) : (onoffEvents, out) {
    highlight = new Painter(out);
    highlight->SetColors(out->GetBgColor(), out->GetFgColor());
    normal = out;
}

void MenuItem::Delete () {
    delete highlight;
}

void MenuItem::Handle (Event& e) {
    switch (e.eventType) {
	case OnEvent:
	    Highlight();
	    break;
	case OffEvent:
	    UnHighlight();
	    break;
	default:
	    /* shouldn't happen */;
    }
}

/*
 * Default highlighting is to redraw the item with the highlight painter
 * as output.
 */

void MenuItem::Highlight () {
    output = highlight;
    Draw();
}

void MenuItem::UnHighlight () {
    output = normal;
    Draw();
}

const int pad = 2;

/*
 * Simple text menu item.
 */

TextItem::TextItem (Painter* p, const char* t) : (p) {
    text = t;
    shape->width = p->GetFont()->Width(t) + 2*pad;
    shape->height = p->GetFont()->Height() + 2*pad;
    shape->Rigid(0, hfil, 0, 0);
}

void TextItem::Draw () {
    output->ClearRect(canvas, 0, 0, xmax, ymax);
    output->MoveTo((xmax - shape->width) / 2 + pad, pad);
    output->Text(canvas, text);
}

void TextItem::Redraw (Coord, Coord, Coord, Coord) {
    output->ClearRect(canvas, 0, 0, xmax, ymax);
    output->MoveTo((xmax - shape->width) / 2 + pad, pad);
    output->Text(canvas, text);
}
