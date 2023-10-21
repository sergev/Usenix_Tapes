#define D_UPDATE
#define D_SECTDES
#define D_SCTSTR
#define D_SHPSTR
#define D_ICHRSTR
#define D_MCHRSTR
#define D_NSCSTR
#define D_FILES
#include	"empdef.h"

load()
{
	char	*sctp, *shpp, *getstri();
	short	*maxp;
	int	loru, maxamt, amt, item, shipnum;
	int	pcntflg;
	struct	nbstr	nb;
	struct	ichrstr	*ichrp;
	struct	mchrstr	*mchrp;

	if( snxtshp(&nb, argp[1], cnum, "Ship(s) ") == -1 ) return(SYN_RETURN);
	loru = (argp[0][0] == 'l') ? 1 : -1;
	pcntflg = (argp[2] != 0) ? atoi(argp[2]) : 0;
	while( nxtshp(&nb, &ship) != 0 ) {
		shipnum = nb.nb_sno;
		printf("\t%s #%d at %d, %d\n", mchr[ship.shp_type].m_name, shipnum, ship.shp_xp, ship.shp_yp);
		if( getsect(sx = ship.shp_xp, sy = ship.shp_yp, UP_ALL) == -1 ) continue;
		if( owner == 0 && chkok("") != 0 ) continue;
		if( sect.sct_desig != S_HARBR ) continue;
		if( wethr(sx, sy, 0) < 700 ) {
			printf("  Barometer @%d in %d,%d; seas too rough for stevedores.\n", wethr(sx, sy, 0), sx, sy);
			continue;
		}
		mchrp = &mchr[ship.shp_type];
		for( item=0; ichrp = &ichr[item], ichrp->i_name != 0; item++ ) {
			if( ichrp->i_mch == 0 ) continue;
			maxp = (short *)((int)ichrp->i_mch + (char *)mchrp);
			if( *maxp == 0 ) continue;
			shpp = (int)ichrp->i_shp + (char *)&ship;
			sctp = item + (char *)&sect.sct_owned;
			if( loru == 1 ) {
				maxamt = (*sctp < *maxp - *shpp) ? *sctp : *maxp - *shpp;
			} else {
				maxamt = (*shpp <= 127 - *sctp) ? *shpp : 127 - *sctp;
			}
			if( maxamt == 0 ) continue;
			if( pcntflg > 0 ) {
				amt = (pcntflg * maxamt + 50)/ 100;
				printf("%s : %s %d\n", ichrp->i_name, argp[0], amt);
			} else {
				sprintf(fmtbuf,"%s? (max %d) ", ichrp->i_name, maxamt);
				amt = atopi(getstri(fmtbuf));
			}
			amt = ((amt < maxamt) ? amt : maxamt) * loru;
			*sctp -= amt;
			*shpp += amt;
		}
		putsect(sx, sy);
		putship(shipnum, &ship);
	}
	return(NORM_RETURN);
}
