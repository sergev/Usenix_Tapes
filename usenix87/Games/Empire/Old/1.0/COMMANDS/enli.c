#define D_UPDATE
#define D_SECTDES
#define D_NEWSVERBS
#define D_TRTYCLAUSE
#define D_SCTSTR
#define D_TRTSTR
#define	D_NSCSTR
#define D_FILES
#include	"empdef.h"

enli()
{
	register	q, n;
	float	tot;
	struct	nstr	nsct;

	if( snxtsct(&nsct, argp[1]) == -1 ) return(SYN_RETURN);
	if( trechk(cnum, 0, TRTENL) == -1 ) return(FAIL_RETURN);
	q = onearg(argp[2], "Number enlisted : ");
	tot = 0;
	while( nxtsct(&nsct, UP_OWN) >  0 ) {
		if( owner == 0 ) continue;
		if( sect.sct_desig == S_URBAN ) continue;
		n = sect.sct_civil;
		n >>= 1;
		if( n > q ) n = q;
		if( n > 127 - sect.sct_milit ) {
			n = 127 - sect.sct_milit;
		}
		if( n == 0 ) continue;
		sect.sct_civil -= n;
		sect.sct_milit += n;
		printf("%3d enlisted in %d,%d (%d)\n", n, nsct.n_x, nsct.n_y, sect.sct_milit);
		putsect(nsct.n_x, nsct.n_y);
		tot += n;
	}
	printf("Total new enlistment : %.0f\n", tot);
	printf("Paperwork at recruiting stations ... %.1f\n", tot / 50.);
	ntused = (tot / 50.) + ntused + .5;
	return(NORM_RETURN);
}
