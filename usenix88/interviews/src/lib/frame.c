/*
 * A frame surrounds another interactor, providing borders, title banners, etc.
 */

#include <InterViews/frame.h>
#include <InterViews/shape.h>
#include <InterViews/canvas.h>
#include <InterViews/sensor.h>
#include <InterViews/painter.h>
#include <InterViews/banner.h>
#include <InterViews/border.h>
#include <InterViews/box.h>

Frame::Frame (Painter* out, Interactor* i, int width) : (onoffEvents, out) {
    border = width;
    if (i != nil) {
	Insert(i);
    }
}

void Frame::DoChange (Interactor* i, Shape* s) {
    MonoScene::DoChange(i, s);
    shape->width += 2*border;
    shape->height += 2*border;
}

void Frame::Resize () {
    canvas->SetBackground(output->GetBgColor());
    Place(component, border, border, xmax - border, ymax - border);
}

void Frame::Redraw (Coord x1, Coord y1, Coord x2, Coord y2) {
    register Coord right = xmax - border;
    register Coord top = ymax - border;
    if (x1 < border || y1 < border || x2 > right || y2 > top) {
	/* top strip */
	output->FillRect(canvas, 0, top+1, xmax, ymax);
	/* bottom strip */
	output->FillRect(canvas, 0, 0, xmax, border-1);
	/* left strip */
	output->FillRect(canvas, 0, border, border-1, top);
	/* right strip */
	output->FillRect(canvas, right+1, border, xmax, top);
    }
}

void Frame::Handle (Event& e) {
    if (e.eventType == OnEvent) {
	Highlight(true);
    } else if (e.eventType == OffEvent) {
	Highlight(false);
    } else {
	component->Handle(e);
    }
}

void Frame::Highlight (boolean) {
    /* default is to do nothing */
}

/*
 * A title frame is a frame around a box containing
 * a banner, border, and the component.
 */

TitleFrame::TitleFrame (Painter* out, int width) : (out, nil, width) {
    banner = nil;
}

TitleFrame::TitleFrame (
    Painter* out, Banner* b, Interactor* i, int width
) : (out, new VBox(b, new HBorder(out), i), width) {
    banner = b;
}

void TitleFrame::DoInsert (
    Interactor* i, boolean, Coord& x, Coord& y, Shape*
) {
    Box* b = new VBox(banner, new HBorder(output), i);
    Frame::DoInsert(b, false, x, y, b->shape);
}

void TitleFrame::Highlight (boolean b) {
    banner->highlight = b;
    banner->Draw();
}

BorderFrame::BorderFrame (Painter* out, Interactor* i, int w) : (out, i, w) {
    outline = new Painter(out);
    outline->SetPattern(gray);
}

void BorderFrame::Delete () {
    delete outline;
}

void BorderFrame::Highlight (boolean b) {
    register Painter* tmp;

    outline->SetPattern(b ? solid : gray);
    tmp = output;
    output = outline;
    Redraw(0, 0, xmax, ymax);
    output = tmp;
}
