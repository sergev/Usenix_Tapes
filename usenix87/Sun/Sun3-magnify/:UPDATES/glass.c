/*
 *  Magnifying glass		(for Sun-3 workstations)
 *
 *  Copyright 1986, 1987 by Scott Schwartz
 *  This program may be copied with the provision that this copyright 
 *  message remains, and that further distribution of this code and it's 
 *  derivatives is not subject to additional restrictions.
 * 
 *  Lots of changes, but no Copyright, by Mark Weiser.
 * 
 *  compile with -lsuntool -lsunwindow -lpixrect
 *
 *  vi: set ts=8
 *
 *  revision history:
 *  10 Nov 86	initial coding		 		Scott Schwartz 
 *
 *  version 1.0		25 Nov 86
 *	modified to draw big pixels with raster-ops		ses
 *
 *  version 1.1		27 Jan 87
 *	does magnification in memory, 				ses
 *	to avoid screen access
 *
 *  version 1.2		27 Jan 87
 *	flood destination with white, 				ses/af
 *      then draw only black spots
 *
 *  version 2.0		15 Apr 87				mark weiser
 *	vastly faster magnification algorithm using only 2k blits
 *	instead of k**2 (where k is the height or width of the final).
 *	Also added a panel for friendliness, and stopped using *all* the cpu!
 *
 *  version 2.1		17 Apr 87				mark weiser
 *	added locking.
 *
 *  version 2.2		20 Apr 87				mark weiser
 *	added changes for color suns (not tested here).
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <suntool/sunview.h>
#include <suntool/canvas.h>
#include <suntool/panel.h>
#include <suntool/textsw.h>
#include <pixrect/pixrect_hs.h>

/* I HATE these macros	-mdw */
#undef min
#undef max


/* useful macros */
#define ERROR(msg)  { fprintf(stderr, "%s: %s\n", argv[0], msg); exit(1); }

/* forward declarations */
extern char *getenv(), *sprintf();
extern Notify_error notify_dispatch();
static Notify_value notice_destroy_event();
extern void pw_mag();
extern void view();
void update();
void glass_time_proc();
void lock_proc(), unlock_proc();
Notify_value update_value_proc();

/* constants */
static char frame_label[] = "Magnifying Glass 2.2";
static short icon_image[] = {
#include "glass.icon"
};
DEFINE_ICON_FROM_IMAGE(icon, icon_image);
#define NILPR ((struct pixrect *)0)
#define SCREEN_MAX_X  1152
#define SCREEN_MAX_Y  900
#define MAX_MAG 32

/* global vars */
static struct pixrect	*tmpsrc; /* working space, to avoid screen access */ 
static struct pixrect	*tmpdst;
static int done = 0;
struct pixrect *srcpr; /* the source pixrect, i.e. the whole screen */
int rootfd;  /* file descriptor for root window, for cursor location */

/* things to draw on */
Panel panel;
Frame frame;	/* the actual window frame */
Canvas canvas;	/* our particular canvas, filling the frame */

int mag = 1;			/* magnification in output window */
int delay = 1000;
int locked = 0;
int locked_x, locked_y;
Panel_item time_left_item;

/* 
 *  main routine
 */
main(argc,argv)
int argc;
char **argv;
{
	/* externals */
	extern struct pixrect *tmpsrc, *tmpdst;

	/* things to magnify */
	char *parent; /* name of the parent window, usually win0 */

	/* misc */
	char *magstr = "x99999";	/* allocate space for actual string */
	int retcode;

	/* open rootwindow */
	parent = getenv("WINDOW_PARENT");
	if (parent==NULL) 
		ERROR("can't get WINDOW_PARENT from environment.")
	rootfd = open(parent, O_RDONLY, 0);
	if (rootfd<0)
		perror(argv[0]); 

	/* open frame buffer */
	srcpr = pr_open("/dev/fb");
	if (srcpr==NILPR)
		perror(argv[0]);

	/* create output window */
	frame = window_create(NULL, FRAME, 
			FRAME_LABEL, frame_label,
			FRAME_ICON, &icon,
			FRAME_ARGC_PTR_ARGV, &argc, argv,
			0);

	/* handle arguments */		
	if (argc > 1) {
		retcode = sscanf(argv[1], "%d", &mag);
		if (retcode <= 0) 
			ERROR("problem evaluating arguments\nUsage: glass magnification [suntools-options]");
		mag = (mag > MAX_MAG) ? MAX_MAG : mag;
	}

	panel = window_create(frame, PANEL, 0);
	(void) panel_create_item(panel, PANEL_SLIDER,
		PANEL_LABEL_STRING, "Mag:",
		PANEL_VALUE, mag,
		PANEL_MIN_VALUE, 1,
		PANEL_MAX_VALUE, MAX_MAG,
		PANEL_SHOW_RANGE, FALSE,
		PANEL_SHOW_VALUE, TRUE,
		PANEL_SLIDER_WIDTH, 100,
		PANEL_NOTIFY_PROC, update_value_proc,
		PANEL_CLIENT_DATA, &mag,
		0);
	(void) panel_create_item(panel, PANEL_SLIDER,
		PANEL_LABEL_STRING, "Delay:",
		PANEL_VALUE, 10000,
		PANEL_MIN_VALUE, 1000,
		PANEL_MAX_VALUE, 999999,
		PANEL_SHOW_RANGE, FALSE,
		PANEL_SHOW_VALUE, FALSE,
		PANEL_SLIDER_WIDTH, 100,
		PANEL_NOTIFY_PROC, update_value_proc,
		PANEL_CLIENT_DATA, &delay,
		PANEL_ITEM_X, ATTR_COL(0),
		PANEL_ITEM_Y, ATTR_ROW(1),
		0);

	(void) panel_create_item(panel, PANEL_BUTTON,
		PANEL_LABEL_IMAGE, panel_button_image(panel, "Lock", 6, 0),
		PANEL_NOTIFY_PROC, lock_proc,
		PANEL_ITEM_X, ATTR_COL(0),
		PANEL_ITEM_Y, ATTR_ROW(2),
		0);

	(void) panel_create_item(panel, PANEL_BUTTON,
		PANEL_LABEL_IMAGE, panel_button_image(panel, "Unlock", 6, 0),
		PANEL_NOTIFY_PROC, unlock_proc,
		0);

	time_left_item = panel_create_item(panel, PANEL_SLIDER,
		PANEL_LABEL_STRING, "Time Left:",
		PANEL_VALUE, 10,
		PANEL_MIN_VALUE, 0,
		PANEL_MAX_VALUE, 10,
		PANEL_SHOW_RANGE, FALSE,
		PANEL_SHOW_VALUE, FALSE,
		PANEL_SLIDER_WIDTH, 100,
		PANEL_ITEM_X, ATTR_COL(0),
		PANEL_ITEM_Y, ATTR_ROW(2),
		PANEL_SHOW_ITEM, FALSE,
		0);
	window_fit_height(panel);

	/* allocate static pixrects */
	/* (If the last parameter is change from 1 to 8, this might work for color -mdw */
	tmpsrc = mem_create(SCREEN_MAX_X, SCREEN_MAX_Y, srcpr->pr_depth);
	tmpdst = mem_create(SCREEN_MAX_X, SCREEN_MAX_Y, srcpr->pr_depth);

	/* gather data on output pixwin */
	canvas = window_create(frame, CANVAS,
			WIN_HEIGHT, 200,
			WIN_WIDTH, 200,
			0);

	window_fit(frame);

	/* set up an interposed event handler so we know when to quit */
	(void)notify_interpose_destroy_func(frame, notice_destroy_event);
	
	/* start us in a second */
	do_with_delay(glass_time_proc, 1, 0);

	/* copy input to output forever */
	window_main_loop(frame);
	exit(0);
}

/*
 * Thing to do at intervals.
 */
void
glass_time_proc()
{
	if (done) return;
	if (window_get(frame, FRAME_CLOSED)) {
		do_with_delay(glass_time_proc, 2, 0);
	} else {
		view(rootfd, srcpr, canvas, mag);
		do_with_delay(glass_time_proc, 0, delay);
	}
}

/*
 * view does the work of displaying the (possibly) magnified image
 * at the location indicated by rootfd (mouse). view copies srcpr to dstpw.
 */
void view(rootfd, srcpr, canvas, mag)
	int rootfd;		/* root window, for mouse data */	
	struct pixrect *srcpr;		/* screen source pixrect */
	Canvas canvas;
	int mag;
{
	/* constants */
	int maxy = (srcpr->pr_size.y);
	int maxx = (srcpr->pr_size.x);
	Pixwin *dstpw = canvas_pixwin(canvas);

	/* local vars */	
	int x,y;
	int w, h;

	/* 
	 * read mouse coords from vuid register.  sadly, the sunview
	 * programmers guide is not clear on this.  appendix A tells some,
	 * but you have to look at <sundev/vuid_event.h> to see what
 	 * virtual events you can find out about.  luckily, the mouse is
	 * one of them.
	 */
	if (locked) {
		x = locked_x;
		y = locked_y;
	} else {
		x = win_get_vuid_value(rootfd, LOC_X_ABSOLUTE);
		y = win_get_vuid_value(rootfd, LOC_Y_ABSOLUTE);
	}

	/* 
	 * find out how big our drawing surface is.
	 */
	w = (int) window_get(canvas, CANVAS_WIDTH);
	h = (int) window_get(canvas, CANVAS_HEIGHT);

	/*
	 * draw on it.
	 */
	if (mag<=1) {
		x = min(maxx-w, max(x-w, 0));
		y = min(maxy-h, max(y-h, 0));
		update(dstpw, 0, 0, w, h, PIX_SRC, srcpr, x, y);
	}
	else {
		x = min(maxx-w/mag, max(x-w/mag, 0));
		y = min(maxy-h/mag, max(y-h/mag, 0));
		pw_mag(dstpw, 0, 0, w, h, mag, srcpr, x, y);
	}
}

/*
 * pw_mag copies a magnified view of spr to dpw using pixel replication.
 * the arguments are the same as those to the pw_rop library call, except
 * that magnification is passed instead of raster-op.
 */
void pw_mag(dpw, dx, dy, w, h, mag, spr, sx, sy)
	Pixwin *dpw;	/* destination pixwin */
	int dx, dy;  	/* destination x,y */
	int w, h;	/* width and height of block to copy */
	int mag; 	/* magnification */
	struct pixrect *spr;	/* source pixrect */
	int sx,sy;	/* location in source to copy from */
{
	/* locals */
	register short si, sj;
	register short di, dj;
	register short jmax = h/mag + 1, imax = w/mag + 1;
	register short x, y, delta;

	struct pixrect r;	/* holds the size of the drawing region when */
			/* gaining access to the screen */
	r.pr_size.x = w;
	r.pr_size.y = h;

	/* make off screen copy of source */
	pr_rop(tmpsrc, 0, 0, w, h, PIX_SRC|PIX_DONTCLIP, spr, sx, sy);

	for (x = 0; x < imax; x += 1) {
		for (delta = 0; delta < mag; delta += 1) {
			pr_rop(tmpdst, x*mag+delta, 0, 1, jmax, PIX_SRC|PIX_DONTCLIP, tmpsrc, x, 0);
		}
	}
	for (y = jmax; y >= 0; y -= 1) {
		for (delta = 0; delta < mag; delta += 1) {
			pr_rop(tmpdst, 0, y*mag+delta, w, 1, PIX_SRC|PIX_DONTCLIP, tmpdst, 0, y);
		}
	}


	/* draw */
	update(dpw, dx, dy, w, h, PIX_SRC, tmpdst, 0, 0);
}

/* for debugging purposes, mostly */
void update(dpw, dx, dy, w, h, mag, spr, sx, sy)
	Pixwin *dpw;	/* destination pixwin */
	int dx, dy;  	/* destination x,y */
	int w, h;	/* width and height of block to copy */
	int mag; 	/* magnification */
	struct pixrect *spr;
	/* source pixrect */
	int sx,sy;	/* location in source to copy from */
{
pw_rop(dpw, dx, dy, w, h, mag, spr, sx, sy);
}


/* 
 * a service routine that gets called when the frame is destroyed
 * by someone selecting 'quit'.  this is basically right out of the 
 * manual.
 */
static Notify_value
notice_destroy_event(frame, status)
	Frame *frame;
	Destroy_status status;
{
	if (status != DESTROY_CHECKING) {
		done = 1;
	}
	return (notify_next_destroy_func(frame,status));
}

/*
 * The routines below I have found enormously handy when dispatching
 * things via the notifier.  Use them in good health, or bad.  I do.
 *		-mark weiser
 */

/*
 * Call procedure f in a little while.
 */
do_with_delay(f, secs, usecs)
void (*f)();
int secs, usecs;
{
	Notify_value do_the_call();
	struct itimerval timer;

	/* Sigh, so much work just to wait a bit before starting up. */
	timer.it_interval.tv_usec = 0;
	timer.it_interval.tv_sec = 0;
	timer.it_value.tv_usec = usecs;
	timer.it_value.tv_sec = secs;
	notify_set_itimer_func(f, do_the_call,
		ITIMER_REAL, &timer, NULL);
}

/*
 * Wrapper to make sure procedures from do_with_delay return good values
 * to the notifier.
 */
Notify_value
do_the_call(f, which)
void (*f)();
{
	(*f)();
	return NOTIFY_DONE;
}

Notify_value
update_value_proc(item, value)
Panel_item item;
{
	int *ptr;
	ptr = (int *)panel_get(item, PANEL_CLIENT_DATA);
	*ptr = value;
	return NOTIFY_DONE;
}

void
lock_proc(item, event)
Panel_item;
Event *event;
{
	extern void do_the_lock();
	popup_msg(frame, event, 
		"After buttoning 'Done' in this window,\n\
you will have ten (10) seconds to put the\n\
cursor someplace.  At the end of 10 seconds,\n\
glass will be locked into looking at that position.");
	locked = 0;
}

void
unlock_proc()
{
	locked = 0;
}

void
do_the_lock()
{
	static void popup_textsw_done();
	locked_x = win_get_vuid_value(rootfd, LOC_X_ABSOLUTE);
	locked_y = win_get_vuid_value(rootfd, LOC_Y_ABSOLUTE);
	locked = 1;
}

/*
 * The stuff below is some standard hacks I have started using,
 * especially in the 'sdi' game.  I have inserted them here for convenience.
 *		-mark weiser
 */

static Frame popup_frame = NULL;
static Textsw popup_text;
static Panel popup_panel;
static Panel_item popup_msg_item;
static void popup_yes_proc(), popup_no_proc(), popup_textsw_done();

/*
 * Fake an event, so anyone can popup a message. 
 */
easy_pop(msg)
char *msg;
{
	Event event;
	event_x(&event) = (int)window_get(frame, WIN_X); 
	event_y(&event) = (int)window_get(frame, WIN_Y);
	popup_msg(frame, &event, msg);
}

/*
 * Pop up a message inside a textsw.  Since textsw's don't really
 * popup (in SunOS 3.2), just display it and put up a 'done' button
 * to kill it when the user is done.
 * Frame should be the frame in which Event occured.  Msg can
 * contain imbedded newlines.
 */
popup_msg(frame, event, msg)
Frame *frame;
Event *event;
char *msg;
{
	int lines = count_lines(msg);
	if (popup_frame != NULL) {
		/* Can only do one of these at a time. */
		return;
	}
	init_popup_msg(frame, msg, lines);
	textsw_insert(popup_text, msg, strlen(msg));
	window_set(popup_frame, WIN_X, event_x(event),
		WIN_Y, event_y(event),
		WIN_SHOW, TRUE,
		0);
}

/*
 * A helper proc to do all the work of window creation for message popups
 */
init_popup_msg(baseframe, msg, lines)
Frame baseframe;
char *msg;
{
	popup_frame = window_create(baseframe, FRAME,
		WIN_ERROR_MSG, "Can't create window.",
		0);
	popup_panel = window_create(popup_frame, PANEL,
		/* WIN_BELOW, popup_text, */
		WIN_X, ATTR_COL(0),		/* bug workaround, should not be necessary */
		0);
	panel_create_item(popup_panel, PANEL_BUTTON,
		PANEL_LABEL_IMAGE, panel_button_image(popup_panel, "Done", 4, NULL),
		PANEL_NOTIFY_PROC, popup_textsw_done,
		0);
	window_fit(popup_panel);
	popup_text = window_create(popup_frame, TEXTSW,
		WIN_ERROR_MSG, "Can't create window.",
		WIN_ROWS, min(30, lines),
		WIN_COLUMNS, max_line(msg)+3,
		TEXTSW_IGNORE_LIMIT, TEXTSW_INFINITY,
		TEXTSW_CONTENTS, msg,
		TEXTSW_BROWSING, TRUE,
		TEXTSW_DISABLE_LOAD, TRUE,
		TEXTSW_DISABLE_CD, TRUE,
		0);
	window_fit(popup_frame);
}

void
timeout_proc()
{
	int val = (int)panel_get_value(time_left_item);
	if (val > 0) {
		panel_set(time_left_item, PANEL_VALUE, val-1, 0);
		do_with_delay(timeout_proc, 1, 0);
	} else {
		panel_set(time_left_item, PANEL_SHOW_ITEM, FALSE, 0);
		panel_paint(panel, PANEL_CLEAR);
		do_the_lock();
	}
}

/* A helper for killing message popups. */
static void
popup_textsw_done()
{
	window_set(popup_frame, FRAME_NO_CONFIRM, TRUE, 0);
	window_destroy(popup_frame);
	popup_frame = NULL;
	panel_set(time_left_item, PANEL_SHOW_ITEM, TRUE, PANEL_VALUE, 10, 0);
	do_with_delay(timeout_proc, 1, 0);
}

/*
 * Find the size of the longest line in a string of lines separated by newlines
 */
max_line(s)
char *s;
{
	int max = 0, count = 0;
	while (*s) {
		if (*s++ == '\n') {
			if (count > max)
				max = count;
			count = 0;
			continue;
		}
		count += 1;
	}
	if (count > max)
		max = count;
	return max;
}

/*
 * Count the number of lines in a string of lines separated by newlines.
 */
count_lines(s)
char *s;
{
	int count = 0;
	while (*s) {
		if (*s++ == '\n')
			count += 1;
	}
	return count+1;
}

/* need functions, not macros, because of non-idempotent funcall arguments */
max(x,y)
{
	return x<y ? y : x;
}

min(x,y)
{
	return x<y ? x : y;
}
