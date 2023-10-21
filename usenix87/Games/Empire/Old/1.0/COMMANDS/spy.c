#define	D_ICHRSTR
#define	D_NEWSVERBS
#define	D_TRTYCLAUSE
#define	D_NATSTAT
#define	D_SECTDES
#define	D_DCHRSTR
#define	D_UPDATE
#define	D_SCTSTR
#define	D_NSCSTR
#define	D_FILES
#include	"empdef.h"

spy()
{
	register	i, j;
	char	chart[36][36], *cname(), *xytoa();
	int	slx, shx, sly, shy, spies, k, vnum;
	int	nsects, pkgs, xoff, yoff;
	struct	nstr	nsct;

	if( snxtsct(&nsct, argp[1]) == -1 ) return(SYN_RETURN);
	slx = nsct.n_lx - nsct.n_ix;
	shx = nsct.n_hx + nsct.n_ix;
	sly = nsct.n_ly - nsct.n_iy;
	shy = nsct.n_hy + nsct.n_iy;
	if( (shx - slx) * nsct.n_ix >= 36 ) goto X204;
	if( (shy - sly) * nsct.n_iy < 36 ) goto X214;
X204:	
	printf("Too large an area (max range is 32)");
	return(SYN_RETURN);
X214:	
	xoff = (slx < shx) ? slx : shx;
	yoff = (sly < shy) ? sly : shy;
	j = sly;
	goto X370;
X276:	
	i = slx;
	goto X344;
X304:	
	chart[i-xoff][j-yoff] = 0;
	i += nsct.n_ix;
X344:	
	if( i != shx ) goto X304;
	j += nsct.n_iy;
X370:	
	if( j != shy ) goto X276;
	goto X646;
X410:	
	if( owner == 0 ) goto X646;
	chart[nsct.n_x-xoff][nsct.n_y-yoff] = -128;
	chart[nsct.n_x-xoff][nsct.n_y-yoff-1]++;
	chart[nsct.n_x-xoff][nsct.n_y-yoff+1]++;
	chart[nsct.n_x-xoff-1][nsct.n_y-yoff]++;
	chart[nsct.n_x-xoff+1][nsct.n_y-yoff]++;
X646:	
	if( nxtsct(&nsct, UP_NONE) >  0 ) goto X410;
	nsects = 0;
	j = sly;
	goto X2722;
X704:	
	i = slx;
	goto X2702;
X714:	
	spies = chart[i-xoff][j-yoff];
	if( spies >  0 ) goto X760;
	goto X2674;
X760:	
	getsect(i, j, UP_NONE);
	if( owner == 0 ) goto X1006;
	goto X2674;
X1006:	
	if( sect.sct_owned != 0 ) goto X1030;
	if( sect.sct_desig > S_RURAL ) goto X1030;
	goto X2674;
X1030:	
	vnum = sect.sct_owned;
	if( vnum == 0 ) goto X1076;
	if( trechk(cnum, vnum, TRTSPY) != -1 ) goto X1076;
	goto X2674;
X1076:	
	if( sect.sct_milit >  rand()%((spies * 100) + 100) ) goto X1144;
	goto X1644;
X1144:	
	if( nstat != STAT_GOD ) goto X1160;
	goto X1644;
X1160:	
	k = getrel(vnum, cnum);
	if( k != ALLIED ) goto X1214;
	goto X1644;
X1214:	
	if( k == AT_WAR ) goto X1350;
	printf("Spy deported from %d,%d.\n", i, j);
	if( vnum != 0 ) goto X1260;
	goto X2674;
X1260:	
	sprintf(fmtbuf,"%s spy deported from %s\n", cname(cnum), xytoa(i, j, vnum));
	wu(0, vnum, fmtbuf);
	goto X2674;
X1350:	
	neigh(i, j, cnum, 0);
	getsect(nbrx, nbry, UP_ALL);
	if( sect.sct_milit == 0 ) goto X1430;
	sect.sct_milit--;
	goto X1434;
X1430:	
	sect.sct_civil--;
X1434:	
	putsect(nbrx, nbry);
	printf("BANG!! a spy from %d,%d was shot in %d,%d!\n", nbrx, nbry, i, j);
	if( vnum != 0 ) goto X1522;
	goto X2674;
X1522:	
	nreport(cnum, N_SPY_SHOT, vnum);
	sprintf(fmtbuf,"%s spy from %s shot in %s\n", cname(cnum), xytoa(nbrx, nbry, vnum), xytoa(i, j, vnum));
	wu(0, vnum, fmtbuf);
	goto X2674;
X1644:	
	if( nsects++ != 0 ) goto X1670;
	printf("  sect   #    eff  civ mil  shl  gun  ore  pln\n");
X1670:	
	if( curup - sect.sct_lstup <= 3 ) goto X1724;
	update(i, j, UP_ALL);
X1724:	
	printf("%3d,%-3d%3d ", i, j, sect.sct_owned);
	if( wethr(i, j, 0) >= 700 ) goto X2026;
	if( nstat == STAT_GOD ) goto X2026;
	printf("   No report due to bad weather.\n");
	goto X2674;
X2026:	
	pkgs = dchr[sect.sct_desig].d_pkg;
	k = 12;
	printf("%c %3d%% %4d", dchr[sect.sct_desig].d_mnem, round(sect.sct_effic, 10), round(sect.sct_civil, 10) * ichr[k].i_pkg[pkgs]);
	k = 13;
	printf(" %3d", round(sect.sct_milit, 10) * ichr[k].i_pkg[pkgs]);
	k = 14;
	printf(" %4d", round(sect.sct_shell, 10) * ichr[k].i_pkg[pkgs]);
	k = 15;
	printf(" %4d", round(sect.sct_guns, 5) * ichr[k].i_pkg[pkgs]);
	k = 17;
	printf(" %4d", round(sect.sct_ore, 10) * ichr[k].i_pkg[pkgs]);
	k = 16;
	printf(" %3d\n", round(sect.sct_plane, 3) * ichr[k].i_pkg[pkgs]);
X2674:	
	i += nsct.n_ix;
X2702:	
	if( i == shx ) goto X2714;
	goto X714;
X2714:	
	j += nsct.n_iy;
X2722:	
	if( j == shy ) goto X2734;
	goto X704;
X2734:	
	ntused = ntused + (nsects * .25) + .5;
	return(NORM_RETURN);
}
