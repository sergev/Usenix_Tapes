/************************************  intersect.c  ******************/
#include "sdi.h"

/*
 * Copyright 1987 by Mark Weiser.
 * Permission to reproduce and use in any manner whatsoever on Suns is granted
 * so long as this copyright and other identifying marks of authorship
 * in the code and the game remain intact and visible.  Use of this code
 * in other products is reserved to me--I'm working on Mac and IBM versions.
 */

/*
 * The code to intersect blasts, missiles, and lasers is in here,
 * as well as some blast-specific code.
 */

#define LINE_GRAIN 20

/* each 'line' is the head of a linked list of current blasts */
static struct blast *lines_launch[MAX_LINES/LINE_GRAIN + 1], *lines_city[MAX_LINES/LINE_GRAIN + 1];
static struct blast **init_line_from_bid(), **init_line_from_mid();

/*
 * Clear out data structures which keep track of blasts. 
 */
init_intersect()
{
	int i;
	for (i=0; i<(MAX_LINES/LINE_GRAIN +1); i++) {
		lines_launch[i] = NULL;
		lines_city[i] = NULL;
	}
}

/*
 * Register a blast for later display.  After this call the blast will
 * automatically be incremented and decremented in size, and will be checked
 * for intersection with missiles.
 */
add_blast(bid)
register struct blast *bid;
{
	/* for now don't sort, just stick at the head */
	register int pos = bid->orig_y / LINE_GRAIN;
	struct blast **lines;
	if (pos < 0 || pos >= MAX_LINES/LINE_GRAIN + 1) {
		return;
	}
	lines = init_line_from_bid(bid);
	bid->next = lines[pos];
	lines[pos] = bid;
	blast_count++;		
}

/*
 * Remove a blast from the display list.  The blast will no longer
 * intersect missiles, change in size, or be displayed. The blast must
 * have already been cleared from the display elsewhere. The storage
 * is not freed here, in case the blast is statically allocated.
 */
remove_blast(bid)
register struct blast *bid;
{	
	struct blast **lines = init_line_from_bid(bid);
	register struct blast *ptr = lines[bid->orig_y / LINE_GRAIN];
	if (ptr == bid) {
		/* head is special case */
		lines[bid->orig_y / LINE_GRAIN] = bid->next;
	} else {
		while ( ptr != NULL && ptr->next != bid ) 
			ptr = ptr->next;
		if (ptr != NULL) 
			ptr->next = bid->next;
	}
	blast_count--;
}

/*
 * See if the missile 'mid' has run into any blasts.
 */
intersect(mid)
struct missile *mid;
{
	int start = max((mid->y - (MAX_CIRCLE >> 1) - 1) / LINE_GRAIN, 0);
	int end = min((mid->y + (MAX_CIRCLE >> 1) + 1) / LINE_GRAIN, max_y);
	register int i;
	register struct blast *ptr;
	struct blast **lines = init_line_from_mid(mid);
	for (i=start; i <= end; i++) {
		ptr = lines[i];
		while (ptr != NULL) {
			if (single_intersect(ptr, mid))
				return TRUE;
			ptr = ptr->next;
		}
	}
	return FALSE;
}

/*
 * See if the missile 'mid' is passing through blast 'bid'.  This routine
 * is also used for laser initialization, with a fake 'bid' to simulate
 * the laser range.  For purposes of computing intersection, the circular
 * blast is considerd to be a rectangle with the same center and approximately
 * the same area.  An intersection has occured if the missile passed
 * through the blast between its last update and now, even if both endpoints
 * are outside the blast.
 */
single_intersect(bid,mid)
struct missile *mid;
struct blast *bid;
{
	int x0 = mid->old_x, y0 = mid->old_y, x1 = mid->x, y1 = mid->y;
	return rect_clipvector(&bid->r, &x0, &y0, &x1, &y1);
}

/*
 * Procedure 'func' is called for every blast on the display lists.
 */
doto_blasts(func)
int (*func)();
{
	int i;
	struct blast *bid, *next;
	struct blast **lines;
	lines = lines_launch;
	for (i=0; i< (MAX_LINES/LINE_GRAIN + 1); i++) {
		bid = lines[i];
		while (bid != NULL) {
			next = bid->next; /* in case func destroys it */
			(*func)(bid);
			bid = next;
		}
	}
	lines = lines_city;
	for (i=0; i<(MAX_LINES/LINE_GRAIN + 1); i++) {
		bid = lines[i];
		while (bid != NULL) {
			next = bid->next; /* in case func destroys it */
			(*func)(bid);
			bid = next;
		}
	}
}

/*
 * Given a blast, return the proper display on which it will be found.
 */
static struct blast **
init_line_from_bid(bid)
struct blast *bid;
{
	if (bid->pw == launchpw) {
		return lines_launch;
	} else {
		return lines_city;
	}
}

/*
 * Given a missile, return the proper display list on which intersecting
 * blasts will be found.
 */
static struct blast **
init_line_from_mid(mid)
struct missile *mid;
{
	if (mid->pw == launchpw) {
		return lines_launch;
	} else {
		return lines_city;
	}
}

/*
 * Remove all blasts from the display lists and free their storage.
 * Nothing is done here about getting them off the display.
 */
static struct blast **line_list[] = {lines_launch, lines_city};
free_all_blasts()
{
	int i, l;
	struct blast *bid, *nextbid, **lines;
	for (l=0; l < 2; l++) {
		lines = line_list[l];
			for (i=0; i<(MAX_LINES/LINE_GRAIN + 1); i++) {
			bid = lines[i];
			lines[i] = NULL;
			while (bid != NULL) {
				nextbid = bid->next;
				free(bid);
				bid = nextbid;
			}
		}
	}
	blast_count = 0;
}

/*
	This is the old style, now commented out.  It would let a fast
	moving missile pass right through, but did consider blasts to 
    be real circles.

single_intersect(bid, mid)
struct blast *bid;
struct missile *mid;
{
	register short offx = B_WIDTH(bid)/2;
	register short x = bid->x - mid->x;
	register short y = bid->y - mid->y;
	return (x*x + y*y) < (offx*offx);
}
*/

