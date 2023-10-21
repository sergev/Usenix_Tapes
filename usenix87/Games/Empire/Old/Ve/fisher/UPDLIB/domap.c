/* Copyright (c) 1985 by Thomas S. Fisher - Westminster, CO 80030 */

#include	"emp.h"

extern	struct	sector	s;

domap(mapx, y, col, mapchar)
int	mapx, y, col;
char	mapchar;
{
	int	x, n;

	x = (col - 6)/2 + mapx;
	getsec(&s, (n = secno(x, y)));
	if( mapchar == '?' ) {
		if( s.s_des == ' ' || s.s_des == '\0' ) s.s_des = '?';
		if( s.s_coun == 0 ) s.s_coun = 98;
	} else {
		s.s_des = mapchar;
	}
	putsec(&s, n);
}
