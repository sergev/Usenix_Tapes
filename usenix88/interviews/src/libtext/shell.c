/*
 * Implementation of shell windows.
 */

#include <InterViews/Text/shell.h>
#include <InterViews/shape.h>
#include <InterViews/cursor.h>
#include <InterViews/sensor.h>
#include <InterViews/painter.h>
#include <InterViews/frame.h>
#include <InterViews/border.h>
#include <InterViews/box.h>
#include <InterViews/banner.h>
#include <InterViews/Text/emulator.h>
#include <InterViews/Text/textbuffer.h>
#include <sys/ioctl.h>
#include <stdio.h>

extern int open(char*, int, int);
extern void close(int);
extern int read(int, void*, int);
extern void write(int, const void*, int);
extern void ioctl(int, int, ...);

extern int strlen(const char*);
extern void strcpyn(char* dst, const char* src, int);

Shell.Shell (Sensor* in, Painter* out) : (in, out) {
    banner = new Banner(out, "shell view", nil, nil);
    text = new TextBuffer(in, out);
    Init();
}

Shell.Shell (int rows, int cols, Sensor* in, Painter* out) : (in, out) {
    banner = new Banner(out, "shell view", nil, nil);
    text = new TextBuffer(rows, cols, in, out);
    Init();
}

Shell.Shell (char* name, Sensor* in, Painter* out) : (in, out) {
    banner = new Banner(out, "shell view", name, nil);
    text = new TextBuffer(in, out);
    Init();
}

Shell.Shell (
    char* name, int rows, int cols, Sensor* in, Painter* out
) : (in, out) {
    banner = new Banner(out, "shell view", name, nil);
    text = new TextBuffer(rows, cols, in, out);
    Init();
}

void Shell.Init () {
    Painter* bg;

    if (input == nil) {
	input = new Sensor(onoffEvents);
	input->Catch(KeyEvent);
    }
    term = new Emulator(text);
    bg = new Painter;
    bg->SetColors(output->GetBgColor(), output->GetFgColor());
    frame = new Frame(
	output,
	new VBox(
	    banner, new HBorder(output), new Frame(bg, text)
	)
    );
    delete bg;
    *shape = *frame->shape;
    text->cursor = ltextCursor;
    Insert(frame);
    pty = -1;
}

void Shell.Delete () {
    if (frame != nil) {
	delete frame;
    }
    Scene::Delete();
}

void Shell.Draw () {
    frame->Draw();
}

void Shell.Resize () {
    Place(frame, 0, 0, xmax, ymax);
#   ifdef TIOCSWINSZ
	struct winsize w;
	int rows, cols;

	text->GetSize(rows, cols);
	w.ws_row = rows;
	w.ws_col = cols;
	ioctl(pty, TIOCSWINSZ, &w);
#   endif
}

void Shell.Run () {
    Event e;
    int r;
    char buf[4096];

    term->SetDevice(pty);
    input->CatchChannel(pty);
    for (;;) {
	Read(e);
	if (e.target == this) {
	    switch (e.eventType) {
		case OnEvent:
		    banner->highlight = true;
		    banner->Draw();
		    Sync();
		    break;
		case OffEvent:
		    banner->highlight = false;
		    banner->Draw();
		    Sync();
		    break;
		case KeyEvent:
		    if (e.keystring != nil) {
			write(pty, e.keystring, e.len);
		    }
		    break;
		case ChannelEvent:
		    r = read(pty, buf, sizeof(buf));
		    if (r > 0) {
			term->Write(buf, r);
		    } else {
			return;
		    }
		    break;
		default:
		    /* ignore */
		    break;
	    }
	} else {
	    e.target->Handle(e);
	}
    }
}

void Shell.Write (const char* buf, int len) {
    write(pty, buf, len);
}
