/* Copyright (c) 1985 by Thomas S. Fisher - Westminster, CO 80030 */

#include	"emp.h"

extern	struct 	sector	s;

dospy(line)
char	*line;
{
	int	x, y, n;

	if( sscanf(line," %d,%d", &x, &y ) != 2 ) return;
	getsec( &s, (n = secno(x,y)) );
	if( sscanf(&line[7]," %d %*c %d%% %d %d %d %d %d %d",
		&s.s_coun,
		&s.s_eff,
		&s.s_rsrc[CIV],		&s.s_rsrc[MIL],
		&s.s_rsrc[SHL],		&s.s_rsrc[GUN],
		&s.s_rsrc[ORE],		&s.s_rsrc[PLN] ) != 8 ) return;
	s.s_des = line[11];
	s.s_eff *= 2;
	s.s_rsrc[CIV] *= 2;
	s.s_rsrc[MIL] *= 2;
	s.s_rsrc[SHL] *= 2;
	s.s_rsrc[GUN] *= 2;
	s.s_rsrc[PLN] *= 2;
	s.s_rsrc[ORE] *= 2;
	putsec( &s, n );
}
