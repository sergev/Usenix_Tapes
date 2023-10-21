#define D_SHPSTR
#define D_MCHRSTR
#define D_FILES
#include	"empdef.h"

getship(n, str)
int	n;
struct	shpstr	*str;
{
	register	t;
	long	addr, lseek();

	addr = (long)n * sizeof(ship);
	lseek(shipf, addr, 0);
	if( read(shipf, str, sizeof(ship)) != sizeof(ship) ) return(-1);
	str->shp_xp = xwrap(str->shp_xp - capx);
	str->shp_yp = ywrap(str->shp_yp - capy);
	if( curup < str->shp_lstp ) {
		str->shp_lstp = curup - 24;
		putship(n, str);
	}
	t = max127(curup - str->shp_lstp);
	if( t < 3 ) return(0);
	str->shp_mbl = max127(str->shp_mbl + t);
	str->shp_effc = (str->shp_effc + t < 100) ? str->shp_effc + t : 100;
	str->shp_lstp = curup;
	if( t == 127 ) putship(n, str);
	return(0);
}

putship(n, str)
int	n;
struct	shpstr	*str;
{
	long	addr, lseek();

	if( str->shp_effc <= 20 ) {
		printf("%s #%d sunk!\n", mchr[str->shp_type].m_name, n);
		if( cnum != str->shp_own ) {
			sprintf(fmtbuf,"%s #%d sunk!\n", mchr[str->shp_type].m_name, n);
			wu(0, str->shp_own, fmtbuf);
		}
		str->shp_own = 0;
	}
	addr = n * sizeof(ship);
	lseek(shipf, addr, 0);
	str->shp_xp += capx;
	str->shp_yp += capy;
	write(shipf, str, sizeof(ship));
	str->shp_xp -= capx;
	str->shp_yp -= capy;
	return(0);
}
