#define D_SHIPTYP
#define D_NEWSVERBS
#define D_SHPSTR
#define D_MCHRSTR
#define D_FILES
#include	"empdef.h"
#include	<stdio.h>

torp()
{
	register	i, vnum;
	char	 *xytoa();
	int	count, dam, chance, aship, vship;
	struct	shpstr	shp, as, vs;
	double	dsq, dx, dy, rangesq, tfact();

	vship = getshno(argp[1], "Victim ship #? ", &vs);
	if( vship == -1 || vs.shp_own == 0 ) {
		printf("Victim ship # ?");
		return(SYN_RETURN);
	}
	aship = getshno(argp[2], "From sub #", &as);
	if( aship == -1 || cnum != as.shp_own ) {
		printf("Not yours");
		return(SYN_RETURN);
	}
	if( as.shp_type == S_SUB ) goto X144;
	printf("Not a sub, dingaling");
	return(FAIL_RETURN);
X144:	
	if( as.shp_gun == 0 ) goto X162;
	if( as.shp_shels >= 3 ) goto X172;
X162:	
	printf("Insufficient armament");
	return(FAIL_RETURN);
X172:	
	if( as.shp_effc >= 60 ) goto X212;
	printf("Torpedo tubes inoperative.");
	return(FAIL_RETURN);
X212:	
	sigsave();
	rangesq = (tfact(cnum, 2.) * as.shp_effc) / 100.;
	printf("Effective torpedo range is %.1f\n", rangesq);
	rangesq = rangesq * rangesq;
	as.shp_shels -= 3;
	if( as.shp_mbl <= 0 ) goto X346;
	as.shp_mbl = 0;
X346:	
	if( as.shp_mbl > -120 ) as.shp_mbl -= 10;
	putship(aship, &as);
	printf("Blooop... ");
	getship(vship, &vs);
	vnum = vs.shp_own;
	dx = xwrap(as.shp_xp - vs.shp_xp);
	dy = ywrap(as.shp_yp - vs.shp_yp);
	dsq = dx*dx + dy*dy;
	i = dsq + .5;
	count = (i > 5) ? 9 : i + 4;
	do {
		printf("%d... ", count);
		fflush(stdout);
		sleep(1);
	} while( --count >= 0 );
	chance = 90 / (i + i + i + 1);
	if( dsq > rangesq ) goto X1200;
	if( chance <= (rand()>>3)%100 ) goto X1200;
	printf("BOOM!...");
	sprintf(fmtbuf,"Sub #%d @%s torpedoed %s %d", aship, xytoa(as.shp_xp, as.shp_yp, vnum), mchr[vs.shp_type].m_name, vship);
	wu(0, vnum, fmtbuf);
	dam = rand()%50 + 50;
	dam = (dam * 100) / (mchr[vs.shp_type].m_prdct + 50);
	shipdam(&vs, dam);
	putship(vship, &vs);
	nreport(vnum, N_TORP_SHIP, 0);
	goto X1336;
X1200:	
	if( dsq <= rangesq ) goto X1226;
	printf("Out of range\n");
	goto X1336;
X1226:	
	printf("Missed\n");
	sprintf(fmtbuf,"Torpedo sighted @%s by %s %d", xytoa(as.shp_xp, as.shp_yp, vnum), mchr[vs.shp_type].m_name, vship);
	wu(0, vnum, fmtbuf);
X1336:	
	i = 0;
	goto X1632;
X1342:	
	if( vnum != shp.shp_own ) goto X1630;
	if( shp.shp_type != S_DES ) goto X1630;
	if( shp.shp_shels == 0 ) goto X1630;
	if( shp.shp_xp != as.shp_xp ) goto X1630;
	if( shp.shp_yp != as.shp_yp ) goto X1630;
	printf("\n\tCAPTAIN!  !!Depth charges!!...\n");
	nreport(vnum, N_FIRE_BACK, cnum);
	sprintf(fmtbuf,"Destroyer #%d dropped a depth charge on sub #%d", i, aship);
	wu(0, vnum, fmtbuf);
	shp.shp_shels--;
	putship(i, &shp);
	dam = (rand() % 32768) / 3000 + 30;
	getship(aship, &as);
	shipdam(&as, dam);
	putship(aship, &as);
	if( as.shp_own == 0 ) goto X1656;
X1630:	
	i++;
X1632:	
	if( getship(i, &shp) != -1 ) goto X1342;
X1656:	
	return(NORM_RETURN);
}
