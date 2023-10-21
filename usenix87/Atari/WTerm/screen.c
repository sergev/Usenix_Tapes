#include <obdefs.h>
#include <gemdefs.h>
#include <define.h>
#include <osbind.h>
#include "wterm.h"

/*
 * Init_Scr
 *
 * Initialize all the stuff associated with the screen:
 *
 * Clear the screen, reset row and column, etc.
 */

Init_Scr()
{
	register int i, j;
	
	for( i=0; i<N_ROW; i++ ) {
		for( j=0; j<N_COL; j++ ) {
			Screen[i][j] = ' ';
		}
		Screen[i][N_COL] = EOS;
	}

	Scr_col = 0;
	Scr_row = 0;
}

/*
 * proc
 *
 * Process a character that is going out onto the screen.
 */

proc(c)
char c;
{
	if( command == TRUE ) {
		do_comd(c);
		return;
	} else if( comd_pos > 0 ) {
		do_pos(c);
		comd_pos--;
		return;
	}
	if( Sysline == TRUE && c != (char) 27 ) {
		show_char(c);
		Scr_col++;
		if( Scr_col >= N_COL )
			Scr_col = N_COL - 1;
		return;
	}

	/*
	 * There are two distinct modes:  ALPHA and Graphics, the latter
	 * emulating a Tektronix 4010 mode.
	 */
	
	if( GRAPH == T_mode ) {
		tek(c);		/* process as if this were a TEK 4010 */
		return;
	}
	switch( c ) {

	case '\r':	Scr_col = 0;				break;
	case '\n':	Scr_row++;
			if( Scr_row >= N_ROW )
				Scroll();
			break;
	case 7:		Cconout( 7 );				break;
	case 8:		Scr_col--;
			if( Scr_col < 0 )
				Scr_col = 0;
			break;
	case '\t':	Scr_col = 8 * ( Scr_col/8 + 1 );
			if( Scr_col >= N_COL ) {
				Scr_col = 0;
				Scr_row++;
				if( Scr_row >= N_ROW )
					Scroll();
			}
			break;
	case 27:	command = TRUE;				break;
	case GS:	T_mode = GRAPH;
			T_draw = FALSE;
			T_count = 0;
			break;
	case US:	T_mode = ALPHA;				break;
	default:	
			if( c > (char)31 ) {
				Screen[Scr_row][Scr_col] = c;
				show_char(c);
				Scr_col++;
				if( Scr_col >= N_COL ) {
					Scr_col = 0;
					Scr_row++;
					if( Scr_row >= N_ROW )
						Scroll();
				}
			}
			break;
	}
}

/*
 * do_comd
 *
 * a terminal command.
 */

do_comd(c)
char c;
{
	switch(c) {
	case 'A':	Scr_row = (Scr_row > 0) ? --Scr_row : 0; break;
	case 'B':	Scr_row = (Scr_row < N_ROW-1) ? ++Scr_row : N_ROW-1;
			break;
	case 'C':	if( Scr_col < N_COL-1 ) {
				Scr_col++;
			} else {
				Scr_col = 0;
				Scr_row++;
				if( Scr_row >= N_ROW )
					Scroll();
			}
			break;
	case 'D':	Scr_col = (Scr_col > 0) ? --Scr_col : 0; break;
	case FF:	/* for TEK mode ... */
	case 'E':	Clear();				break;
	case 'H':	Scr_col = Scr_row = 0;			break;
	case 'J':	EOP_Clear();				break;
	case 'K':	EOL_Clear();				break;
	case 'L':	Ins_Line();				break;
	case 'M':	Del_Line();				break;
	case 'N':	Sysl_On();				break;
	case 'O':	Sysl_Off();				break;
	case 'Y':	comd_pos = 2;				break;
	case 'l':	Ins_Char();				break;
	case 'm':	Del_Char();				break;
	case 'p':	vst_effects(Handle,2);
			break;
	case 'q':	vst_effects(Handle,0);
			break;
	default:	break;
	}
	command = FALSE;
}

do_pos(c)
register char c;
{
	if( comd_pos == 2 ) {
		Scr_row = (int) (c - ' ');
		if( Scr_row >= N_ROW )
			Scr_row = N_ROW-1;
	} else {
		Scr_col = (int) (c - ' ');
		if( Scr_col >= N_COL )
			Scr_col = N_COL-1;
	}
}
/*
 * Scroll
 *
 * Scroll in a new line (do this the dumb way for now).
 */

Scroll()
{
	register int i, j;
	int x,y,w,h;
	register char *from, *to;

	int wr_mode, pxy[8], src[10], des[10];

	from = &Screen[1][0];
	to = &Screen[0][0];
	for( i=1; i<N_ROW; i++, to++, from++ )
		for( j=0; j<N_COL; j++ )
			*to++ = *from++;

	Scr_row = N_ROW - 1;

	for( j=0; j<N_COL; j++ )
		Screen[Scr_row][j] = ' ';

	/*
	 * Try using a raster operation to scroll... ha ha
	 */
	wr_mode = 3;
	pxy[0] = Ix;
	pxy[1] = Iy + CharH;
	pxy[2] = Ix + N_COL * CharW;
	pxy[3] = Iy + N_ROW * CharH;
	pxy[4] = Ix;
	pxy[5] = Iy;
	pxy[6] = Ix + N_COL * CharW;
	pxy[7] = Iy + (N_ROW-1) * CharH;
	for( i=0; i<10; i++ ) {
		src[i] = 0;
		des[i] = 0;
	}
	vro_cpyfm( Handle, wr_mode, pxy, src, des );
	eraseWindow( Handle, Ix, Iy+(N_ROW-1)*CharH,
			N_COL*CharW, CharH );
}

Clear()
{
	Init_Scr();
	eraseWindow( Handle, Ix, Iy, Iw, Ih-CharH );
}

EOP_Clear()
{
	register int i, j;
	EOL_Clear();
	i = Scr_row++;
	j = Scr_col;
	Scr_col = 0;
	for( ; Scr_row < N_ROW; Scr_row++ )
		EOL_Clear();
	Scr_row = i;
	Scr_col = j;
}

EOL_Clear()
{
	register int i;
	for( i=Scr_col; i<N_COL; i++ )
		Screen[Scr_row][i] = ' ';
	eraseWindow( Handle, Ix+Scr_col*CharW, Iy+Scr_row*CharH,
			(N_COL-Scr_col)*CharW, CharH );
}

Ins_Line()
{
	register int i, j;
	register char *p, *q;
	int wr_mode, pxy[8], src[10], des[10];

	for( i=N_ROW-1; i>Scr_row; i-- ) {
		p = &Screen[i][0];
		q = &Screen[i-1][0];
		for( j=0; j<N_COL; j++ )
			*p++ = *q++;
	}
	/*
	 * Try using a raster operation to scroll... ha ha
	 */
	wr_mode = 3;
	pxy[0] = Ix;
	pxy[1] = Iy + Scr_row * CharH;
	pxy[2] = Ix + N_COL * CharW;
	pxy[3] = Iy + (N_ROW-1) * CharH;
	pxy[4] = Ix;
	pxy[5] = Iy + (Scr_row+1) * CharH;
	pxy[6] = Ix + N_COL * CharW;
	pxy[7] = Iy + N_ROW * CharH;
	for( i=0; i<10; i++ ) {
		src[i] = 0;
		des[i] = 0;
	}
	vro_cpyfm( Handle, wr_mode, pxy, src, des );

	Scr_col = 0;
	EOL_Clear();
}

Del_Line()
{
	register int i, j;
	register char *p, *q;
	int wr_mode, pxy[8], src[10], des[10];
	
	for( i=Scr_row; i<N_ROW; i++ ) {
		p = &Screen[i][0];
		q = &Screen[i+1][0];
		for( j=0; j<N_COL; j++ )
			*p++ = *q++;
	}
	/*
	 * Try using a raster operation to scroll... ha ha
	 */
	wr_mode = 3;
	pxy[0] = Ix;
	pxy[1] = Iy + (Scr_row+1) * CharH;
	pxy[2] = Ix + N_COL * CharW;
	pxy[3] = Iy + N_ROW * CharH;
	pxy[4] = Ix;
	pxy[5] = Iy + Scr_row * CharH;
	pxy[6] = Ix + N_COL * CharW;
	pxy[7] = Iy + (N_ROW-1) * CharH;
	for( i=0; i<10; i++ ) {
		src[i] = 0;
		des[i] = 0;
	}
	vro_cpyfm( Handle, wr_mode, pxy, src, des );

	Scr_col = 0;
	i = Scr_row;
	Scr_row = N_ROW - 1;
	EOL_Clear();
	Scr_row = i;
}

Del_Char()
{
	register int i, j;
	register char *p, *q;
	
	p = &Screen[Scr_row][Scr_col];
	q = &Screen[Scr_row][Scr_col+1];
	for( i=Scr_col; i<N_COL-1; i++ )
			*p++ = *q++;

	/* Shift chars over */
	Raster( 3, Ix+(Scr_col+1)*CharW, Iy+Scr_row*CharH,
		Ix+Scr_col*CharW, Iy+Scr_row*CharH,
		(N_COL-Scr_col-1)*CharW, CharH );
	/* Clear last byte */
	Raster( 0, Ix+(N_COL-1)*CharW, Iy+Scr_row*CharH,
		Ix+(N_COL-1)*CharW, Iy+Scr_row*CharH,
		CharW, CharH );
}
Ins_Char()
{
	register int i, j;
	register char *p, *q;
	
	p = &Screen[Scr_row][Scr_col+1];
	q = &Screen[Scr_row][Scr_col];
	for( i=N_COL-1; i>Scr_col; i-- )
			*p++ = *q++;

	Raster( 3, Ix+Scr_col*CharW, Iy+Scr_row*CharH,
		Ix+(Scr_col+1)*CharW, Iy+Scr_row*CharH,
		(N_COL-Scr_col-1)*CharW, CharH );
	/* Clear current byte */
	Raster( 0, Ix+Scr_col*CharW, Iy+Scr_row*CharH,
		Ix+Scr_col*CharW, Iy+Scr_row*CharH,
		CharW, CharH );
}

Sysl_On()
{
	Sysline = TRUE;
	Sysl_row = Scr_row;
	Sysl_col = Scr_col;
	Scr_row = N_ROW;	/* Sysline is on bottom row */
	Scr_col = 0;
	Clear_Sysl();
	vst_effects(Handle,2);
}

Sysl_Off()
{
	vst_effects(Handle,0);
	Scr_row = Sysl_row;
	Scr_col = Sysl_col;
	Sysline = FALSE;
}

Clear_Sysl()
{
	Raster( 0, 0, 0, Ix, Iy+N_ROW*CharH, N_COL*CharW, CharH );
}
