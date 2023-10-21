#define D_UPDATE
#define D_NATSTR
#define D_FILES
#include	"empdef.h"

upda()
{
	register	i, j, jnum;
	int	num, mode;
	char	 *splur();

	i = 1;
	if( argp[i] == '\0' ) goto X42;
	if( argp[i][0] < 'q' ) goto X164;
X42:	
	hx = w_xsize / 2;
	lx = -hx;
	hy = w_ysize / 2;
	ly = -hy;
	ix = iy = 1;
X126:	
	if( argp[i] == '\0' ) goto X220;
	if( argp[i][0] != 'v' ) goto X220;
	mode = (UP_VERBOSE | UP_TIME | UP_OWN);
	goto X264;
X164:	
	if( sargs(argp[i++]) != -1 ) goto X126;
	return(SYN_RETURN);
X220:	
	if( argp[i] == '\0' ) goto X256;
	if( argp[i][0] != 'q' ) goto X256;
	mode = (UP_QUIET | UP_OWN);
	goto X264;
X256:	
	mode = (UP_GOD | UP_OWN);
X264:	
	nrealm[0].b_xl = nrealm[0].b_yl = 127;
	nrealm[0].b_xh = nrealm[0].b_yh = -127;
	num = 0;
	for( j = ly; j != hy; j += iy ) {
		jnum = 0;
		for( i = lx; i != hx; i += ix ) {
			getsect(i, j, mode);
			if( owner == 0 ) continue;
			if( i < nrealm[0].b_xl ) nrealm[0].b_xl = i; 
			if( i > nrealm[0].b_xh ) nrealm[0].b_xh = i;
			jnum++;
		}
		if( jnum == 0 ) continue;
		if( j < nrealm[0].b_yl ) nrealm[0].b_yl = j;
		if( j > nrealm[0].b_yh ) nrealm[0].b_yh = j;
		num += jnum;
	}
	printf("Your \"realm 0\" (#0) is %d:%d,%d:%d", nrealm[0].b_xl, nrealm[0].b_xh, nrealm[0].b_yl, nrealm[0].b_yh);
	printf(" and consists of %d sector%s.\n", num, splur(num));
	getnat(cnum);
	nat.nat_b[0].b_xl = nrealm[0].b_xl;
	nat.nat_b[0].b_xh = nrealm[0].b_xh;
	nat.nat_b[0].b_yl = nrealm[0].b_yl;
	nat.nat_b[0].b_yh = nrealm[0].b_yh;
	putnat(cnum);
	return(NORM_RETURN);
}
