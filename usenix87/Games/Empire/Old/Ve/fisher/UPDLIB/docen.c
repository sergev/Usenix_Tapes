/* Copyright (c) 1985 by Thomas S. Fisher - Westminster, CO 80030 */

#include	<stdio.h>
#include	"emp.h"

char	Stype[] = { 'p', 'm', 'd', 's', 'f', 't', 'b', 'c' };
extern	struct	sector	s;

docen(line)
char	*line;
{
		int	x, y, n;

		if( sscanf(line," %d,%d ", &x, &y) != 2 ) {
			fprintf(stderr,"empupd: bad x,y\n");
			return(ERROR);
		}
                if( getsec(&s, (n = secno(x,y))) == ERROR ) { 
			fprintf(stderr,"getsec: ERROR\n");
			return(ERROR);
		}
		s.s_coun = 0;
		s.s_ckpt = line[7];
		setdel( line[8], &s.s_del[0] );
                setdel( line[9], &s.s_del[1] );
                setdel( line[10], &s.s_del[2] );
                setdel( line[11], &s.s_del[3] );
                setdel( line[12], &s.s_del[4] );
                setdel( line[13], &s.s_del[5] );
                setdel( line[14], &s.s_del[6] );
		s.s_def  = line[15]; 
		s.s_des  = line[17]; 
		sscanf(&line[18],"%d%*c %d %d %d %d %d %d %d %d %d %d %d",
		&s.s_eff, &s.s_min, &s.s_gold, &s.s_mob,
		&s.s_rsrc[0], &s.s_rsrc[1], &s.s_rsrc[2], &s.s_rsrc[3],
		&s.s_rsrc[4], &s.s_rsrc[5], &s.s_rsrc[6], &s.s_prod);
		if( putsec(&s, n) == ERROR ) {
			printf(stderr,"putsec: ERROR\n");
			return(ERROR);
		}
}

setdel( dchar, sdel )
char	dchar, *sdel;
{
	*sdel &= 0360;
	switch( dchar ) {
	case '.':
		break;
	case '$':
		*sdel |= 01;
		break;
	default:
		*sdel |= ((dchar - '0')&017) | 010;
		break;
	}
}
