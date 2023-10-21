#define	D_NSCSTR
#define	D_SHPSTR
#define	D_MCHRSTR
#define	D_ICHRSTR
#define	D_SCTSTR
#define	D_SECTDES
#define	D_UPDATE
#define	D_FILES
#include	"empdef.h"

set()
{	register char	*cp, c;
	char	*dp, *getstri();
	int	i, ptyp, maxprice;
	double	price;
	struct	nbstr	nb;
	struct	ichrstr	*ip;
	struct	nstr	nsct;

	if( landorsea(argp[1]) != LAND ) goto X30;
	goto X414;
X30:	
	if( snxtshp(&nb, argp[1], cnum, "Ship(s)? ") == -1 ) return(SYN_RETURN);
	goto X242;
X100:	
	printf("not for sale at the moment.");
X110:	
	cp = getstri("  New price? ");
	if( *cp == '\0' ) goto X140;
	ship.shp_spric = atopi(cp);
X140:	
	if( ship.shp_spric == 0 ) goto X214;
	printf("Asking price is recorded as $%d.\n", mchr[ship.shp_type].m_prdct * ship.shp_spric);
	goto X224;
X214:	
	printf("not for sale.\n");
X224:	
	putship(nb.nb_sno, &ship);
X242:	
	if( nxtshp(&nb, &ship) == 0 ) goto X410;
	printf("%s #%d is ", mchr[ship.shp_type].m_name, nb.nb_sno);
	if( ship.shp_spric == 0 ) goto X100;
	printf("valued at $%d/ton.  ($%d total)", ship.shp_spric, mchr[ship.shp_type].m_prdct * ship.shp_spric);
	goto X110;
X410:	
	return(NORM_RETURN);
X414:	
	if( snxtsct(&nsct, argp[1]) == -1 ) return(SYN_RETURN);
	if( argp[2] != 0 ) {
		for( ptyp = 14; (c = ichr[ptyp].i_mnem) != '\0'; ptyp++ ) {
			if( c == argp[2][0] ) break;
		}
		if( c == '\0' ) {
			printf("'%s' can't be sold", argp[2]);
			return(SYN_RETURN);
		}
	} else {
		ptyp = 0;
	}
	dp = &sect.sct_c_use;
X556:	
	if( nxtsct(&nsct, UP_OWN) <= 0 ) goto X410;
	if( owner == 0 ) goto X556;
	if( sect.sct_desig != S_XCHNG ) goto X556;
	i = 2;
X626:	
	if( (ip = &ichr[i + 12])->i_mnem == '\0' ) goto X556;
	if( ptyp == 0 ) goto X674;
	if( i == ptyp ) goto X674;
	goto X1324;
X674:	
	price = ip->i_value / 10. * *(dp + i);
	if( *(dp + i) == 0 ) goto X1020;
	printf("%s currently on sale for $%.2f in %d,%d.", ip->i_name, price, nsct.n_x, nsct.n_y);
	goto X1060;
X1020:	
	printf("%s not being sold in %d,%d.", ip->i_name, nsct.n_x, nsct.n_y);
X1060:	
	maxprice = (ip->i_value * 127) / 10;
	sprintf(fmtbuf,"  New price? (max %d)", maxprice);
	cp = getstri(fmtbuf);
	if( *cp == '\0' ) goto X1324;
	getsect(nsct.n_x, nsct.n_y, UP_OWN);
	price = (atopi(cp) * 10.) / ip->i_value;
	if( price <= 127. ) goto X1262;
	printf("Too much, price not set\n");
	goto X1302;
X1262:	
	*(dp + i) = price;
X1302:	
	putsect(nsct.n_x, nsct.n_y);
X1324:	
	i++;
	goto X626;
}
