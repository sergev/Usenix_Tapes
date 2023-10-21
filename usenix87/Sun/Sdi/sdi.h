/*
 * Copyright 1987 by Mark Weiser.
 * Permission to reproduce and use in any manner whatsoever is granted
 * so long as this copyright and other identifying marks of authorship
 * in the code and the game remain intact and visible.
 */
/* includes needed by definitions in here, and so may as well put here */
#include <suntool/sunview.h>
#include <suntool/canvas.h>
#include <suntool/panel.h>

/* General Definitions */
#undef min
#undef max
#define ABS(x) (((x) < 0) ? -(x) : (x))
#define DOWN 1
#define UP -1
#define TRUE 1
#define FALSE 0
#define FIELD_MARGIN field_margin

#define USER_NAME (user_name[0] == '\0' ? panel_get_value(user_name_item) : user_name)
#define SAVE_FILE_NAME (save_file_name[0] == '\0' ? panel_get_value(save_file_item) : save_file_name)

/* space to leave on either side of row of cities. */
#define MARGIN 30

/* minimum window size */
#define MINWIN 128+(MARGIN*2)+(FIELD_MARGIN*2)

/* Definitions affecting blasts */
extern struct circ {
	int num_circles;
	struct pixrect **circles;
	struct pixrect **masks
}	*lasercircles,
	*laserkillcircles,
	*bigblastcircles,
	*littleblastcircles,
	*blastkillcircles,
	*citykillcircles,
	*littlerockcircles,
	*bigrockcircles,
	*init_circ();
extern struct pixrect **blankcircles;
extern int blast_count;
struct blast {
	Pixwin *pw;
	short x, y, circ, width;
	short orig_y;		/* used only for hashing */
	short x_inc, y_inc;
	short num_circles;
	struct pixrect **circles;
	struct pixrect **masks;
	struct blast *next;
	Rect r;
	};
#define MAX_LINES	1024
#define MAX_NUM_CIRCLES 8
#define MAX_CIRCLE 64
#define CIRCLE_SIZE_INC (MAX_CIRCLE/(MAX_NUM_CIRCLES+1))
#define B_WIDTH(bid) ((bid)->width)
#define B_HEIGHT(bid) ((bid)->width)
#define B_OFFSET_X(bid) ((bid)->x-(B_WIDTH(bid)/2))
#define B_OFFSET_Y(bid) ((bid)->y-(B_HEIGHT(bid)/2))

/* Definitions affecting missiles */
extern int missile_count, burst_distance;
struct missile {
	Pixwin *pw;
    short start_x, start_y,
		old_x, old_y,
		x, y, inc_x, inc_y,
		slip_cnt, slip, speed, refs, destroyed;
	struct missile *next;
	};
#define BALLISTIC_DELAY 5


/* Definitions affecting everybody */
extern int suspended;
extern long blast_delay;
extern Pixwin *citypw, *launchpw;
extern int max_x, max_y;
extern int foe_value, running;
extern Panel_item level_item, interceptor_item, foe_item, score_item,
	ballistic_item, btime_item, next_round_item, new_game_item, skill_item,
	resume_item, suspend_item, laser_item, non_stop_item, name_item, 
	rock_item, user_name_item, save_file_item;
extern char user_name[], save_file_name[];
extern int city_fd;
extern int launch_fd;
extern Canvas citycanvas;
extern Frame cityframe, controlframe, launchframe;
extern char *panel_common[];
struct pixrect *make_circle();
extern Canvas launchcanvas;
extern int launch_delay;
extern int num_cities;
void no_events();
extern int total_cities_lost;
#define NUM_SCORES 10
extern struct scores {
	char name[64];
	int score, level, skill;
	} sc[], *sc_end;
extern float carryover_divisor, foe_divisor;
extern int foe_factor;
extern int min_missile_speed, max_missile_speed;
extern char *scorefile;
extern struct pixfont *font;	/* struct used for 3.0 compatibility */
extern Pixwin *need_a_bell;
extern int field_margin;
extern Panel_item total_foe_item;
