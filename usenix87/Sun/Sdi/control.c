/*****************************  control.c  ******************************/
#include <sunwindow/notify.h>
#include "sdi.h"

/*
 * Copyright 1987 by Mark Weiser.
 * Permission to reproduce and use in any manner whatsoever on Suns is granted
 * so long as this copyright and other identifying marks of authorship
 * in the code and the game remain intact and visible.  Use of this code
 * in other products is reserved to me--I'm working on Mac and IBM versions.
 */

/*
 * The main code for setting up the control panel lives here.
 * Other panel-related code is in control_procs.c, helpers.c, and main.c.
 */

static short i_pic_array[] = {
#include "interceptor_picture.h"
};
mpr_static(interceptor_pic, 16, 16, 1, i_pic_array);

static short l_pic_array[] = {
#include "laser_picture.h"
};
mpr_static(laser_pic, 16, 16, 1, l_pic_array);

static short r_pic_array[] = {
#include "rock_picture.h"
};
mpr_static(rock_pic, 16, 16, 1, r_pic_array);

static short f_pic_array[] = {
#include "foe_picture.h"
};
mpr_static(foe_pic, 16, 16, 1, f_pic_array);

static short c1_pic_array[] = {
#include "cursor.h"
};
mpr_static(normal_pic, 16, 16, 1, c1_pic_array);

static short c2_pic_array[] = {
#include "dyna_picture.h"
};
mpr_static(dyna_pic, 16, 16, 1, c2_pic_array);

static short c3_pic_array[] = {
#include "cross_picture.h"
};
mpr_static(cross_pic, 16, 16, 1, c3_pic_array);

static short c4_pic_array[] = {
#include "silly_picture.h"
};
mpr_static(silly_pic, 16, 16, 1, c4_pic_array);

extern void next_round_proc(), quit_proc(), master_proc(), no_events(),
	suspend_proc(), resume_proc(), open_proc(), scores_proc(), end_proc(),
	help_proc(), about_proc(), higher_proc(), cycle_time_proc(),
	save_proc(), restore_proc(), text_options_proc(), instructions_proc(),
	cursor_notify_proc(), icon_option_proc(), misc_options_proc(),
	ballistic_time_proc(), new_game_proc(), non_stop_notify_proc(),
	version_proc();
extern Panel_setting save_file_notify_proc(), name_notify_proc();

Panel_item cycle_time_item, timeout_item;

/*
 * Build and place all the items in the control panel.
 */
init_control(panel)
Panel panel;
{
	extern Panel_item rock_item;
	extern int gamemaster, continuous, time_to_play, starting_skill;
	extern struct pixfont *bigfont, *buttonfont; /* use 'struct pixfont', not 'Pixfont',
											 for 3.0 compatibility */
	if ((bigfont = (struct pixfont *)pf_open(
		"/usr/lib/fonts/fixedwidthfonts/screen.b.14")) == NULL) {
			bigfont = (struct pixfont *)pf_default();
	}
	if ((buttonfont = (struct pixfont *)pf_open(
		"/usr/lib/fonts/fixedwidthfonts/screen.r.12")) == NULL) {
			buttonfont = (struct pixfont *)pf_default();
	}
	next_round_item = panel_create_item(panel, PANEL_BUTTON,
		PANEL_LABEL_IMAGE, panel_button_image(panel, "Next Round", 12, buttonfont),
		PANEL_NOTIFY_PROC, next_round_proc,
		PANEL_SHOW_ITEM, FALSE,
		0);
	(void) panel_create_item(panel, PANEL_BUTTON,
		PANEL_LABEL_IMAGE, panel_button_image(panel, "Quit", 6, buttonfont),
		PANEL_NOTIFY_PROC, quit_proc,
		0);
	(void) panel_create_item(panel, PANEL_BUTTON,
		PANEL_LABEL_IMAGE, panel_button_image(panel, "New Game", 10, buttonfont),
		PANEL_NOTIFY_PROC, new_game_proc,
		0);
	suspend_item = panel_create_item(panel, PANEL_BUTTON,
		PANEL_LABEL_IMAGE, panel_button_image(panel, "Suspend", 9, buttonfont),
		PANEL_NOTIFY_PROC, suspend_proc, 
		0);
	resume_item = panel_create_item(panel, PANEL_BUTTON,
		PANEL_LABEL_IMAGE, panel_button_image(panel, "Resume", 9, buttonfont),
		PANEL_NOTIFY_PROC, resume_proc,
		PANEL_SHOW_ITEM, FALSE,
		PANEL_ITEM_X, panel_get(suspend_item, PANEL_ITEM_X), 
		PANEL_ITEM_Y, panel_get(suspend_item ,PANEL_ITEM_Y), 
		0);
	level_item = panel_create_item(panel, PANEL_TEXT,
		PANEL_LABEL_STRING, "Level:", 
		PANEL_VALUE, "0",
		(char *)PANEL_EVENT_PROC, (char *)no_events,
		PANEL_VALUE_DISPLAY_LENGTH, 2,
		0);
	score_item = panel_create_item(panel, PANEL_TEXT,
		PANEL_LABEL_STRING, "Score:", 
		PANEL_VALUE, "0",
		PANEL_VALUE_DISPLAY_LENGTH, 8,
		(char *)PANEL_EVENT_PROC, (char *)no_events,
		PANEL_VALUE_FONT, bigfont,
		0);
	interceptor_item = panel_create_item(panel, PANEL_SLIDER,
		ATTR_LIST, panel_common,
#ifdef STRINGLABELS
		PANEL_LABEL_STRING, "Interceptors:     ",
#else
		PANEL_LABEL_IMAGE, &interceptor_pic,
#endif
		0);
	rock_item = panel_create_item(panel, PANEL_SLIDER,
		ATTR_LIST, panel_common,
#ifdef STRINGLABELS
		PANEL_LABEL_STRING, "Rocks:            ",
#else
		PANEL_LABEL_IMAGE, &rock_pic,
#endif
		0);
	laser_item = panel_create_item(panel, PANEL_SLIDER,
		ATTR_LIST, panel_common,
#ifdef STRINGLABELS
		PANEL_LABEL_STRING, "X-ray lasers:     ",
#else
		PANEL_LABEL_IMAGE, &laser_pic,
#endif
		0);
#define FOE_WIDTH 270
	foe_item = panel_create_item(panel, PANEL_SLIDER,
		ATTR_LIST, panel_common,
#ifdef STRINGLABELS
		PANEL_LABEL_STRING, "Foe in flight:", 
#else
		PANEL_LABEL_IMAGE, &foe_pic,
#endif
		PANEL_SLIDER_WIDTH, FOE_WIDTH,
		0);
	total_foe_item = panel_create_item(panel, PANEL_TEXT,
		PANEL_LABEL_STRING, "Total foe killed:", 
		PANEL_VALUE, "0",
		PANEL_VALUE_DISPLAY_LENGTH, 24,
		(char *)PANEL_EVENT_PROC, (char *)no_events,
		0);
	if (time_to_play) {
		timeout_item = panel_create_item(panel, PANEL_SLIDER,
			ATTR_LIST, panel_common,
			PANEL_LABEL_STRING, "Seconds remaining:", 
			PANEL_MIN_VALUE, 0, PANEL_MAX_VALUE, time_to_play,
			PANEL_VALUE, time_to_play,
			PANEL_SLIDER_WIDTH, 500,
			0);
		continuous = 1;
		start_timeout(timeout_item);
	}
	(void) panel_create_item(panel, PANEL_BUTTON,
		PANEL_LABEL_IMAGE, panel_button_image(panel, "Options", 9, buttonfont),
		PANEL_NOTIFY_PROC, misc_options_proc,
		0);
	(void) panel_create_item(panel, PANEL_BUTTON,
		PANEL_LABEL_IMAGE, panel_button_image(panel, "Things to Read", 16, buttonfont),
		PANEL_NOTIFY_PROC, text_options_proc,
		0);
	(void) panel_create_item(panel, PANEL_BUTTON,
		PANEL_LABEL_IMAGE, panel_button_image(panel, "Icon Options", 14, buttonfont),
		PANEL_NOTIFY_PROC, icon_option_proc,
		0);
	skill_item = panel_create_item(panel, PANEL_CYCLE,
		PANEL_LABEL_STRING, " Skill:",
		PANEL_CHOICE_STRINGS, "Novice", "Occasional", "Expert", 0,
		PANEL_NOTIFY_PROC, new_game_proc,
		PANEL_VALUE, starting_skill,
		0);
	(void) panel_create_item(panel, PANEL_BUTTON,
		PANEL_LABEL_IMAGE, panel_button_image(panel, "Instructions", 14, buttonfont),
		PANEL_NOTIFY_PROC, instructions_proc,
		0);
	if (gamemaster) {
		(void) panel_create_item(panel, PANEL_TOGGLE,
			PANEL_CHOICE_STRINGS, "Gamemaster", 0,
			PANEL_NOTIFY_PROC, master_proc,
			PANEL_SHOW_ITEM, TRUE,
			0);
	}
}

void
misc_options_proc()
{
	extern int cursor_type, ballistic_delay;
	extern struct pixfont *buttonfont; /* use 'struct pixfont' for 3.0 compatiblity */
	char *s;
	Panel panel, make_popup_panel();
	void options_done();
	if ((panel = make_popup_panel("      SDI Options", options_done)) == NULL) {
		return;
	}

	(void) panel_create_item(panel, PANEL_CYCLE,
		PANEL_LABEL_STRING, "Cursor:",
		PANEL_CHOICE_IMAGES, &normal_pic, &dyna_pic, &cross_pic, &silly_pic, 0,
		PANEL_MENU_CHOICE_STRINGS, "normal", "dynamic", "crosshair", "silly", 0,
		PANEL_NOTIFY_PROC, cursor_notify_proc,
		PANEL_VALUE, cursor_type,
		0);
	(void) panel_create_item(panel, PANEL_TOGGLE,
		PANEL_CHOICE_STRINGS, "Non-stop", 0,
		PANEL_NOTIFY_PROC, non_stop_notify_proc,
		PANEL_VALUE, continuous,
		0);
	(void) panel_create_item(panel, PANEL_BUTTON,
		PANEL_LABEL_IMAGE, panel_button_image(panel, "Melt", 4, buttonfont),
		PANEL_NOTIFY_PROC, end_proc,
		0);
	if (user_name[0] == '\0') {
		s = (char *)get_name();
		if (s[0] != '\0') strcpy(user_name, s);
	}
	user_name_item =  panel_create_item(panel, PANEL_TEXT,
		PANEL_LABEL_STRING, "Name: ",
		PANEL_VALUE_DISPLAY_LENGTH, 16,
		PANEL_VALUE_STORED_LENGTH, 63,
		PANEL_VALUE, user_name,
		0);
	panel_fit_width(panel);
	(void) panel_create_item(panel, PANEL_BUTTON,
		PANEL_LABEL_IMAGE, panel_button_image(panel, "Save", 6, buttonfont),
		PANEL_NOTIFY_PROC, save_proc,
		0);
	(void) panel_create_item(panel, PANEL_BUTTON,
		PANEL_LABEL_IMAGE, panel_button_image(panel, "Restore", 7, buttonfont),
		PANEL_NOTIFY_PROC, restore_proc,
		0);
	save_file_item = panel_create_item(panel, PANEL_TEXT,
		PANEL_LABEL_STRING, "Save file:",
		PANEL_VALUE_DISPLAY_LENGTH, 16,
		PANEL_VALUE_STORED_LENGTH, 63,
		PANEL_VALUE, save_file_name,
		0);
	(void) panel_create_item(panel, PANEL_SLIDER,
		PANEL_LABEL_STRING, "Cycle time (ms): ", 
		PANEL_NOTIFY_LEVEL, PANEL_ALL,
		PANEL_SLIDER_WIDTH, 100,
		PANEL_MIN_VALUE, 50,
		PANEL_MAX_VALUE, 250,
		PANEL_SHOW_RANGE, FALSE,
		PANEL_SHOW_VALUE, TRUE,
		PANEL_VALUE, blast_delay/1000,
		PANEL_SHOW_ITEM, TRUE,
		PANEL_NOTIFY_PROC, cycle_time_proc,
		0);
	(void) panel_create_item(panel, PANEL_SLIDER,
		PANEL_LABEL_STRING, "Ballistic time: ", 
		PANEL_SLIDER_WIDTH, 50,
		PANEL_MIN_VALUE, 0,
		PANEL_MAX_VALUE, 10,
		PANEL_SHOW_RANGE, FALSE,
		PANEL_SHOW_VALUE, TRUE,
		PANEL_VALUE, ballistic_delay,
		PANEL_SHOW_ITEM, TRUE,
		PANEL_NOTIFY_PROC, ballistic_time_proc,
		0);
	user_name[0] = '\0';
	save_file_name[0] = '\0';
	display_popup_panel(panel);
}

void
options_done()
{
	strcpy(user_name, panel_get_value(user_name_item));
	strcpy(save_file_name, panel_get_value(save_file_item));
}
