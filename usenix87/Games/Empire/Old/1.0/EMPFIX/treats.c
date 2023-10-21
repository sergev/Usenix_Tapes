#define	D_TRTSTR
#define	D_FILES
#include	"empdef.h"

extern int	xflg, wflg, mflg;

treats()
{
	register	i, k;
	char	*cp, *getstri();

	trtf = open(treatfil, O_RDWR);
X30:	
	wflg = xflg = mflg = 0;
	cp = getstri("#? ");
	if( *cp != '\0' ) goto X106;
	close(trtf);
	return;
X106:	
	i = atoi(cp);
	k = i * sizeof(trty);
	lseek(trtf, (long)k, 0);
	i = read(trtf, &trty, sizeof(trty));
	if( i >= sizeof(trty) ) goto X216;
	printf("Only %d bytes in that one...\n", i);
X216:	
	bytefix("cna", &trty.trt_cna, 0);
	bytefix("cnb", &trty.trt_cnb, 0);
	bytefix("acond", &trty.trt_acond, 0);
	bytefix("bcond", &trty.trt_bcond, 0);
	longfix("exp", &trty.trt_exp, 0L);
	if( mflg == 0 ) goto X30;
	lseek(trtf, (long)k, 0);
	write(trtf, &trty, sizeof(trty));
	printf("Rewritten\n");
	goto X30;
}
