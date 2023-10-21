/* There were several sections that didn't converge 06/19/83 */

#define D_UPDATE
#define D_NATSTAT
#define D_SECTDES
#define D_SHIPTYP
#define D_NEWSVERBS
#define D_NATSTR
#define D_SCTSTR
#define D_SHPSTR
#define D_MCHRSTR
#define D_NSCSTR
#define D_FILES
#include	"empdef.h"

navi()
{
	register	i, j;
	char	*cp, *cname(), *getstri();
	int	fltsiz, x, y, dx, dy, end, view;
	int	snums[NBLISTMAX];
	double	w, cst, mu[NBLISTMAX], minmu, movdist, tfact();
	struct	nbstr	nb;
	struct	shpstr	sh[NBLISTMAX], *fs, *sp;

	if( snxtshp(&nb, argp[1], cnum, "Ship(s)? ") == -1 ) return(SYN_RETURN);
	minmu = 10000.;
	i = 0;
X72:	
	if( nxtshp(&nb, &sh[i]) != 0 ) goto X134;
	goto X662;
X134:	
	snums[i] = nb.nb_sno;
	sp = &sh[i];
	mu[i] = sp->shp_mbl;
	if( mu[i] > 0. ) goto X362;
	printf("%s #%d is out of mobility & stays in %d,%d\n", mchr[sp->shp_type].m_name, snums[i], sp->shp_xp, sp->shp_yp);
	if( i == 0 ) return(FAIL_RETURN);
	snums[i] = -1;
	goto X426;
X362:	
	if( minmu <= mu[i] ) goto X426;
	minmu = mu[i];
X426:	
	if( (w = wethr(sp->shp_xp, sp->shp_yp, 0)) >= 715. ) goto X646;
	fs = &sh[0];
	goto X552;
X514:	
	if( fs->shp_xp != sp->shp_xp ) goto X544;
	if( fs->shp_yp == sp->shp_yp ) goto X562;
X544:	
	fs++;
X552:	
	if( fs < sp ) goto X514;
X562:	
	if( fs < sp ) goto X646;
	printf("Barometer @%.0f in %d, %d\n", w, sp->shp_xp, sp->shp_yp);
X646:	
	i++;
	if( i >= NBLISTMAX ) goto X662;
	goto X72;
X662:	
	if( i == 0 ) return(FAIL_RETURN);
	fltsiz = i;
	fs = sh;
	end = 0;
	cp = argp[2];
	if( cp != 0 ) goto X722;
	cp = "";
X722:	
	sigsave();
X732:	
	w = (wethr(fs->shp_xp, fs->shp_yp, 0) + (-300) < 430) ? wethr(fs->shp_xp, fs->shp_yp, 0) + (-300) : 430;
	view = 0;
	dx = dy = 0;
	if( *cp != '\0' ) goto X1166;
	sprintf(fmtbuf,"<%.1f:%.1f: %d, %d> ", mu[0], minmu, fs->shp_xp, fs->shp_yp);
	cp = getstri(fmtbuf);
X1166:	
	movdist = 1.;
	j = *cp;
	cp++;
	if( j != 'u' ) goto X1440;
X1214:	
	dy = -1;
X1222:	
	minmu = 10000.;
	i = 0;
	goto X4220;
X1240:	
	if( snums[i] != -1 ) goto X1262;
	goto X4216;
X1262:	
	sp = &sh[i];
	x = xwrap(sp->shp_xp + dx);
	y = ywrap(sp->shp_yp + dy);
	w = (wethr(x, y, 0) + (-300) >= 430) ? wethr(x, y, 0) + (-300) : 430;
	goto X2016;
X1440:	
	if( j != 'd' ) goto X1456;
X1446:	
	dy = 1;
	goto X1222;
X1456:	
	if( j != 'l' ) goto X1474;
	dx = -1;
	goto X1222;
X1474:	
	if( j != 'r' ) goto X1512;
	dx = 1;
	goto X1222;
X1512:	
	if( j != 'v' ) goto X1526;
	view++;
	goto X1222;
X1526:	
	if( j != 'e' ) goto X1542;
	end++;
	goto X1222;
X1542:	
	if( j != '/' ) goto X1642;
	movdist = 1.414;
	j = *cp;
	cp++;
	if( j == 'l' ) goto X1604;
	if( j != 'd' ) goto X1614;
X1604:	
	dx = -1;
	goto X1446;
X1614:	
	if( j == 'r' ) goto X1630;
	if( j != 'u' ) goto X1742;
X1630:	
	dx = 1;
	goto X1214;
X1642:	
	if( j != '\\' ) goto X1742;
	movdist = 1.414;
	j = *cp;
	cp++;
	if( j == 'l' ) goto X1704;
	if( j != 'u' ) goto X1716;
X1704:	
	dx = -1;
	goto X1214;
X1716:	
	if( j == 'r' ) goto X1732;
	if( j != 'd' ) goto X1742;
X1732:	
	dx = 1;
	goto X1446;
X1742:	
	printf("??? u=up, d=down, l=left, r=right, \\l=upleft,\n");
	printf(" /r=upright, /l=downleft, \\r=downright,\n");
	printf("v=view, e=end\n");
	cp =  "";
	goto X732;
X2016:
	if( view != 0 ) goto X2042;
	if( end != 0 ) goto X2042;
	if( nstat != STAT_GOD ) goto X2050;
X2042:	
	cst = 0;
	goto X2176;
X2050:	
	cst = sp->shp_effc * mchr[sp->shp_type].m_speed * .01 * w;
	cst = (tfact(cnum, cst) + cst) / 2.;
	cst = movdist * 103200. / cst;
X2176:	
	if( getsect(x, y, UP_NONE) >= 0 ) goto X2226;
	goto X3156;
X2226:	
	if( view == 0 ) goto X2306;
	printf("%s #%d ", mchr[sp->shp_type].m_name, snums[i]);
X2306:	
	if( sect.sct_desig != S_WATER ) goto X2652;
	if( view == 0 ) goto X2352;
	printf("on open sea @ %d,%d\n", x, y);
X2352:	
	sp->shp_xp = x;
	sp->shp_yp = y;
	mu[i] -= cst;
	if( mu[i] < 0 ) goto X2460;
	if( end != 0 ) goto X2460;
	goto X3324;
X2460:	
	printf("%s #%d stopped at %d,%d\n", mchr[sp->shp_type].m_name, snums[i], sp->shp_xp, sp->shp_yp);
X2560:	
	sp->shp_mbl = mu[i];
	putship(snums[i], sp);
	snums[i] = -1;
	goto X4216;
X2652:	
	if( sect.sct_desig != S_BSPAN ) goto X2706;
	if( view == 0 ) goto X2352;
	printf("under bridge span @ %d,%d\n", x, y);
	goto X2352;
X2706:	
	if( sect.sct_desig != S_HARBR ) goto X3156;
	if( sect.sct_effic < 2 ) goto X3156;
	if( view == 0 ) goto X2774;
	printf("in %d%% harbor @ %d,%d\n", sect.sct_effic, x, y);
X2774:	
	if( owner == 0 ) goto X3006;
	goto X2352;
X3006:	
	if( (j = chkok("")) <  0 ) goto X3026;
	goto X2352;
X3026:	
	if( j != -2 ) goto X3144;
	printf("%s #%d impounded @ %d,%d by %s\n", mchr[sp->shp_type].m_name, snums[i], x, y, cname(sect.sct_owned));
	sp->shp_own = sect.sct_owned;
	goto X2560;
X3144:	
	if( j == -1 ) goto X3156;
	goto X2352;
X3156:	
	printf("%s #%d can't go to %d,%d\n", mchr[sp->shp_type].m_name, snums[i], x, y);
	cp = "";
	if( minmu >  mu[i] ) goto X3300;
	goto X4216;
X3300:	
	minmu = mu[i];
	goto X4216;
X3324:	
	if( minmu <= mu[i] ) goto X3370;
	minmu = mu[i];
X3370:	
	if( view == 0 ) goto X3402;
	goto X4216;
X3402:	
	if( sp->shp_type != S_MIN ) goto X3514;
	if( sect.sct_shell == 0 ) goto X3514;
	if( owner != 0 ) goto X3514;
	j = 0;
X3432:	
	if( sect.sct_shell == 0 ) goto X3466;
	if( rand() % 32768 >= 20000 ) goto X3466;
	printf("Sweep...\n");
	sect.sct_shell--;
X3466:	
	j++;
	if( j < 5 ) goto X3432;
	putsect(x, y);
X3514:	
	if( sect.sct_desig == S_HARBR ) goto X4002;
	if( sect.sct_desig == S_BSPAN ) goto X4002;
	if( sect.sct_shell == 0 ) goto X4002;
	if( (rand() % 32768) / sect.sct_shell >= 1638 ) goto X4002;
	printf("Kawhomp! Mine detected by %s #%d in %d,%d!\n", mchr[sp->shp_type].m_name, snums[i], x, y);
	nreport(cnum, N_HIT_MINE, 0);
	j = rand() % 25;
	j += 25;
	shipdam(sp, j);
	sect.sct_shell--;
	putsect(x, y);
	cp = "";
	printf("%d%% damage sustained\n", j);
X4002:	
	if( w >= 400. ) goto X4216;
	if( w >= rand() % 450 ) goto X4216;
	j = rand() % 10;
	j += 15;
	printf("%s #%d in %d,%d reports %d%% storm damage!\n", mchr[sp->shp_type].m_name, snums[i], x, y, j);
	shipdam(sp, j);
	nreport(cnum, N_SHP_STORM, 0); 
	cp = "";
X4216:	
	i++;
X4220:	
	if( i >= fltsiz ) goto X4232;
	goto X1240;
X4232:	
	if( end == 0 ) goto X4246;
	return(NORM_RETURN);
X4246:	
	if( snums[0] == -1 ) goto X4262;
	goto X732;
X4262:	
	cp = "e";
	goto X732;
}
