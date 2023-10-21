/*********************************  incoming.c  ****************************/
#include <pixrect/pixrect_hs.h>
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
 * This file contains routines for lauching 'foe' missiles.  Since
 * a round is over when all the foes are gone, end-of-round work
 * is done here as well.  And since the game is over when there are
 * no cities left at the end of the round, end-of-game computations
 * are mostly done here too. And, since the only difference between
 * different skills is the difficulty of foe missiles, skill adjustment
 * is also done in here.  Why not just do everything here?  Wouldn't fit.
 */

/* The following two constants determine launch rates. */
#define MAX_TIME_FOR_A_ROUND	45000000
#define AVG_DELAY_BETWEEN_MISSILES 1000000

static short *missiles_to_launch = NULL;
static int next_missile_launch = 0;
static int num_launch_intervals;

static bonus_threshold = 5000;

/*
 * Increase the game level by one, within reason.
 */
#define MAX_LEVEL 49

int
bump_level()
{
	int level;
	char buf[32];
	level = atol((int)panel_get_value(level_item)) + 1;
	if (level > MAX_LEVEL) {
		level = MAX_LEVEL;
		panel_set(next_round_item, PANEL_SHOW_ITEM, FALSE, 0);
	}
	sprintf(buf,"%d", level);
	panel_set_value(level_item, buf);
	return level;
}

/*
 * Compute the skill differentials, get the missiles ready to launch,
 * and set the 'running' switch on.  Normally 'init_incoming' is the last
 * thing called when a round is started, and setting the 'running' switch
 * immediately releases various notifications so the game really begins.
 */
init_incoming()
{
	extern Panel_item rock_item, foe_ground_item;
	int foe_val, friend_val, laser_val, rocks_val, level, skill, count, range;
	int base = 2;
	char buf[128];
	if (running)
		return;

	skill = (int)panel_get_value(skill_item);
	switch (skill) {
		case 0: /* novice */
			carryover_divisor = 1.0;
			foe_divisor = 2.0;
			min_missile_speed = 5;
			max_missile_speed = 15;
			foe_factor = 2;
			break;
		case 1: /* occasional */
			carryover_divisor = 1.3;
			foe_divisor = 2.5;
			min_missile_speed = 10;
			max_missile_speed = 20;
			foe_factor = 4;
			break;
		case 2: /* expert */
			carryover_divisor = 1.6;
			foe_divisor = 3.0;
			min_missile_speed = 15;
			max_missile_speed = 25;
			foe_factor = 6;
			break;
	}

	level = bump_level();

	foe_val = foe_factor * level;
	panel_set(foe_ground_item, PANEL_MAX_VALUE, foe_val, PANEL_VALUE, foe_val, 0);
	panel_set(foe_item, PANEL_MAX_VALUE, foe_val*3, 0);

	laser_val = (int)panel_get_value(laser_item);
	laser_val = ((int)((float)foe_val/foe_divisor)) + laser_val/carryover_divisor;
	laser_val += base;
	panel_set(laser_item, PANEL_MAX_VALUE, laser_val, PANEL_VALUE, laser_val,
		0);
	panel_set(ballistic_item, PANEL_MAX_VALUE, foe_val, 0);

	friend_val = (int)panel_get_value(interceptor_item);
	friend_val = ((int)((float)foe_val/foe_divisor)) + friend_val/carryover_divisor; 
	friend_val += base;
	panel_set(interceptor_item, PANEL_MAX_VALUE, friend_val, PANEL_VALUE, friend_val,
		0);

	rocks_val = (int)panel_get_value(rock_item);
	rocks_val = ((int)((float)foe_val/foe_divisor)) + rocks_val/carryover_divisor; 
	rocks_val += base;
	panel_set(rock_item, PANEL_MAX_VALUE, rocks_val, PANEL_VALUE, rocks_val,
		0);

	if (missiles_to_launch != NULL)
		free(missiles_to_launch);
	num_launch_intervals = MAX_TIME_FOR_A_ROUND/blast_delay + 2;
	missiles_to_launch = (short *)calloc(sizeof(short), num_launch_intervals);
	next_missile_launch = 0;
	count = (int)panel_get_value(foe_ground_item);
	range = min( MAX_TIME_FOR_A_ROUND, AVG_DELAY_BETWEEN_MISSILES*count);
	while (count--) {
		missiles_to_launch[(random() % range) / blast_delay] += 1;
	}
	new_score();
	if (level <= 1)
		bonus_threshold = 5000;

}

/*
 * Called at each display update timestep, this routine launches
 * any missiles which have come due.
 */
doto_launch()
{
	int level;
	int num_missiles; 

	if (!suspended) {
		int x;
		if (next_missile_launch >= num_launch_intervals)
			return;	
		num_missiles = missiles_to_launch[next_missile_launch++];
		if (num_missiles) {
			panel_set_value(foe_ground_item, 
				panel_get_value(foe_ground_item) - num_missiles);
			level = atol((char *)panel_get_value(level_item));
			while (num_missiles--) {
				x = random() % max_x;
				start_missile(x, UP,
					min(max(normal(min_missile_speed+level/2,level/2),
						min_missile_speed),
						max_missile_speed), launchpw);
			}
		}
	}
}

/*
 * Compute which cites are still left, bonus scores, and end-of-game.
 * Get ready for the next round.
 */
finish_round()
{
	char buf[128], score_buf[32];
	int cities_lost = compute_cities(citypw);
	extern int continuous;
	int level = atol((char *)panel_get_value(level_item));
	int score = atol((char *)panel_get_value(score_item));
	int bonus, extra_city = 0;

	running = 0;
	put_text(25, "Round Over");
	total_cities_lost += cities_lost;
	bonus =  level*10*(num_cities-total_cities_lost);
	update_cities(citypw, 1);
	if ((score+bonus) >= bonus_threshold && total_cities_lost > 0) {
		extra_city = 1;
		total_cities_lost -= 1;
		bonus_threshold += bonus_threshold;
	}
	bonus =  level*10*(num_cities-total_cities_lost);
	sprintf(buf, "You have %d cities left, for %d bonus points.",num_cities-total_cities_lost, bonus);
	panel_set_value(score_item, sprintf(score_buf, "%d", score+bonus));
	if(total_cities_lost >= num_cities) {
		panel_set(skill_item, PANEL_EVENT_PROC, panel_default_handle_event, 0);
		if (!continuous)
			do_game_over();
		update_scores();
		new_game_proc();
	} else {
		put_text(50, buf);
		if (extra_city) {
			put_text(0, "BONUS CITY");
			new_city(citypw);
		}
		next_round_proc();
		if (continuous) {
			sleep(1);
			start_next_round();
		}
	}
}

/*
 * this keeps any more missiles from being launched.  Actually freeing
 * of allocated structures happens on the next 'init_incoming' call.
 */
free_foe()
{
	num_launch_intervals = 0;
}
