#define	D_SCTSTR
#define	D_UPDATE
#define	D_FILES
#include	"empdef.h"

neigh(x, y, cno, dflg)
int	x, y, cno, dflg;
{
	register	dx, dy;

	for( dx = -1; dx <= 1; dx++ ) {
		for( dy = -1; dy <= 1; dy++ ) {
			if( dx == 0 && dy == 0 ) continue;
			if( dflg != 0 && (dx * dy) != 0 ) continue;
			if( getsect(x + dx, y + dy, UP_NONE) == -1 ) continue;
			if( cno != sect.sct_owned ) continue;
			nbrx = x + dx;
			nbry = y + dy;
			getsect(x, y, UP_OWN);
			return(1);
		}
	}
	getsect(x, y, UP_NONE);
	return(0);
}
