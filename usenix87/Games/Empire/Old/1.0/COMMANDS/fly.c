#define D_UPDATE
#define D_NATSTAT
#define D_SECTDES
#define D_SHIPTYP
#define D_NEWSVERBS
#define D_SCTSTR
#define D_SHPSTR
#define D_DCHRSTR
#define D_MCHRSTR
#define D_FILES
#include	"empdef.h"
#include	<stdio.h>

fly()
{
	register char	*cp, c;
	register	i;
	int	nop, nob, dam, shipflak, weight, aship, vship;
	short	buzzed[MAX_MAXNOC];
	double	mu, movdist, w;
	struct	mchrstr	*mp;
	char	 *getstar(), *getstri(), *splur();
	char	*xytoa(), *cname();
	double	tfact();

	cp = getstar(argp[1], "From : ");
	if( landorsea(cp) == LAND ) goto X46;
	goto X656;
X46:	
	if( getsno(cp, "from airport at ") >= 0 ) goto X74;
X66:	
	return(SYN_RETURN);
X74:	
	i = 0;
	if( owner != 0 ) goto X134;
	
	if( (i = chkok("Not yours")) == 0 ) goto X134;
	if( i == 0 ) goto X66;
	return(FAIL_RETURN);
X134:	
	if( sect.sct_desig == S_AIRPT ) goto X154;
	printf("Not an airfield.");
	return(SYN_RETURN);
X154:	
	if( sect.sct_effic >= 60 ) goto X174;
	printf("Sector under construction.");
	return(SYN_RETURN);
X174:	
	if( sect.sct_plane != 0 ) goto X212;
	printf("Build some planes first.");
	return(SYN_RETURN);
X212:	
	i = (sect.sct_plane < (sect.sct_milit>>1) ) ? sect.sct_plane : (sect.sct_milit>>1);
	sprintf(fmtbuf,"Number of planes in mission? (max %d) ", i);
	nop = onearg(argp[2], fmtbuf);
	nop = min(nop, i);
	if( nop <= 0 ) goto X66;
	i = ((sect.sct_shell / nop) > 5 ) ? 5 : (sect.sct_shell / nop);
	sprintf(fmtbuf,"Number of bombs per plane? (max %d) ", i);
	nob = onearg(argp[3], fmtbuf);
	nob = min(nob, i);
	weight = ((nob + 2) / 3) + 1;
	mu = (sect.sct_mobil<<2) / nop;
	mu = min(32., mu);
	sect.sct_milit -= (nop<<1);
	sect.sct_shell -= (nop * nob);
	sect.sct_plane -= nop;
	sect.sct_mobil -= ((nop * mu) / 4.) + .5;
	putsect(sx, sy);
	goto X1524;
X656:	
	if( (aship = getshno(cp, "Carrier #? ", &ship)) != -1 ) goto X720;
	printf("no such ship");
	return(SYN_RETURN);
X720:	
	if( cnum == ship.shp_own ) goto X742;
	printf("not yours");
	return(SYN_RETURN);
X742:	
	if( ship.shp_type != S_CAR ) goto X760;
	if( ship.shp_plns != 0 ) goto X770;
X760:	
	printf("No aircraft on board");
	return(FAIL_RETURN);
X770:	
	sx = ship.shp_xp;
	sy = ship.shp_yp;
	i = wethr(sx, sy, 0);
	if( i >= 700 ) goto X1062;
	printf("Barometer @ %d; seas too rough to fly", i);
	return(SYN_RETURN);
X1062:	
	i = (ship.shp_plns < (ship.shp_crew>>1)) ? ship.shp_plns : (ship.shp_crew>>1);
	sprintf(fmtbuf,"Number of planes in mission? (max %d) ", i);
	nop = onearg(argp[2], fmtbuf);
	nop = min(nop, i);
	if( nop > 0 ) goto X1172;
	return(FAIL_RETURN);
X1172:	
	i = ((ship.shp_shels / nop) > 5) ? 5 : ship.shp_shels / nop;
	if( i <= 0 ) goto X1274;
	sprintf(fmtbuf,"Number of bombs per plane? (max %d) ", i);
	nob = onearg(argp[3], fmtbuf);
	goto X1300;
X1274:	
	nob = 0;
X1300:	
	nob = min(nob, i);
	weight = ((nob + 2) / 3) + 1;
	mu = (ship.shp_mbl<<2) / nop;
	mu = min(32., mu);
	ship.shp_crew -= (nop<<1);
	ship.shp_shels -= (nop * nob);
	ship.shp_plns -= nop;
	ship.shp_mbl -= (((nop * mu) / 4.) + .5);
	putship(aship, &ship);
X1524:	
	mu = tfact(cnum, mu);
	i = wethr(sx, sy, 0);
	if( i >= 710 ) goto X1644;
	if( i >= 685 ) goto X1622;
	printf("particularly ");
X1622:	
	printf("bad weather. barometer @%d\n", i);
X1644:	
	cp = argp[4];
	if( cp != 0 ) goto X1656;
	cp = "";
X1656:	
	shipflak = 0;
	sigsave();
X1666:	
	if( nop >  0 ) goto X1702;
X1674:	
	return(NORM_RETURN);
X1702:	
	lx = sx = xwrap(sx);
	ly = sy = ywrap(sy);
	if( mu >  0. ) goto X1770;
	cp = "e";
	printf("You're out of gas (and altitude)...");
X1770:	
	if( *cp != '\0' ) goto X2046;
	sprintf(fmtbuf,"<%.1f:%d:%d:%d,%d> ", mu, nop, nob, sx, sy);
	cp = getstri(fmtbuf);
X2046:	
	movdist = 1.;
	c = *cp++;
	if( c == 'e' ) goto X2072;
	goto X3612;
X2072:	
	getsect(sx, sy, UP_ALL | UP_TIME);
	switch( sect.sct_desig ) {
	case S_AIRPT:
		if( sect.sct_effic >= 60 ) goto X2206;
		printf("Sector under construction...\n");
		i = ((sect.sct_effic * 10) + 3) / 6;
		goto X2212;
X2206:	
		i = 100;
X2212:	
		nop = crashla(nop, nob, i, 0, 0);
X2220:	
		if( nop <= 0 ) {
			putsect(sx, sy);
			return(NORM_RETURN);
		}
		sect.sct_milit = max127(sect.sct_milit + (nop<<1));
		sect.sct_shell = max127(nop * nob + sect.sct_shell);
		sect.sct_plane = max127(sect.sct_plane + nop);
		i = ((nop * mu) + 2.) / 4.;
		sect.sct_mobil = max127(sect.sct_mobil + i);
		if( sect.sct_owned != 0 ) goto X2444;
		if( sect.sct_desig == S_SANCT ) goto X2444;
		sect.sct_owned = cnum;
		sect.sct_lstup = curup;
X2444:	
		putsect(sx, sy);
X2460:	
		return(NORM_RETURN);
	case S_WATER:
	case S_HARBR:
	case S_BSPAN:
		if( mu >= 0. ) goto X2540;
		for( i = 0; i < nop; i++ ) {
			fflush(stdout);
			sleep(2);
			printf(" SPLASH!\n");
		}
		return(NORM_RETURN);
X2540:	
		cp = getstri("Land where? ");
		if( landorsea(cp) != LAND ) goto X2574;
		nop = crashla(nop, nob, 0, 0, 0);
		goto X2220;
X2574:	
		vship = atopi(cp);
		cp = "e";
		if( getship(vship, &ship) == -1 ) goto X2662;
		if( sx != ship.shp_xp ) goto X2662;
		if( sy == ship.shp_yp ) goto X3006;
X2662:	
		printf("No ship #%d sighted!\n", vship);
X2706:	
		nop = flakche(nop, shipflak);
X2730:	
		if( nstat != STAT_GOD ) goto X2744;
		goto X1666;
X2744:	
		w = (wethr(sx, sy, 0) >= 700) ? 1. : 1.25;
		goto X6056;
X3006:	
		mp = &mchr[ship.shp_type];
		if( ship.shp_type != S_SUB ) goto X3050;
		printf("There's not room on the deck of a submarine!\n");
X3042:	
		goto X2706;
X3050:	
		if( mp->m_plns == 0 ) goto X3070;
		i = ship.shp_effc;
		goto X3110;
X3070:	
		i = (mp->m_prdct + (-30)) / 5;
X3110:	
		nop = crashla(nop, nob, i, 1, vship);
		if( nop >  0 ) goto X3154;
		putship(vship, &ship);
		goto X1674;
X3154:	
		ship.shp_crew = (mp->m_milit > ship.shp_crew + (nop<<1)) ? ship.shp_crew + (nop<<1) : mp->m_milit;
		ship.shp_shels = (mp->m_shels > nop * nob + ship.shp_shels) ? nop * nob + ship.shp_shels : mp->m_shels;
		ship.shp_plns = (mp->m_plns > ship.shp_plns + nop) ? ship.shp_plns + nop : mp->m_plns;
		i = ((nop * mu) + 2.) / 4.;
		ship.shp_mbl = max127(ship.shp_mbl + i);
		putship(vship, &ship);
		goto X2460;
	case S_HIWAY:
		printf("Trying to land on a highway???\n");
		nop = crashla(nop, nob, (sect.sct_effic / 3) + 40, 0, 0);
		goto X2220;
	default:
		printf("Landing on this %s is going to be rough ... ", dchr[sect.sct_desig].d_name);
		i = (500 - (sect.sct_effic>>2)) / 10;
		goto X2212;
	}
X3612:	
	if( c != 'u' ) goto X3630;
X3620:	
	sy--;
	goto X2706;
X3630:	
	if( c != 'd' ) goto X3646;
X3636:	
	sy++;
	goto X2706;
X3646:	
	if( c != 'l' ) goto X3664;
	sx--;
	goto X2706;
X3664:	
	if( c != 'r' ) goto X3702;
	sx++;
	goto X2706;
X3702:	
	if( c != '/' ) goto X3772;
	movdist = 1.414;
	c = *cp++;
	if( c == 'l' ) goto X3736;
	if( c != 'd' ) goto X3744;
X3736:	
	sx--;
	goto X3636;
X3744:	
	if( c == 'r' ) goto X3764;
	if( c == 'u' ) goto X3764;
	goto X5766;
X3764:	
	sx++;
	goto X3620;
X3772:	
	if( c != '\\' ) goto X4062;
	movdist = 1.414;
	c = *cp++;
	if( c == 'l' ) goto X4026;
	if( c != 'u' ) goto X4034;
X4026:	
	sx--;
	goto X3620;
X4034:	
	if( c == 'r' ) goto X4054;
	if( c == 'd' ) goto X4054;
	goto X5766;
X4054:	
	sx++;
	goto X3636;
X4062:	
	if( c == 'b' ) goto X4074;
	goto X5112;
X4074:	
	if( nob >= 1 ) goto X4114;
	printf("No bombs on board\n");
	goto X6040;
X4114:	
	getsect(sx, sy, UP_NONE);
	nob--;
	switch( sect.sct_desig ) {
	case S_HARBR:
	case S_BSPAN:
		if( *(getstri("ship or land? ")) != 's' ) break;
	case S_WATER:
		vship = atopi(getstri("ship? "));
		if( getship(vship, &ship) == -1 ) goto X4300;
		if( ship.shp_own == 0 ) goto X4300;
		if( sx != ship.shp_xp ) goto X4300;
		if( sy == ship.shp_yp ) goto X4340;
X4300:	
		printf("Ship #%d not sighted.   SPLASH!\n", vship);
		shipflak = ship.shp_own;
		goto X2706;
X4340:	
		dam = 10;
		i = nop;
		goto X4402;
X4354:	
		printf("kaBLAM!  ");
		shipdam(&ship, dam); 
X4402:	
		if( --i >= 0 ) goto X4354;
		shipflak = ship.shp_own;
		sprintf(fmtbuf,"Country #%d dropped %d bomb%s on %s #%d", cnum, nop, splur(nop), mchr[ship.shp_type].m_name, vship);
		wu(0, ship.shp_own, fmtbuf);
		nreport(cnum, N_SHP_BOMB, ship.shp_own);
		putship(vship, &ship);
		goto X2706;
	case S_SANCT:
		printf("Fizzzzzzle ...\n");
		goto X3042;
	}
	dam = 40 / (dchr[sect.sct_desig].d_dstr + 2);
	i = nop;
	goto X4714;
X4640:	
	if( rand() % 32768 <= 20000 ) goto X4674;
	printf("SPLASH!  ");
	goto X4714;
X4664:	
	if( sect.sct_desig == S_BSPAN ) goto X4640;
X4674:	
	printf("BLAMMMO!  ");
	sectdam(dam); 
X4714:	
	if( --i >= 0 ) goto X4664;
	putsect(sx, sy);
	shipflak = sect.sct_owned;
	if( sect.sct_owned == 0 ) goto X5062;
	sprintf(fmtbuf,"Country #%d dropped %d bomb%s on %s", cnum, nop, splur(nop), xytoa(sx, sy, sect.sct_owned));
	wu(0, sect.sct_owned, fmtbuf);
X5062:	
	nreport(cnum, N_SCT_BOMB, sect.sct_owned);
	goto X2706;
X5112:	
	if( c == 'v' ) goto X5124;
	goto X5766;
X5124:	
	getsect(sx, sy, UP_NONE);
	if( sect.sct_desig == S_WATER ) goto X5162;
	if( sect.sct_desig != S_MOUNT ) goto X5212;
X5162:	
	printf("Flying over %s.\n", dchr[sect.sct_desig].d_name);
	goto X5260;
X5212:	
	printf("Now over %s constructed %s.\n", effadv[sect.sct_effic / 30], dchr[sect.sct_desig].d_name);
X5260:	
	if( owner != 0 ) goto X5410;
	if( sect.sct_owned == 0 ) goto X5410;
	sprintf(fmtbuf,"%d country #%d plane%s sighted over sector %s", nop, cnum, splur(nop), xytoa(sx, sy, sect.sct_owned));
	wu(0, sect.sct_owned, fmtbuf);
X5410:	
	if( sect.sct_desig == S_WATER ) goto X5452;
	if( sect.sct_desig == S_HARBR ) goto X5452;
	if( sect.sct_desig != S_BSPAN ) goto X2730;
X5452:	
	vship = 0;
	for( i = 0; i < maxnoc; i++ ) buzzed[i] = 0;
X5456:	
	if( getship(vship, &ship) != -1 ) goto X5506;
	goto X2706;
X5506:	
	if( sx != ship.shp_xp ) goto X5760;
	if( sy != ship.shp_yp ) goto X5760;
	if( ship.shp_own == 0 ) goto X5760;
	if( cnum == ship.shp_own ) goto X5672;
	if( buzzed[ship.shp_own] == 0 ) {
		buzzed[ship.shp_own] = 1;
		sprintf(fmtbuf,"%d country #%d plane%s buzzed ship #%d",
			nop, cnum, splur(nop), vship);
		wu(0, ship.shp_own, fmtbuf);
	}
	printf("%-12s #%-4d\n", mchr[ship.shp_type].m_name, vship);
	goto X5760;
X5672:	
	printf("%-12s #%-4d %s operational\n", mchr[ship.shp_type].m_name, vship, effadv[(ship.shp_effc + (-20)) / 25]);
X5760:	
	vship++;
	goto X5456;
X5766:	
	printf("\"%c\" is not legal...\n", *(cp-1));
	printf("u for -y, d for +y, l for -x, r for +x,\n");
	printf("\\u for-x-y, /u for +x-y, \\d for +x+y, /d for -x+y\n");
	printf("b for bomb, v for view, e to end\n");
X6040:	
	*cp = '\0';
	goto X1666;
X6056:	
	mu -= ((weight + nob * .2) * movdist * w);
	goto X1666;
}

crashla(nop, nob, chnce, lors, vship)
int	nop, nob, chnce, lors, vship;
{
	register	i, j;
	int	dam;
	double	q;
	struct	mchrstr	*mp;
	char	 *cname(), *splur(), *xytoa();

	if( chnce != 100 ) goto X24;
	goto X744;
X24:	
	printf("We have a %d%% chance of making it ...", chnce);
	if( lors != SEA ) goto X100;
	mp = &mchr[ship.shp_type];
X100:	
	fflush(stdout);
	sleep(2);
	printf("BOUNCE! .....  skreeEEEE! ... ");
	i = nop;
	goto X734;
X130:	
	if( lors == LAND ) goto X152;
	if( ship.shp_plns + 1 >= 128 ) goto X236;
X152:	
	printf( chnce < 50 ? " WHEW!!   " : " whew  " );
	goto X734;
X204:	
	fflush(stdout);
	sleep(1);
	if( chnce >  rand() % 100 ) goto X130;
X236:	
	printf("KRUNCHO!!   ");
	nop--;
	dam = 94;
	q = 1.0 - ((lors == LAND) ? .32 / (dchr[sect.sct_desig].d_dstr + 2.) : .32 / ((mp->m_prdct / 30.) + 2.));
	j = nob;
	goto X414;
X370:	
	printf("BLAM!");
	dam = dam * q;
X414:	
	if( --j >= 0 ) goto X370;
	dam = 100 - dam;
	if( lors != LAND ) goto X574;
	sectdam(dam);
	if( cnum == sect.sct_owned ) goto X734;
	sprintf(fmtbuf,"A %s (%d) plane carrying %d bomb%s crashed in sector %s", cname(cnum), cnum, nob, splur(nob), xytoa(sx, sy, sect.sct_owned));
	wu(0, sect.sct_owned, fmtbuf);
	goto X734;
X574:	
	shipdam(&ship, dam);
	if( cnum == ship.shp_own ) goto X734;
	sprintf(fmtbuf,"A %s (%d) plane carrying %d bomb%s crashed into %s %d", cname(cnum), cnum, nob, splur(nob), mp->m_name, vship);
	wu(0, ship.shp_own, fmtbuf);
X734:	
	if( --i <  0 ) goto X744;
	goto X204;
X744:	
	printf("%d plane%s landed.\n", nop, splur(nop));
	return(nop);
}

flakche(nop, shipflak)
int	nop, shipflak;
{
	register	i, j;
	int	flakshel, flakker, prcnt;
	double	sqrt();
	char	 *xytoa();

	if( getsect(sx, sy, UP_NONE) < 0 ) return(nop);
	flakker = sect.sct_owned;
	switch( sect.sct_desig ) {
	case S_CAPIT:
	case S_AIRPT:
	case S_FORTR:
		if( sect.sct_owned == 0 ||
		    owner != 0 ||
		    sect.sct_guns == 0 ||
		    sect.sct_shell == 0 ||
		    getrel(cnum, sect.sct_owned) == ALLIED ) return(nop);
		if( getrel(cnum, sect.sct_owned) != AT_WAR ) {
			if( chkok("") >= 0 ) return(nop);
		}
		if( (flakshel = min((sect.sct_shell + 3) / 4, (sect.sct_guns + 1) / 2)) == 0 ) return(nop);
		break;
	case S_WATER:
	case S_HARBR:
	case S_BSPAN:
		flakshel = 0;
		for( i = 0; getship(i, &ship) != -1; i++ ) {
			if( sx != ship.shp_xp ) continue;
			if( sy != ship.shp_yp ) continue;
			if( ship.shp_own == cnum ||
				ship.shp_own == 0 ) continue;
			if( shipflak != ship.shp_own ) {
				if( getrel(ship.shp_own, cnum) != AT_WAR ) continue;
			}
			j = ship.shp_type;
			if( j == S_BAT || j == S_DES || j == S_CAR ) {
				j = min((ship.shp_shels + 3) / 4, ship.shp_gun);
				flakshel += ((ship.shp_effc * j) / 100);
				flakker = ship.shp_own;
			}
		}
		if( flakshel == 0 ) return(nop);
		break;
	default:
		return(nop);
	}
	printf("\n\tF L A K !    ");
	j = nop;
/*
	This was the original statement.  The sqrt was not declared a double
	so the return code was treated as an integer.  This gave a distribution
	of approx. prcnt = 7 - 12.  In this version, sqrt has been declared
	a double, and the statement changed to give the same distribution
	as before.

	prcnt = sqrt(nop * 100.);

*/
	prcnt = sqrt(nop * 1.5) + 6;
	do {
		i = rand() % 5;
		while( i-- != 0 ) printf("\n");
		i = rand() % 60;
		while( i-- != 0 ) printf(" ");
		printf("POOF!   ");
		fflush(stdout);
		sleep(1);
		if( prcnt > rand() % 100 ) {
			printf("AAAaaaarghhH!!!\n");
			if( --j <=  0 ) break;
		} else {
			printf("\n");
		}
	} while( --flakshel != 0 );
	if( j == nop ) {
		sprintf(fmtbuf,"%d planes from country %d evaded flak at %s", nop, cnum, xytoa(sx, sy, flakker));
		wu(0, flakker, fmtbuf);
	} else {
		sprintf(fmtbuf,"%d out of %d country %d planes downed by flak at %s", nop - j, nop, cnum, xytoa(sx, sy, flakker));
		wu(0, flakker, fmtbuf);
	}
	nreport(flakker, N_FLAK, cnum);
	if( j <=  0 ) return(0);
	nop = j;
	return(nop);
}
