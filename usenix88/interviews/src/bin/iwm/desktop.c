/*
 *  InterViews Desktop WorldView (for window managers)
 *
 *  This contains the built-in operations the desktop can perform.
 *  To add a new operation, five things must be updated:
 *
 *	o OperationCode			(in desktop.h)
 *	o Operations			(in desktop.h)
 * 	o DesktopFunctionNames[]	(in desktop.c)
 *	o DesktopFunctions()		(in desktop.c)
 *	o Desktop::Do<function>(Event&)	(in desktop.c)
 *
 *  This file also contains the Desktop constructor and destructor, Run(),
 *  and Config(), none of which need to be modified for new operations.
 */

#include <InterViews/cursor.h>
#include <InterViews/canvas.h>
#include <InterViews/reqerr.h>
#include <InterViews/world.h>
#include <InterViews/bitmap.h>
#include <InterViews/Xinput.h>
#include "rubber.h"
#include "desktop.h"
#include "dispatch.h"
#include <string.h>

extern char* program_name;				/* iwm.c */

extern boolean Equal(const char*, const char*);

Icon::Icon (Painter* p, const char* name, int padding) : (noEvents, p) {
    s = new char[strlen(name)+1];
    strcpy(s, name);
    shape->width = output->GetFont()->Width(name) + 2*padding;
    shape->height = output->GetFont()->Height() + padding;
}

Icon::~Icon () {
    delete s;
}

boolean Icon::IsOld (const char* name) {
    if (s != nil && name != nil) {
	return strcmp(s, name) != 0;
    }
    return true;
}

void Icon::Resize () {
    xpos = (xmax - output->GetFont()->Width(s)) >> 1;
    ypos = (ymax - output->GetFont()->Height()) >> 1;
}

void Icon::Redraw (Coord x1, Coord y1, Coord x2, Coord y2) {
    output->ClearRect(canvas, x1, y1, x2, y2);
    output->MoveTo(xpos, ypos);
    output->Text(canvas, s);
}

static char* ButtonNames (unsigned mask) {
    char* s = new char[28];
    s[0] = '\0';

    if (mask&LockMask) {
	strcat(s, "lock ");
    }
    if (mask&ShiftMask) {
	strcat(s, "shift ");
    }
    if (mask&ControlMask) {
	strcat(s, "ctrl ");
    }
    if (mask&Mod1Mask) {
	strcat(s, "meta ");
    }
    if (mask&LeftMask) {
	strcat(s, "left");
    } else if (mask&MiddleMask) {
	strcat(s, "middle");
    } else if (mask&RightMask) {
	strcat(s, "right");
    }
    return s;
}

class ErrHandler : public ReqErr {
public:
    ErrHandler () { Install(); }
    void Error();
};

void ErrHandler::Error () {
    fprintf(stderr, "iwm: X Error ");
    fprintf(stderr, "%s on request %d for %d\n", message, request, id);
}

Desktop::Desktop (World* w) : (w, nil, nil) {
    output = new Painter;

    Config();
    if (logosize > 0) {
	if (logo_bitmap != nil) {
	    logo = new BitmapLogo(logo_bitmap, output, logosize);
	} else {
	    logo = new PolygonLogo(output, logosize);
	}
    }
    lock = new ScreenLock(output);
    dispatch = new DesktopDispatcher(this, output);	/* reads .iwmrc */
    icons = nil;
    if (!GrabEm()) {
	UnGrabEm();
	exit(1);
    }
    UnGrabEm();
    new ErrHandler;
}

Desktop::~Desktop () {
    RemoteInteractor i;

    while (icons != nil) {
	i = GetIcon(icons->ri);
	if (i != nil) {
	    Map(i);
	}
	Unmap(icons->ri);
	if (i != nil) {
	    UnassignIcon(i);
	}
	delete icons->icon->Parent();
	icons = icons->next;
    }

    delete dispatch;
    delete logo;
    WorldView::Delete();
    Sync();
}

#ifdef X10

static void ConvertButtonMask (unsigned bmask, unsigned& b, unsigned& m) {
    b = 0;
    m = bmask;
}

#else

static void ConvertButtonMask (unsigned bmask, unsigned& b, unsigned& m) {
    b = 0;
    m = 0;
    if ((bmask&LeftMask) != 0) {
	b |= Button1;
    }
    if ((bmask&MiddleMask) != 0) {
	b |= Button2;
    }
    if ((bmask&RightMask) != 0) {
	b |= Button3;
    }
    if ((bmask&ShiftMask) != 0) {
	m |= ShiftMask;
    }
    if ((bmask&ControlMask) != 0) {
	m |= ControlMask;
    }
    if ((bmask&LockMask) != 0) {
	m |= LockMask;
    }
    if ((bmask&Mod1Mask) != 0) {
	m |= Mod1Mask;
    }
}

#endif

boolean Desktop::GrabEm() {
    boolean ok = true;
    register ButtonList* b = dispatch->Buttons();
    unsigned button, mask;

    while (b != nil) {
	ConvertButtonMask(b->mask, button, mask);
	if (!GrabButton(button, mask, b->cursor)) {
	    fprintf(stderr, "%s: couldn't grab `%s button'\n",
		    program_name, ButtonNames(b->mask));
	    ok = false;
	}
	if (ignorecaps) {
	    GrabButton(button, mask|LockMask, defaultCursor);
	}
	b = b->next;
    }

    return(ok);
}

void Desktop::UnGrabEm() {
    register ButtonList* b = dispatch->Buttons();
    unsigned button, mask;

    while (b != nil) {
	ConvertButtonMask(b->mask, button, mask);
	UngrabButton(button, mask);
	b = b->next;
    }
}

void Desktop::Run() {
    Event e;

    GrabEm();
    if (logo != nil) {
	world->Insert(logo, logo_x, logo_y);
    }
    running = true;
    while (running) {
	Read(e);
	if (e.eventType == DownEvent) {
	    if (dispatch->NoFunction(e) && FindIcon(Find(e.x, e.y)) != nil) {
		DoIconify(e);
	    } else {
		dispatch->Perform(e);
	    }
	}
    }
    if (logo != nil) {
	world->Remove(logo);
    }
    UnGrabEm();
    Sync();
}

char* DesktopFunctionNames[Operations] = {
    "Focus", "Lower", "Raise", "Redraw", "Iconify",
    "Move", "Resize", "Lock", "Exit", "Null"
};

DesktopFunction DesktopFunctions (OperationCode op) {
    switch(op) {
	case FOCUS:
	    return(DesktopFunction(&Desktop::DoFocus));
	case LOWER:
	    return(DesktopFunction(&Desktop::DoLower));
	case RAISE:
	    return(DesktopFunction(&Desktop::DoRaise));
	case REDRAW:
	    return(DesktopFunction(&Desktop::DoRedraw));
	case ICONIFY:
	    return(DesktopFunction(&Desktop::DoIconify));
	case MOVE:
	    return(DesktopFunction(&Desktop::DoMove));
	case RESIZE:
	    return(DesktopFunction(&Desktop::DoResize));
	case SCREENLOCK:
	    return(DesktopFunction(&Desktop::DoLock));
	case EXIT:
	    return(DesktopFunction(&Desktop::DoExit));
	case NULL_OP:
	    return(nil);
    }
}

void Desktop::InsertRemote (void* i) {
    Coord x1, y1, x2, y2, tmp;
    int w, h;
    RubberRect* r;
    Event e;

    GetInfo(i, x1, y1, x2, y2);
    w = x2 - x1 + 1;
    h = y2 - y1 + 1;
    GrabMouse(upperleft);
    do {
	Poll(e);
    } while (!e.leftmouse && !e.middlemouse && !e.rightmouse);
    e.target = this;
    e.GetAbsolute(x1, y1);
    if (e.leftmouse && w > 1) {
	x2 = x1 + w - 1;
	y2 = y1 + h - 1;
	r = new SlidingRect(output, canvas, x1, y1, x2, y2, x1, y2);
	r->Track(x1, y1);
    } else {
	do {
	    Poll(e);
	    e.target = this;
	    e.GetAbsolute(x2, y2);
	    x2 = e.x; y2 = e.y;
	} while (x2 == x1 && y2 == y1 &&
	    (e.leftmouse || e.middlemouse || e.rightmouse)
	);
	r = new RubberRect(output, canvas, x1, y1, x2, y1);
	r->Track(x2, y1);
	GrabMouse(lowerright);
    }
    /* Lock(); */
    while (e.leftmouse || e.middlemouse || e.rightmouse) {
	Poll(e);
	e.target = this;
	e.GetAbsolute(x1, y1);
	x1 = e.x; y1 = e.y;
	r->Track(x1, y1);
    }
    /* Unlock(); */
    UngrabMouse();
    r->GetCurrent(x1, y1, x2, y2);
    delete r;
    if (x1 > x2) {
	tmp = x1; x1 = x2; x2 = tmp;
    }
    if (y1 > y2) {
	tmp = y1; y1 = y2; y2 = tmp;
    }
    Change(i, x1, y2, x2-x1+1, y2-y1+1);
    MapRaised(i);
}

void Desktop::DoFocus (Event& e) {
    RemoteInteractor i;

    if (e.eventType == UpEvent) {
	i = Choose(crosshairs, false);
    } else {
	i = Find(e.x, e.y);
    }
    Focus(i);
}

void Desktop::DoLower (Event& e) {
    RemoteInteractor i;

    if (e.eventType == UpEvent) {
	i = Choose(crosshairs, false);
    } else {
	i = Find(e.x, e.y);
    }
    if (i != nil) {
	Lower(i);
    }
}

void Desktop::DoRaise (Event& e) {
    RemoteInteractor i;

    if (e.eventType == UpEvent) {
	i = Choose(crosshairs, false);
    } else {
	i = Find(e.x, e.y);
    }
    if (i != nil) {
	Raise(i);
    }
}

void Desktop::DoRedraw (Event&) {
    RedrawAll();
}

void Desktop::AddIcon (Icon* icon) {
    register IconList* i = new IconList;
    i->ri = icon->Parent()->canvas->Id();
    i->icon = icon;
    i->next = icons;
    icons = i;
}

void Desktop::RemoveIcon (RemoteInteractor ri) {
    register IconList* i, * prev;

    prev = nil;
    for (i = icons; i != nil; i = i->next) {
	if (i->ri == ri) {
	    if (prev != nil) {
		prev->next = i->next;
	    } else {
		icons = i->next;
	    }
	    i->ri = GetIcon(i->ri);
	    if (i->ri != nil) {
		UnassignIcon(i->ri);
	    }
	    delete i->icon->Parent();
	    delete i;
	    break;
	}
	prev = i;
    }
}

boolean Desktop::IconIsOld (RemoteInteractor ri, const char* s) {
    Icon* i = FindIcon(ri);
    if (i != nil && i->IsOld(s)) {
	RemoveIcon(ri);
	return true;
    }
    return false;
}

Icon* Desktop::FindIcon (RemoteInteractor ri) {
    register IconList* i;

    for (i = icons; i != nil; i = i->next) {
	if (i->ri == ri) {
	    return i->icon;
	}
    }
    return nil;
}

void Desktop::DoIconify (Event& e) {
    RemoteInteractor i, icon;
    Coord x, y, x1, y1, x2, y2;
    int width, height;
    Frame* f = nil;
    Icon* new_icon;
    char* s;

    if (e.eventType == UpEvent) {
	i = Choose(crosshairs, false);
    } else {
	i = Find(e.x, e.y);
    }
    if (i == nil) {
	return;
    }
    icon = GetIcon(i);
    if (FindIcon(i) == nil) {
	/*
	 * Not on our list of icons.  If the remote interactor has an
	 * icon, use it, otherwise create one for it based on its name.
	 */
	s = GetName(i);
	if (icon == nil || IconIsOld(icon, s)) {
	    if (s == nil) {
		s = "<window>";
	    }
	    new_icon = new Icon(output, s);
	    f = new Frame(output, new_icon);
	    width = f->shape->width;
	    height = f->shape->height;
	} else {
	    GetInfo(icon, x1, y1, x2, y2);
	    width = x2 - x1;
	    height = y2 - y1;
	}
	Poll(e);
	x = e.x - (width >> 1) + 1;
	if (x < 0) {
	    x = 0;
	}
	if (x + width + 2 > world->Width()) {
	    x = world->Width() - width - 2;
	}
	y = e.y - (height >> 1) + 1;
	if (y < 0) {
	    y = 0;
	}
	if (y + height + 2 > world->Height()) {
	    y = world->Height() - height - 2;
	}
	if (f != nil) {
	    world->Insert(f, x, y);
	    AssignIcon(i, f->canvas->Id());
	    AddIcon(new_icon);
	    icon = nil;
	}
    }
    if (icon != nil) {
	MapRaised(icon);
    }
    Unmap(i);
}

void Desktop::DoExit(Event&) {
    running = false;
}

void Desktop::DoMove (Event& e) {
    Coord startleft, startbottom;
    Coord left, bottom, right, top;
    RemoteInteractor i;

    GrabMouse(crosshairs);

    while (e.eventType != DownEvent) {
	Read(e);
    }
    i = Find(e.x, e.y);
    if (i == nil) {
	UngrabMouse();
	return;
    }
    GetInfo(i, left, bottom, right, top);
    startleft = left;
    startbottom = bottom;
    Poll(e);
    SlidingRect r(output, canvas, left, bottom, right, top, e.x, e.y);
    Lock();
    r.Draw();
    boolean aborted = false;
    boolean tracking = true;
    while (tracking && !aborted) {
	int buttoncount = 0;
	if ( e.leftmouse ) {
	    ++buttoncount;
	}
	if ( e.middlemouse ) {
	    ++buttoncount;
	}
	if ( e.rightmouse ) {
	    ++buttoncount;
	}
	if ( buttoncount > 1 ) {
	    aborted = true;
	} else if ( buttoncount == 0 ) {
	    tracking = false;
	} else {
	    r.Track(e.x, e.y);
	    Poll(e);
	}
    }
    r.Erase();
    Unlock();
    UngrabMouse();
    ClearInput();
    if ( !aborted ) {
	r.GetCurrent(left, bottom, right, top);
	Move(i, left, top);
    }
}

void Desktop::DoResize (Event& e) {
    Coord left, bottom, right, top;
    Coord xfixed, yfixed, xmoved, ymoved;
    int width, height;
    RemoteInteractor i;

    GrabMouse(crosshairs);
    while (e.eventType != DownEvent) {
	Read(e);
    }
    i = Find(e.x, e.y);
    if (i == nil) {
	UngrabMouse();
	return;
    }
    GetInfo(i, left, bottom, right, top);
    width = right - left + 1;
    height = top - bottom + 1;
    Poll(e);
    SizingRect r(
	output, canvas,
	left, bottom, right, top, e.x, e.y,
	snap_resize, constrain_resize
    );
    r.Draw();
    boolean aborted = false;
    boolean tracking = true;
    while (tracking && !aborted) {
	int buttoncount = 0;
	if (e.leftmouse) {
	    ++buttoncount;
	}
	if (e.middlemouse) {
	    ++buttoncount;
	}
	if (e.rightmouse) {
	    ++buttoncount;
	}
	if (buttoncount > 1) {
	    aborted = true;
	} else if (buttoncount == 0) {
	    tracking = false;
	} else {
	    r.Track(e.x, e.y);
	    Poll(e);
	}
    }
    r.Erase();
    Unlock();
    UngrabMouse();
    ClearInput();
    if (!aborted) {
	r.GetCurrent(xfixed, yfixed, xmoved, ymoved);
	if (xfixed > xmoved) {
	    width = xfixed - xmoved + 1;
	    left = xmoved;
	} else {
	    width = xmoved - xfixed + 1;
	    left = xfixed;
	}
	if (yfixed > ymoved) {
	    height = yfixed - ymoved + 1;
	    bottom = ymoved;
	} else {
	    height = ymoved - yfixed + 1;
	    bottom = yfixed;
	}
	Change(i, left, bottom + height - 1, width, height);
    }
}

void Desktop::DoLock (Event&) {
    lock->InsertInto(world);
    Focus(Find(world->Width()/2, world->Height()/2));
    GrabMouse(lock->LockCursor());
    if (lock_server) {
	Lock();
    }
    lock->Activate();
    if (lock_server) {
	Unlock();
    }
    UngrabMouse();
    Focus(nil);
    ClearInput();
    lock->RemoveFrom(world);
}

void Desktop::Config () {
    char* v;
    int w, h;
    unsigned mask;
    int height = world->Height();
    int width = world->Width();
    Font* f;

    v = world->GetDefault("logo");
    if (v != nil) {
	mask = world->ParseGeometry(v, logo_x, logo_y, w, h);
    } else {
	mask = 0;
    }
    if (mask & GeomHeightValue) {
	if (w != h) {
	    fprintf(stderr, "%s: .Xdefaults: for the logo, do not specify a height (the logo is square)\n", program_name);
	    exit(1);
	}
    } else if (!(mask & GeomWidthValue)) {
	/* width should have form 13n + 1 */
	w = 53;
    }
    logosize = w;

    if (mask & GeomYValue) {
	logo_y = (mask & GeomYNegative) ? -logo_y : height - logosize - logo_y;
    } else {
	logo_y = height - logosize - 4;
    }
    if (mask & GeomXValue) {
	logo_x = (mask & GeomXNegative) ? width - logosize + logo_x : logo_x;
    } else {
	logo_x = width - logosize - 4;
    }
    v = world->GetDefault("inverse");
    if (v != nil) {
	if (Equal(v, "true")) {
	    output->SetColors(output->GetBgColor(), output->GetFgColor());
	} else if (!Equal(v, "false")) {
	    fprintf(stderr, "%s: .Xdefaults: inverse must be true or false, `%s' is invalid\n", program_name, v);
	    exit(1);
	}
    }
    ignorecaps = false;
    v = world->GetDefault("ignorecaps");
    if (v != nil) {
	if (Equal(v, "true")) {
	    ignorecaps = true;
	} else if (!Equal(v, "false")) {
	    fprintf(stderr, "%s: .Xdefaults: ignorecaps must be true or false, `%s' is invalid\n", program_name, v);
	    exit(1);
	}
    }

    lock_server = false;
    v = world->GetDefault("lock");
    if (v != nil) {
	if (Equal(v, "true")) {
	    lock_server = true;
	} else if (!Equal(v, "false")) {
	    fprintf(stderr, "%s: .Xdefaults: lock must be true or false, `%s' is invalid\n", program_name, v);
	    exit(1);
	}
    }
    v = world->GetDefault("font");
    if (v != nil) {
	f = new Font(v);
	if (f->Height() > 0) {
	    output->SetFont(f);
	} else {
	    fprintf(stderr, "%s: .Xdefaults: `%s' is not a valid font\n", program_name, v);
	    exit(1);
	}
    }
    constrain_resize = true;
    v = world->GetDefault("constrainresize");
    if (v != nil) {
	if (Equal(v, "false")) {
	    constrain_resize = false;
	} else if (!Equal(v, "true")) {
	    fprintf(stderr, "%s: .Xdefaults: constrainresize must be true or false, `%s' is invalid\n", program_name, v);
	    exit(1);
	}
    }
    snap_resize = false;
    v = world->GetDefault("snapresize");
    if (v != nil) {
	if (Equal(v, "true")) {
	    snap_resize = true;
	} else if (!Equal(v, "false")) {
	    fprintf(stderr, "%s: .Xdefaults: snapresize must be true or false, `%s' is invalid\n", program_name, v);
	    exit(1);
	}
    }
    v = world->GetDefault("bitmap");
    if (v != nil) {
	logo_bitmap = new Bitmap(v);
	if (logo_bitmap->Width() <= 0) {
	    fprintf(stderr, "%s: .Xdefaults: bitmap file `%s' not found\n", program_name, v);
	    exit(1);
	}
    }
}
