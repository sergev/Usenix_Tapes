#define D_UPDATE
#define D_NATSTAT
#define D_SECTDES
#define D_NEWSVERBS
#define D_TRTYCLAUSE
#define D_NATSTR
#define D_SCTSTR
#define D_SHPSTR
#define	D_DCHRSTR
#define D_MCHRSTR
#define D_FILES
#include	"empdef.h"
#include	<stdio.h>

assa()
{
	register	at, vt, odds;
	char	 *xytoa();
	int	aship, vnum, dam;
	short	dx, dy;
	double	vloss, aloss, vd, dsq, vrng, landgun(), tfact();

	if( sargs(argp[1]) == -1 ) return(SYN_RETURN);
	if( getsect(lx, ly, UP_ALL) == -1 ||
	    sect.sct_desig == S_WATER ||
	    sect.sct_desig == S_SANCT ||
	    sect.sct_desig == S_MOUNT ) {
		printf("Don't bother...");
		return(SYN_RETURN);
	}
	vnum = sect.sct_owned;
	if( vnum == 0 ) goto X174;
	if( trechk(cnum, vnum, SEAATT) == -1 ) return(FAIL_RETURN);
X174:
	sprintf(fmtbuf,"Assault sector %d, %d from ship #", lx, ly);
	aship = getshno(argp[2], fmtbuf, &ship);
	if( aship == -1 ) goto X264;
	if( cnum == ship.shp_own ) goto X274;
X264:	
	printf("Not yours");
	return(SYN_RETURN);
X274:	
	dx = xwrap(ship.shp_xp - lx);
	dy = ywrap(ship.shp_yp - ly);
	if( dx*dx + dy*dy <= 1 ) goto X374;
	printf("You'll have to get there first...");
	return(FAIL_RETURN);
X374:	
	if( mchr[ship.shp_type].m_milit == 0 ) goto X420;
	if( ship.shp_crew != 0 ) goto X444;
X420:	
	printf("No military on ship #%d", aship);
	return(FAIL_RETURN);
X444:	
	if( wethr(lx, ly, 0) >= 700 ) goto X534;
	printf("Barometer @%.0f; Seas too rough to assault...", wethr(lx, ly, 0));
	return(SYN_RETURN);
X534:	
	if( sect.sct_owned != 0 ) goto X574;
	if( sect.sct_milit != 0 ) goto X574;
	printf("Sector %d, %d is undefended, (a piece of cake)\n", lx, ly);
	goto X700;
X574:	
	printf("Sector %d, %d is a %d%% %s with approx. %d military.\n", lx, ly,round(sect.sct_effic, 10), dchr[sect.sct_desig].d_name, round(sect.sct_milit, 10));
X700:	
	sprintf(fmtbuf,"Number of troops in assault? (max %d) ", ship.shp_crew);
	at = onearg(argp[3], fmtbuf);
	at = (at < ship.shp_crew) ? at : ship.shp_crew;
	if( at == 0 ) return(FAIL_RETURN);
	ship.shp_crew -= at;
	putship(aship, &ship);
	vd = ((dchr[sect.sct_desig].d_dstr / 2.) + (-1.)) * (sect.sct_effic / 100.) + 1.;
	vt = sect.sct_milit * vd;
	odds = (at * 32767.) / (at + vt + vt);
	printf("Your success odds are %.1f%%\n", odds / 327.67);
	sigsave();
	if( vnum == 0 ) goto X1264;
	sprintf(fmtbuf,"Country #%d assaulted you @%s", cnum, xytoa(lx, ly, vnum));
	wu(0, vnum, fmtbuf);
X1264:	
	vloss = aloss = 0;
	if( vt != 0 ) goto X1306;
	goto X2132;
X1306:	
	if( sect.sct_dfend != 0 ) goto X1320;
	goto X2132;
X1320:	
	dx = sect.sct_dfend<<8;
	dx = (dx>>12) + lx;
	dy = sect.sct_dfend<<12;
	dy = (dy>>12) + ly;
	dsq = dx*dx + dy*dy;
	getsect(dx, dy, UP_NONE);
	vrng = tfact(vnum, (float)((sect.sct_guns < 7) ? sect.sct_guns : 7) * sect.sct_effic  * .01);
	if( owner == 0 ) goto X1760;
	if( sect.sct_desig != S_FORTR ) goto X1760;
	if( sect.sct_shell == 0 ) goto X1760;
	if( dsq >  vrng*vrng ) goto X1760;
	printf("Incoming shell!\n");
	nreport(vnum, N_FIRE_BACK, cnum);
	sect.sct_shell--;
	putsect(dx, dy);
	dam = shelldam(landgun(sect.sct_effic, seadef(ship.shp_type)));
	shipdam(&ship, dam);
	putship(aship, &ship);
	if( ship.shp_own != 0 ) goto X1734;
	odds = 0;
X1734:	
	odds = (((rand() % 32768) / 100000.) + .6) * odds;
X1760:	
	getsect(lx, ly, UP_NONE);
	goto X2132;
X2002:	
	printf("!");
	vloss += 1. / vd;
	vt--;
	goto X2132;
X2042:	
	if( at <= 0 ) goto X2136;
	if( rand() % 32768 <= 20000 ) goto X2074;
	fflush(stdout);
	sleep(1);
X2074:	
	if( odds >= rand() % 32768 ) goto X2002;
	printf("@");
	aloss += 1.;
	at--;
X2132:	
	if( vt >  0 ) goto X2042;
X2136:	
	if( at <= vt ) goto X2460;
	printf("You have taken sector %d, %d\n", lx, ly);
	if( vnum == 0 ) goto X2264;
	sprintf(fmtbuf,"& lost %.1f taking %s\n", aloss, xytoa(lx, ly, vnum));
	wu(0, vnum, fmtbuf);
X2264:	
	nreport(cnum, N_WON_SECT, vnum);
	sect.sct_owned = cnum;
	sect.sct_lstup = curup;
	sect.sct_milit = at;
	sect.sct_chkpt = sect.sct_dfend = 0;
	if( sect.sct_desig != S_CAPIT ) goto X2566;
	getnat(vnum);
	if( lx != nat.nat_xcap - capx ) goto X2566;
	if( ly != nat.nat_ycap - capy ) goto X2566;
	printf("which happens to be %s's capital!\n", nat.nat_cnam);
	sect.sct_desig = S_FORTR;
	nat.nat_stat = STAT_NOCAP;
	putnat(vnum);
	goto X2566;
X2460:	
	printf("You have been defeated!\n");
	sprintf(fmtbuf," & lost %.1f", aloss);
	wu(0, vnum, fmtbuf);
	nreport(cnum, N_SCT_LOSE, vnum);
	sect.sct_milit = vt / vd;
X2566:	
	printf("Casualties :\nYours... %.1f\n", aloss);
	printf("Theirs.. %.1f\n", vloss);
	putsect(lx, ly);
	ntused = (ntused + (aloss + vloss) * .15) + .5;
	return(NORM_RETURN);
}
