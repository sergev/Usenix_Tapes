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
 * All laser-specific code is here.
 */

/* Kill up to this many missiles with one laser blast */
#define MAX_LASERS 6

/* A missile must be lasered for this many time steps to kill it */
#define LASER_KILL_TIME 3

/* A laser kill is worth this many points in the launch window. */
static short laser_value = 50;

int find_lasers();

static struct laser {
	Pixwin *pw;
	short x, y;
	short count, maxlasers;
	short oldx[MAX_LASERS];
	short oldy[MAX_LASERS];
	struct missile *ptr[MAX_LASERS];
	struct laser *next;
	};	

struct laser *l_head = NULL;

/*
 * Initialize a linked list of lasers.  Free any already
 * on there.
 */
init_laser()
{
	struct laser *tmp;
	while (l_head != NULL) {
		tmp = l_head;
		l_head = l_head->next;
		free(tmp);
	}
}		

/* communication into 'find_lasers' */
static struct blast fake_bid;
static struct laser *current_l;
static int global_laser_count;

/*
 * Start a laser blast at position x,y in pixwin pw.
 */
start_laser(x, y, pw, laser_count, laser_range)
int x, y;
Pixwin *pw;
{
	struct blast *bid = &fake_bid;
	struct laser *l = (struct laser *)calloc(sizeof(struct laser), 1);
	struct missile *mptr;
	int i;
	/* Create a fake bid to use the intersection code. */
	bid->x = x;
	bid->y = y;
	bid->width = laser_range;
	update_blast_rect(bid);
	/* initialize the laser structure */
	l->pw = pw;
	l->count = 0;
	l->maxlasers = 0;
	l->x = x;
	l->y = y;
	l->next = l_head;
	l_head = l;
	current_l = l;
	global_laser_count = laser_count;
	doto_missiles(find_lasers);
	mptr = l->ptr[0];
	for (i = 0; i < l->maxlasers; i += 1) {
		l->oldx[i] = l->ptr[i]->x;
		l->oldy[i] = l->ptr[i]->y;
	}
	start_blast(x, y, 0, 0, pw, lasercircles);
}

/*
 * Helper routine passed into 'doto_missiles'.  Uses the global variables
 * 'current_l' and 'fake_bid' to try to find MAX_LASERS missiles within the
 * range of this laser.
 */
find_lasers(mid)
struct missile *mid;
{
	register current_count = current_l->maxlasers;
	if (current_count < global_laser_count &&
			single_intersect(&fake_bid, mid) &&
			mid->pw == current_l->pw) {
		current_l->ptr[current_count] = mid;
		mid->refs += 1;
		current_l->maxlasers += 1;
	}
	return current_count >= global_laser_count;
}

/*
 * This routine is called once each game timestep to update the status
 * of all lasers.
 */
doto_lasers()
{
	/* bump the count of each laser.
	   for each whose count is >= LASER_KILL_TIME, kill its old arcs, start
	   a small blast at each missiles current position, and free the laser.
	*/
	struct laser *lptr, *old_lptr = NULL;
	int i;
	lptr = l_head;
	while (lptr != NULL) {
		lptr->count += 1;
		for (i=0; i < lptr->maxlasers; i += 1) {
			if (lptr->count > 1) {
				/* remove old laser path */
				pw_vector(lptr->pw, lptr->x, lptr->y,
					lptr->oldx[i], lptr->oldy[i],
					PIX_NOT(PIX_SRC), 1);
			}
			if (lptr->count > LASER_KILL_TIME) {
				/* this laser is all done */
				destroy_missile(lptr->ptr[i]);
				if (lptr->ptr[i]->y >= 0) {
					/* blast only if the missile is within range */
					start_blast(lptr->ptr[i]->x, lptr->ptr[i]->y,
						0, 0, lptr->pw, laserkillcircles);
				}
				if (lptr->pw == citypw)
					bump_score(laser_value/5);
				else bump_score(laser_value);
				if (old_lptr == NULL) {
					/* at the head */
					l_head = lptr->next;
				} else {
					old_lptr->next = lptr->next;
				}
			} else {
				/* track the missile */
				lptr->oldx[i] = lptr->ptr[i]->x;
				lptr->oldy[i] = lptr->ptr[i]->y;
				pw_vector(lptr->pw, lptr->x, lptr->y,
					lptr->ptr[i]->x, lptr->ptr[i]->y, PIX_SRC, 1);

				old_lptr = lptr;
			}
		} /* end of for */
		lptr = lptr->next;
	} /* end of while */
}

