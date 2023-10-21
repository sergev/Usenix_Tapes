#include "dispatch.h"
#include "desktop.h"
#include <InterViews/cursor.h>
#include <InterViews/Xinput.h>
#include <InterViews/world.h>
#include <string.h>

extern int close(int);
extern int execl(const char*, const char* ...);
extern int vfork();
extern int wait(int*);
extern void _exit(int);
extern char* getenv(const char*);

extern char* DesktopFunctionNames[Operations];		   /* desktop.c */
extern DesktopFunction DesktopFunctions(OperationCode);	   /* desktop.c */

static CursorPattern subcurbits = {
   0x0000, 0x0000, 0x0000, 0x0000,
   0x0010, 0x0018, 0x001c, 0x7ffe,
   0x7ffe, 0x001c, 0x0018, 0x0010,
   0x0000, 0x0000, 0x0000, 0x0000
};

static CursorPattern subcurmask = {
   0x0000, 0x0000, 0x0000, 0x0038,
   0x003c, 0x003e, 0xffff, 0xffff,
   0xffff, 0xffff, 0x003e, 0x003c,
   0x0038, 0x0000, 0x0000, 0x0000
};

static Cursor* subCursor;

inline char tolower (char c) {
    if (c >= 'A' && c <= 'Z') {
	return c + 'a' - 'A';
    } else {
	return c;
    }
}

boolean Equal (const char* s1, const char* s2) {
    if (s1 != nil && s2 != nil) {
	if (tolower(*s1) == tolower(*s2)) {
	    if (*s1 == '\0') {
		return true;
	    } else {
		return Equal(++s1, ++s2);
	    }
	} else {
	    return false;
	}
    } else {
	return s1 == s2;
    }
}

class DesktopMenu : public Menu {
    char* name;
    DesktopMenu* next;
    boolean persistent;
public:
    DesktopMenu(const char* name, DesktopMenu* next,
		Painter* p, boolean persistent) : (nil, p, persistent) {
	this->name = new char[strlen(name)+1];
	strcpy(this->name, name);
	this->next = next;
	this->persistent = persistent;
    }
    ~DesktopMenu() { delete name; }

    DesktopMenu* Find(const char* name) {
	if (Equal(name, this->name)) {
	    return this;
	} else if (next != nil) {
	    return next->Find(name);
	} else return nil;
    }

    DesktopMenu* Next() { return next; }
    boolean Persistent() { return persistent; }
};

class DesktopSelection : public TextItem {
protected:
    Operation* op;
public:
    DesktopSelection(Painter* p, const char* s) : (p, s) {
	op = new Operation;
	op->f = nil; op->uop = nil; op->menu = nil;
    }
    virtual Operation* Op();
};

Operation* DesktopSelection::Op () { return op; }

class FunctionSelection : public DesktopSelection {
public:
    FunctionSelection(Painter* p, const char* s, DesktopFunction f) : (p, s) {
	op->f = f;
    }
};

class UserSelection : public DesktopSelection {
public:
    UserSelection(Painter* p, const char* s, const char* uop) : (p, s) {
	op->uop = (char*)uop;
    }
    ~UserSelection() { delete op->uop; }
};

class SubMenuSelection : public DesktopSelection {
    DesktopMenu* menu;
    DesktopSelection* ms;
    Sensor* sensor;
    int threshold;
    boolean ready, needs_highlighting;
public:
    SubMenuSelection (Painter* p, const char* s, DesktopMenu* menu) : (p, s) {
	op->menu = menu;
	this->menu = menu;
	ms = nil;
	if (!menu->Persistent()) {
	    sensor = new Sensor(onoffEvents);
	    sensor->Catch(MotionEvent);
	    Listen(sensor);
	    ready = false;
	    needs_highlighting = false;
	    threshold = 0;
	}
    }

    Operation* Op () {
	if (menu->Persistent()) {
	    return op;
	} else if (ms != nil) {
	    return ms->Op();
	} else return nil;
    }

    void Handle(Event&);
};

void SubMenuSelection::Handle (Event& e) {
    switch (e.eventType) {
	case MotionEvent:
	    if (!threshold) threshold = xmax - (xmax / 8) + 1;
	    if (ready) {
		if (e.x >= threshold) {
		    menu->Popup(e, ms);
		    ready = false;
		    UnHighlight();
		    needs_highlighting = true;
		}
	    } else {
		if (needs_highlighting) {
		    Highlight();
		    needs_highlighting = false;
		}
		if (e.x < threshold) ready = true;
	    }
	    break;
	case OnEvent:
	    if (!menu->Persistent()) {
		SetCursor(subCursor);
	    }
	    /* fall through */
	default:
	    DesktopSelection::Handle(e);
	    ready = false;
    }
}

DesktopDispatcher::DesktopDispatcher(Desktop* d, Painter* p) {
    register DesktopMenu* m;
    char* home;
    char* iwmpath;

    forked = true;
    char* s = world->GetDefault("fork");
    if (s != nil && Equal(s, "false")) {
	forked = false;
    }
    subCursor = new Cursor(15, 8, subcurbits, subcurmask, black, white);
    output = p;
    desk = d;
    home = getenv("HOME");
    iwmpath = new char[strlen(home)+strlen(".iwmrc")+1];
    strcpy(iwmpath, home);
    strcat(iwmpath, "/.iwmrc");
    configfile = fopen(iwmpath, "r");
    if (configfile == nil) {
	configfile = fopen("/usr/local/lib/iwmrc.default", "r");
	if (configfile == nil) {
	    fprintf(stderr, "iwm: no configuration file found\n");
	    exit(1);
	}
    }
    for (int i = 0; i < Activators; i++) {
	ops[i] = nil;
    }
    menus = nil;
    buttons = nil;
    Config();
    m = menus;
    while (m != nil) {
	m->Compose();
	m = m->Next();
    }
}

DesktopDispatcher::~DesktopDispatcher () {
    register DesktopMenu* m;
    register ButtonList* b;
    register i;

    for (i = 0; i < Activators; i++) delete ops[i];
    delete ops;
    while (menus != nil) {
	m = menus->Next();
	delete(menus);
	menus = m;
    }
    while (buttons != nil) {
	b = buttons->next;
	delete(buttons);
	buttons = b;
    }
    delete output;
}

void DesktopDispatcher::Perform (Operation* op, Event& e) {
    DesktopSelection* s;

    if (op != nil) {
	if (op->f != nil) {
	    (*op->f)(desk, e);
	} else if (op->uop != nil) {
	    int pid = vfork();
	    if (pid < 0) {
		fprintf(stderr, "can't fork to execute '%s'\n", op->uop);
	    } else if (pid == 0) {
		/* child */
		int i;

		for (i = 3; close(i) != -1; i++);
		/* fork again so that sh process belongs to init */
		pid = vfork();
		if (pid < 0) {
		    fprintf(stderr, "can't fork to execute '%s'\n", op->uop);
		} else if (pid == 0) {
		    execl("/bin/sh", "sh", "-c", op->uop, 0);
		    _exit(127);
		}
		exit(0);
	    } else {
		/* parent */
		int status;

		wait(&status);
	    }
	} else if (op->menu != nil) {
	    op->menu->Popup(e, s);
	    if (s != nil) {
		Perform(s->Op(), e);
	    }
	}
    }
}

void DesktopDispatcher::Perform (Event& e) {
    Perform(GetOperation(e), e);
}

Operation* DesktopDispatcher::GetOperation (Event& e) {
    register Activator a;

    switch (e.button) {
	case LEFTMOUSE:
	    a = LEFT;
	    break;
	case MIDDLEMOUSE:
	    a = MIDDLE;
	    break;
	case RIGHTMOUSE:
	    a = RIGHT;
	    break;
    }
    if (e.control) {
	a |= CTRL_MASK;
    }
    if (e.meta) {
	a |= META_MASK;
    }
    if (e.shift && !e.shiftlock) {
	a |= SHIFT_MASK;
    }
    if (a < Activators) {
	if (ops[a] != nil) {
	    return ops[a];
	} else if (e.shiftlock) {
	    a |= SHIFT_LOCK_MASK;
	    if (ops[a] != nil) {
		return ops[a];
	    } else {
		a |= SHIFT_MASK;
		if (ops[a] != nil) {
		    return ops[a];
		} else {
		    a &= ~(SHIFT_LOCK_MASK);
		    if (ops[a] != nil) {
			return ops[a];
		    } else {
			a &= (LEFT|MIDDLE|RIGHT);
			return ops[a];
		    }
		}
	    }
	} else {
	    a &= (LEFT|MIDDLE|RIGHT);
	    return ops[a];
	}
    }
}

boolean DesktopDispatcher::NoFunction (Event& e) {
    Operation* op = GetOperation(e);
    if (op == nil || (op->f == nil && op->uop == nil)) {
	return true;
    }
    return false;
}

const int argblocks = 5;
static char argblock[argblocks][80];

static char x[10];
static char s[80];

boolean GetLine(FILE* CF, int& user_arg) {
    char* p;
    int matches;

    do {
	p = fgets(s, 80, CF);
    } while (p != nil && (s[0] == '#' || s[0] == '\n'));

    if (p == nil) {
	return false;
    }

    *argblock[0] = '\0'; *argblock[1] = '\0'; *argblock[2] = '\0';
    *argblock[3] = '\0'; *argblock[4] = '\0';

    if (strchr(s, '"') != nil) {
	matches = sscanf(s, "%s \"%[^\"]%[\"] %[^\n]\n",
	    argblock[0], argblock[1], x, argblock[2]);
	if (matches < 3) {
	    matches = sscanf(s, "%s %s \"%[^\"]%[\"] %[^\n]\n",
		argblock[0], argblock[1], argblock[2], x, argblock[3]);
	    if (matches < 4) {
		matches = sscanf(s, "%s %s %s \"%[^\"]%[\"] %[^\n]\n",
		    argblock[0], argblock[1], argblock[2],
		    argblock[3], x, argblock[4]);
		if (matches < 5) {
		    fprintf(stderr, "iwm: config: invalid entry `%s'\n", s);
		    return GetLine(CF, user_arg);
		} else {
		    user_arg = 3;
		}
	    } else {
		user_arg = 2;
	    }
	} else {
	    user_arg = 1;
	}
    } else {
	matches = sscanf(s, "%s %s %s %s %[^\n]\n",
	    argblock[0], argblock[1], argblock[2], argblock[3], argblock[4]);
	if (matches < 4) {
	    matches = sscanf(s, "%s %s %s %[^\n]\n",
		argblock[0], argblock[1], argblock[2], argblock[3]);
	    if (matches < 3) {
		matches = sscanf(s, "%s %s %[^\n]\n",
		    argblock[0], argblock[1], argblock[2]);
		if (matches < 2) {
		    fprintf(stderr, "iwm: config: invalid entry `%s'\n", s);
		    return GetLine(CF, user_arg);
		}
	    }
	}
	user_arg = 0;
    }
    return true;
}

void DesktopDispatcher::Config () {
    Activator a, b;
    OperationCode op;
    DesktopFunction f;
    char* uop;
    DesktopMenu* a_menu;
    DesktopMenu* op_menu;
    int arg;
    char* t;
    int user_arg;
    unsigned keymask;

    while (GetLine(configfile, user_arg)) {
	a = 0; f = nil; uop = nil; keymask = 0;
	a_menu = nil; op_menu = nil; arg = 0;

	if (Equal(argblock[arg], "shift")) {
	    keymask |= ShiftMask; a |= SHIFT_MASK; arg++;
	} else if (Equal(argblock[arg], "ctrl") ||
		   Equal(argblock[arg], "control")) {
	    keymask |= ControlMask; a |= CTRL_MASK; arg++;
	} else if (Equal(argblock[arg], "meta")) {
	    keymask |= Mod1Mask; a |= META_MASK; arg++;
	} else if (Equal(argblock[arg], "lock")) {
	    keymask |= LockMask; a |= SHIFT_LOCK_MASK; arg++;
	}
	if (arg) {
	    if (Equal(argblock[arg], "shift")) {
		keymask |= ShiftMask; a |= SHIFT_MASK; arg++;
	    } else if (Equal(argblock[arg], "ctrl") ||
		       Equal(argblock[arg], "control")) {
		keymask |= ControlMask; a |= CTRL_MASK; arg++;
	    } else if (Equal(argblock[arg], "meta")) {
		keymask |= Mod1Mask; a |= META_MASK; arg++;
	    } else if (Equal(argblock[arg], "lock")) {
		keymask |= LockMask; a |= SHIFT_LOCK_MASK; arg++;
	    }
	    if (arg) {
		if (Equal(argblock[arg], "shift")) {
		    keymask |= ShiftMask; a |= SHIFT_MASK; arg++;
		} else if (Equal(argblock[arg], "ctrl") ||
			   Equal(argblock[arg], "control")) {
		    keymask |= ControlMask; a |= CTRL_MASK; arg++;
		} else if (Equal(argblock[arg], "meta")) {
		    keymask |= Mod1Mask; a |= META_MASK; arg++;
		} else if (Equal(argblock[arg], "lock")) {
		    keymask |= LockMask; a |= SHIFT_LOCK_MASK; arg++;
		}
		if (arg) {
		    if (Equal(argblock[arg], "shift")) {
			keymask |= ShiftMask; a |= SHIFT_MASK; arg++;
		    } else if (Equal(argblock[arg], "ctrl") ||
			       Equal(argblock[arg], "control")) {
			keymask |= ControlMask; a |= CTRL_MASK; arg++;
		    } else if (Equal(argblock[arg], "meta")) {
			keymask |= Mod1Mask; a |= META_MASK; arg++;
		    } else if (Equal(argblock[arg], "lock")) {
			keymask |= LockMask; a |= SHIFT_LOCK_MASK; arg++;
		    }
		}
	    }
	}
	if (Equal(argblock[arg], "left")) {
	    keymask |= LeftMask; a |= LEFT; arg++;
	} else if (Equal(argblock[arg], "middle")) {
	    keymask |= MiddleMask; a |= MIDDLE; arg++;
	} else if (Equal(argblock[arg], "right")) {
	    keymask |= RightMask; a |= RIGHT; arg++;
	} else {
	    if (menus == nil ||
		(a_menu = menus->Find(argblock[arg])) == nil) {
		fprintf(stderr, "iwm: config: unknown activator `%s'\n",
			argblock[arg]);
		continue;
	    } else {
		arg++;
	    }
	}

	if (keymask && !(keymask&(Mod1Mask|ShiftMask|ControlMask|LockMask))) {
	    keymask |= Mod1Mask; a |= META_MASK;
	}

	if (user_arg == arg) {
	    uop = new char[strlen(argblock[arg])+(forked ? 30 : 3)];
	    strcpy(uop, argblock[arg]);
	    if (forked) {
		if (strrchr(uop, '<') == nil) {
		    strcat(uop, " </dev/null");
		}
		if (strrchr(uop, '>') == nil) {
		    strcat(uop, " >/dev/null");
		}
		strcat(uop, " 2>&1");
	    }
	} else {
	    op = 0;
	    while ((op < Operations) &&
		   !Equal(DesktopFunctionNames[op], argblock[arg])) {
		op++;
	    }
	    if (op < Operations) {
		f = DesktopFunctions(op);
	    } else {
		if ((menus == nil) ||
		    (op_menu = menus->Find(argblock[arg])) == nil) {
		    if (!strcmp(argblock[arg+1], "*")) {
			op_menu = new DesktopMenu(
			    argblock[arg], menus, output, true
			);
			menus = op_menu;
			arg++;
		    } else {
			op_menu = new DesktopMenu(
			    argblock[arg], menus, output, false
			);
			menus = op_menu;
		    }
		}
	    }
	}

	arg++;

	if (a_menu != nil) {
	    if (strlen(argblock[arg]) > 0) {
		t = new char[81];
		strcpy(t, argblock[arg]); arg++;
		while (arg < argblocks && strlen(argblock[arg]) > 0) {
		    strcat(t, " ");
		    strcat(t, argblock[arg]);
		    arg++;
		}
	    } else {
		if (op_menu == nil) {
		    t = (char*)DesktopFunctionNames[op];
		} else {
		    fprintf(stderr, "iwm: config: no title for menu entry\n");
		    t = "???";
		}
	    }

	    if (f != nil) {
		a_menu->Insert(new FunctionSelection(output, t, f));
	    } else if (uop != nil) {
		a_menu->Insert(new UserSelection(output, t, uop));
	    } else if (op_menu != nil) {
		a_menu->Insert(new SubMenuSelection(output, t, op_menu));
	    } else {
		a_menu->Insert(new FunctionSelection(output, t, nil));
	    }
	} else {
	    delete ops[a];
	    ops[a] = new Operation;
	    ops[a]->f = f;
	    ops[a]->uop = uop;
	    ops[a]->menu = op_menu;
	    b = a & (LEFT|MIDDLE|RIGHT);
	    if (ops[b] == nil) {
		ops[b] = ops[a];
	    } else {
		ops[b] = nil;
		ops[b] = new Operation;
		ops[b]->f = nil;
		ops[b]->uop = nil;
		ops[b]->menu = nil;
	    }
	    if (ops[a]->f != nil) {
		AddButton(keymask, crosshairs);
	    } else if (ops[a]->uop != nil) {
		AddButton(keymask, noCursor);
	    } else if (ops[a]->menu != nil) {
		AddButton(keymask, arrow);
	    }
	}
    }
}
