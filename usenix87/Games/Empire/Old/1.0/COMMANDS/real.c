#define D_NATSTR
#define D_FILES
#include	"empdef.h"

real()
{
	register struct	boundstr	*nrp, *bp;
	int	n;

	if( argp[1] == 0 ) {
		printf("Specify a realm number");
		return(SYN_RETURN);
	}
	if( (n = atoi(argp[1])) < 0 || n > 3 ) {
		printf("Realm number must be in the range 0:3");
		return(SYN_RETURN);
	}
	nrp = &nrealm[n];
	if( argp[2] == 0 ) {
		printf("Realm #%d is %d:%d,%d:%d\n", n, nrp->b_xl, nrp->b_xh, nrp->b_yl, nrp->b_yh);
		return(NORM_RETURN);
	}
	if( sargs(argp[2]) == -1 ) return(SYN_RETURN);
	bp = &nat.nat_b[n];
	getnat(cnum);
	bp->b_xl = nrp->b_xl = lx;
	bp->b_xh = nrp->b_xh = hx - ix;
	bp->b_yl = nrp->b_yl = ly;
	bp->b_yh = nrp->b_yh = hy - iy;
	putnat(cnum);
	return(NORM_RETURN);
}
