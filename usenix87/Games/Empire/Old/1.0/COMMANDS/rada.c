#define D_UPDATE
#define D_SECTDES
#define D_SHIPTYP
#define D_SCTSTR
#define D_SHPSTR
#define	D_DCHRSTR
#define D_MCHRSTR
#define D_NSCSTR
#define D_FILES
#include	"empdef.h"

rada()
{
	char	*cp, *getstar();
	double	sonar;
	struct	nstr	nsct;
	struct	nbstr	nb;

	cp = getstar(argp[1], "From? ");
	if( landorsea(cp) != LAND ) goto X226;
	if( snxtsct(&nsct, cp) < 0 ) return(SYN_RETURN);
X100:	
	if( nxtsct(&nsct, UP_OWN) != 0 ) goto X130;
	goto X514;
X130:	
	if( sect.sct_desig != S_RADAR ) goto X100;
	if( owner != 0 ) goto X162;
	if( chkok("") != 0 ) goto X100;
X162:	
	printf("radar at %d, %d\n", nsct.n_x, nsct.n_y);
	radmap(nsct.n_x, nsct.n_y, sect.sct_effic, .061, 0.);
	goto X100;
X226:	
	if( snxtshp(&nb, cp, cnum, "from ship(s)? ") != -1 ) goto X470;
	printf("Specify at least one ship");
	return(SYN_RETURN);
X300:	
	if( ship.shp_type == S_DES ) {
		sonar = .5;
	} else {
		sonar = 0.;
	}
	printf("%s %d at %d, %d\n", mchr[ship.shp_type].m_name, nb.nb_sno, ship.shp_xp, ship.shp_yp);
	radmap(ship.shp_xp, ship.shp_yp, ship.shp_effc, mchr[ship.shp_type].m_vrnge / 100., sonar);
X470:	
	if( nxtshp(&nb, &ship) != 0 ) goto X300;
X514:	
	return(NORM_RETURN);
}

radmap(rx, ry, eff, range, sonarang)
int	rx, ry, eff;
double	range, sonarang;
{
	register	i, j;
	char	screen[30][32];
	char	*cp;
	double	rangesq, q, bad_vis;

	bad_vis = 684.;
	range *= (float)eff;
	rangesq = range * range;
	sonarang *= rangesq;
	printf("Efficiency %d%%, barometer at %d, max range %.1f\n", eff, wethr(rx, ry, 0), range);
	lx = rx - range;
	ly = ry - range;
	hx = rx + range;
	hy = ry + range;
	j = ly;
	goto X624;
X232:	
	i = lx;
	goto X614;
X242:	
	screen[j - ly][i - lx] = ' ';
	q = idsq(xwrap(i - rx), ywrap(j - ry));
	if( q > rangesq ) goto X612;
	if( bad_vis <= wethr(i, j, 0) ) goto X434;
	screen[j - ly][i - lx] = '/';
	goto X612;
X434:	
	if( getsect(i, j, UP_NONE) == -1 ) goto X612;
	if( owner != 0 ) goto X514;
	if( sect.sct_desig <= S_RURAL ) goto X514;
	if( q >= rangesq / 8. ) goto X560;
X514:	
	screen[j - ly][i - lx] = dchr[sect.sct_desig].d_mnem;
	goto X612;
X560:	
	screen[j - ly][i - lx] = '?';
X612:	
	i++;
X614:	
	if( i <= hx ) goto X242;
	j++;
X624:	
	if( j <= hy ) goto X232;
	i = 0;
	goto X1166;
X640:	
	if( ship.shp_own == 0 ) goto X1164;
	if( lx >  ship.shp_xp ) goto X1164;
	if( ly >  ship.shp_yp ) goto X1164;
	if( hx <  ship.shp_xp ) goto X1164;
	if( hy <  ship.shp_yp ) goto X1164;
	if( screen[ship.shp_yp - ly][ship.shp_xp - lx] == '/' ) goto X1164;
	rangesq = mchr[ship.shp_type].m_visib * range / 20.;
	rangesq *= rangesq;
	q = idsq(ship.shp_xp - rx, ship.shp_yp - ry);
	if( q > rangesq ) goto X1164;
	if( ship.shp_type != S_SUB ) goto X1110;
	if( q > sonarang ) goto X1164;
X1110:	
	screen[ship.shp_yp - ly][ship.shp_xp -lx] = *mchr[ship.shp_type].m_name & ~040;
X1164:	
	i++;
X1166:	
	if( getship(i, &ship) != -1 ) goto X640;
	j = 0;
	goto X1364;
X1214:	
	cp = junk;
	i = 0;
	goto X1312;
X1226:	
	*cp = screen[j][i];
	cp++;
	if( *(cp-1) != '/' ) goto X1276;
	*cp = '/';
	goto X1304;
X1276:	
	*cp = ' ';
X1304:	
	cp++;
	i++;
X1312:	
	if( i <= hx - lx ) goto X1226;
	cp--;
	*cp = '\0';
	printf("%s\n", junk);
	j++;
X1364:	
	if( j <= hy - ly ) goto X1214;
}
