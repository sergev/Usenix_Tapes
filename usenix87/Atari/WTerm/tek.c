/*
 * tek
 *
 * This routine will attempt to emulate a Tektronix 4010 storage scope
 * terminal...
 */

#include "wterm.h"

#define FALSE   0
#define TRUE    1

int Ly, Lx, Hy, Hx;

tek(c)
char c;
{
	register int y, x;

	if( GS == c ) {
		T_mode = GRAPH;
		T_draw = FALSE;
		T_count = 0;
	} else if( US == c ) {
		T_mode = ALPHA;
	} else {
		switch( (int) (c & 0x60) ) {

		case 0x20: /* high order Y or X */
			if( T_count > 0 ) {
				/* high order X */
				Hx = c & 0x1f;
			} else {
				/* high order Y */
				Hy = (int) (c & 0x1f);
			}
			break;

		case 0x60: /* low order Y */
			Ly = (int) (c & 0x1f);
			T_count = 1;  /* X next */
			break;

		case 0x40: /* low X */
			Lx = (int) (c & 0x1f);
			x = Lx | (int) (Hx << 5);
			y = Ly | (int) (Hy << 5);
			plot(x,y,T_draw);
			T_count = 0;
			T_draw = TRUE;
			break;

		default:
			/*
			 * this really should not happen but we all
			 * know better...
			 */
			break;		/* just ignore (lazy, I guess) */
		}
	}
}

plot(x,y,draw)
{
	int pxy[8];

	x = x / 2;
	y = 399 - (y/2);
	if( y < 0 )
		y = 0;
	pxy[2] = x;
	pxy[3] = y;

	/*
	 * if draw is TRUE then we are joining the previous point to
	 * the current one.  Otherwise, we just plot a point.
	 */
	if( TRUE == draw ) {
		pxy[0] = T_x;
		pxy[1] = T_y;
	} else {
		pxy[0] = x;
		pxy[1] = y;
	}
	v_pline( Handle, 2, pxy );
	T_x = x;
	T_y = y;
}




