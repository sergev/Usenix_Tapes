#define D_UPDATE
#define D_NATSTAT
#define D_NEWSVERBS
#define D_NATSTR
#define D_SCTSTR
#define D_SHPSTR
#define D_MCHRSTR
#define D_TRTSTR
#define D_LONSTR
#define D_FILES
#include	"empdef.h"

diss()
{
	register	i, j;
	char	 *cname();

	cleared();
	for( j = -w_ysize / 2; j < w_ysize / 2; j++ ) {
		for( i = -w_xsize / 2; i < w_xsize / 2; i++ ) {
			getsect(i, j, UP_OWN);
			if( cnum != sect.sct_owned ) continue;
			printf("Bye-bye %d people in %d,%d\n", sect.sct_civil + sect.sct_milit, i, j);
			sect.sct_owned = 0;
			putsect(i, j);
		}
	}
	for( i = 0; getship(i, &ship) != -1; i++ ) {
		if( cnum != ship.shp_own ) continue;
		printf("%s #%d ", mchr[ship.shp_type].m_name, i);
		switch((rand()>>4) % 4) {
		case 0:
			printf("scuttled!\n");
			ship.shp_own = 0;
			break;
		case 1:
			printf("abandoned!\n");
			ship.shp_crew = 0;
			break;
		case 2:
			j = rand() % 127;
			j++;
			printf("has turned mercenary (for sale at %d/ton)\n", j);
			ship.shp_spric = j;
			break;
		case 3:
			j = rand() % 80;
			j += 10;
			printf("crew has rioted! (%d%% damage)\n", j);
			shipdam(&ship, j);
			break;
		}
		putship(i, &ship);
	}
	for( i = 0; gettre(i) != -1; i++ ) {
		if( cnum == trty.trt_cna ||
		    cnum == trty.trt_cnb ) {
			j = (cnum == trty.trt_cna) ? trty.trt_cnb : trty.trt_cna;
			printf("No more treaty with %s\n", cname(j));
			sprintf(fmtbuf,"Treaty #%d with %s voided", i, cname(cnum));
			wu(0, j, fmtbuf);
			trty.trt_cna = trty.trt_cnb = 0;
			puttre(i);
		}
	}
	for( i = 0; getloan(i) != -1; i++ ) {
		if( cnum == loan.l_loner ||
		    cnum == loan.l_lonee ) {
			if( cnum == loan.l_loner ) {
				printf("Loan #%d to %s will never be repaid\n", i, cname(loan.l_lonee));
				sprintf(fmtbuf,"Loan #%d from %s has been voided\n", i, cname(cnum));
				wu(0, loan.l_lonee, fmtbuf);
			} else {
				printf("Loan #%d from %s will never be repaid\n", i, cname(loan.l_loner));
				sprintf(fmtbuf,"Loan #%d to %s has been voided\n", i, cname(cnum));
				wu(0, loan.l_loner, fmtbuf);
			}
		loan.l_loner = loan.l_lonee = 0;
		putloan(i);
		}
	}
	getnat(cnum);
	nat.nat_stat = STAT_DEAD;
	putnat(cnum);
	nreport(cnum, N_DISS_GOV, 0);
	return(NORM_RETURN);
}
