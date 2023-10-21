/*
 * Button management.
 */

#include <InterViews/button.h>
#include <InterViews/shape.h>
#include <InterViews/sensor.h>
#include <InterViews/painter.h>

const int pad = 3;
const int sep = 7;

static Painter* grayout;	/* for disabled buttons */

/*
 * Simple list of buttons.
 */

class ButtonList {
public:
    Button* cur;
    ButtonList* next;

    ButtonList (Button* b) { cur = b; next = nil; }
};

static void Remove (ButtonList*& blist, Button* b) {
    register ButtonList* bl;
    register ButtonList* prev;

    prev = nil;
    for (bl = blist; bl != nil; bl = bl->next) {
	if (bl->cur == b) {
	    if (prev == nil) {
		blist = bl->next;
	    } else {
		prev->next = bl->next;
	    }
	    delete bl;
	    break;
	}
	prev = bl;
    }
}

static void DeleteList (ButtonList* blist) {
    register ButtonList* bl;
    register ButtonList* next;

    for (bl = blist; bl != nil; bl = next) {
	next = bl->next;
	delete bl;
    }
}

/*
 * A button state is a value that is settable from one or more buttons.
 * When the state is modified, it updates all its buttons (views) by
 * choose/unchoosing them depending on their value.
 */

void ButtonState::Init (void* v) {
    value = v;
    views = nil;
}

void ButtonState::Modify (void* v) {
    register ButtonList* bl;

    if (value != v) {
	value = v;
	for (bl = views; bl != nil; bl = bl->next) {
	    bl->cur->Check();
	}
    }
}

void ButtonState::Attach (Button* b) {
    ButtonList* head;

    head = new ButtonList(b);
    head->next = views;
    views = head;
    b->Check();
}

void ButtonState::Detach (Button* b) {
    Remove(views, b);
}

ButtonState::~ButtonState () {
    register ButtonList* bl;
    register ButtonList* next;

    for (bl = views; bl != nil; bl = next) {
	next = bl->next;
	bl->cur->Disconnect();
	delete bl;
    }
}

/*
 * A button has a ButtonState subject that it modifies when pressed.
 * Also, a button may have associated buttons that are enabled/disabled
 * when it is chosen/unchosen.
 */

void Button::Init (ButtonState* s, void* v) {
    value = v;
    subject = s;
    associates = nil;
    enabled = true;
    hit = false;
    subject->Attach(this);
    input = new Sensor(updownEvents);
    input->Catch(OnEvent);
    input->Catch(OffEvent);
}

void Button::Delete () {
    if (subject != nil) {
	subject->Detach(this);
    }
    DeleteList(associates);
}

void Button::Disconnect () {
    subject = nil;
}

void Button::Attach (Button* b) {
    ButtonList* head;

    head = new ButtonList(b);
    head->next = associates;
    associates = head;
    if (chosen) {
	b->Enable();
    } else {
	b->Disable();
    }
}

void Button::Detach (Button* b) {
    Remove(associates, b);
}

void Button::Enable () {
    if (!enabled) {
	enabled = true;
	if (canvas != nil) {
	    Draw();
	}
    }
}

void Button::Disable () {
    if (enabled) {
	enabled = false;
	if (canvas != nil) {
	    Draw();
	}
    }
}

void Button::Choose () {
    register ButtonList* bl;

    if (!chosen) {
	chosen = true;
	if (enabled) {
	    if (canvas != nil) {
		Update();
	    }
	    for (bl = associates; bl != nil; bl = bl->next) {
		bl->cur->Enable();
	    }
	}
    }
}

void Button::UnChoose () {
    register ButtonList* bl;

    if (chosen) {
	chosen = false;
	if (enabled) {
	    if (canvas != nil) {
		Update();
	    }
	    for (bl = associates; bl != nil; bl = bl->next) {
		bl->cur->Disable();
	    }
	}
    }
}

void Button::SetDimensions (int width, int height) {
    register Shape* s = shape;

    s->hshrink = min(0, width - s->width);
    s->vshrink = min(0, height - s->height);
    s->width = width;
    s->height = height;
}

void Button::Draw () {
    if (canvas != nil) {
	Redraw(0, 0, xmax, ymax);
    }
}

void Button::Handle (register Event& e) {
    if (e.eventType == DownEvent && e.target == this) {
	boolean inside = true;
	do {
	    if (enabled && e.target == this) {
		if (e.eventType == OnEvent) {
		    inside = true;
		} else if (e.eventType == OffEvent) {
		    inside = false;
		}
		if (inside) {
		    if (!hit) {
			hit = true;
			Update();
		    }
		} else {
		    if (hit) {
			hit = false;
			Update();
		    }
		}
	    }
	    Read(e);
	} while (e.eventType != UpEvent);
	if (hit) {
	    hit = false;
	    Update();
	}
	if (enabled && inside) {
	    Press();
	}
    }
}

void Button::Press () {
    if (subject != nil) {
	subject->SetValue(value);
    } else {
	Update();
    }
}

void Button::Check () {
    void* v;

    subject->GetValue(v);
    if (!chosen && value == v) {
	Choose();
    } else if (chosen && value != v) {
	UnChoose();
    }
}

TextButton::TextButton (
    const char* str, ButtonState* s, void* v, Painter* out
) : (s, v, out) {
    text = str;
    if (text != nil) {
	Font* f = output->GetFont();
	shape->width = f->Width(str);
	shape->height = f->Height();
    }
    shape->Rigid();
    background = new Painter(output);
    background->SetColors(output->GetBgColor(), output->GetFgColor());
    if (grayout == nil) {
	grayout = new Painter(background);
	grayout->SetPattern(gray);
	grayout->FillBg(false);
    }
}

void TextButton::Delete () {
    delete background;
    Button::Delete();
}

void PushButton::Init () {
    shape->width += output->GetFont()->Width("    ");
    shape->height += 2*pad;
}

void PushButton::Redraw (Coord x1, Coord y1, Coord x2, Coord y2) {
    output->ClearRect(canvas, x1, y1, x2, y2);
    Update();
}

void PushButton::Update () {
    register int r;
    Coord x[16], y[16];
    Coord tx, ty;

    r = min(10*pixels, min(xmax+1, ymax+1)/6);
    x[0] = 0; y[0] = r;
    x[1] = 0; y[1] = r + r;
    x[2] = 0; y[2] = ymax - r - r;
    x[3] = 0; y[3] = ymax - r;
    x[4] = r; y[4] = ymax;
    x[5] = r + r; y[5] = ymax;
    x[6] = xmax - r - r; y[6] = ymax;
    x[7] = xmax - r; y[7] = ymax;
    x[8] = xmax; y[8] = ymax - r;
    x[9] = xmax; y[9] = ymax - r - r;
    x[10] = xmax; y[10] = r + r;
    x[11] = xmax; y[11] = r;
    x[12] = xmax - r; y[12] = 0;
    x[13] = xmax - r - r; y[13] = 0;
    x[14] = r + r; y[14] = 0;
    x[15] = r; y[15] = 0;
    tx = (xmax - output->GetFont()->Width(text))/2;
    ty = pad;
    if (enabled && (chosen || hit)) {
	output->FillBSpline(canvas, x, y, 16);
	background->MoveTo(tx, ty);
	background->Text(canvas, text);
    } else {
	background->FillBSpline(canvas, x, y, 16);
	output->ClosedBSpline(canvas, x, y, 16);
	output->MoveTo(tx, ty);
	output->Text(canvas, text);
	if (!enabled) {
	    grayout->FillBSpline(canvas, x, y, 16);
	}
    }
}

inline int radius (int h) { return round(0.3*h); }

void RadioButton::Init () {
    shape->width += 2*radius(shape->height) + sep;
    shape->height += 2*pad;
}

void RadioButton::Redraw (Coord x1, Coord y1, Coord x2, Coord y2) {
    int r = radius(shape->height-2*pad);
    Coord cx = r;
    Coord cy = (ymax+1)/2;
    output->ClearRect(canvas, x1, y1, x2, y2);
    output->FillCircle(canvas, cx, cy, r);
    background->FillCircle(canvas, cx, cy, r-1);
    Coord tx = 2*r + sep;
    Coord ty = (ymax + 1 - output->GetFont()->Height() - pad) / 2 + pad;
    output->MoveTo(tx, ty);
    output->Text(canvas, text);
    Update();
}

void RadioButton::Update () {
    int r = radius(shape->height-2*pad);
    Coord cx = r;
    Coord cy = (ymax+1)/2;
    if (enabled) {
	if (hit) {
	    output->FillCircle(canvas, cx, cy, r);
	    background->FillCircle(canvas, cx, cy, r-2);
	} else {
	    background->FillCircle(canvas, cx, cy, r-1);
	}
	if (chosen) {
	    output->FillCircle(canvas, cx, cy, r-3);
	}
    } else {
	background->FillCircle(canvas, cx, cy, r-1);
	grayout->FillRect(canvas, 0, 0, xmax, ymax);
    }
}

void CheckBox::Init (void* v) {
    shape->width += shape->height + sep;
    shape->height += 2*pad;
    offvalue = v;
    Check();
}

void CheckBox::Redraw (Coord x1, Coord y1, Coord x2, Coord y2) {
    register int n = shape->height - 2*pad - 1;
    output->ClearRect(canvas, x1, y1, x2, y2);
    output->Rect(canvas, 0, pad, n, pad + n);
    output->MoveTo(n + sep, pad);
    output->Text(canvas, text);
    Update();
}

void CheckBox::Press () {
    if (chosen) {
	subject->GetValue(value);
	subject->SetValue(offvalue);
    } else {
	subject->SetValue(value);
    }
}

void CheckBox::Check () {
    void* v;

    subject->GetValue(v);
    if (v != offvalue) {
	Choose();
	value = v;
    } else {
	UnChoose();
    }
}

void CheckBox::Update () {
    register int n = shape->height - 2*pad - 1;
    Coord right = n;
    Coord bottom = pad;
    Coord top = bottom + n;
    if (enabled) {
	if (hit) {
	    output->Rect(canvas, 1, bottom+1, right-1, top-1);
	} else {
	    background->Rect(canvas, 1, bottom+1, right-1, top-1);
	}
	if (chosen) {
	    output->Line(canvas, 0, bottom, right, top);
	    output->Line(canvas, 0, top, right, bottom);
	} else {
	    background->FillRect(canvas, 2, bottom+2, right-2, top-2);
	}
    } else {
	background->FillRect(canvas, 1, bottom+1, right-1, top-1);
	grayout->FillRect(canvas, 0, 0, xmax, ymax);
    }
}
