/*********************************  circles.c  ************************/
#include <pixrect/pixrect_hs.h>
#include <stdio.h>
#include "sdi.h"

/*
 * Copyright 1987 by Mark Weiser.
 * Permission to reproduce and use in any manner whatsoever on Suns is granted
 * so long as this copyright and other identifying marks of authorship
 * in the code and the game remain intact and visible.  Use of this code
 * in other products is reserved to me--I'm working on Mac and IBM versions.
 */

/*
 * Here be everything associated with drawing circles, especially
 * all the different kinds of blasts (which are done by clipping
 * different pixrects to different shapes of circles.)  All the
 * blast pixrects are precomputed for every size to save time during
 * game display updating.
 */

static short default_laser_blast[] = {
#include "laser.h"
};
mpr_static(laser_blast_pr, 64, 64, 1, default_laser_blast);

static short default_laser_kill[] = {
#include "laserkill.h"
};
mpr_static(laser_kill_pr, 64, 64, 1, default_laser_kill);


static short default_city_blast[] = {
#include "mushroom.h"
};
mpr_static(city_blast_pr, 64, 64, 1, default_city_blast);

static short default_missile_kill[] = {
#include "missilekill.h"
};
mpr_static(missile_kill_pr, 64, 64, 1, default_missile_kill);

static short default_rocks[] = {
#include "rocks.h"
};
mpr_static(rocks_pr, 64, 64, 1, default_rocks);

struct pixrect	*circles[MAX_NUM_CIRCLES];
struct pixrect	*limited_circles[MAX_NUM_CIRCLES];
struct pixrect	*rock_limited_circles[MAX_NUM_CIRCLES];

static struct {
	int x, y;
	} center = { 0, 0 };

static struct fill_line  {
   int	left;
   int	right;
   int	y;
}		 scan_lines[MAX_LINES], *free_line;

static void circle_fill_pair(), circle_fill();


#define LIMIT(n) (((n) < 0) ? 0 : (((n) >= MAX_LINES) ? MAX_LINES-1 : (n)))

init_circles()
{
	int i, size = 2*CIRCLE_SIZE_INC;
	int limited_size = (MAX_NUM_CIRCLES-4)*CIRCLE_SIZE_INC + size;
	int rock_limited_size = 2*CIRCLE_SIZE_INC + size;
	for (i=0; i < MAX_NUM_CIRCLES; i++) {
		if (size > MAX_CIRCLE)
			size = MAX_CIRCLE;
		circles[i] = make_circle(size);
		limited_circles[i] = make_circle(size > limited_size ? limited_size : size);
		rock_limited_circles[i] = make_circle(size > rock_limited_size ? rock_limited_size : size);
		size += CIRCLE_SIZE_INC;
	}
	blankcircles =  circles;
	lasercircles = init_circ(3, circles);
	laserkillcircles = init_circ(3, circles);
	bigblastcircles = init_circ(MAX_NUM_CIRCLES, circles);
	littleblastcircles = init_circ(MAX_NUM_CIRCLES-2, limited_circles);
	blastkillcircles = init_circ(MAX_NUM_CIRCLES-3, circles);
	citykillcircles = init_circ(MAX_NUM_CIRCLES, circles);
	littlerockcircles = init_circ(4, rock_limited_circles);
	bigrockcircles = init_circ(7, rock_limited_circles);
	change_circ(lasercircles, &laser_blast_pr);
	change_circ(laserkillcircles, &laser_kill_pr);
	change_circ(citykillcircles, &city_blast_pr);
	change_circ(blastkillcircles, &missile_kill_pr);
	change_circ(littlerockcircles, &rocks_pr);
	change_circ(bigrockcircles, &rocks_pr);
}

/*
 * Create a new circ structure, and initialize it to contain the pixrect
 * list 'c'.
 */
struct circ *
init_circ(n, c)
struct pixrect **c;
{
	struct circ *r;
	r = (struct circ *)calloc(sizeof(struct circ), 1);
	r->masks = r->circles = c;
	r->num_circles = n;
	return r;
}

/*
 * Routine to replace a list of pixrects in a circ structure with a new
 * list.  The new list is constructed by 'and'ing parameter p with
 * circles of the diameter contained in the old list.  This is the
 * basic routine used to construct all the different kinds of blasts.
 * 
 * Calling change_circ several times on the same circ structure
 * will waste memory because the calloc from previous calls is
 * never freed, nor are the pixrects.  It is not much memory, and nothing will die from it.
 */
change_circ(c, p)
struct circ *c;
struct pixrect *p;
{
	int i;
	struct pixrect **newcircles;
	int size, psize;
	newcircles = (struct pixrect **)calloc(sizeof(struct pixrect *),c->num_circles);
	psize = p->pr_size.x;
	for (i = 0; i < c->num_circles; i++) {
		size = c->masks[i]->pr_size.x;
		newcircles[i] = mem_create(size, size, 1);
		pr_rop(newcircles[i], 0, 0, size, size, PIX_SRC, c->masks[i],
			0, 0);
		pr_rop(newcircles[i], 0, 0, size, size, PIX_SRC & PIX_DST,
			p, psize/2 - size/2, psize/2 - size/2);
	}
	c->circles = newcircles;

}

/*
 * Build a black circle of the specified diameter, and return it in
 * a pixrect of just the right size to hold it.
 */
struct pixrect *
make_circle(diameter)
int diameter;
{
	struct pixrect *pr = mem_create(diameter, diameter, 1);
	center.x = diameter/2;
	center.y = diameter/2;
	free_line = scan_lines;
	circle_accept(pr, LIMIT((diameter/2)-2));
	return pr;
}

/*
 * The routines below borrow ideas from the Sun Bresenham 
 * circle code in iconedit.
 */

/*
 * Draw a circle inside pr, with length as given here, and with center as
 * set in global variable 'center'.
 */
circle_accept(pr, length)
struct pixrect *pr;
{
	int		d, x, y;

	x = 0;
	y = length;
	d = 3 - 2*y;

	while ( x < y ) {
		circle_fill_pair(pr, x, y);
		if (d < 0)
			d += 4*x + 6;
		else {
			d += 4*(x-y) +10;
			y -= 1;
		}
		x++;
	}
	if (x == y) {
		circle_fill_pair(pr, x, y);
	}
	circle_fill(pr);
}

static void
circle_fill_pair(pr, x, y)
struct pixrect *pr;
register int	x, y;
{
	register struct fill_line *line_ptr = free_line;

	free_line += 4;

	line_ptr->left	= LIMIT(center.x - x);
	line_ptr->right = LIMIT(center.x + x);
	line_ptr->y	= LIMIT(center.y - y);

	++line_ptr;
	line_ptr->left  = LIMIT(center.x - y); 
	line_ptr->right = LIMIT(center.x + y); 
	line_ptr->y     = LIMIT(center.y - x);

	++line_ptr;
	line_ptr->left  = LIMIT(center.x - y);
	line_ptr->right = LIMIT(center.x + y);
	line_ptr->y     = LIMIT(center.y + x);

	++line_ptr;
	line_ptr->left  = LIMIT(center.x - x);
	line_ptr->right = LIMIT(center.x + x);
	line_ptr->y     = LIMIT(center.y + y);
}

static int
compare_lines(l1, l2)
	register struct fill_line  *l1, *l2;
{
	if (l1->y < l2->y)
		return -1;
	else if (l1->y > l2->y)
		return 1;
	else if (l1->left < l2->left || l1->right > l2->right)
		return -1;
	else if (l1->left > l2->left || l1->right  <  l2->right)
		return 1;
	else return 0;
}

static void
circle_fill(pr)
struct pixrect *pr;
{
	register struct fill_line *line_ptr = scan_lines;
	register int		   len, y = -1;

	qsort(scan_lines, free_line - scan_lines,
		    sizeof(struct fill_line), compare_lines);
	while (line_ptr < free_line)  {
		register int	x0;

		if (line_ptr->y != y) {
			x0 = line_ptr->left;
			y = line_ptr->y;
			len = line_ptr->right - x0 + 1;
			pr_vector(pr, x0, y, x0+len, y, PIX_SET, 1);
		}
		line_ptr++;
	}
}
