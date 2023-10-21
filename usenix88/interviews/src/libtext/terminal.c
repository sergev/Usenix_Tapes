/*
 * Support for terminal operations on a text buffer.
 */

#include <InterViews/Text/terminal.h>
#include <InterViews/shape.h>
#include <InterViews/sensor.h>
#include <InterViews/painter.h>
#include <InterViews/cursor.h>
#include <InterViews/frame.h>
#include <InterViews/banner.h>
#include <InterViews/border.h>
#include <InterViews/box.h>
#include <InterViews/Text/textbuffer.h>

extern char* XGetDefault(const char* name, const char* param);
extern int strcmp(const char*, const char*);
const char* AUTOHIDE = "AutoHideCursor";
const char* AUTOHIDEON = "on";

Terminal::Terminal (Sensor* in, Painter* out, boolean hide) : (in, out) {
    heading = new Banner(out, "Terminal View", nil, nil);
    text = new TextBuffer(in, out);
    const char * hidedefault = XGetDefault("", AUTOHIDE);
    if (hidedefault == nil) {
	autohide = hide;
    } else if (strcmp(hidedefault, AUTOHIDEON) == 0) {
	autohide = true;
    } else {
	autohide = false;
    }
    autohidden = false;
    Init();
}

Terminal::Terminal (
    int rows, int cols, Sensor* in, Painter* out, boolean hide
) : (in, out) {
    heading = new Banner(out, "Terminal View", nil, nil);
    text = new TextBuffer(rows, cols, in, out);
    const char * hidedefault = XGetDefault("",AUTOHIDE);
    if (hidedefault == nil) {
	autohide = hide;
    } else if (strcmp(hidedefault, AUTOHIDEON) == 0) {
	autohide = true;
    } else {
	autohide = false;
    }
    autohidden = false;
    Init();
}

void Terminal::Init () {
    Painter* bg;

    bg = new Painter(output);
    bg->SetColors(output->GetBgColor(), nil);
    frame = new Frame(output,
	new VBox(heading, new HBorder(output), new Frame(bg, text))
    );
    delete bg;
    *shape = *frame->shape;
    text->cursor = ltextCursor;
    if (input == nil) {
	input = new Sensor(onoffEvents);
	input->Catch(KeyEvent);
	if ( autohide ) {
	    input->Catch(MotionEvent);
	}
    }
    showcursor = true;
    currow = -1; curcol = -1;
    Insert(frame);
}

void Terminal::Delete () {
    if (frame != nil) {
	delete frame;
    }
    Scene::Delete();
}

void Terminal::Draw () {
    frame->Draw();
}

void Terminal::Resize () {
    Place(frame, 0, 0, xmax, ymax);
}

void Terminal::CursorOn () {
    if (!showcursor) {
	showcursor = true;
	text->CursorOn();
    }
}

void Terminal::CursorOff () {
    if (showcursor) {
	showcursor = false;
	text->CursorOff();
    }
}

inline void Terminal::BeginOp () {
    if (showcursor) {
	text->CursorOff();
    }
}

inline void Terminal::EndOp () {
    if (showcursor) {
	text->CursorOn();
    }
}

void Terminal::BatchedScrolling (boolean b) {
    text->BatchedScrolling(b);
}

void Terminal::GetSize (int& rows, int& cols) {
    text->GetSize(rows, cols);
}

void Terminal::ClearLines (int where, int count) {
    BeginOp();
    text->ClearLines(where, count);
    EndOp();
}

void Terminal::ClearScreen () {
    BeginOp();
    text->ClearScreen();
    text->SavePos();
    EndOp();
}

void Terminal::EraseBOL () {
    BeginOp();
    text->EraseBOL();
    EndOp();
}

void Terminal::EraseEOL () {
    BeginOp();
    text->EraseEOL();
    EndOp();
}

void Terminal::EraseEOS () {
    BeginOp();
    text->EraseEOS();
    EndOp();
}

void Terminal::EraseLine () {
    BeginOp();
    text->EraseLine();
    EndOp();
}

void Terminal::CursorLeft (int n) {
    BeginOp();
    text->CursorLeft(n);
    EndOp();
}

void Terminal::CursorUp (int n) {
    BeginOp();
    text->CursorUp(n);
    EndOp();
}

void Terminal::CursorRight (int n) {
    BeginOp();
    text->CursorRight(n);
    EndOp();
}

void Terminal::CursorDown (int n) {
    BeginOp();
    text->CursorDown(n);
    EndOp();
}

void Terminal::Goto (int row, int col) {
    BeginOp();
    text->Goto(row, col);
    text->SavePos();
    EndOp();
}

void Terminal::InsertCharacters (int n) {
    BeginOp();
    text->InsertCharacters(n);
    EndOp();
}

void Terminal::DeleteCharacters (int n) {
    BeginOp();
    text->DeleteCharacters(n);
    EndOp();
}

void Terminal::InsertLines (int n) {
    BeginOp();
    text->InsertLines(n);
    EndOp();
}

void Terminal::DeleteLines (int n) {
    BeginOp();
    text->DeleteLines(n);
    EndOp();
}

void Terminal::ScrollDown (int where, int count) {
    BeginOp();
    text->ScrollDown(where, count);
    EndOp();
}

void Terminal::ScrollUp (int where, int count) {
    BeginOp();
    text->ScrollUp(where, count);
    EndOp();
}

void Terminal::AddChar (char c) {
    BeginOp();
    text->AddChar(c);
    text->Flush();
    EndOp();
}

void Terminal::AddString (const char* s) {
    register char* p;

    BeginOp();
    for (p = (char*) s; *p != nil; p++) {
	text->AddChar(*p);
    }
    text->Flush();
    EndOp();
}

void Terminal::AddString (const char* s, int n) {
    register char* p;
    register int i;

    BeginOp();
    for (p = (char*) s, i = 0; *p != nil && i < n; p++, i++) {
	text->AddChar(*p);
    }
    text->Flush();
    EndOp();
}

void Terminal::CarriageReturn () {
    BeginOp();
    text->CarriageReturn();
    EndOp();
}

void Terminal::LineFeed () {
    BeginOp();
    text->Flush();
    text->ForwardScroll();
    text->Flush();
    EndOp();
}

void Terminal::BackSpace () {
    BeginOp();
    text->BackSpace();
    EndOp();
}

void Terminal::Tab () {
    BeginOp();
    text->Tab();
    EndOp();
}

void Terminal::Underline (boolean b) {
    text->Underline(b);
}

void Terminal::Inverse (boolean b) {
    text->Inverse(b);
}

void Terminal::Bold (boolean b) {
    text->Bold(b);
}

void Terminal::Blink (boolean b) {
    text->Blink(b);
}

void Terminal::RingBell (int vol) {
//  world->RingBell(vol);
}

inline boolean Inside(int row, int col, int height, int width)
{
    return (row > 0) && (col > 0) && (row <= height) && (col <= width);
}

void Terminal::QMotion () {
    if ( ! autohide ) {
	input->Catch(MotionEvent);
	Listen(input);
    }
}

void Terminal::UnQMotion () {
    if ( ! autohide ) {
	input->Ignore(MotionEvent);
	Listen(input);
    }
}

void Terminal::QMouseButton (int b) {
    input->CatchButton(DownEvent, b);
    input->CatchButton(UpEvent, b);
    Listen(input);
}

void Terminal::UnQMouseButton (int b) {
    input->IgnoreButton(DownEvent, b);
    input->IgnoreButton(UpEvent, b);
    Listen(input);
}

void Terminal::Read (TerminalInput& i) {
    Event e;

    for (;;) {
	Interactor::Read(e);
	if (e.target == this && Interesting(e, i)) {
	    break;
	}
    }
}

boolean Terminal::QTest () {
    return Check();
}

boolean Terminal::Interesting (Event& e, TerminalInput& i) {
    boolean b;
    register int r, c;

    b = false;
    if (e.eventType == OnEvent) {
	heading->highlight = true;
	heading->Draw();
	if (showcursor) {
	    text->CursorOff();
	    text->OutlineCursor(false);
	    text->CursorOn();
	} else {
	    text->OutlineCursor(false);
	}
	Sync();
    } else if (e.eventType == OffEvent) {
	heading->highlight = false;
	heading->Draw();
	if (showcursor) {
	    text->CursorOff();
	    text->OutlineCursor(true);
	    text->CursorOn();
	} else {
	    text->OutlineCursor(true);
	}
	Sync();
    } else {
	r = text->Row(e.y);
	c = text->Column(e.x);
	switch (e.eventType) {
	    case MotionEvent:
		if ( autohide && autohidden ) {
		    text->SetCursor( text->cursor );
		    autohidden = false;
		}
		if ((r != currow || c != curcol) &&
		    e.x > 0 && e.x < text->xmax &&
		    e.y > 0 && e.y < text->ymax
		) {
		    i.eventType = TMOTION;
		    i.motion.row = r;
		    i.motion.col = c;
		    currow = r;
		    curcol = c;
		    b = true;
		}
		break;
	    case DownEvent:
	    case UpEvent:
		i.eventType = (TerminalEvent) e.button;
		i.mouse.row = r;
		i.mouse.col = c;
		i.mouse.down = (e.eventType == DownEvent);
		b = true;
		break;
	    case KeyEvent:
		if ( autohide && ! autohidden ) {
		    text->SetCursor( noCursor );
		    autohidden = true;
		}
		if (e.len != 0) {
		    i.eventType = TCHAR;
		    i.tchar = e.keystring[0];
		    b = true;
		}
		break;
	    default:
		/* ignore */;
	}
    }
    return b;
}

void Terminal::SetBannerString (const char* s) {
    heading->left = (char*) s;
    heading->Update();
}

void Terminal::SetCursor(Cursor * c) {
    text->SetCursor(c);
}
