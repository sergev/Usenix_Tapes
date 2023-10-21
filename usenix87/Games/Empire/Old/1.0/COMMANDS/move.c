#define D_UPDATE
#define D_NATSTAT
#define D_SECTDES
#define D_NEWSVERBS
#define D_PLGSTAGES
#define D_SCTSTR
#define D_DCHRSTR
#define D_ICHRSTR
#define D_FILES
#include	"empdef.h"

move()
{
	register char	*cp, *pp;
	register	fcnt;
	int	cc, i, tcnt;
	int	funits, tunits, weight, movcst, plague, fdesig;
	double	omu, mu, d, movdist;
	struct	dchrstr	*dp;
	struct	ichrstr	*ip;
	char	 *getstar(), *getstri();

	cp = getstar(argp[1], "move what? ");
	pp = (char *)&sect.sct_civil;
	ip = &ichr[12];
X42:	
	if( ip->i_name != 0 ) goto X100;
	printf("what's %c?", *cp);
	return(SYN_RETURN);
X100:	
	if( ip->i_lbs == 0 ) goto X120;
	if( *cp == ip->i_mnem ) goto X132;
X120:	
	pp++;
	ip++;
	goto X42;
X132:	
	if( getsno(argp[2], "from sector : ") <  0 ) goto X332;
	i = 0;
	if( owner == 0 ) goto X306;
X166:	
	getsect(sx, sy, UP_TIME);
	hx = sx;
	hy = sy;
	fdesig = sect.sct_desig;
	dp = &dchr[sect.sct_desig];
	if( sect.sct_p_stage != PLG_INFECT ) goto X342;
	plague = PLG_DYING;
	goto X346;
X272:	
	ntused += 2;
	return(FAIL_RETURN);
X306:	
	if( (i = chkok("Not yours\n")) == 0 ) goto X166;
	if( i == -2 ) goto X272;
X332:	
	return(SYN_RETURN);
X342:	
	plague = 0;
X346:	
	if( fdesig != S_SANCT ) goto X366;
	printf("Beware: this move will 'break sanctuary'!\n");
X366:	
	if( wethr(hx, hy, 0) >= 710 ) goto X504;
	i = wethr(hx, hy, 9);
	if( i >= 685 ) goto X460;
	printf("Particularly ");
X460:	
	printf("Bad weather. Barometer @%d\n", i);
X504:	
	omu = mu = sect.sct_mobil;
	funits = ip->i_pkg[dp->d_pkg];
	weight = ip->i_lbs;
	sprintf(fmtbuf,"Quantity? (max %d) ", *pp * funits);
	fcnt = onearg(argp[3], fmtbuf) / funits;
	if( fcnt <= *pp ) goto X672;
	fcnt = *pp;
	printf("Only moving %d.\n", fcnt * funits);
X672:	
	if( fcnt <= 0 ) goto X332;
	if( (cp = argp[4]) != 0 ) goto X710;
	cp = "";
X710:	
	sigsave();
X714:	
	lx = sx;
	ly = sy;
	if( *cp != '\0' ) goto X776;
	sprintf(fmtbuf,"<%.1f: %d,%d> ", mu, sx, sy);
	cp = getstri(fmtbuf);
X776:	
	if( cp != 0 ) goto X1006;
	cp = "e";
X1006:	
	if( (cc = *cp++) == 'e' ) goto X1026;
	goto X2214;
X1026:	
	getsect(sx, sy, (UP_TIME | UP_ALL));
	if( (tunits = ip->i_pkg[dchr[sect.sct_desig].d_pkg]) <= 1 ) goto X1176;
	if( sect.sct_effic >= 60 ) goto X1136;
	if( ip->i_mnem == 'c' ) goto X1136;
	printf("Sector under construction");
	return(FAIL_RETURN);
X1136:	
	if( sect.sct_civil + 10 >= (fcnt * funits) / 5 ) goto X1176;
	printf("Too much traffic for workers in this sector");
	return(FAIL_RETURN);
X1176:	
	if( &sect.sct_ore != pp ) goto X1300;
	if( sect.sct_desig != S_GMINE ) goto X1242;
	if( fdesig == S_GMINE ) goto X1242;
	printf("Sorry, the union won't let you leave ordinary ore here...\n");
	return(FAIL_RETURN);
X1242:	
	if( sect.sct_desig == S_GMINE ) goto X1300;
	if( fdesig != S_GMINE ) goto X1300;
	printf("Sorry, the union won't let you leave gold ore here...\n");
	return(FAIL_RETURN);
X1300:	
	i = (fcnt * funits) / tunits;
	i = (i * tunits) / funits;
	i = (i * funits) / tunits;
	tcnt = (i >= 127 - *pp) ? 127 - *pp : i;
	if( tcnt == i ) goto X1502;
	if( hx != sx ) goto X1446;
	if( hy == sy ) goto X1502;
X1446:	
	printf("Only room for %d (the rest returned)\n", tcnt * tunits);
	fcnt = ((tcnt * tunits) + funits -1) / funits;
	goto X1604;
X1502:	
	if( fcnt * funits == tcnt * tunits ) goto X1604;
	if( hx != sx || hy != sy ) {
		printf("Because of containerization, (%d),  only %d can be left\n", tunits, tcnt * tunits);
	}
	fcnt = (tcnt * tunits) / funits;
X1604:	
	*pp = *pp + tcnt;
	if( sect.sct_owned != 0 ) goto X1714;
	if( sect.sct_desig == S_SANCT ) goto X1714;
	if( sect.sct_civil !=  0 ) goto X1650;
	if( sect.sct_milit == 0 ) goto X1714;
X1650:	
	sect.sct_owned = cnum;
	sect.sct_lstup = curup;
	printf("Sector %d,%d is now yours.\n", sx, sy);
X1714:	
	if( plague == PLG_HEALTHY ) goto X1736;
	if( sect.sct_p_stage != PLG_HEALTHY ) goto X1736;
	sect.sct_p_stage = PLG_EXPOSED;
X1736:	
	putsect(sx, sy);
	getsect(hx, hy, UP_NONE);
	*pp = *pp - fcnt;
	mu = mu + sect.sct_mobil - omu;
	i = (mu <  0) ? 0 : mu;
	sect.sct_mobil = max127(i);
	if( sect.sct_desig != S_SANCT ) goto X2170;
	if( nstat == STAT_GOD ) goto X2170;
	sect.sct_desig= S_CAPIT;
	nreport(cnum, N_BROKE_SANCT, 0);
	printf("%d,%d is no longer a sanctuary.\n", hx, hy);
X2170:	
	putsect(hx, hy);
	return(NORM_RETURN);
X2214:	
	if( cc != 'u' ) goto X2370;
X2224:	
	sy--;
X2230:	
	if( lx != sx ) goto X2254;
	if( ly != sy ) goto X2254;
	goto X714;
X2254:	
	if( getsect(sx, sy, UP_NONE) >= 0 ) goto X2304;
	goto X3552;
X2304:	
	if( nstat != STAT_GOD ) goto X2320;
	goto X714;
X2320:	
	if( sect.sct_desig == S_SANCT ) goto X2334;
	goto X3202;
X2334:	
	if( nstat != STAT_GOD ) goto X2350;
	goto X3202;
X2350:	
	*cp = 'e';
	printf("Converts, huh?\n");
X2360:	
	goto X714;
X2370:	
	if( cc != 'd' ) goto X2406;
X2400:	
	sy++;
	goto X2230;
X2406:	
	if( cc != 'l' ) goto X2424;
	sx--;
	goto X2230;
X2424:	
	if( cc != 'r' ) goto X2442;
	sx++;
	goto X2230;
X2442:	
	if( cc != '/' ) goto X2654;
	cc = *cp++;
	if( cc == 'l' ) goto X2474;
	if( cc != 'd' ) goto X2502;
X2474:	
	sx--;
	goto X2400;
X2502:	
	if( cc == 'r' ) goto X2522;
	if( cc != 'u' ) goto X2530;
X2522:	
	sx++;
	goto X2224;
X2530:	
	printf("\"/%c\" is not legal...", *(cp-1));
	goto X2634;
X2544:	
	cc = *cp++;
	if( cc == 'l' ) goto X2566;
	if( cc != 'u' ) goto X2574;
X2566:	
	sx--;
	goto X2224;
X2574:	
	if( cc == 'r' ) goto X2614;
	if( cc != 'd' ) goto X2622;
X2614:	
	sx++;
	goto X2400;
X2622:	
	printf("\"\\%c\" is not legal...", *(cp-1));
X2634:	
	goto X3562;
X2654:	
	if( cc == '\\' ) goto X2544;
	if( cc != 'v' ) goto X3144;
	dp = &dchr[sect.sct_desig];
	tunits = 12;
	tunits = ichr[tunits].i_pkg[dp->d_pkg];
	printf("%d%% %s with %d civilians", sect.sct_effic, dchr[sect.sct_desig].d_name, sect.sct_civil*tunits);
	if( &sect.sct_civil == pp ) goto X3130;
	tunits = ip->i_pkg[dp->d_pkg];
	printf(" and %d %s.\n", *pp * tunits, ip->i_name);
	goto X2230;
X3130:	
	printf(".\n");
	goto X2230;
X3144:	
	printf("\"%c\" is not legal...", *(cp-1));
	printf("u = -y, d = +y, l = -x, r = +x, v = view, e = end\n");
	goto X3556;
X3202:	
	if( (movcst = dchr[sect.sct_desig].d_mcst & 0377) == 0 ) goto X3552;
	if( sect.sct_owned == 0 ) goto X3310;
	if( owner != 0 ) goto X3310;
	i = chkok("");
	if( i == -1 ) goto X3552;
	if( i != -2 ) goto X3310;
	*cp = 'e';
	printf("You have been arrested!\n");
	goto X2360;
X3310:	
	d = (movcst*80. + 20. - sect.sct_effic)/500.;
	if( wethr(sx, sy, 0) >= 725 ) goto X3444;
	d = d + 10. / (wethr(sx, sy, 0) - 300);
X3444:	
	movdist = (lx==sx || ly==sy) ? 1. : 1.40625;
	d = d * fcnt * weight * movdist;
	if( mu >= d ) goto X3624;
	printf("Not enough mobility.  ");
X3552:	
	printf("You can't go there...\n");
X3556:	
X3562:	
	getsect(sx=lx, sy=ly, UP_NONE);
	*cp = 0;
	goto X714;
X3624:	
	mu = mu - d;
	goto X714;
}
