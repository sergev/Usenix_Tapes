#
/*
 *
 * search: multiplayer chase and destroy game
 * Greg Ordy, Case Western Reserve University, 1978
 * Copyright (c)
 *
 * This file contains code to maintain the linked lists,
 * which describe what a player is currently seeing
 * on his viewscreen
*/

#include	"stuff.h"
#include	"sgame.h"

struct plist   *gnode ();


enteritem (x, y, xchar)
char    x,
        y,
        xchar;
{
    register struct plist  *xp;
    struct plist   *cp;
    register int    glob,
                   *pnt;
    int    *pdp;
    xp = player[wholist].plstp;
    pdp = &(player[wholist].plstp);
    glob = x | (y << 8);
    while (xp != 0)
    {
	pnt = xp;
	if (*pnt < glob)
	{
	    pdp = &(xp -> zpnt);
	    xp = xp -> zpnt;
	    continue;
	};
	if (*pnt == glob)
	{
	    if (xp -> zflg == 1)
		return;
	    xp -> zflg = 1;
	    if (xchar == xp -> zchar)
		return;
	    xp -> zchar = xchar;
	    cursor (x + CENTX, y, wholist);
	    rite (&xchar, 1);
	    return;
	};
	if (*pnt > glob)
	{
	    *pdp = gnode (x, y, xchar);
	    cursor (x + CENTX, y, wholist);
	    rite (&xchar, 1);
	    cp = *pdp;
	    cp -> zpnt = xp;
	    return;
	};
    };
    *pdp = gnode (x, y, xchar);
    cursor (x + CENTX, y, wholist);
    rite (&xchar, 1);
}
uplst ()
{
    register struct plist  *xp;
    struct plist   *cp;
    int    *pdp;
    register char   x,
                    y;
    char    c;
    xp = player[wholist].plstp;
    pdp = &(player[wholist].plstp);
    while (xp != 0)
    {
	x = xp -> zx;
	y = xp -> zy;
	cp = xp -> zpnt;
	switch (xp -> zflg)
	{
	    case 0: 
		cursor (x + CENTX, y, wholist);
		c = ' ';
		if (x == 15 && y != 7)
		    c = '|';
		else
		    if (x != 15 && y == 7)
			c = '-';
		rite (&c, 1);
		xp -> zpnt = 0;
		fnode (xp);
		xp = cp;
		*pdp = cp;
		break;
	    case 1: 
		xp -> zflg = 0;
		pdp = &(xp -> zpnt);
		xp = cp;
		break;
	    default: 
		write (2, "BAD ARG TO UPLST: ", 18);
		write (2, itoa (xp -> zflg), 4);
		xp -> zflg = 0;
		pdp = &(xp -> zpnt);
		xp = cp;
		break;
	};
    };
}
struct plist   *gnode (x, y, c)
char    x,
        y,
        c;
{
    register struct plist  *p,
                           *q;
    register int    i;


    if (avl == 0)
    {
	avl = alloc (100 * sizeof *avl);
	q = avl;
	p = &avl[1];
	for (i = 0; i < 99; i++)
	    (q++) -> zpnt = p++;
	q -> zpnt = 0;
    }
    p = avl;
    avl = avl -> zpnt;
    p -> zx = x;
    p -> zy = y;
    p -> zchar = c;
    p -> zflg = 1;
    p -> zpnt = 0;
    numbnode++;
    return (p);
}

fnode (p)
register struct plist  *p;
{
    register struct plist  *q;
    q = p;
    if (p == 0)
    {
	return;
    };
    while (p -> zpnt)
    {
	p = p -> zpnt;
	numbnode--;
    }
    numbnode--;
    p -> zpnt = avl;
    avl = q;
}
