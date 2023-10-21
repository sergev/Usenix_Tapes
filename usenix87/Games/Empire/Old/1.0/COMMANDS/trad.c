#define D_UPDATE
#define D_SHPSTR
#define D_SECTDES
#define D_SCTSTR
#define D_NATSTR
#define D_ICHRSTR
#define D_MCHRSTR
#define D_NEWSVERBS
#define D_MCHRSTR
#define D_FILES
#include	"empdef.h"

trad()
{
	register char	*cp;
	char	*gp, *up, *getstri();
	int	i, j, k, flag, lotnum, landonly, seaonly;
	struct	ichrstr	*ip;
	struct	sctstr	s[MAX_W_XSIZE];
	struct {
		short	l_x;
		short	l_y;
	}	l[256];

	printf("\tEmpire Trade Report\n    ");
	prdate();
	lotnum = 0;
	landonly = seaonly = 0;
	if( argp[1] == 0 ) goto X100;
	if( argp[1][0] != 'n' ) goto X64;
	seaonly++;
X64:	
	if( argp[1][0] != 'l' ) goto X100;
	landonly++;
X100:	
	if( seaonly == 0 ) goto X112;
	goto X756;
X112:	
	lseek(sectf, 0L, 0);
	j = 0;
X136:	
	if( j < w_ysize ) goto X152;
	goto X756;
X152:	
	read(sectf, s, w_xsize * sizeof(sect));
	i = 0;
X212:	
	if( i < w_xsize ) goto X226;
	goto X632;
X226:	
	if( s[i].sct_owned != 0 ) goto X252;
	goto X746;
X252:	
	if( s[i].sct_desig == S_XCHNG ) goto X300;
	goto X746;
X300:	
	if( s[i].sct_effic >= 60 ) goto X326;
	goto X746;
X326:	
	flag = 0;
	gp = (char *)&s[i].sct_civil;
	up = (char *)&s[i].sct_c_use;
	for( k = 2; (ip = &ichr[k + 12])->i_mnem != '\0'; k++ ) {
		if( *(gp + k) == 0 ) continue;
		if( *(up + k) == 0 ) continue;
		printf("%d %s @ $%.2f  ", flag = *(gp + k), ip->i_name, ip->i_value / 9.52 * *(up + k));
	}
	if( flag == 0 ) goto X746;
	if( lotnum < 256 ) goto X642;
	printf(" ... and more ...\n");
X632:	
	j++;
	goto X136;
X642:	
	l[lotnum].l_x = i - capx;
	l[lotnum].l_y = j - capy;
	printf("lot #%d\n", lotnum);
	lotnum++;
X746:	
	i++;
	goto X212;
X756:	
	if( landonly != 0 ) goto X1266;
	i = 0;
X770:	
	if( getship(i, &ship) == -1 ) goto X1266;
	if( ship.shp_spric == 0 ) goto X1330;
	l[lotnum].l_x = -99;
	l[lotnum].l_y = i;
	j = mchr[ship.shp_type].m_prdct * ship.shp_spric;
	printf("%d%% %s crew:%d guns:%d shells:%d @$%.2f", ship.shp_effc, mchr[ship.shp_type].m_name, ship.shp_crew, ship.shp_gun, ship.shp_shels, j * 1.05);
	printf("  lot #%d\n", lotnum);
	lotnum++;
	if( lotnum < 256 ) goto X1330;
	printf(" ... and more ...\n");
X1266:	
	if( lotnum != 0 ) goto X1304;
	printf("Nothing on the market at the moment.\n");
	return(FAIL_RETURN);
X1304:	
	cp = getstri("Which lot? (<CR> for none) ");
	if( *cp != '\0' ) goto X1336;
	return(NORM_RETURN);
X1330:	
	i++;
	goto X770;
X1336:	
	i = atoi(cp);
	if( i >= lotnum || i < 0 ) {
		printf("No such lot exists");
		return(FAIL_RETURN);
	}
	buyfrom(l[i].l_x, l[i].l_y);
	goto X1304;
}

buyfrom(fx, fy)
int	fx, fy;
{
	register	i, j;
	char	*gp, *up;
	struct	ichrstr	*ip;
	double price;

	if( fx != -99 ) goto X270;
	if( getship(fy, &ship) == -1 ||
	    ship.shp_spric == 0 ) {
		printf("Sorry... ship %d not for sale", fy);
		return(FAIL_RETURN);
	}
	price = mchr[ship.shp_type].m_prdct * ship.shp_spric;
	sigsave();
	if( payoff(ship.shp_own, price) == -1 ) {
		printf("A little short aren't you???");
		return(FAIL_RETURN);
	}
	ship.shp_own = cnum;
	ship.shp_fleet = ' ';
	ship.shp_spric = 0;
	putship(fy, &ship);
	printf("You have just bought ship #%d\n", fy);
	return(NORM_RETURN);
X260:	
	return(FAIL_RETURN);
X270:	
	if( getsno("", "To be delivered to exchange at ") == -1 ) goto X260;
	if( owner == 0 ) goto X332;
	if( sect.sct_desig == S_XCHNG ) goto X342;
X332:	
	printf("Not your exchange sector");
	return(FAIL_RETURN);
X342:	
	if( sect.sct_effic >= 60 ) goto X362;
	printf("Tradin' post not completed yet!");
	return(FAIL_RETURN);
X362:	
	gp = (char *)&sect.sct_civil;
	up = (char *)&sect.sct_c_use;
	i = 2;
X402:	
	if( (ip = &ichr[i + 12])->i_mnem == '\0' ) return(NORM_RETURN);
	getsect(fx, fy, UP_NONE);
	if( *(gp + i) == 0 ) goto X1024;
	if( *(up + i) == 0 ) goto X1024;
	sprintf(fmtbuf,"How many %s? <max %d> ", ip->i_name, *(gp + i));
	j = onearg("", fmtbuf);
	j = (j < *(gp + i)) ? j : *(gp + i);
	price = ip->i_value / 10. * *(up + i);
	sigsave();
	if( payoff(sect.sct_owned, j * price) != -1 ) goto X702;
	printf("You don't have the cash.");
	return(FAIL_RETURN);
X702:	
	*(gp +i) -= j;
	putsect(fx, fy);
	getsect(sx, sy, UP_NONE);
	*(gp + i) = max127(*(gp + i) + j);
	putsect(sx, sy);
X1024:	
	i++;
	goto X402;
}

payoff(seller, dol)
int	seller;
double	dol;
{
	double	assets;

	getnat(cnum);
	assets = nat.nat_money - dol * 1.05;
	if( assets < 0 ) return(-1);
	nat.nat_money = assets;
	putnat(cnum);
	getnat(seller);
	nat.nat_money = (float)nat.nat_money + dol;
	putnat(seller);
	sprintf(fmtbuf,"You made a $%.2f sale to country #%d", dol, cnum);
	wu(0, seller, fmtbuf);
	nreport(seller, N_MAKE_SALE, cnum);
	printf("You are now $%.2f poorer.\n", dol * 1.05);
	return(NORM_RETURN);
}
