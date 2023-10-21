/* Copyright (c) 1985 by Thomas S. Fisher - Westminster, CO 80030 */

#include	<stdio.h>
#include	"emp.h"

char	Rscan[MAXSCN][2*MAXSCN]; 
char    Raddes[] = "-.^cudg*ihamwtfr+#)=!bxs?"; 
int	Nrows = 0;
int	Xrad, Yrad, Rngrad;
extern	struct	sector	s;
extern	FILE	*fdsec;

dorad()
{

	int	i, j, lcol, rcol, trow, brow;
	int	cencol, cenrow;
	int	row, col, n, x, y;
	char	c;

	cencol = cenrow = 0;
	for( i=0; i<Nrows; i++ ) {
		lcol = rcol = -1;
		for( j=0; j<MAXSCN*2; j += 2 ) {
			if( Rscan[i][j] != ' ' && Rscan[i][j] != '\0' ) break;
		}
		if( j < MAXSCN*2 ) {
			if( strspn(&Rscan[i][j], Raddes) == 1 ) lcol=j;
		}
		for( j=MAXSCN*2 - 2; j>=0; j -= 2 ) {
			if( Rscan[i][j] != ' ' && Rscan[i][j] != '\0' ) break;
		}
		if( j >= 0 ) {
			if( strspn(&Rscan[i][j], Raddes) == 1 ) rcol=j;
		}
		if( lcol >= 0 && rcol > 0 ) {
			if( ((rcol - lcol)%4) == 0 ) {
				cencol = (rcol - lcol)/2 + lcol;
				break;
			}
		}
	}
	if( cencol == 0 ) {
		Nrows = 0;
		return;
	}
	for( i=0; i<MAXSCN*2; i += 2 ) {
		trow = brow = -1;
		for( j=0; j<Nrows; j++ ) {
			if( Rscan[j][i] != ' ' && Rscan[j][i] != '\0' ) break;
		}
		if( j < Nrows ) {
			if( strspn(&Rscan[j][i], Raddes) == 1 ) trow=j;
		}
		for( j=Nrows-1; j>=0; j-- ) {
			if( Rscan[j][i] != ' ' && Rscan[j][i] != '\0' ) break;
		}
		if( j >= 0 ) {
			if( strspn(&Rscan[j][i], Raddes) == 1 ) brow=j;
		}
		if( trow >= 0 && brow > 0 ) {
			if( ((brow - trow)%2) == 0 ) {
				cenrow = (brow - trow)/2 + trow;
				break;
			}
		}
	}
	if( cenrow == 0 ) {
		Nrows = 0;
		return;
	}
	if( (cenrow-Rngrad+1) < 0 || (cenrow+Rngrad-1) >= Nrows ||
	    (cencol-2*Rngrad+2) < 0 || (cencol+2*Rngrad-2) >= 2*MAXSCN ) {
		Nrows = 0;
		return;
	}
	y = Yrad - Rngrad;
	for( row=cenrow-Rngrad; row<=cenrow+Rngrad; row++ ) {
		x = Xrad - Rngrad;
		for( col=cencol-2*Rngrad; col<=cencol+2*Rngrad; col += 2) {
			if( strspn(&Rscan[row][col], Raddes) == 1 ) {
				getsec( &s, (n = secno(x, y)) );
				c = Rscan[row][col];
				if( c == '?' ) {
					if( s.s_coun == 0 ) s.s_coun = 98;
					if( s.s_des == ' ' ) s.s_des = '?';
				} else {
					s.s_des = c;
				}
				putsec( &s, n );
			}
			x++;
		}
		y++;
	}
	Nrows = 0;
}
