#define D_TRTSTR
#define D_TCHRSTR
#define D_FILES
#include	"empdef.h"

offe()
{
	register char	*cp;
	register	tno, i;
	char	*getstri(), *cname();
	int	arg, acnd, vcnd;
	long	now;

	arg = natarg(argp[1], "Treaty offered to ?");
	if( arg == -1 ) return(SYN_RETURN);
	for( tno = 0; (i = (gettre(tno) == -1) ? 0 : 1); tno++ ) {
		if( trty.trt_cna == 0 ) break;
	}
	if( i == -1 ) {
		printf("error in treaty file");
		return(SYN_RETURN);
	}
	printf("Terms for %s :\n", cname(arg));
	vcnd = 0;
	i = 0;
	do {
		sprintf(fmtbuf,"%s? ", tchr[i].t_name);
		cp = getstri(fmtbuf);
		if( *cp != 'y' ) continue;
		vcnd |= tchr[i].t_cond;
	} while( ++i < 8 );
	printf("Terms for you :\n");
	acnd = 0;
	for( i=0; i < 8; i++ ) {
		sprintf(fmtbuf,"%s? ", tchr[i].t_name);
		cp = getstri(fmtbuf);
		if( *cp != 'y' ) continue;
		acnd |= tchr[i].t_cond;
	}
	i = atopi(getstri("Proposed treaty duration? (days) "));
	time(&now);
	trty.trt_exp = 86400L * i + now;
	trty.trt_cna = cnum;
	trty.trt_cnb = -arg;
	trty.trt_acond = acnd;
	trty.trt_bcond = vcnd;
	sprintf(fmtbuf,"Treaty #%d proposed to you by %s", tno, cname(cnum));
	wu(0, arg, fmtbuf);
	puttre(tno);
	printf("You have proposed treaty #%d\n", tno);
	return(NORM_RETURN);
}
