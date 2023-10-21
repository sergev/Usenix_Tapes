/************************************  main.c  *************************/
#include "sdi.h"

/*
 * Copyright 1987 by Mark Weiser.
 * Permission to reproduce and use in any manner whatsoever on Suns is granted
 * so long as this copyright and other identifying marks of authorship
 * in the code and the game remain intact and visible.  Use of this code
 * in other products is reserved to me--I'm working on Mac and IBM versions.
 */

/*
 * A big, long, initialization routine.
 */

static short cursor_data[] = {
#include "cursor.h"
};
mpr_static(cursor_pr, 16, 16, 1, cursor_data);

static short icon_image[] = {
#include "city_icon1.h"
};
DEFINE_ICON_FROM_IMAGE(icon, icon_image);

static short i_pic_array[] = {
#include "incoming_picture.h"
};
mpr_static(incoming_pic, 16, 16, 1, i_pic_array);

static short g_pic_array[] = {
#include "foe_ground_picture.h"
};
mpr_static(foe_ground_pic, 16, 16, 1, g_pic_array);

static void done_proc();
extern void main_event_proc(), canvas_resize_proc();
extern Notify_value canvas_input_proc(), asynch_event_proc(), synch_event_proc();
Notify_value scheduler(), input_notify();

#define FONT_NAME "/usr/lib/fonts/fixedwidthfonts/serif.r.14"

static char tenblanks[] = "          ";

Panel launchpanel;

/*
 * In need of no comment:
 */
main(argc, argv)
char **argv;
{
	extern char *version;
	Menu menu, null_menu_gen();
	char tmp_string[128];
	Panel controlpanel;
	struct timeval tp;
	char *s;

	/* randomize us */
	gettimeofday(&tp, 0);
	srandom(tp.tv_sec);

	/* init score file from environment, if possible */
	if ((s = (char *)getenv("SDI_SCORES")) != NULL) 
		scorefile = s;

	init_circles();

	init_city_bits(NULL);

	fixup_font(&argc, &argv, FONT_NAME);

	open_our_font(FONT_NAME);
	sprintf(tmp_string, "   SDI Control Panel%s%s%s%s%sby mark weiser",
		tenblanks,tenblanks,tenblanks,tenblanks,tenblanks);

	s = (char *)get_name();
	if (s && s[0] != '\0')
		strcpy(user_name, s);

	/* no background pre-write to our icon--otherwise we flicker. */
	icon.ic_flags = 0;

	/* make the control window, which serves as the base for all others. */
	controlframe = window_create(NULL, FRAME,
		FRAME_ARGC_PTR_ARGV, &argc, argv,
		FRAME_LABEL, tmp_string,
		FRAME_ICON, &icon,
		WIN_ERROR_MSG, "Can't create window.",
		WIN_FONT, font,
		WIN_X, 0, WIN_Y, 0, /* start at zero, move later. */
		0);

	process_args(argc, argv);

	init_icons();

	build_playing_fields();

	controlpanel = window_create(controlframe, PANEL, 
		WIN_VERTICAL_SCROLLBAR, scrollbar_create(0), /* but provide a loophole */
		PANEL_LABEL_FONT, font,
		PANEL_VALUE_FONT, font,
		WIN_FONT, font,
		WIN_ERROR_MSG, "Can't create window.",
		/* magic numbers which seem to look good: */
		WIN_WIDTH, 730,
		PANEL_ITEM_X_GAP, 30,
		PANEL_ITEM_Y_GAP, 15,
		0);
	init_control(controlpanel);
	window_fit(controlpanel);
	window_fit(controlframe);

	city_fd = (int)window_get(citycanvas, WIN_FD);
	launch_fd = (int)window_get(launchcanvas, WIN_FD);

	max_x = (int)window_get(citycanvas, CANVAS_WIDTH);
	max_y = (int)window_get(citycanvas, CANVAS_HEIGHT);

	{ /* little block for a bunch of little variables */
	struct screen screen;
	int playwidth = (int)window_get(cityframe, WIN_WIDTH);
	int controlwidth = (int)window_get(controlframe, WIN_WIDTH);
	int controlheight = (int)window_get(controlframe, WIN_HEIGHT);
	int center;
	win_screenget(city_fd, &screen);
	center =  screen.scr_rect.r_width/2;

	/* put the playing frames into position (control is at 0,0) */
	window_set(cityframe, WIN_X, center - playwidth, 0);
	window_set(launchframe, WIN_X, center, 0);

	/* center the control frame */
	window_set(controlframe, WIN_X, center - controlwidth/2, 
		/* magic number which seems to look good: */
		WIN_Y, 200, 0);

	/* put the playing frames below it. */
	window_set(cityframe, WIN_Y, controlheight, 0);
	window_set(launchframe, WIN_Y, controlheight, 0);
	} /* end of little block */

	/*
	 * menus in the playing fields are a distraction, but
	 * they can't be just menu-destroyed, because that leaves a
	 * dangling pointer inside sunview (and a core dump). 
	 * So instead, we cleverly(?) modify the menu so that it will 
	 * generate an empty contents.
	 * (Ychhhh!)
	 */
	menu = window_get(launchframe, WIN_MENU);
	menu_set(menu, MENU_GEN_PROC, null_menu_gen, 0);
	menu = window_get(cityframe, WIN_MENU);
	menu_set(menu, MENU_GEN_PROC, null_menu_gen, 0);

	notify_interpose_event_func(launchframe, synch_event_proc, NOTIFY_SAFE);
	notify_interpose_event_func(cityframe, synch_event_proc, NOTIFY_SAFE);
	notify_interpose_event_func(controlframe, synch_event_proc, NOTIFY_SAFE);

	notify_set_scheduler_func(scheduler);

	notify_set_input_func(citycanvas, input_notify, city_fd);
	notify_set_input_func(launchcanvas, input_notify, launch_fd);

	init_cursor();
	update_cursor();

	window_main_loop(controlframe);
	exit(0);
}

static void
done_proc(frame)
Frame frame;
{
	/* do nothing */
}

build_playing_fields()
{
	Panel citypanel;
	extern Panel_item foe_ground_item;

	cityframe = window_create(controlframe, FRAME,
		FRAME_LABEL, "   SDI Friend Cities",
		FRAME_SHOW_LABEL, TRUE,
		FRAME_DONE_PROC, done_proc,
		WIN_ERROR_MSG, "Can't create window.",
		WIN_FONT, font,
		FRAME_SUBWINDOWS_ADJUSTABLE, FALSE,
		0);

	citypanel = window_create(cityframe, PANEL, 0);
	ballistic_item = panel_create_item(citypanel, PANEL_SLIDER,
		ATTR_LIST, panel_common,
		PANEL_LABEL_STRING, "Incoming missiles: ",
		0);
	window_fit_height(citypanel);

	citycanvas = window_create(cityframe, CANVAS,
		WIN_CONSUME_PICK_EVENTS, WIN_UP_EVENTS,  0,
		WIN_EVENT_PROC, main_event_proc,
		WIN_CURSOR, cursor_create(CURSOR_IMAGE, &cursor_pr,
			CURSOR_XHOT, 8, CURSOR_YHOT, 8,
			CURSOR_OP, PIX_SRC ^ PIX_DST,
			0),
		WIN_WIDTH, max_x+(2*FIELD_MARGIN), WIN_HEIGHT, max_y+(2*FIELD_MARGIN),
		CANVAS_RESIZE_PROC, canvas_resize_proc,
		CANVAS_RETAINED, TRUE,	/* need retained for city computations */
		WIN_ERROR_MSG, "Can't create window.",
		CANVAS_MARGIN, 0,
		0);
	window_fit(cityframe);
	citypw = pw_region(canvas_pixwin(citycanvas), FIELD_MARGIN, FIELD_MARGIN, 
		max_x, max_y);

	launchframe = window_create(controlframe, FRAME,
		FRAME_LABEL, "   SDI Foe Launch",
		FRAME_SHOW_LABEL, TRUE,
		FRAME_DONE_PROC, done_proc,
		WIN_ERROR_MSG, "Can't create window.",
		WIN_FONT, font,
		FRAME_SUBWINDOWS_ADJUSTABLE, FALSE,
		0);

	launchpanel = window_create(launchframe, PANEL, 0);
	foe_ground_item = panel_create_item(launchpanel, PANEL_SLIDER,
		ATTR_LIST, panel_common,
		PANEL_LABEL_STRING, "Missiles on the ground:", 
		0);
	window_fit_height(launchpanel);
	launchcanvas = window_create(launchframe, CANVAS,
		WIN_CONSUME_PICK_EVENTS, WIN_UP_EVENTS,  0,
		WIN_EVENT_PROC, main_event_proc,
		WIN_CURSOR, cursor_create(CURSOR_IMAGE, &cursor_pr,
			CURSOR_XHOT, 8, CURSOR_YHOT, 8,
			CURSOR_OP, PIX_SRC ^ PIX_DST,
			0),
		WIN_WIDTH, max_x+(2*FIELD_MARGIN), WIN_HEIGHT, max_y+(2*FIELD_MARGIN),
		CANVAS_RESIZE_PROC, canvas_resize_proc,
		CANVAS_RETAINED, TRUE,
		WIN_ERROR_MSG, "Can't create window.",
		CANVAS_MARGIN, 0,
		0);

	window_fit(launchframe);

	launchpw = pw_region(canvas_pixwin(launchcanvas), FIELD_MARGIN, FIELD_MARGIN,
		max_x, max_y);

/*
 *	A noble sentiment, but too much trouble to handle resizing correctly.
	window_set(launchcanvas, 
		WIN_HEIGHT, window_get(launchcanvas, WIN_HEIGHT),
		WIN_Y, 0,
 		0);
	window_set(launchpanel, WIN_BELOW, launchcanvas, 
		0);

*/

	draw_background();

}
