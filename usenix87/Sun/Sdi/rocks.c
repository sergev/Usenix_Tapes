/******************************  laser.c *******************************/
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
 * All rock-specific code is here.
 */

struct rock {
	int x1, y1, x2, y2, number;
	struct circ *type;
	struct rock *next;
	Pixwin *pw;
} *rock_head = NULL;



/*
 * Throw some rocks out.
 */
start_rocks(pw, x1, y1, x2, y2, number, type)
Pixwin *pw;
struct circ *type;
{
	struct rock real_r, *r = &real_r;
	r->x1 = x1; r->x2 = x2; r->y1 = y1; r->y2 = y2;
	r->number = number; r->type = type; r->pw = pw;
	draw_rock(r);
}

draw_rock(r)
register struct rock *r;
{
	double x, y, xinc, yinc, fabs();
	int i;
	x = r->x1;
	y = r->y1;
	xinc = (double)(r->x2-r->x1)/(double)r->number;
	yinc = (double)(r->y2-r->y1)/(double)r->number;
	for (i=0; i < r->number; i += 1) {
		start_blast((int)x, (int)y, 0, 4, r->pw, r->type);
		x += xinc;
		y += yinc;
	}
}

/*
 * Add_rock, doto_rocks, and delete_rock are unused at this time, but
 * they were once used, and worked.
 */

add_rock(r)
struct rock *r;
{
	r->next = rock_head;
	rock_head = r;
}

doto_rocks()
{
}

delete_rock(r)
struct rock *r;
{
	struct rock *tr = rock_head;
	if (r == tr) {
		rock_head = r->next;
		return;
	}
	while (tr != NULL && tr->next != r) {
		tr = tr->next;
	}
	if (tr != NULL) {
		tr->next = r->next;
	}
}
