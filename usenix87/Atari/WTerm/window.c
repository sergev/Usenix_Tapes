#include <obdefs.h>
#include <gemdefs.h>
#include <define.h>
#include <osbind.h>
#include "wterm.h"

eraseWindow( handle, x, y, w, h)
int	x,y,w,h;
int	handle;
{
	int	pxy[4];
	pxy[0] = x;
	pxy[1] = y;
	pxy[2] = w+x;
	pxy[3] = h+y;

	v_bar( handle, pxy );
}

/* wshow - 
 * 	actual display of the whole window
 */
wshow(x,y,w,h)
int	x,y,w,h;
{
	register int	i, yy;

	eraseWindow(Handle,Ix,Iy,Iw,Ih-CharH);

	/*
	 * Scroll from bottom up...
	 * (looks nicer, they say)
	 */
	yy = Iy - Baseline-1 + (N_ROW+1)*CharH;
	for( i = N_ROW; i >= 0; i-- )
	{
		v_gtext( Handle, Ix, yy, &Screen[i][0] );
		yy -= CharH;
	}
}

/*
 * show_char
 *
 * Update screen by showing the latest character that has come in.
 */

show_char(c)
char c;
{
	int x, y, w, h;
	char s[2];

	/*
	 * stuff char into string
	 */
	s[0] = c;
	s[1] = EOS;
	
	v_gtext(Handle, Ix + Scr_col*CharW,
			Iy - Baseline - 1 + (Scr_row+1)*CharH,
			s);
}


/*
 * Cursor
 *
 * Either show or hide (toggle) cursor (steady block).
 */

Cursor()
{
	int wr_mode, pxy[8], src[10], des[10];

	/*
	 * Use raster operation to Invert pixels at current 
	 * location
	 */
	Raster( 12, Ix + Scr_col*CharW, Iy + Scr_row*CharH,
		Ix + Scr_col*CharW, Iy + Scr_row*CharH,
		CharW - 1, CharH - 1 );
}
Raster(wr_mode, xfrom, yfrom, xto, yto, w, h )
{
	register int i;
	int pxy[8], src[10], des[10];
	pxy[0] = xfrom;
	pxy[1] = yfrom;
	pxy[2] = xfrom+w;
	pxy[3] = yfrom+h;
	pxy[4] = xto;
	pxy[5] = yto;
	pxy[6] = xto+w;
	pxy[7] = yto+h;
	for( i=0; i<10; i++ ) {
		src[i] = 0;
		des[i] = 0;
	}
	vro_cpyfm( Handle, wr_mode, pxy, src, des );
}
