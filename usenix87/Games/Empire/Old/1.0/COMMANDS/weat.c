#include	"empdef.h"

weat()
{
	register	i, j, k;
	int	lo, hi;

	if( sargs(argp[1]) == -1 ) return(SYN_RETURN);
	printf("    ");
	for( i = lx; i != hx; i += ix ) {
		if( i < 0 && i > -10 ) {
			printf(" -");
		} else {
			printf(" %d", ((i < 0 ) ? -i : i)/10);
		}
	}
	printf("\n    ");
	for( i = lx; i != hx; i += ix ) {
		printf(" %d", ((i < 0) ? -i : i)%10);
	}
	printf("\n");
	for( j = ly; j != hy; j += iy ) { 
		printf("%3d ", j);
		for( i = lx; i != hx; i += ix ) {
			k = wethr(i, j, 0);
			k = min(max((k-741)/12, -9), 9);
			printf("%2d", k);
		}
		printf("%4d\n", j);
	}
	printf("    ");
	for( i = lx; i != hx; i += ix ) {
		if( i < 0 && i > -10 ) {
			printf(" -");
		} else {
			printf(" %d", ((i < 0 ) ? -i : i)/10);
		}
	}
	printf("\n    ");
	for( i = lx; i != hx; i += ix ) {
		printf(" %d", ((i < 0) ? -i : i)%10);
	}
	printf("\n");
	lo = wethr(wxl, wyl, 0);
	hi = wethr(wxh, wyh, 0);
	printf( (lo<600)?"extreme":((lo<660)?"":"mild") );
	printf(" low pressure center at %d,%d; ", xwrap(wxl), ywrap(wyl));
	printf( (hi>900)?"extreme":((hi>840)?"":"mild") );
	printf(" high at %d,%d\n", xwrap(wxh), ywrap(wyh));
	return(NORM_RETURN);
}
