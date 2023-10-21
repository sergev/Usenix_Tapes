#define D_NEWSVERBS
#define D_TRTYCLAUSE
#define D_TRTSTR
#define D_FILES
#include	"empdef.h"

trechk(anum, vnum, cond)
int	anum, vnum, cond;
{
	register	i, ret;
	char	*cp, *getstri();
	char	treaties[32];
	int	n;
	long	now;

	time(&now);
	ret = 0;
	n = N_VIOL_TRE;
	for( i=0; i < maxnoc; i++ ) {
		treaties[i] = 0;
	}
	for( i=0; gettre(i) != -1; i++ ) {
		if( trty.trt_cna == 0 ) continue;
		if( trty.trt_cnb == 0 ) continue;
		if( trty.trt_exp <  now ) continue;
		if( vnum != 0 ) goto X244;
		if( anum == trty.trt_cna ) goto X172;
		if( anum != -trty.trt_cna ) goto X202;
X172:	
		if( (cond & trty.trt_acond) != 0 ) goto X414;
X202:	
		if( anum == trty.trt_cnb ) goto X230;
		if( anum != -trty.trt_cnb ) continue;
X230:	
		if( (cond & trty.trt_bcond) != 0 ) goto X414;
		continue;
X244:	
		if( anum == trty.trt_cna ) goto X272;
		if( anum != -trty.trt_cna ) goto X330;
X272:	
		if( vnum == trty.trt_cnb ) goto X320;
		if( vnum != -trty.trt_cnb ) goto X330;
X320:	
		if( (cond & trty.trt_acond) != 0 ) goto X414;
X330:	
		if( anum == trty.trt_cnb ) goto X356;
		if( anum != -trty.trt_cnb ) continue;
X356:	
		if( vnum == trty.trt_cna ) goto X404;
		if( vnum != -trty.trt_cna ) continue;
X404:	
		if( (cond & trty.trt_bcond) == 0 ) continue;
X414:	
		switch( ret ) {
		case 0:
			ret = 1;
		case 1:
			printf("This action is in contravention of ");
			if( trty.trt_cna <  0 ||
			    trty.trt_cnb < 0 ) {
				printf("pending ");
			}
			printf(" treaty #%d\n", i);
			cp = getstri("Do you wish to go ahead with it anyway? (yes/no) ");
			if( *cp == 'n' ) {
				n = N_HONOR_TRE;
				ret = -1;
			}
		case -1:
			if( anum != trty.trt_cna ) goto X626;
			if( trty.trt_cnb <= 0 ) goto X626;
			treaties[trty.trt_cnb] = 1;
X626:	
			if( anum != trty.trt_cnb ) continue;
			if( trty.trt_cna >  0 ) goto X652;
			continue;
X652:	
			treaties[trty.trt_cna] = 1;
			continue;
		}
	}
	if( ret != 0 ) {
		n = N_HONOR_TRE;
		if( ret == 1 ) {
			n = N_VIOL_TRE;
		}
		for( i=0; i < maxnoc; i++ ) {
			if( treaties[i] == 0 ) continue;
			nreport(anum, n, i);
		}
	}
	return(ret);
}
