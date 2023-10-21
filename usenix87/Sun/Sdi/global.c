/**********************************  global.c  ***********************/
#include "sdi.h"

/*
 * Copyright 1987 by Mark Weiser.
 * Permission to reproduce and use in any manner whatsoever on Suns is granted
 * so long as this copyright and other identifying marks of authorship
 * in the code and the game remain intact and visible.  Use of this code
 * in other products is reserved to me--I'm working on Mac and IBM versions.
 */

int		ballistic_delay = 4;
Panel_item	ballistic_item;
struct pixfont	*bigfont, *buttonfont; /* use 'struct pixfont' for 3.0 compatibility */
int			blast_count = 0;
long		blast_delay = 150000;
int			burst_distance = 32;
float		carryover_divisor = 1.0;
int			city_fd;
int			cursor_type = 0;
int			launch_fd;
Canvas		citycanvas;
Frame		cityframe;
Pixwin		*citypw;
int			continuous;
Frame		controlframe;
float		foe_divisor = 2.0;
int			foe_factor = 5;
Panel_item	foe_ground_item;
Panel_item	foe_item;
int			foe_value = 5;
struct pixfont	*font;	/* struct used for 3.0 compatibility */
int			gamemaster = 0;
int			field_margin = 32;
Panel_item	interceptor_item;
Panel_item	laser_item;
int			launch_delay = 4;
Canvas		launchcanvas;
Frame		launchframe;
Pixwin		*launchpw;
Panel_item	level_item;
int			max_x = 440;
int			max_y = 250;
int			max_missile_speed = 25;
int			min_missile_speed = 7;
int			missile_count = 0;
Pixwin		*need_a_bell = NULL;
Panel_item	next_round_item;
int			num_cities = 0;
char 		*panel_common[] = {
	(char *)PANEL_VALUE,		(char *)1,
	(char *)PANEL_MIN_VALUE,	(char *)0,
	(char *)PANEL_MAX_VALUE,	(char *)0,
	(char *)PANEL_VALUE,		(char *)0,
	(char *)PANEL_SHOW_RANGE,	(char *)FALSE,
	(char *)PANEL_SHOW_VALUE,	(char *)TRUE,
	(char *)PANEL_EVENT_PROC,	(char *)no_events,
	(char *)PANEL_SLIDER_WIDTH,	(char *)150,
	(char *)0 };
int			restoring_game = 0;
Panel_item	resume_item;
Panel_item	rock_item;
int			running = 0;
Panel_item	save_file_item;
char 		save_file_name[128] = "sdi_saved_game";
struct scores sc[NUM_SCORES+1], 
			*sc_end = &sc[NUM_SCORES];
char		*scorefile = NULL;
Panel_item	score_item;
Panel_item	skill_item;
int			starting_icon = 1;
int			starting_icon_time = 5;
int			starting_skill = 0;
Panel_item	suspend_item;
int			suspended = 0;
int			time_to_play = 0;
Panel_item	total_foe_item;
int			total_cities_lost;
char 		user_name[128] = "(nobody)";
Panel_item	user_name_item;

struct circ *lasercircles;
struct circ *laserkillcircles;
struct circ *bigblastcircles;
struct circ *littleblastcircles;
struct circ *blastkillcircles;
struct circ *citykillcircles;
struct circ *littlerockcircles;
struct circ *bigrockcircles;

struct pixrect **blankcircles;
