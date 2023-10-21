#define D_UPDATE
#define D_NATSTAT
#define D_SECTDES
#define D_NEWSVERBS
#define	D_TRTYCLAUSE
#define D_NATSTR
#define D_SCTSTR
#define D_SHPSTR
#define D_DCHRSTR
#define D_ICHRSTR
#define D_MCHRSTR
#define D_FILES
#include	"empdef.h"
#include	<stdio.h>

atta()
{
	register	i, nsects;
	char	 *xytoa(), *cname(), *getstri();
	int	vnum, vx, vy, defx, defy, vt, at;
	short	x, y;
	int	vd, tat, tats, odds, vdes, temp;
	int	success, vmovcst, movcost, dam;
	double	vloss, aloss, occper, q, vgun, tfact(), landgun();
	struct	{
		int	u_x;
		int	u_y;
		float	u_q;
		int	u_at;
	}	unit[4];

	if( sargs(argp[1]) == -1 || 
	    getsect(lx, ly, UP_ALL) == -1 ) return(SYN_RETURN);
	if( neigh(lx, ly, cnum, UP_NONE) != 0 ) goto X150;
	printf("You are not adjacent to %d,%d\n", lx, ly);
	return(FAIL_RETURN);
X150:	
	if( sect.sct_desig == S_WATER ||
	    sect.sct_desig == S_SANCT ) {
		printf("%d,%d is a %s!\n", lx, ly, dchr[sect.sct_desig].d_name);
		return(SYN_RETURN);
	}
	if( sect.sct_owned == 0 ) goto X302;
	if( trechk(cnum, sect.sct_owned, LANATT) != -1 ) goto X302;
	return(FAIL_RETURN);
X302:	
	vx = lx;
	vy = ly;
	vnum = sect.sct_owned;
	vdes = sect.sct_desig;
	vmovcst = dchr[vdes].d_mcst & 0377;
	vd = ((dchr[sect.sct_desig].d_dstr / 2.) + (-1.)) * sect.sct_effic + 100.;
	vt = (float)vd * sect.sct_milit * .01;
	printf("%d,%d is a %d%% %s with %d military or so.\n", vx, vy, round(sect.sct_effic, 10), dchr[sect.sct_desig].d_name, round(sect.sct_milit, 10));
	defx = defy = 0;
	if( sect.sct_dfend == 0 ) goto X1056;
	x = sect.sct_dfend<<8;
	x = (x>>12) + vx;
	y = sect.sct_dfend<<12;
	y = (y>>12) + vy;
	getsect(x, y, UP_NONE);
	if( vnum != sect.sct_owned ) goto X1056;
	if( sect.sct_desig != S_FORTR ) goto X1056;
	if( sect.sct_shell == 0 ) goto X1056;
	if( (q = (sect.sct_guns < 7) ? sect.sct_guns : 7) <= 0. ) goto X1056;
	if( tfact(vnum, q) < (float)idist(x - vx, y - vy) ) goto X1056;
	defx = x;
	defy = y;
X1056:	
	nsects = tat = tats = 0;
	if( wethr(vx, vy, 0) >= 700 ) goto X1130;
	printf(" Inclement weather...\n");
X1130:	
	i = 0;
X1132:	
	x = dn[i][0] + vx;
	y = dn[i][1] + vy;
	if( getsect(x, y, UP_OWN ) != -1 ) goto X1232;
	goto X2212;
X1232:	
	if( owner != 0 ) goto X1244;
	goto X2212;
X1244:	
	if( dchr[sect.sct_desig].d_ostr >= 1 ) goto X1272;
	goto X2212;
X1272:	
	movcost = (dchr[sect.sct_desig].d_mcst & 0377) + (vmovcst<<1);
	temp = (sect.sct_mobil * 24)/ movcost;
	if( temp >= sect.sct_milit ) goto X1444;
	printf("The %d mobility units in %d,%d will only", sect.sct_mobil, x, y);
	printf(" support %d troops, \n", temp);
	goto X1506;
X1444:	
	temp = sect.sct_milit;
X1506:	
	if( sect.sct_civil == 0 && temp == sect.sct_milit ) {
		printf("One military will be left behind on watch in %d,%d\n", x, y);
		temp--;
	}
	if( temp > 0 ) goto X1520;
	printf("No troops available from %d,%d\n", x, y);
	goto X2212;
X1520:	
	printf("Number of troops from %s at %d,%d (max %d) : ", dchr[sect.sct_desig].d_name, x, y, temp);
	at = atopi(getstri(""));
	if( at != 0 ) goto X1622;
	goto X2212;
X1622:	
	getsect(x, y, UP_NONE);
	temp = (temp < sect.sct_milit) ? temp : sect.sct_milit;
	at = (at < temp) ? at : temp;
	tats += at;
	sect.sct_milit -= at;
	sect.sct_mobil -= (movcost * at) / 24;
	putsect(x, y);
	unit[nsects].u_x = x;
	unit[nsects].u_y = y;
	q = dchr[sect.sct_desig].d_ostr / 2.;
	q = ((q + (-1.)) * sect.sct_effic) / 100. + 1.;
	unit[nsects].u_q = q;
	at = at * q + .5;
	unit[nsects].u_at = at;
	nsects++;
	if( nsects != 1 ) goto X2204;
	sigsave();
X2204:	
	tat += at;
X2212:	
	i++;
	if( i >= 4 ) goto X2226;
	goto X1132;
X2226:	
	if( nsects <= 0 ) goto X2240;
	if( tat >  0 ) goto X2330;
X2240:	
	sprintf(fmtbuf,"Country #%d considered attacking you @%s", cnum, xytoa(vx, vy, vnum));
	wu(0, vnum, fmtbuf);
	printf("No troops in attack...\n");
	return(FAIL_RETURN);
X2330:	
	odds = (tat * 32767.) / (tat + vt);
	i = wethr(vx, vy, 0);
	if( i >= 730. ) goto X2434;
	odds = (i / 730.) * odds;
X2434:	
	printf("Your combat odds are %.1f%%\n", odds / 327.67);
	vloss = aloss = 0;
	if( defx != 0 ) goto X2514;
	goto X3114;
X2514:	
	getsect(defx, defy, UP_NONE);
	vgun = landgun(sect.sct_effic);
	sect.sct_shell--;
	putsect(defx, defy);
	i = (rand()>>2) % nsects;
	getsect(unit[i].u_x, unit[i].u_y, UP_NONE);
	dam = shelldam(vgun, landdef(sect.sct_desig));
	sectdam(dam);
	putsect(unit[i].u_x, unit[i].u_y);
	tat = (1. - ((dam * .01) / nsects)) * tat;
	printf("Incoming shell! %d%% damage done to %d,%d\n", dam, unit[i].u_x, unit[i].u_y);
	nreport(vnum, N_FIRE_BACK, cnum);
X3114:	
	if( vdes == S_FORTR ) goto X3130;
	goto X3540;
X3130:	
	getsect(vx, vy, UP_NONE);
	if( sect.sct_guns == 0 ) goto X3540;
	if( sect.sct_shell == 0 ) goto X3540;
	sect.sct_shell--;
	putsect(vx, vy);
	i = rand() % nsects;
	vgun = landgun(sect.sct_effic);
	getsect(unit[i].u_x, unit[i].u_y, UP_NONE);
	dam = shelldam(vgun, landdef(sect.sct_desig));
	sectdam(dam);
	putsect(unit[i].u_x, unit[i].u_y);
	tat = (1. - ((dam * .01) / nsects)) * tat;
	printf("Incoming shell! %d%% damage done to %d,%d\n", dam, unit[i].u_x, unit[i].u_y);
	nreport(vnum, N_FIRE_BACK, cnum);
X3540:	
	i = 0;
X3542:	
	i = i % nsects;
	if( unit[i].u_at >  0 ) goto X3574;
	goto X4224;
X3574:	
	if( odds <= rand() % 32768 ) goto X4102;
	vloss = (100. / vd) + vloss;
	printf("!");
	if( --vt >  0 ) goto X4176;
X3646:	
	if( tat > vt ) goto X3662;
	goto X5060;
X3662:	
	success = 1;
	printf("You have captured %d,%d!\n", vx, vy);
	if( vnum == 0 ) goto X4016;
	sprintf(fmtbuf,"Country #%d lost %.1f troops defeating you @%s", cnum, aloss, xytoa(vx, vy, vnum));
	wu(0, vnum, fmtbuf);
X4016:	
	nreport(cnum, N_WON_SECT, vnum);
	occper = tats - aloss;
	goto X4236;
X4102:	
	printf((--unit[i].u_at != 0) ? "@" : "*");
	aloss = (1. / unit[i].u_q) + aloss;
	if( --tat <= 0 ) goto X3646;
X4176:	
	if( rand() % 32768 <= 26000 ) goto X4224;
	fflush(stdout);
	sleep(1);
X4224:	
	i++;
	goto X3542;
X4236:	
	occper = 127. / ((occper > 1.) ? occper : 1.);
	occper = (occper < 1.) ? occper : 1.;
	vt = 0;
	i = 0;
	goto X4612;
X4306:	
	temp = unit[i].u_at / unit[i].u_q;
	vt += temp * occper;
	printf("%.0f troops return to %d,%d\n", (1. - occper) * temp, unit[i].u_x, unit[i].u_y);
	getsect(unit[i].u_x, unit[i].u_y, UP_NONE);
	sect.sct_milit = ((1. - occper) * temp) + sect.sct_milit;
	putsect(unit[i].u_x, unit[i].u_y);
	i++;
X4612:	
	if( i < nsects ) goto X4306;
	printf("%d of your troops now occupy %d,%d\n", vt, vx, vy);
	if( vdes == S_CAPIT ) goto X4670;
	goto X5242;
X4670:	
	getnat(vnum);
	if( xwrap(vx + capx - nat.nat_xcap) != 0 ) goto X5242;
	if( ywrap(vy + capy - nat.nat_ycap) != 0 ) goto X5242;
	printf("You have captured %s's capital!\n", cname(vnum));
	nat.nat_stat = STAT_NOCAP;
	nat.nat_btu = (nat.nat_btu > 1) ? nat.nat_btu : 1;
	putnat(vnum);
	goto X5242;
X5060:	
	success = 0;
	printf("You have been defeated!\n");
	nreport(cnum, N_SCT_LOSE, vnum);
	vt = ((vt * 100.) / vd) + .9;
	if( vnum == 0 ) goto X5242;
	sprintf(fmtbuf,"Country #%d lost %.1f troops attacking you @%s", cnum, aloss, xytoa(vx, vy, vnum));
	wu(0, vnum, fmtbuf);
X5242:	
	printf("Casualties :\nYours ... %.1f\n", aloss);
	printf("Theirs .. %.1f\n", vloss);
	printf("Papershuffling ... %.1f B.T.U\n", (aloss + vloss) * .15);
	ntused = ntused + (aloss + vloss) * .15 + .5;
	getsect(vx, vy, UP_NONE);
	if( success == 0 ) goto X5462;
	sect.sct_owned = cnum;
	sect.sct_lstup = curup;
	sect.sct_chkpt = sect.sct_dfend = 0;
X5462:	
	sect.sct_milit = vt;
	putsect(vx, vy);
	return(NORM_RETURN);
}
