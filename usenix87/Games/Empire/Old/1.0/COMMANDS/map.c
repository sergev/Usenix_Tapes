#define D_UPDATE
#define D_SECTDES
#define D_SCTSTR
#define D_DCHRSTR
#define D_FILES
#include	"empdef.h"

map()
{
	register	i, j, bit;
	int	bitmap[1024];
	int	ybit;

	if( sargs(argp[1]) != -1 ) goto X36;
	return(SYN_RETURN);
X36:	
	i = 1024;
	while( --i >= 0 ) bitmap[i] = 0;
	j = ly;
	goto X1464;
X242:	
	ybit = j<<07;
	i = lx;
	goto X1446;
X264:	
	if( getsect(i, j, UP_NONE) >= 0 ) goto X310;
	goto X1442;
X310:	
	if( owner != 0 ) goto X322;
	goto X1442;
X322:	
	bit = i;
	bit += ybit;
	setbit(bit, bitmap, 01);
	setbit(bit - 1, bitmap, 01);
	setbit(bit - 128, bitmap, 01);
	setbit(bit + 128, bitmap, 01);
	setbit(bit + 1, bitmap, 01);
	if( sect.sct_effic >= 20 ) goto X512;
	goto X1442;
X512:	
	setbit(bit - 129, bitmap, 01);
	setbit(bit + 127, bitmap, 01);
	setbit(bit - 127, bitmap, 01);
	setbit(bit + 129, bitmap, 01);
	if( sect.sct_effic >= 40 ) goto X656;
	goto X1442;
X656:	
	setbit(bit - 2, bitmap, 01);
	setbit(bit - 256, bitmap, 01);
	setbit(bit + 256, bitmap, 01);
	setbit(bit + 2, bitmap, 01);
	if( sect.sct_effic >= 60 ) goto X1022;
	goto X1442;
X1022:	
	setbit(bit - 130, bitmap, 01);
	setbit(bit + 126, bitmap, 01);
	setbit(bit - 257, bitmap, 01);
	setbit(bit + 255, bitmap, 01);
	setbit(bit - 255, bitmap, 01);
	setbit(bit + 257, bitmap, 01);
	setbit(bit - 126, bitmap, 01);
	setbit(bit + 130, bitmap, 01);
	if( sect.sct_effic < 80 ) goto X1442;
	setbit(bit - 258, bitmap, 01);
	setbit(bit + 254, bitmap, 01);
	setbit(bit - 254, bitmap, 01);
	setbit(bit + 258, bitmap, 01);
X1442:	
	i += ix;
X1446:	
	if( i == hx ) goto X1460;
	goto X264;
X1460:	
	j += iy;
X1464:	
	if( j == hy ) goto X1476;
	goto X242;
X1476:	
	border(lx, hx, ix, "    ");
	j = ly;
	goto X1646;
X1534:	
	printf("%3d ", j);
	ybit = j<<07;
	i = lx;
	goto X1612;
X1576:	
	printf("  ");
X1602:	
	i += ix;
X1612:	
	if( i != hx ) goto X1712;
	printf(" %d\n", j);
	j += iy;
X1646:	
	if( j != hy ) goto X1534;
	border(lx, hx, ix, "    ");
	return(NORM_RETURN);
X1712:	
	if( getbit(i + ybit, bitmap) == 0 ) goto X1576;
	getsect(i, j, UP_NONE);
	switch( sect.sct_desig ) {
	case S_WATER:
		printf(" .");
		goto X1602;
	case S_MOUNT:
		printf(" ^");
		goto X1602;
	case S_SANCT:
		printf(" s");
		goto X1602;
	case S_RURAL:
		if( sect.sct_owned == 0 ) goto X2032;
		if( owner == 0 ) goto X2040;
X2032:	
		printf(" -");
		goto X1602;
X2040:	
		printf(" ?");
		goto X1602;
	}
	if( sect.sct_desig > S_XCHNG ) {
		printf("??");
		goto X1602;
	}
	if( owner == 0 ) goto X2132;
	printf(" %c", dchr[sect.sct_desig].d_mnem);
	goto X1602;
X2132:	
	printf(" ?");
	goto X1602;
}

border(lowx, hix, incx, in)
int	lowx, hix, incx;
char	*in;
{
	register	i;

	printf("%s", in);
	i = lowx;
	goto X120;
X22:	
	if( i >= 0 ) goto X46;
	if( i <= -10 ) goto X46;
	printf(" -");
	goto X114;
X46:	
	printf(" %d", ((i >= 0) ? i : -i)/10);
X114:	
	i += incx;
X120:	
	if( i != hix ) goto X22;
	printf("\n");
	printf("%s", in);
	i = lowx;
	goto X226;
X154:	
	printf(" %d", ((i >= 0) ? i : -i)%10);
	i += incx;
X226:	
	if( i != hix ) goto X154;
	printf("\n");
	return;
}
