/***************************************  blast.c  ********************/
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
 * Blast-related routines are here and in intersect.c.  Also here is
 * the main dispatch point for display updating called, for historical
 * reasons, 'blast_timer'.
 */

static Notify_value blast_timer();
extern int draw_blast(), doto_missiles(), update_missile();

/*
 * Start the main dispatch timer.
 */
init_blast()
{
	static char *me = (char *)&me;
	struct itimerval timer;
	timer.it_interval.tv_usec = 0;
	timer.it_interval.tv_sec = 0;
	timer.it_value.tv_usec = 1;
	timer.it_value.tv_sec = 0;
	notify_set_itimer_func(me, blast_timer, ITIMER_REAL, &timer, NULL);
}

/*
 * Construct a new blast and put it on the display list.
 */
start_blast(x, y, xinc, yinc, pw, circle_type)
int x, y;
Pixwin *pw;
struct circ *circle_type;
{
	struct blast *bid = (struct blast *)malloc(sizeof(struct blast));
	bid->pw = pw;
	bid->next = NULL;
	bid->circ = 0;
	bid->x = x;
	bid->orig_y = bid->y = y;
	bid->x_inc = xinc;
	bid->y_inc = yinc;
	bid->num_circles = circle_type->num_circles;
	bid->circles = circle_type->circles;
	bid->masks = circle_type->masks;
	bid->width = bid->circles[0]->pr_size.x;
	update_blast_rect(bid);
	add_blast(bid);
	draw_blast(bid);
}

#define INSCRIBING_FUDGE_FACTOR 2

/*
 * Construct a square which approximates the size of the current
 * blast circle.
 */
update_blast_rect(bid)
struct blast *bid;
{
	register int w = bid->width / 2 - INSCRIBING_FUDGE_FACTOR;
	register int h = bid->width - 2*INSCRIBING_FUDGE_FACTOR;
	bid->r.r_left = bid->x - w;
	bid->r.r_top = bid->y - w;
	bid->r.r_width = h;
	bid->r.r_height = h;
}

/*
 * The main dispatch routine.
 * 'blast_timer' is called at fixed intervals, queries routines
 * for launch, missiles, lasers, and blasts for their next update.
 */
static Notify_value
blast_timer(me, which)
int *me;
int which;
{
	extern Panel_item foe_ground_item;
	Pixwin *pw1 = citypw;
	Pixwin *pw2 = launchpw;
	struct itimerval timer;
	struct rect r;
	Event event;

	if (!running) return NOTIFY_DONE;

	if (suspended) {
		suspendor(blast_timer, me, which, 1);
		return NOTIFY_DONE;
	}

	if (missile_count <= 0 && blast_count <= 0 &&
		panel_get_value(ballistic_item) <= 0 &&
		panel_get_value(foe_ground_item) <= 0)
			finish_round();

	checkinput();

	timer.it_interval.tv_usec = 0;
	timer.it_interval.tv_sec = 0;
	timer.it_value.tv_usec = blast_delay;
	timer.it_value.tv_sec = 0;
	notify_set_itimer_func(pw1, blast_timer, ITIMER_REAL, &timer, NULL);


	(void) pw_get_region_rect(pw1, &r);
	pw_batch_on(pw1);
	pw_lock(pw1, &r);
	(void) pw_get_region_rect(pw2, &r);
	pw_batch_on(pw2);
	pw_lock(pw2, &r);

	if (need_a_bell != NULL) {
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 30000; /* this is the minimal reliable bell length */
		win_bell(window_get(cityframe, WIN_FD), tv, need_a_bell);
		need_a_bell = NULL;
	}

	doto_missiles(update_missile);
	doto_lasers();
	doto_blasts(draw_blast);
	doto_rocks();
	doto_launch();

	pw_unlock(pw2);
	pw_batch_off(pw2);
	pw_unlock(pw1);
	pw_batch_off(pw1);

	return NOTIFY_DONE;
}

/*
 * Helper routine passed into 'doto_blasts'.  Updates the
 * blast circle, and frees the blast structure if done.
 */
draw_blast(bid)
struct blast *bid;
{
	int old_circ =  bid->circ;

	if (bid->circ < 0 || bid->x_inc || bid->y_inc) {
		/* shrinking or moving, remove old blast */
		pw_rop(bid->pw, B_OFFSET_X(bid), B_OFFSET_Y(bid),
			B_WIDTH(bid), B_HEIGHT(bid),
			PIX_NOT(PIX_SRC) & PIX_DST,
			bid->masks[ABS(bid->circ)], 0, 0);
	}


	/* Update the next blast circle.  
	   Positive values are growing, negative are shrinking
	 */
	bid->circ += 1;
	if (bid->circ >= bid->num_circles) {
		bid->circ = -bid->num_circles +1;
	}
	if (bid->circ) {
		/* Draw the new blast */
		bid->x += bid->x_inc;
		bid->y += bid->y_inc;
		bid->width = bid->circles[ABS((bid)->circ)]->pr_size.x;
		pw_rop(bid->pw, B_OFFSET_X(bid), B_OFFSET_Y(bid),
			B_WIDTH(bid), B_HEIGHT(bid),
			PIX_SRC | PIX_DST, bid->circles[ABS(bid->circ)], 0, 0);
	}
	if (old_circ == -1) {
		remove_blast(bid);
		free(bid);
	} 
	bid->width = bid->circles[ABS((bid)->circ)]->pr_size.x;
	update_blast_rect(bid);
}
