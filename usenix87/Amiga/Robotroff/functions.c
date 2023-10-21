/* :ts=8 bk=0
 *
 * functions.c:		Functions to do obscene things to mouse pointer.
 *
 * Leo L. Schwab			8703.18		(415)-456-6565
 */

#include <exec/types.h>
#include <intuition/intuition.h>
#include <graphics/sprite.h>
#include <hardware/custom.h>
#include "robotroff.h"


extern UWORD	hr1[], hr2[], hr3[], hl1[], hl2[], hl3[], hu1[], hu2[], hu3[],
		gr1[], gr2[], gr3[], nforce[],
		hulkcolor[], gruntcolor[], nforcecolor[];

extern struct IOStdReq		*ioreq;
extern struct InputEvent	mouse;
extern struct SimpleSprite	spr;
extern struct Preferences	prefs;
extern struct Window		*win;
extern struct Screen		*wbs;
extern long			sprnum;
extern int			wide, high;
extern void			*vp;

struct Custom *cstm = 0xdff000;

UWORD			*leftseq[] = { hl1, hl2, hl1, hl3 };
UWORD			*rightseq[] = { hr1, hr2, hr1, hr3 };
UWORD			*udseq[] = { hu1, hu2, hu1, hu3 };
UWORD			*gruntseq[] = { gr1, gr2, gr1, gr3 };

static int idxsav, dirchange;


/*
 * This particular piece of code turned out to be gut-wrenchingly messy.
 * But it's the only way I could think of to make it work the way I wanted.
 * If you know of a cleaner algorithm, write it, send it to me, and I'll
 * use it in a revised version of the program.
 */
dohulk ()
{
	register UWORD **sequence;
	register int idx = 0, dx, dy;
	int mx, my, ox, oy, dhx, dhy, x1, y1, x2, y2,  flag = 0;

	setcolors (vp, (int) sprnum, hulkcolor);
	startxy (&spr.x, &spr.y);
	if (spr.x == MINX || spr.x == wide)	/*  Determine starting dir  */
		flag = 1;
	spr.height = HULKHEIGHT;
	ChangeSprite (0L, &spr, leftseq[idx]);
	mx = cstm -> clxdat;	/*  Clear collision status  */

trackmouse:
	while (1) {
		getmousexy (&mx, &my, HULKHEIGHT);
		dx = mx - spr.x;  dy = my - spr.y;
		dhx = 4 * sgn (dx);  dhy = 2 * sgn (dy);
		if (flag) {
			x1 = x2 = rnd (abs (dx)) * sgn (dx) + spr.x;
			y1 = spr.y;
			y2 = my;
		} else {
			y1 = y2 = rnd (abs (dy)) * sgn (dy) + spr.y;
			x1 = spr.x;
			x2 = mx;
		}
		ox = mx;  oy = my;

		if (flag) {
			dx = dhx;  dy = 0;
			if (walkhulkx (spr.x, x1, dhx))
				break;
			getmousexy (&mx, &my, HULKHEIGHT);
			if (mx != ox || my != oy)
				continue;

			dx = 0;  dy = dhy;
			if (walkhulky (y1, y2, dhy))
				break;
			getmousexy (&mx, &my, HULKHEIGHT);
			if (mx != ox || my != oy)
				continue;

			dx = dhx;  dy = 0;
			if (walkhulkx (x2, mx, dhx))
				break;
		} else {
			dx = 0;  dy = dhy;
			if (walkhulky (spr.y, y1, dhy))
				break;
			getmousexy (&mx, &my, HULKHEIGHT);
			if (mx != ox || my != oy)
				continue;

			dx = dhx;  dy = 0;
			if (walkhulkx (x1, x2, dhx))
				break;
			getmousexy (&mx, &my, HULKHEIGHT);
			if (mx != ox || my != oy)
				continue;

			dx = 0;  dy = dhy;
			if (walkhulky (y2, my, dhy))
				break;
		}
		flag = rnd (2);
	}

	if (dx)
		sequence = dx>0 ? rightseq : leftseq;
	else
		sequence = udseq;
	idx = idxsav;
	ox = oy = -9999;
	while (1) {
		spr.x += dx;  spr.y += dy;
		idx = ++idx & 3;
		ChangeSprite (0L, &spr, sequence[idx]);
		movemouse (dx, dy);
		mx = collision();
		Delay (6L);
		if (!collision()) {
			idxsav = idx;
			goto trackmouse;
		}
		getmousexy (&mx, &my, HULKHEIGHT);
		if (mx == ox && my == oy)	/*  Mouse at limit  */
			break;
		ox = mx;  oy = my;
	}

	/*  Finish walking hulk off screen  */
	while ((int) spr.x < wide && (int) spr.x > MINX &&
	       (int) spr.y < high && (int) spr.y > -HULKHEIGHT) {
		spr.x += dx;  spr.y += dy;
		idx = ++idx & 3;
		ChangeSprite (0L, &spr, sequence[idx]);
		Delay (6L);
	}
}

dogrunt ()
{
	register int idx = 0;
	int gx, gy, mx, my;

	setcolors (vp, (int) sprnum, gruntcolor);
	startxy (&gx, &gy);
	spr.x = gx;  spr.y = gy;  spr.height = GRUNTHEIGHT;
	ChangeSprite (0L, &spr, gruntseq[idx]);
	mx = cstm -> clxdat;	/*  Clear collision status  */

	/*  This will keep chasing you around until it gets you :->  */
	while (!collision()) {
		idx = ++idx & 3;
		getmousexy (&mx, &my, GRUNTHEIGHT);
		if (mx != gx)
			gx += mx > gx ? 4 : -4;
		if (my != gy)
			gy += my > gy ? 4 : -4;
		spr.x = gx;  spr.y = gy;
		ChangeSprite (0L, &spr, gruntseq[idx]);
		Delay (rnd (7) + 4L);
	}

	flashpointer ();
	MoveSprite (0L, &spr, (long) MINX, 0L);
}

enforce ()
{
	register int track, lim, inc;
	int mx, my, ox = -999, oy = -999, ex, ey, dx, dy, flag = 0;

	setcolors (vp, (int) sprnum, nforcecolor);
	startxy (&ex, &ey);
	spr.x = ex;  spr.y = ey;  spr.height = NFORCEHEIGHT;
	ChangeSprite (0L, &spr, nforce);
	mx = cstm -> clxdat;	/*  Clear collision bits  */

	/*
	 * This is a DDA algorithm that attempts to track the mouse pointer
	 * in a straight line, no matter where it is.  It keeps tracking
	 * until it gets it.
	 */
	while (!collision()) {
		getmousexy (&mx, &my, NFORCEHEIGHT);
		if (mx != ox || my != oy) {	/*  Mouse moved  */
			dx = mx - ex;  dy = my - ey;
			if (abs (dx) > abs (dy)) {
				flag = 1;
				lim = abs (dx);
				inc = abs (dy);
			} else {
				flag = 0;
				lim = abs (dy);
				inc = abs (dx);
			}
			track = lim/2;
			ox = mx;  oy = my;
		}
		if (flag) {
			ex += sgn (dx);
			if ((track += inc) > lim) {
				track -= lim;
				ey += sgn (dy);
			}
		} else {
			ey += sgn (dy);
			if ((track += inc) > lim) {
				track -= lim;
				ex += sgn (dx);
			}
		}

		MoveSprite (0L, &spr, (long) ex, (long) ey);
		WaitTOF ();
	}

	flashpointer ();
	MoveSprite (0L, &spr, (long) MINX, 0L);
}

walkhulkx (startx, endx, dx)
{
	register UWORD **sequence;
	register int x, idx = 0;

	if (dirchange)
		idx = idxsav;
	dirchange = 1;
	sequence = dx>0 ? rightseq : leftseq;

	for (x=startx; dx>0 ? x<endx : x>endx; x+=dx, idx = ++idx & 3) {
		spr.x = x;
		ChangeSprite (0L, &spr, sequence[idx]);
		Delay (6L);
		if (collision()) {
			idxsav = idx;
			return (1);
		}
	}
	idxsav = idx;
	return (0);
}

walkhulky (starty, endy, dy)
{
	register int y, idx = 0;

	if (!dirchange)
		idx = idxsav;
	dirchange = 0;

	for (y=starty; dy>0 ? y<endy : y>endy; y+=dy, idx = ++idx & 3) {
		spr.y = y;
		ChangeSprite (0L, &spr, udseq[idx]);
		Delay (6L);
		if (collision()) {
			idxsav = idx;
			return (1);
		}
	}
	idxsav = idx;
	return (0);
}

startxy (x, y)
register int *x, *y;
{
	if (rnd (2)) {
		*x = rnd (2) ? MINX : wide;
		*y = rnd (200);
	} else {
		*y = rnd (2) ? MINY : high;
		*x = rnd (320);
	}
}

getmousexy (x, y, sprheight)
register int *x, *y;
{
	register struct Screen *s = wbs;

	*x = (s -> MouseX >> 1) - 8;
	if (s -> ViewPort.Modes & LACE)
		*y = (s -> MouseY >> 1) + s -> TopEdge - sprheight/2;
	else
		*y = s -> MouseY + s -> TopEdge - sprheight/2;
}

setcolors (vp, spritenum, clist)
void *vp;
register UWORD *clist;
{
	long r, g, b, colorbase;
	register int i;

	colorbase = getcbase (spritenum);
	for (i=1; i<4; i++) {
		r = getr (clist[i]);
		g = getg (clist[i]);
		b = getb (clist[i]);
		SetRGB4 (vp, colorbase + i, r, g, b);
	}
}

flashpointer ()
{
	long rs, gs, bs,  dr, dg, db,  rd, gd, bd;
	register int i, n;

	rs = gs = bs = 15;
	rd = getr (prefs.color0);
	gd = getg (prefs.color0);
	bd = getb (prefs.color0);

	/*  Compute increments  */
	dr = rd - rs;
	dg = gd - gs;
	db = bd - bs;

	for (n=15; n>6; n--) {		/*  First flash  */
		for (i=0; i<3; i++)
			SetRGB4 (vp, 17L + i, (long) n, (long) n, (long) n);
		WaitTOF ();
	}

	rs <<= 4;  gs <<= 4;  bs <<= 4;
	for (n=0; n<16; n++) {		/*  Fade to background  */
		rs += dr;
		gs += dg;
		bs += db;
		for (i=0; i<3; i++)
			SetRGB4 (vp, 17L + i, rs>>4, gs>>4, bs>>4);
		WaitTOF ();  WaitTOF ();  WaitTOF ();  WaitTOF ();
	}

	Delay (150L);
	SetRGB4 (vp, 17L, (long) getr (prefs.color17),	/*  Yuck!  */
			  (long) getg (prefs.color17),
			  (long) getb (prefs.color17));
	SetRGB4 (vp, 18L, (long) getr (prefs.color18),
			  (long) getg (prefs.color18),
			  (long) getb (prefs.color18));
	SetRGB4 (vp, 19L, (long) getr (prefs.color19),
			  (long) getg (prefs.color19),
			  (long) getb (prefs.color19));
}

movemouse (dx, dy)
{
	mouse.ie_NextEvent = 0;
	mouse.ie_Class = IECLASS_RAWMOUSE;
	mouse.ie_TimeStamp.tv_secs = mouse.ie_TimeStamp.tv_micro = 0;
	mouse.ie_Code = IECODE_NOBUTTON;
	mouse.ie_Qualifier = IEQUALIFIER_RELATIVEMOUSE;
	mouse.ie_X = prefs.PointerTicks * (dx + dx);
	/*  Is this *really* right?  */
	mouse.ie_Y = prefs.PointerTicks * (dy + dy);
	if (DoIO (ioreq))
		die ("I/O error.");
}
