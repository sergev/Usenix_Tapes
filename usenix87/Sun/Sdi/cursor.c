/*****************************  cursor.c  ******************************/
#include "sdi.h"

/*
 * Copyright 1987 by Mark Weiser.
 * Permission to reproduce and use in any manner whatsoever on Suns is granted
 * so long as this copyright and other identifying marks of authorship
 * in the code and the game remain intact and visible.  Use of this code
 * in other products is reserved to me--I'm working on Mac and IBM versions.
 */

/*
 * Code to manage the dynamic cursor.
 */

struct cursor *compute_cursor();
struct pixrect *compute_cursor_image();

static short dynacursor_data[] = {
#include "dynacursor.h"
};
mpr_static(dynabase_cursor_pr, 16, 16, 1, dynacursor_data);

static short cursor_data[] = {
#include "cursor.h"
};
mpr_static(base_cursor_pr, 16, 16, 1, cursor_data);

struct cursor *
init_cursor()
{
	static struct cursor *main_cursor = NULL;
	extern int cursor_type;
	if (main_cursor) {
		cursor_destroy(main_cursor);
	}
	switch(cursor_type) {
		case 0:
			main_cursor = (struct cursor *)cursor_create(CURSOR_IMAGE, &base_cursor_pr,
				CURSOR_XHOT, 8, CURSOR_YHOT, 8,
				CURSOR_OP, PIX_SRC ^ PIX_DST,
				0);
			break;
		case 1:
			main_cursor = (struct cursor *)cursor_create(CURSOR_OP, PIX_SRC ^ PIX_DST,
				CURSOR_IMAGE, &dynabase_cursor_pr,
				CURSOR_XHOT, 8, CURSOR_YHOT, 3,
				0);
			break;
		case 2:
			main_cursor  = (struct cursor *)cursor_create(CURSOR_SHOW_CROSSHAIRS, TRUE,
				CURSOR_OP, PIX_SRC ^ PIX_DST,
				0);
			break;
		case 3:
			main_cursor = (struct cursor *)cursor_create(CURSOR_SHOW_CROSSHAIRS, TRUE,
				CURSOR_OP, PIX_SRC ^ PIX_DST,
				CURSOR_IMAGE, &dynabase_cursor_pr,
				CURSOR_XHOT, 8, CURSOR_YHOT, 3,
				0);
			break;
		}
	window_set(launchcanvas, WIN_CURSOR, main_cursor, 0);
	window_set(citycanvas, WIN_CURSOR, main_cursor, 0);
}

update_cursor()
{
	extern int cursor_type;
	switch (cursor_type) {
		case 1: case 3:
			window_set(launchcanvas,
				WIN_CURSOR, compute_cursor(window_get(launchcanvas, WIN_CURSOR)),
				0);
			window_set(citycanvas,
				WIN_CURSOR, compute_cursor(window_get(citycanvas, WIN_CURSOR)),
				0);
			break;
		default:
			break;
	}
}

struct cursor *
compute_cursor(c)
struct cursor * c;
{
	cursor_set(c, 
		CURSOR_IMAGE, compute_cursor_image(cursor_get(c, CURSOR_IMAGE)),
		0);
	return c;
}

/* Returns a static structure, so get rid of it before reusing it. */
struct pixrect *
compute_cursor_image(pr)
struct pixrect *pr;
{
	extern Panel_item rock_item;
	int left_current = (int)panel_get_value(interceptor_item);
	int left_max = (int)panel_get(interceptor_item, PANEL_MAX_VALUE);
	int middle_current = (int)panel_get_value(rock_item);
	int middle_max = (int)panel_get(rock_item, PANEL_MAX_VALUE);
	int right_current = (int)panel_get_value(laser_item);
	int right_max = (int)panel_get(laser_item, PANEL_MAX_VALUE);

	pr_rop(pr, 0, 0, 16, 16, PIX_SRC, &dynabase_cursor_pr, 0, 0);
	put_line(pr, 1, 8, 15, left_current, left_max);
	put_line(pr, 7, 8, 15, middle_current, middle_max);
	put_line(pr, 13, 8, 15, right_current, right_max);
	return pr;
}

/*
 * Draw a double-width line in pr from start to finish, with 
 * length equal current/max. assumes finish > start.
 */
put_line(pr, x, start, finish, current, max)
struct pixrect *pr;
{
	int length = (current*(finish-start+1))/max;
	if (length == 0 && current != 0) length = 1;
	pr_vector(pr, x, start-1, x, start+length-1, PIX_SRC, 1);
	pr_vector(pr, x+1, start-1, x+1, start+length-1, PIX_SRC, 1);
	pr_vector(pr, x+2, start-1, x+2, start+length-1, PIX_SRC, 1);
}
