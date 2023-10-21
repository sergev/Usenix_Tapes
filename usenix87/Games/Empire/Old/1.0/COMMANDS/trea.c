#define D_UPDATE
#define D_NATSTAT
#define D_NEWSVERBS
#define D_TRTYCLAUSE
#define D_TRTSTR
#define D_TCHRSTR
#define D_FILES
#include	"empdef.h"

trea()
{
	register	i, j, k;
	char	 *ctime(), *cname();
	int	a, b;
	long	now;

	time(&now);
	for( i=0; gettre(i) != -1; i++ ) {
		if( trty.trt_cna == 0 ) continue;
		if( cnum != trty.trt_cna &&
		    cnum != -trty.trt_cna &&
		    cnum != trty.trt_cnb &&
		    cnum != -trty.trt_cnb &&
		    nstat != STAT_GOD ) continue;
		if( now > trty.trt_exp ) continue;
		printf("\n  *** Empire Treaty #%d ***\n", i);
		if( trty.trt_cna <  0 ||
		    trty.trt_cnb < 0 ) {
			printf("(proposed)\n");
		}
		trty.trt_cna = (trty.trt_cna < 0) ? -trty.trt_cna : trty.trt_cna;
		trty.trt_cnb = (trty.trt_cnb < 0) ? -trty.trt_cnb : trty.trt_cnb;
		printf("between %s and ", cname(trty.trt_cna));
		printf("%s  expires %s", cname(trty.trt_cnb), ctime(&trty.trt_exp));
		printf("%10.10s terms ", cname(trty.trt_cna));
		printf("%10.10s terms\n", cname(trty.trt_cnb));
		j = 0;
		do {
			k = tchr[j].t_cond;
			b = trty.trt_bcond & k;
			a = trty.trt_acond & k;
			if( a != 0 || b != 0 ) {
				if( a != 0 ) {
					printf("%-20s", tchr[j].t_name);
				} else {
					printf("%20s", "");
				}
				if( b != 0 ) {
					printf("%-20s\n", tchr[j].t_name);
				} else {
					printf("\n");
				}
			}
		} while( ++j < 8 );
	}
	return(NORM_RETURN);
}
