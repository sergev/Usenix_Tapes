#ifndef lint
static char rcsid[] = "$Header: viewwin.c,v 1.1 84/08/25 17:11:34 jcoker Exp $";
#endif

#include <stdio.h>
#include "../h/sunscreen.h"
#include "../h/hallmap.h"
#include "../h/corners.h"

extern struct pixrect	*px_viewwin;

/*
 *  Draw the 3-D view of the passage straight
 *  ahead, as far as we can see.
 *  The wall segment is always one past the last
 *  segment we can see (since we can't see "into"
 *  the wall), so we have to look ahead to find
 *  out info about the segment ahead.
 */

draw_view(view, dist, dir)
	int		*view;
{
	register	i, contents, next;

	prclear(px_viewwin);

	for (i = 1; i < MAXDEPTH - 1; i++) {
		contents = *view & H_ITEMMASK;
		next = *(view+1) & H_ITEMMASK;

		if (contents & H_LEFTP) {
			/* draw the vertical line marking the passage */
			if ((next & H_WALL) == 0 || (next & H_LEFTP))
				line(px_viewwin, xpoints[0][i], ypoints[0][i], 
				    xpoints[2][i], ypoints[2][i], OP_CLEAR);

			/* the vertical line to mark the last side */
			if (i > 1)
				line(px_viewwin, xpoints[0][i-1], 
				    ypoints[0][i-1], xpoints[2][i-1], 
				    ypoints[2][i-1], OP_CLEAR);

			if ((next & H_LEFTP) == 0) {
				/* draw the lines back for the side passage */
				line(px_viewwin, xpoints[0][i], ypoints[0][i],
				    xpoints[0][i-1], ypoints[0][i], OP_CLEAR);
				line(px_viewwin, xpoints[2][i], ypoints[2][i],
				    xpoints[2][i-1], ypoints[2][i], OP_CLEAR);
			}
		} else {
			/* draw diagonals back to last passage */
			line(px_viewwin, xpoints[0][i], ypoints[0][i],
			    xpoints[0][i-1], ypoints[0][i-1], OP_CLEAR);
			line(px_viewwin, xpoints[2][i], ypoints[2][i],
			    xpoints[2][i-1], ypoints[2][i-1], OP_CLEAR);
		}

		if (contents & H_RIGHTP) {
			/* draw the vertical line marking the passage */
			if ((next & H_WALL) == 0 || (next & H_RIGHTP))
				line(px_viewwin, xpoints[1][i], ypoints[1][i], 
				    xpoints[3][i], ypoints[3][i], OP_CLEAR);

			/* the vertical line to mark the last side */
			if (i > 1)
				line(px_viewwin, xpoints[1][i-1], 
				    ypoints[1][i-1], xpoints[3][i-1], 
				    ypoints[3][i-1], OP_CLEAR);

			if ((next & H_RIGHTP) == 0) {
				/* draw the lines back for the side passage */
				line(px_viewwin, xpoints[1][i], ypoints[1][i],
				    xpoints[1][i-1], ypoints[1][i], OP_CLEAR);
				line(px_viewwin, xpoints[3][i], ypoints[3][i],
				    xpoints[3][i-1], ypoints[3][i], OP_CLEAR);
			}
		} else {
			/* draw diagonals back to last passage */
			line(px_viewwin, xpoints[1][i], ypoints[1][i],
			    xpoints[1][i-1], ypoints[1][i-1], OP_CLEAR);
			line(px_viewwin, xpoints[3][i], ypoints[3][i],
			    xpoints[3][i-1], ypoints[3][i-1], OP_CLEAR);
		}
		
		if (next & H_WALL) {	/* can't see past here */
			/*
			 *  Draw the top and bottom lines
			 *  (and side lines, if no passages)
			 *  and break out of the loop.
			 */
			line(px_viewwin, xpoints[0][i], ypoints[0][i], 
			    xpoints[1][i], ypoints[1][i], OP_CLEAR);
			line(px_viewwin, xpoints[2][i], ypoints[2][i], 
			    xpoints[3][i], ypoints[3][i], OP_CLEAR);

			if ((contents & H_LEFTP) == 0)
				line(px_viewwin, xpoints[0][i], ypoints[0][i],
				    xpoints[2][i], ypoints[2][i], OP_CLEAR);

			if ((contents & H_RIGHTP) == 0)
				line(px_viewwin, xpoints[1][i], ypoints[1][i],
				    xpoints[3][i], ypoints[3][i], OP_CLEAR);

			break;
		}

		/* increment the current view section */
		view++;

		if (i == dist) {
			int	x, ydiff, y, size;

#define PLAYER_SCALE	0.80
			ydiff = ypoints[2][i] - ypoints[0][i];
			size = (double)ydiff * PLAYER_SCALE;
			y = ypoints[2][i] - size / 2;
			x = xpoints[0][i] + 
			    (xpoints[1][i] - xpoints[0][i]) / 2;
			draw_other_player(x, y, size, dir);
		}
	}
	copyat(px_viewwin, VIEWX, VIEWY, OP_WRITE);
}
