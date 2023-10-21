/**********************************  missile.c  ***********************/
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
 * Code to start and update missiles lives here, including missiles
 * traveling between the two windows.  Launching of missiles
 * is done in incoming.c.
 */

static struct missile *m_head = NULL;
static Notify_value ballistic_timer();

/*
 * Throw a missile onto a window. Only x is suppied, because direction
 * determines whether the missile starts at the top or bottom of the window.
 * Speed is in units of approximate pixels-per-timestep.  Direction
 * should be UP or DOWN, which are defined in sdi.h
 */
start_missile(x, direction, speed, pw)
int x, direction, speed;
Pixwin *pw;
{
	struct missile *mid = (struct missile *)malloc(sizeof(struct missile));
	int ratio, number_of_steps;
	int final_x = (random() % (max_x - MARGIN)) + (MARGIN/2);

	panel_set_value(foe_item, panel_get_value(foe_item) + 1);

	mid->start_x = mid->x = x;
	mid->speed = speed;
	mid->refs = 1;
	mid->destroyed = FALSE;

	number_of_steps = max(1,(ABS(final_x - mid->start_x) + max_y)/speed);
	mid->inc_x = (final_x - mid->start_x)/number_of_steps;
	mid->inc_y = max_y/number_of_steps;
	if (mid->inc_y < 1) {
		mid->inc_y += 1;
		mid->inc_x -= 1;
	}

	
	/* I'm making this up as I go... */
	{
	    double desired = (double)(ABS(final_x - mid->start_x)/(double)max_y);
	    double actual = (double)ABS(mid->inc_x)/(double)ABS(mid->inc_y);
	    mid->slip = (double)1.0 / (desired - actual);
	    mid->slip_cnt = 0;		
	} 

	if (direction == DOWN) {
		mid->start_y = mid->y = 0;
	} else {
		mid->start_y = mid->y = max_y;
		mid->inc_y = -mid->inc_y;
	}
	mid->pw = pw;
			
	inc_missile(mid);

	missile_count++;
	mid->next = m_head;
	m_head = mid;
}

/*
 * Move a missile ahead and see if hits anything.
 * Helper routine passed into doto_missiles.
 */
update_missile(mid)
struct missile *mid;
{
	inc_missile(mid);
	if (intersect(mid)) {
		start_blast(mid->x, mid->y, 0, 0, mid->pw, blastkillcircles);
		destroy_missile(mid);
		if (mid->pw == citypw)
			bump_score(foe_value/5);
		else bump_score(foe_value);
	} else if (mid->inc_y > 0 && mid->y >= max_y-burst_distance) {
		start_blast(mid->x, mid->y - 10, 0, 0, mid->pw, citykillcircles);
		destroy_missile(mid);
	} else if (mid->inc_y < 0 && mid->y <= 0) {
		start_ballistic(mid->x, mid->speed);
		destroy_missile(mid);
	} else if (mid->x < 0 || mid->x > max_x) {
		start_blast(mid->x, mid->y, 0, 0, mid->pw, blastkillcircles);
		destroy_missile(mid);
	}
	return 0;
}

/*
 * Update the missile track.
 */
inc_missile(mid)
struct missile *mid;
{
	/* Compute basic update */
	mid->old_x = mid->x;
	mid->old_y = mid->y;
	mid->x += mid->inc_x;
	mid->y += mid->inc_y;

	/* Adjust skew for straighter lines */
	if (mid->slip && ++mid->slip_cnt >= ABS(mid->slip)) {
		mid->slip_cnt = 0;
		if (mid->slip > 0) {
			mid->x += 1;
		} else {
			mid->y += 1;
		}
	}

	/* Draw missile trail */
	pw_vector(mid->pw, mid->old_x-1, mid->old_y, mid->x-1, mid->y, PIX_SRC, 1);
	pw_vector(mid->pw, mid->old_x, mid->old_y, mid->x, mid->y, PIX_SRC, 1);
	pw_vector(mid->pw, mid->old_x+1, mid->old_y, mid->x+1, mid->y, PIX_SRC, 1);
}

/*
 * Get rid of a missile by erasing its track, removing it from
 * the missile display list, and freeing its structure.  Explosion
 * of the missile is the responsibility of the caller.
 */
destroy_missile(mid)
struct missile *mid;
{
	char buff[128];
	struct rect r;

	if (!mid->destroyed) {
		panel_set_value(foe_item, panel_get_value(foe_item) - 1);
		sprintf(buff, "%d", atol(panel_get_value(total_foe_item))+1);
		panel_set_value(total_foe_item, buff);
		mid->destroyed = TRUE;
		pw_vector(mid->pw, mid->start_x - 2, mid->start_y,
			mid->x - 2, mid->y,
			PIX_NOT(PIX_SRC), 1);
		pw_vector(mid->pw, mid->start_x - 1, mid->start_y,
			mid->x - 1, mid->y,
			PIX_NOT(PIX_SRC), 1);
		pw_vector(mid->pw, mid->start_x, mid->start_y,
			mid->x, mid->y,
			PIX_NOT(PIX_SRC), 1);
		pw_vector(mid->pw, mid->start_x + 1, mid->start_y,
			mid->x + 1, mid->y,
			PIX_NOT(PIX_SRC), 1);
		pw_vector(mid->pw, mid->start_x + 2, mid->start_y,
			mid->x + 2, mid->y,
			PIX_NOT(PIX_SRC), 1);
	
		if (m_head == mid) {
			m_head = mid->next;
		} else {
			struct missile *tmpmid = m_head;
			while (tmpmid != NULL && tmpmid->next != mid) 
				tmpmid = tmpmid->next;
			if (tmpmid != NULL)
				tmpmid->next = mid->next;
		}
		missile_count--;
	}
	if (--mid->refs == 0) {
		free(mid);
	}
}

/*
 * Update the score by 'inc', augmented by skill level.
 */
bump_score(inc)
{
	int score, skill;
	float skill_multiplier;
	char buf[128];
	skill = (int)panel_get_value(skill_item);
	switch (skill) {
		case 0: skill_multiplier = 1.0; break;
		case 1: skill_multiplier = 1.5; break;
		case 2: skill_multiplier = 3; break;
	}
	score = atol(panel_get_value(score_item)) + (int)(((float)inc)*skill_multiplier);
	sprintf(buf,"%d", score);
	panel_set_value(score_item, buf);
}

/*
 * Call 'func' for missiles in the display list.  If func
 * returns non-zero, stop.  Search the missiles round-robin,
 * so we don't always find the same ones.
 */
doto_missiles(func)
int (*func)();
{
	struct missile *ptr = m_head, *next;
	while (ptr != NULL) {
		next = ptr->next; /* in case 'func' destroys the missile */
		(*func)(ptr);
		ptr = next;
	}
}

/*
 * Track a missile when traveling between windows.
 */
struct ballistic_type {int x, speed};
start_ballistic(x, speed)
{
	extern int ballistic_delay;
	struct itimerval timer;
	struct ballistic_type *xptr = (struct ballistic_type *)calloc(1,sizeof(struct ballistic_type));
	int old_value = (int)panel_get_value(ballistic_item);
	xptr->x = x;
	xptr->speed = speed;
	panel_set_value(ballistic_item, old_value + 1);
	
	if (old_value == 0) {
		ballistic_warning();
	}
	timer.it_interval.tv_usec = 0;
	timer.it_interval.tv_sec = 0;
	timer.it_value.tv_usec = 0;
	timer.it_value.tv_sec = ballistic_delay;
	if (timer.it_value.tv_sec > 0) {
		notify_set_itimer_func(xptr, ballistic_timer, ITIMER_REAL, &timer, NULL);
	} else {
		ballistic_timer(xptr, NULL);
	}
}

/*
 * Called when the between-window flight time of a missile is up.
 */
static Notify_value
ballistic_timer(xptr, which)
struct ballistic_type *xptr;
int which;
{
	extern int ballistic_delay;
	int val;
	if (running) {
		if (suspended) {
			/* by rechecking at each in-flight interval for each missile, we
			   approximate remembering when the real relaunch rate.
			*/
			suspendor(ballistic_timer, xptr, which, ballistic_delay);
			return NOTIFY_DONE;
		} else {
			val = (int)panel_get_value(ballistic_item);
			if (val > 0) {
				/* Three new missiles appear. */
				start_missile(xptr->x, DOWN, xptr->speed, citypw);
				start_missile(xptr->x, DOWN, xptr->speed, citypw);
				start_missile(xptr->x, DOWN, xptr->speed, citypw);
				panel_set_value(ballistic_item, panel_get_value(ballistic_item)-1);
			}
		}
	}
	free(xptr);
	return NOTIFY_DONE;
}

/*
 * Just what it says.
 */
free_all_missiles()
{
	free_foe();
	panel_set_value(ballistic_item, 0);
	while(m_head != NULL)
		destroy_missile(m_head);
}

do_warn_bell()
{
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 20000;	/* very short bell */
	win_bell(window_get(cityframe, WIN_FD), tv, 0);
}

#define WARN_INTERVAL 100000

ballistic_warning()
{
	do_with_delay(do_warn_bell, 0, WARN_INTERVAL);
	do_with_delay(do_warn_bell, 0, 2*WARN_INTERVAL);
	do_with_delay(do_warn_bell, 0, 3*WARN_INTERVAL);
}
