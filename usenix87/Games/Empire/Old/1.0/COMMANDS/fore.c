#define D_SECTDES
#define D_SCTSTR
#define D_FILES
#include	"empdef.h"

fore()
{
	register	i, j, dh;
	int	k, hi, lo;
	double	r;
	char	 *splur();

	if( sargs(argp[1]) != -1 ) goto X36;
	return(SYN_RETURN);
X36:	
	if( getsno(argp[2], "weather station at ") == -1 ) goto X70;
	if( owner != 0 ) goto X100;
X70:	
	printf("not yours");
	return(SYN_RETURN);
X100:	
	if( sect.sct_desig == S_WETHR) goto X120;
	printf("not a weather station, klod");
	return(SYN_RETURN);
X120:	
	r = sect.sct_effic / 13.5;
	printf("efficiency %d%% range %.1f\n", sect.sct_effic, r);
	r = r*r;
	dh = onearg(argp[3], "forecast for how many hours in the future? ");
	if( dh <= sect.sct_civil ) goto X242;
	printf("too small a staff to predict that far");
	return(FAIL_RETURN);
X242:	
	wethr(0, 0, dh);
	lo = wethr(wxl, wyl, dh);
	hi = wethr(wxh, wyh, dh);
	printf("in %d hour%s there will be a", dh, splur(dh));
	printf((lo<600) ? "n extreme" : ((lo<660)?"":" mild"));
	printf(" low pressure center at %d,%d and\n", wxl, wyl);
	printf((hi>900) ? "an extreme" : ((hi>840)?"":"a mild"));
	printf(" high at %d,%d\n", wxh, wyh);
	wethr(0, 0, dh + 1);
	printf("an hour later the low should be at %d,%d ", wxl, wyl);
	printf("and the high at %d,%d\n", wxh, wyh);
	i = lx;
	goto X726;
X670:	
	printf(" %d", i / 10);
	i += ix;
X726:	
	if( i != hx ) goto X670;
	printf("\n");
	i = lx;
	goto X1010;
X752:	
	printf(" %d", i % 10);
	i += ix;
X1010:	
	if( i != hx ) goto X752;
	printf("\n");
	j = ly;
	goto X1350;
X1034:	
	i = lx;
	goto X1314;
X1042:	
	if( r >= idsq(i - sx, j - sy) ) goto X1110;
	printf("  ");
	goto X1310;
X1110:	
	k = wethr(i, j, dh);
	k = min(max((k-741)/12, -9), 9);
	printf("%2d", k);
X1310:	
	i += ix;
X1314:	
	if( i != hx ) goto X1042;
	printf("%3d\n", j);
	j += iy;
X1350:	
	if( j != hy ) goto X1034;
	return(NORM_RETURN);
}
