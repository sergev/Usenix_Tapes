#define D_SHIPTYP
#define D_SHPSTR
#define D_MCHRSTR
#define D_FILES
#include	"empdef.h"

tend()
{
	register	maximum, amt;
	int	vship, tship;
	char	*cp, *splur(), *getstri();
	struct	shpstr	as;

	vship = getshno(argp[1], "Ship to be tended? ", &ship);
	if( vship == -1 ) {
		printf("Designate one ship");
		return(SYN_RETURN);
	}
	if( cnum != ship.shp_own ) {
		printf("Not your ship");
		return(SYN_RETURN);
	}
	tship = getshno(argp[2], "From tender #", &as);
	if( tship == -1 || cnum != as.shp_own ) {
		printf("Not yours");
		return(SYN_RETURN);
	}
	if( as.shp_type != S_TEN ) {
		printf("You can't tend from a %s!",
			mchr[as.shp_type].m_name);
		return(FAIL_RETURN);
	}
	if( as.shp_xp != ship.shp_xp ||
	    as.shp_yp != ship.shp_yp ) {
		printf("Deck hands can't throw cargo that far");
		return(FAIL_RETURN);
	}
	if( wethr(as.shp_xp, as.shp_yp, 0) < 700 ) {
		printf("Barometer @%.0f;  Seas too rough", 
			wethr(as.shp_xp, as.shp_yp, 0));
		return(SYN_RETURN);
	}
	if( ship.shp_type == S_FRE ) {
		printf("Sorry I can't service that kind of ship");
		return(FAIL_RETURN);
	}
	maximum = min(as.shp_crew, mchr[ship.shp_type].m_milit - ship.shp_crew);
	if( maximum > 0 ) {
		sprintf(fmtbuf,"Crew (max %d)? ", maximum);
		cp = getstri(fmtbuf);
		amt = min(atoi(cp), maximum);
		as.shp_crew -= amt;
		ship.shp_crew += amt;
	}
	maximum = min(as.shp_gun, mchr[ship.shp_type].m_gun - ship.shp_gun);
	if( maximum > 0 ) {
		sprintf(fmtbuf,"Gun%s (max %d)? ", splur(maximum), maximum);
		cp = getstri(fmtbuf);
		amt = min(atoi(cp), maximum);
		as.shp_gun -= amt;
		ship.shp_gun += amt;
	}
	maximum = min(as.shp_shels, mchr[ship.shp_type].m_shels - ship.shp_shels);
	if( maximum > 0 ) {
		sprintf(fmtbuf,"Shells (max %d)? ", maximum);
		cp = getstri(fmtbuf);
		amt = min(atoi(cp), maximum);
		as.shp_shels -= amt;
		ship.shp_shels += amt;
	}
	putship(vship, &ship);
	putship(tship, &as);
	return(NORM_RETURN);
}
