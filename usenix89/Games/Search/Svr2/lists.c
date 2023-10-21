#ifndef lint
static char rcsid[] = "$Header: lists.c,v 2.1 85/04/10 17:31:19 matt Stab $";
#endif
/*
 *
 * search
 *
 * multi-player and multi-system search and destroy.
 *
 * Original by Greg Ordy	1979
 * Socket code by Dave Pare	1983
 * Ported & improved
 *      by Matt Crawford	1985
 *
 * routines to maintain linked lists that describe what a
 * player is currently seeing on his viewscreen
 *
 * Copyright (c) 1979
 *
 * $Log:	lists.c,v $
 * Revision 2.1  85/04/10  17:31:19  matt
 * Major de-linting and minor restructuring.
 * 
 * Revision 1.3  84/07/08  17:04:00  matt
 * Added Log
 * 
 * Revision 1.2 84/07/08  16:46:28  matt
 * Rplaced the struct hack *pnt with the short key.  The "hack" was
 * not portable to the Sun!
 */

#include "defines.h"
#include "structs.h"


void enteritem(x, y, xchar)
char x, y, xchar;
{
	extern	t_player *wholist;
	struct	plist *gnode();
	register struct plist *xp = wholist->plstp;
	register int glob = x | (y<<8);
	register int *pdp = (int *) &wholist->plstp;
	struct	plist *cp;
	short	key;

	while (xp != 0) {
		key = (xp->zx) | (xp->zy << 8);
		if (key < glob) {
			pdp = (int *) &xp->zpnt;
			xp = xp->zpnt;
			continue;
		}
		if (key == glob) {
			if (xp->zflg == 1)
				return;
			xp->zflg = 1;
			if (xchar == xp->zchar)
				return;
			xp->zchar = xchar;
			move(x+CENTX, y, wholist);	/* macro */
			*wholist->eoq++ = xchar;
			*wholist->eoq = NULL;
			return;
		}
		if (key > glob) {
			*pdp = (int) gnode(x, y, xchar);
			move(x+CENTX, y, wholist);
			*wholist->eoq++ = xchar;
			*wholist->eoq = NULL;
			cp = (struct plist *) *pdp;
			cp->zpnt = xp;
			return;
		}
	}
	*pdp = (int) gnode(x, y, xchar);
	move(x+CENTX, y, wholist);
	*wholist->eoq++ = xchar;
	*wholist->eoq = NULL;
}

void uplst() {
	extern	void fnode();
	extern	t_player *wholist;
	register struct plist *xp;
	register int *pdp;
	register char x, y;
	struct	plist *cp;
	char	c;

	xp = wholist->plstp;
	pdp = (int *) &wholist->plstp;
	while (xp != 0) {
		x = xp->zx;
		y = xp->zy;
		cp = xp->zpnt;
		switch (xp->zflg) {
		case 0: 
			move(x+CENTX, y, wholist);
			c = x == XWIND && y != YWIND ? '|' :
			    x != XWIND && y == YWIND ? '-' : ' ';
			*wholist->eoq++ = c;
			*wholist->eoq = NULL;
			xp->zpnt = 0;
			fnode (xp);
			xp = cp;
			*pdp = (int) cp;
			break;
		case 1: 
			xp->zflg = 0;
			pdp = (int *) &xp->zpnt;
			xp = cp;
			break;
		default: 
			errlog("bad arg to uplst");
			xp->zflg = 0;
			pdp = (int *) &xp->zpnt;
			xp = cp;
			break;
		}
	}
}

struct plist *gnode(x, y, c)
char x, y, c;
{
	extern	core_dump();
	extern	char *malloc();
	extern	struct plist *avl;
	extern	int numbnode;
	register struct plist *px,
		*qx;
	register int i;

	if (avl == 0) {
		if ((avl = (struct plist *) malloc(100*sizeof(*avl))) == (struct plist *)-1) {
			errlog("out of memory in gnode()\n");
			(void)core_dump();
		}
		qx = avl;
		px = &avl[1];
		for (i=0; i<99; i++)
			(qx++)->zpnt = px++;
		qx->zpnt = 0;
	}
	px = avl;
	avl = avl->zpnt;
	px->zx = x;
	px->zy = y;
	px->zchar = c;
	px->zflg = 1;
	px->zpnt = 0;
	numbnode++;
	return (px);
}

/*
 * free the screen nodes up.
 */
void fnode (px)
register struct plist *px;
{
	extern	int numbnode;
	extern	struct plist *avl;
	register struct plist *qx = px;

	if (px == 0)
		return;
	while (px->zpnt) {
		px = px->zpnt;
		numbnode--;
	}
	numbnode--;
	px->zpnt = avl;
	avl = qx;
}
