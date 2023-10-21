/************************************  input.c  *************************/
#include <sunwindow/notify.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <errno.h>
#include "sdi.h"

/*
 * Copyright 1987 by Mark Weiser.
 * Permission to reproduce and use in any manner whatsoever on Suns is granted
 * so long as this copyright and other identifying marks of authorship
 * in the code and the game remain intact and visible.  Use of this code
 * in other products is reserved to me--I'm working on Mac and IBM versions.
 */

/*
 * interrupt handling is here, including mousing, resizing, etc.
 */

extern Panel launchpanel;

Notify_value update_icon();

void
canvas_resize_proc(canvas, width, height)
Canvas canvas;
int width, height;
{
	void new_game_proc();
	void draw_background();
	struct pixwin *pw, *newpw;;

	if (canvas == citycanvas || canvas == launchcanvas) {
	    max_x = width-(2*FIELD_MARGIN)-4;
	    max_y = height-(2*FIELD_MARGIN)-4;
	    newpw = pw_region(canvas_pixwin(canvas), FIELD_MARGIN, FIELD_MARGIN,
		    max_x, max_y);
	}
	/*
	 * Can't do the pw_closes here, because lurking missiles may still
	 * be depending on them.  So never do them--how many resizes are
	 * there per game, anyway?
	 */
	if (canvas == citycanvas) {
		/* pw_close(citypw); */
		citypw = newpw;
		num_cities = 0;
		init_cities();
	} else if (canvas == launchcanvas) {
		/* pw_close(launchpw); */
		launchpw = newpw;
	}
}
int running_icon_pictures = 0;
static int old_frame_width = -1, old_frame_height = -1;

/*
 * This routine gets notifications even before SunView does.  Almost the first
 * thing it does is all 'notify_next_event_func', which lets SunView do
 * its work.  Resizes, opens, and closes are handled here.
 */
Notify_value
synch_event_proc(frame, event, arg, type)
Frame frame;
Event *event;
Notify_arg arg;
Notify_event_type type;
{
	int new_game;
	int closed;
	static int oldclosed = 0;
	Notify_value value;

	/* send on the notification */
	value = notify_next_event_func(frame, event, arg, type);

	/* start post-processing */
	closed = (int) window_get(frame, FRAME_CLOSED);
	if (frame == controlframe) {
		if ( closed && !oldclosed) {
			/* not many events come in while closed,
			 * so if we are closed, we are probably
			 * just now closing.  Close everyone.
			 */
			window_set(cityframe, WIN_SHOW, FALSE,
				0);
			window_set(launchframe, WIN_SHOW, FALSE,
				0);
			if (running)
				suspend_proc();
			if (!running_icon_pictures) {
				/* start the icon stuff */
				running_icon_pictures = 1;
				do_with_delay(update_icon, 0, 1);
			}
		} else if (!closed) {
			/* opening control frame, and so show everybody */
			if (! window_get(cityframe, WIN_SHOW))
				window_set(cityframe, WIN_SHOW, TRUE, 0);
			if (! window_get(launchframe, WIN_SHOW))
				window_set(launchframe, WIN_SHOW, TRUE, 0);
			if (running_icon_pictures) {
				/* stop the icon stuff */
				running_icon_pictures = 0;
			}
		}
		oldclosed = closed;
	} 
	/* Check for a resize request.
	   If either playing field is resized they both are.
	 */
	if (event_id(event) == WIN_RESIZE && 
		(frame == cityframe || frame == launchframe)) {
		int w = (int)window_get(frame, WIN_WIDTH);
		int h = (int)window_get(frame, WIN_HEIGHT);
		if ((old_frame_height == h && old_frame_width == w))
			goto done;
		if ((old_frame_height == -1 && old_frame_width == -1)
			|| (!running)
			|| popup_warning(frame, event, "Ok to restart game?")) {
				new_game = 1;
				old_frame_height = h;
				old_frame_width = w;
		} else {
			new_game = 0;
			h = old_frame_height;
			w = old_frame_width;
		}
		/* can't get too small, or else things start to break. */
		h = max(MINWIN, h);
		w = max(MINWIN, w);
		window_set(cityframe, WIN_WIDTH, w, WIN_HEIGHT, h, 0);
		window_set(launchframe, WIN_WIDTH, w, WIN_HEIGHT, h, 0);
		if (new_game)	{
			/*
			 * If resizing, it must be time to start a new game.
			 */
			do_with_delay(new_game_proc, 0, 100000);
		}
	}
done:
	return value;
}

/*
 * Handle mousing.
 */
#define PINX(x) (max(0,min(max_x,x)))
#define PINY(y) (max(0,min(max_y,y)))

static int rock_x = -1, rock_y;
Event rock_down_event;

void
main_event_proc(window, event, arg)
Window window;
Event *event;
caddr_t arg;
{
	extern Panel_item rock_item;
	int times_around, offset, i;
	int id = event_id(event);
	int inc, left;
	Pixwin *pw;

	/*
	 * The following totally silly call is a kludge to work-around
	 * a Sun bug which, if we *don't* refuse the input focus, loses the 
	 * middle-button down transition when we first are handed the input focus.
	 */
	if (event_id(event) == KBD_REQUEST) {
		window_refuse_kbd_focus(window);
		return;
	}

	if (! event_is_button(event)) return;
	if (suspended) return;

	if (window == citycanvas) {
		pw = citypw;
	} else {
		pw = launchpw;
	}

	if (event_is_down(event)) {
		if (!running) {
			/* if not running, then try to start running. */
			start_next_round();
		} else {
			switch(id) {
			case MS_LEFT: {
			    left = (int)panel_get_value(interceptor_item);
			    if (left > 0) {
					if (event_meta_is_down(event)) {
						times_around = 2;
						offset = event_shift_is_down(event) ? -31 : -20;
					} else {
						times_around = 1;
						offset = 0;
					}
					for (i = 0; i < times_around; i += 1) {
						if (event_shift_is_down(event)) {
						    start_blast(PINX(event_x(event)+offset), event_y(event), 0, 0, pw, bigblastcircles);
						} else {
						    start_blast(PINX(event_x(event)+offset), event_y(event), 0, 0, pw, littleblastcircles);
						}
						offset = ABS(offset);
					}
				    panel_set_value(interceptor_item, left-times_around);
					update_cursor();
				} else {
			    	need_a_bell = pw;
			    }
				break;
			} 
			case MS_RIGHT: {
				left = (int)panel_get_value(laser_item);
				if (left > 0) {
					if (event_meta_is_down(event)) {
						times_around = 2;
						offset = event_shift_is_down(event) ? -100 : -64;
					} else {
						times_around = 1;
						offset = 0;
					}
					for (i = 0; i < times_around; i += 1) {
						if (event_shift_is_down(event)) {
							start_laser(PINX(event_x(event)+offset), event_y(event), 
								pw, 3, 256);
						} else {
							start_laser(PINX(event_x(event)+offset), event_y(event), 
								pw, 6, 128);
						}
					offset = ABS(offset);
					}
					panel_set_value(laser_item, left-times_around);
					update_cursor();
				} else {
					need_a_bell = pw;
				}
				break;
			}
			case MS_MIDDLE: {
				left = (int)panel_get_value(rock_item);
				if (left > 0) {
					rock_x = event_x(event);
					rock_y = event_y(event);
					rock_down_event = *event;
				} else {
					rock_x = -1;
					need_a_bell = pw;
				}
			}} /* end of switch */
		}
	} else if (/* implicit: event_is_up(event) && */
			running && id == MS_MIDDLE && rock_x != -1) {
		/* throw some rocks */
		if (event_meta_is_down(&rock_down_event)) {
			times_around = 2;
			offset = -20;
		} else {
			times_around = 1;
			offset = 0;
		}
		for (i = 0; i < times_around; i += 1) {
			if (event_shift_is_down(&rock_down_event)) {
				start_rocks(pw, rock_x, PINY(rock_y+offset),
					event_x(event), PINY(event_y(event)+offset), 
					3, bigrockcircles);
			} else {
				start_rocks(pw, rock_x, PINY(rock_y+offset),
					event_x(event)+offset, PINY(event_y(event)+offset),
					5, littlerockcircles);
			}
			offset = ABS(offset);
		}
		rock_x = -1;
		panel_set_value(rock_item, panel_get_value(rock_item)-times_around);
		update_cursor();
	}
}
