#define D_UPDATE
#define D_SECTDES
#define D_SHIPTYP
#define D_TRTYCLAUSE
#define D_NATSTR
#define D_SCTSTR
#define D_SHPSTR
#define D_MCHRSTR
#define D_NSCSTR
#define D_FILES
#include	"empdef.h"

buil()
{
	register	k;
	char	*cp, c, sdir, *getstri();
	int	q, typ, nxtshp, ctyp;
	int	costlim, dx, dy, price;
	long	cashav;
	struct	nstr	nsct;

	if( trechk(cnum, 0, TRTBUI) == -1 ) return(FAIL_RETURN);
	nxtshp = -1;
	if( snxtsct(&nsct, argp[1]) == -1 ) return(SYN_RETURN);
	ctyp = -1;
	sdir = 0;
	if( argp[2] == 0 ) goto X260;
	if( argp[2][0] == '\0' ) goto X260;
	c = argp[2][0];
	ctyp = 8;
X142:	
	if( --ctyp <  0 ) goto X172;
	if( c != *mchr[ctyp].m_name ) goto X142;
X172:	
	if( ctyp == -1 ) return(SYN_RETURN);
	if( c == 'u' ) goto X252;
	if( c == 'd' ) goto X252;
	if( c == 'l' ) goto X252;
	if( c != 'r' ) goto X260;
X252:	
	sdir = c;
X260:	
	if( ctyp <  0 ) goto X310;
	costlim = mchr[ctyp].m_prdct;
	goto X320;
X310:	
	costlim = mchr[0].m_prdct;
X320:	
	getnat(cnum);
	cashav = nat.nat_money;
	goto X2420;
X350:	
	q = sect.sct_prdct;
	if( q >= costlim ) goto X372;
	goto X2420;
X372:	
	if( ctyp <= -1 ) goto X552;
	typ = ctyp;
X410:	
	price = mchr[typ].m_prdct * 9;
	if( price > cashav ) goto X464;
	goto X1102;
X464:	
	printf("A %s costs $%d to build, (you only have %ld)\n", mchr[typ].m_name, price, cashav);
	if( ctyp <= -1 ) goto X552;
	goto X2420;
X552:	
	printf("%d prod. units in %d, %d ", q, nsct.n_x, nsct.n_y);
	cp = getstri("kind of ship? ");
	if( *cp != '\0' ) goto X642;
	goto X2420;
X642:	
	typ = 8;
X650:	
	if( --typ <  0 ) goto X700;
	if( *cp != *mchr[typ].m_name ) goto X650;
X700:	
	if( typ >= 0 ) goto X1000;
	printf("types are :\n");
	k = 0;
X720:
	if( k > TMAXNO ) goto X552;
	printf("%12s %d\n", mchr[k].m_name, mchr[k].m_prdct);
	k++;
	goto X720;
X1000:	
	if( q <  mchr[typ].m_prdct ) goto X1026;
	goto X410;
X1026:	
	printf("A %s requires %d prod. units\n", mchr[typ].m_name, mchr[typ].m_prdct);
	goto X552;
X1102:	
	dolcost += price;
	cashav -= price;
X1142:	
	if( getship(++nxtshp, &ship) <  0 ) goto X1176;
	if( ship.shp_own != 0 ) goto X1142;
X1176:	
	ship.shp_own = cnum;
	ship.shp_type = typ;
	ship.shp_effc = 50;
	ship.shp_xp = nsct.n_x;
	ship.shp_yp = nsct.n_y;
	ship.shp_fleet = ' ';
	ship.shp_crew = ship.shp_shels = 0;
	ship.shp_gun = ship.shp_or = ship.shp_gld = 0;
	ship.shp_plns = ship.shp_spric = ship.shp_mbl = 0;
	ship.shp_lstp = curup;
	putship(nxtshp, &ship);
	printf("%s #%d", mchr[typ].m_name, nxtshp);
	printf(" built in sector %d, %d\n", nsct.n_x, nsct.n_y);
	sect.sct_prdct -= mchr[typ].m_prdct;
	putsect(nsct.n_x, nsct.n_y);
	goto X2420;
X1506:	
	printf("%d,%d only has %d prod. units,", nsct.n_x, nsct.n_y, sect.sct_prdct);
	printf("(a bridge span requires 127)\n");
	goto X2420;
X1566:	
	if( owner != 0 ) goto X1600;
	goto X2420;
X1600:	
	if( sect.sct_desig != S_HARBR ) goto X1614;
	goto X350;
X1614:	
	if( sect.sct_desig == S_BHEAD ) goto X1630;
	goto X2420;
X1630:	
	if( sect.sct_prdct < 127 ) goto X1506;
	if( sdir != '\0' ) goto X1724;
	printf("Bridge head at %d,%d; ", nsct.n_x, nsct.n_y);
	cp = getstri("build span in what direction? (udlr) ");
	sdir = *cp;
X1724:	
	if( sdir != '\0' ) goto X1736;
	goto X2420;
X1736:	
	dx = nsct.n_x;
	dy = nsct.n_y;
	if( sdir != 'u' ) goto X2044;
	dy--;
X1772:	
	if( getsect(dx, dy, UP_NONE) == -1 ) goto X2026;
	if( sect.sct_desig == S_WATER ) goto X2140;
X2026:	
	printf("%d,%d is not a water sector\n", dx, dy);
	goto X2420;
X2044:	
	if( sdir != 'd' ) goto X2062;
	dy++;
	goto X1772;
X2062:	
	if( sdir != 'l' ) goto X2100;
	dx--;
	goto X1772;
X2100:	
	if( sdir != 'r' ) goto X2116;
	dx++;
	goto X1772;
X2116:	
	printf("%c? u=up, d=down, l=left, r=right\n", sdir);
	goto X2420;
X2140:	
	price = 1143;
	if( price <= cashav ) goto X2224;
	printf("A span costs $%d to build, (you only have %ld)\n", price, cashav);
	goto X2420;
X2224:	
	dolcost += price;
	cashav -= price;
	sect.sct_desig = S_BSPAN;
	sect.sct_effic = 20;
	putsect(dx, dy);
	getsect(nsct.n_x, nsct.n_y, UP_NONE);
	sect.sct_prdct = 0;
	putsect(nsct.n_x, nsct.n_y);
	printf("Bridge span built over %d,%d\n", dx, dy);
X2420:	
	if( nxtsct(&nsct, UP_OWN) <= 0 ) goto X2450;
	goto X1566;
X2450:	
	return(NORM_RETURN);
}
