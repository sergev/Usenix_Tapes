/*********************  helpers.c ************************************/
#include <sunwindow/notify.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <errno.h>
#include "sdi.h"
#include <suntool/textsw.h>

/*
 * Copyright 1987 by Mark Weiser.
 * Permission to reproduce and use in any manner whatsoever on Suns is granted
 * so long as this copyright and other identifying marks of authorship
 * in the code and the game remain intact and visible.  Use of this code
 * in other products is reserved to me--I'm working on Mac and IBM versions.
 */

/*
 * This file contains routines providing help throughout sdi, although
 * especially to the control panel.
 */


static short background_array[] = {
#include "/usr/include/images/square_17.pr"
};
mpr_static(background_pr, 16, 16, 1, background_array);

void generic_slider_notify();

/* Used as an event_proc in panels to make an item read only */
void
no_events(item, event)
Panel_item item;
Event *event;
{
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

/*
 * Used to initiate the panel timer countdown associated with the
 * '-t' option.
 */
start_timeout(item)
Panel_item item;
{
	Notify_value timeout_proc();
	struct itimerval timer;

	timer.it_interval.tv_usec = 0;
	timer.it_interval.tv_sec = 1;
	timer.it_value.tv_usec = 0;
	timer.it_value.tv_sec = 1;
	notify_set_itimer_func(item, timeout_proc, ITIMER_REAL, &timer, NULL);
}

/*
 * The proc that takes the timer notifications for the panel timer countdown
 * associated with the '-t' option.  Started by proc 'start_timeout'.
 */
Notify_value
timeout_proc(item, which)
Panel_item item;
int which;
{
	extern int continuous;
	struct itimerval timer;
	int val = (int)panel_get_value(item);
	val -= 1;
	if (running) panel_set_value(item, val);
	if (val <= 0) {
		continuous = 0;
		end_proc();
		continuous = 1;
		panel_set_value(item, panel_get(item, PANEL_MAX_VALUE));
		return;
	}
}

/*
 * The remaining code in helper.c is concerned with different kinds
 * of popups or pseudo-popups (for textsw's).  First a few local variables
 */
static Frame popup_frame = NULL;
static Textsw popup_text;
static Panel popup_panel;
static Panel_item popup_msg_item;
static void popup_yes_proc(), popup_no_proc(), popup_textsw_done();

/* 
 * Pop a warning on the screen.  Return 1 if yes, 0 if no 
 * Event is used to determine the x,y position of the popup, and Frame
 * is the frame in which the event was received.
 * Msg is a single-line string containing the message.  It should be
 * in the form of a yes/no question, because it will be followed
 * in the window by 'yes' and 'no' selection boxes.
 */
popup_warning(frame, event, msg)
Frame *frame;
Event *event;
char *msg;
{
	int value;
	init_popup_warn(frame, msg);
	window_set(popup_frame, WIN_X, event_x(event),
		WIN_Y, event_y(event),
		0);
	value = (int)window_loop(popup_frame);
	window_set(popup_frame, FRAME_NO_CONFIRM, TRUE, 0);
	window_destroy(popup_frame);
	popup_frame = NULL;
	return value;
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
	suspend_proc();
	init_popup_msg(frame, msg, lines);
	textsw_insert(popup_text, msg, strlen(msg));
	window_set(popup_frame, WIN_X, event_x(event),
		WIN_Y, event_y(event),
		WIN_SHOW, TRUE,
		0);
}

/*
 * A helper routine to do all the real work of setting up windows for warnings
 */ 
init_popup_warn(baseframe, msg)
Frame baseframe;
char *msg;
{
	popup_frame = window_create(baseframe, FRAME,
		WIN_ERROR_MSG, "Can't create window.",
		FRAME_DONE_PROC, popup_no_proc,
		WIN_GRAB_ALL_INPUT, TRUE,
		0);
	popup_panel = window_create(popup_frame, PANEL,
		WIN_ERROR_MSG, "Can't create window.",
		PANEL_LAYOUT, PANEL_VERTICAL,
		0);
	popup_msg_item = panel_create_item(popup_panel, PANEL_MESSAGE,
		0);
	panel_create_item(popup_panel, PANEL_BUTTON,
		PANEL_LABEL_IMAGE, panel_button_image(popup_panel, "Yes", 3, NULL),
		PANEL_NOTIFY_PROC, popup_yes_proc,
		PANEL_ITEM_Y, ATTR_ROW(1),
		0);
	panel_create_item(popup_panel, PANEL_BUTTON,
		PANEL_LABEL_IMAGE, panel_button_image(popup_panel, "No", 2, NULL),
		PANEL_NOTIFY_PROC, popup_no_proc,
		PANEL_ITEM_Y, ATTR_ROW(1),
		0);
	panel_set(popup_msg_item, PANEL_LABEL_STRING, msg, 0);
	window_fit(popup_panel);
	window_fit(popup_frame);
}

/*
 * popup_yes_proc and popup_no_proc are helpers used for warning popups.
 */
static void
popup_yes_proc(item, event)
Panel_item item;
Event *event;
{
	window_return(1);
}

static void
popup_no_proc(item, event)
Panel_item item;
Event *event;
{
	window_return(0);
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
		FRAME_DONE_PROC, popup_textsw_done,
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

/* A helper for killing message popups. */
static void
popup_textsw_done()
{
	window_set(popup_frame, FRAME_NO_CONFIRM, TRUE, 0);
	window_destroy(popup_frame);
	popup_frame = NULL;
	resume_proc();
}

Menu
null_menu_gen(m, operation)
Menu m;
Menu_generate operation;
{
	/*
	 * If I destroy the menu, then under very rapid buttoning on the
	 * border someone gets confused and passes me a bad menu descripter.
	 * menu_destroy(m);
	 */
	m = menu_create(0);
	return m;
}

/*
 * Fake an event, so anyone can popup a message. 
 */
easy_pop(msg)
char *msg;
{
	Event event;
	event_x(&event) = (int)window_get(controlframe, WIN_X); 
	event_y(&event) = (int)window_get(controlframe, WIN_Y);
	popup_msg(controlframe, &event, msg);
}

easy_warn(msg)
char *msg;
{
	Event event;
	event_x(&event) = (int)window_get(controlframe, WIN_X); 
	event_y(&event) = (int)window_get(controlframe, WIN_Y);
	return popup_warning(controlframe, &event, msg);
}

static Frame popup_panel_frame = 0;
static int (*popup_panel_user_done_func)();

/*
 * Helper for make_popup_panel, below.
 */
static
popup_panel_done()
{
	if (popup_panel_user_done_func)
		(*popup_panel_user_done_func)();
	window_set(popup_panel_frame, FRAME_NO_CONFIRM, TRUE, 0);
	window_destroy(popup_panel_frame);
	popup_panel_frame = 0;
	resume_proc();
}

Panel
make_popup_panel(panel_name, user_done_func)
char *panel_name;
int (*user_done_func)();
{
	extern struct pixfont *buttonfont; /* use 'struct pixfont' for 3.0 compatiblity */
	Panel panel;
	if (popup_panel_frame) {
		/* don't permit nested calls. */
		return NULL;
	}
	suspend_proc();
	popup_panel_user_done_func = user_done_func;
	popup_panel_frame = window_create(controlframe, FRAME,
		FRAME_SHOW_LABEL, TRUE,
		FRAME_DONE_PROC, popup_panel_done,
		FRAME_LABEL, panel_name,
		WIN_ERROR_MSG, "Can't create popup panel frame.",
		0);
	panel = window_create(popup_panel_frame, PANEL,
		WIN_ERROR_MSG, "Can't create popup panel panel.",
		PANEL_ITEM_X_GAP, 30,
		PANEL_ITEM_Y_GAP, 15,
		0);
	(void) panel_create_item(panel, PANEL_BUTTON,
		PANEL_LABEL_IMAGE, panel_button_image(panel, "Done", 6, buttonfont),
		PANEL_NOTIFY_PROC, popup_panel_done,
		0);
	return panel;
}

display_popup_panel(panel)
Panel panel;
{
	if (panel == NULL || popup_panel_frame == NULL)
		return;
	window_fit(panel);
	window_fit(popup_panel_frame);
	window_set(popup_panel_frame, WIN_SHOW, TRUE, 0);
}

start_next_round()
{
	extern void start_running_proc(), init_blast();
	/* 
	 * Notice that things will break if blast_delay+1000
	 * is greater than 999,999.
	 */
	panel_set(skill_item, PANEL_EVENT_PROC, (char *)no_events, 0);
	clear_fields();
	place_cities(citypw);
	init_incoming();
	update_cursor();
	do_with_delay(start_running_proc, 0, blast_delay+1000);
	do_with_delay(init_blast, 1, blast_delay);
}

/*
 * helper to delay the startup.
 */
void
start_running_proc()
{
	running = 1;
}

put_text(y, s)
int y;
char *s;
{
	struct pr_size size;
	size = pf_textwidth(strlen(s), font, s);
	pw_text(citypw, max_x / 2 - size.x / 2, max_y / 2 - 35 - (size.y/2)+ y, PIX_SRC, font, s);
	pw_text(launchpw, max_x / 2 - size.x / 2, max_y / 2 - 35 - (size.y/2)+ y, PIX_SRC, font, s);
}

draw_canvas_background(canvas)
Canvas canvas;
{
    pw_replrop(window_get(canvas, CANVAS_PIXWIN), 0, 0, 
	    window_get(canvas, WIN_WIDTH), window_get(canvas, WIN_HEIGHT), 
	    PIX_SRC, &background_pr, 0, 0);
}

draw_background()
{
	draw_canvas_background(citycanvas);
	draw_canvas_background(launchcanvas);
}

clear_fields()
{
	pw_writebackground(citypw, 0, 0, max_x, max_y, PIX_SRC);
	pw_writebackground(launchpw, 0, 0, max_x, max_y+max_y
, PIX_SRC);
}
