#define D_UPDATE
#define D_NATSTAT
#define D_SECTDES
#define D_NEWSVERBS
#define D_PLGSTAGES
#define D_NATSTR
#define D_SCTSTR
#define D_DCHRSTR
#define D_ICHRSTR
#define D_NSCSTR
#define D_FILES
#include        "empdef.h"

update(x, y, selup)
int     x, y, selup;
{
        register int       i, q, iwork;
        char    *dp, *xytoa();
        int     dt, shipper, prdtyp, prdcst;
        short   dx, dy;
        int     thresh, plgue, sectowner;
        int     amt, unit, funit, tunit, pkgs;
        int     verbose, quiet, wthr, sctdes;
        double  workforce, work, cashout, movease, dtemp;
        struct  ichrstr *ip;
        double ceil();

        sectowner = sect.sct_owned;
        getnat(sectowner);
        cashout = 0;
        verbose = selup & UP_VERBOSE;
        quiet = selup & UP_QUIET;
        if( sect.sct_lstup == 0 ) return(0);
        if( sect.sct_lstup <= curup ) goto X160;
        sect.sct_lstup = curup;
        return(1);
X160:   
        workforce = (sect.sct_civil + (sect.sct_milit * .2)) / 100. + 1.e-6;
        if( workforce >= 1.e-4 ) goto X240;
X232:   
        return(0);
X240:   
        dt = curup - sect.sct_lstup;
	if( (selup & (UP_TIME | UP_WFDEL)) == 0 ) goto X310;
	if( selup & UP_WFDEL ) { /* being updated for wrkforce delivery */
		dt -= 4; /* update sector to within 4 time units */
		if( dt <= 0 ) return(0);
	}
        iwork = dt * workforce;
        if( iwork >= 0 ) goto X402;
        iwork = 077777;
        goto X402;
X310:   
        dt = (dt < 128) ? dt : 128;
        iwork = dt * workforce;
        if( iwork < 1 ) goto X232;
        dt = ceil(iwork / workforce);
X402:   
        if( owner != 0 ) goto X434;
        if( (selup & UP_ALL) != 0 ) goto X434;
        if( iwork < 24 ) goto X232;
        iwork >>= 1;
        dt >>= 1;
X434:   
        work = iwork;
        sctdes = sect.sct_desig;
        pkgs = dchr[sctdes].d_pkg;
        prdtyp = (int)dchr[sctdes].d_ptyp;
        prdcst = ichr[prdtyp].i_prdct;
        wthr = wethr(x, y, 0);
        if( wthr >= 630 ) goto X732;
        if( owner == 0 ) goto X732;
        if( nstat == STAT_GOD ) goto X732;
        if( sect.sct_desig == S_SANCT ) goto X732;
        if( sect.sct_desig == S_CAPIT ) goto X732;
        q = (640 - wthr < 50) ? 640 - wthr : 50;
        printf("%d%% hurricane damage in %d, %d!\n", q, x, y);
        sectdam(q);
        nreport(cnum, N_SCT_STORM, 0);
X732:   
        if( sect.sct_effic >= 100 ) goto X1116;
        q = (iwork > 100 - sect.sct_effic) ? 100 - sect.sct_effic : iwork;
        cashout += q;
        if( wthr <= 700 ) goto X1042;
        sect.sct_effic += q;
        goto X1116;
X1042:  
        if( wthr <= 650 ) goto X1116;
        if( owner == 0 ) goto X1116;
        if( quiet != 0 ) goto X1116;
        printf("Bad weather delays construction in %d,%d\n", x, y);
X1116:  
        q = sect.sct_civil * dt;
        if( sctdes != S_URBAN ) goto X1256;
        q = (sect.sct_ore > q / 100) ? q / 100 : sect.sct_ore;
        q = max127(sect.sct_civil + q) - sect.sct_civil;
        sect.sct_ore -= q;
        sect.sct_civil += q;
        goto X1372;
X1256:  
        if( sctdes != S_BSPAN ) goto X1310;
        sect.sct_civil -=  (q / 400);
        goto X1372;
X1310:  
        sect.sct_civil = max127(sect.sct_civil + ((sect.sct_civil > 31 && sect.sct_civil < 97) ? q / 200 : q / 400));
X1372:  
        sect.sct_mobil = max127(sect.sct_mobil + dt);
        sect.sct_lstup += dt;
        cashout += (((sect.sct_milit>>5) * dt)>>3);
        if( (sect.sct_p_stage & 0177) >  PLG_HEALTHY ) {
                if( dt > sect.sct_p_time ) {
                        sect.sct_p_time = 0;
                } else {
                        sect.sct_p_time -= dt;
                }
                switch(sect.sct_p_stage) {
                case PLG_DYING:
                        dtemp = dt / (sect.sct_p_time + dt + 1.);
                        if( dtemp <= .9 ) goto X1610;
                        dtemp = .9;
X1610:  
                        q = (sect.sct_civil + sect.sct_milit) * dtemp;
                        if( owner == 0 ) goto X1676;
                        printf("%d citizens died of PLAGUE in %d,%d\n", q, x, y)
;
                        goto X1762;
X1676:  
                        sprintf(fmtbuf,"%d citizens died of plague in %s", q, xytoa(x, y, (int)sect.sct_owned));
                        wu(0, sect.sct_owned, fmtbuf);
                        getnat(sectowner);
X1762:  
                        nreport(sect.sct_owned, N_DIE_PLAGUE, 0);
                        sect.sct_civil -= sect.sct_civil * dtemp;
                        sect.sct_milit -= sect.sct_milit * dtemp;
                        break;
                case PLG_INFECT:
                        if( owner == 0 ) break;
                        printf("%d,%d battling PLAGUE\n", x, y);
                        break;
                case PLG_INCUBATE:
                        if( (sect.sct_p_time & 0177) >  0 ) break;
                        if( owner == 0 ) goto X2234;
                        printf("Outbreak of PLAGUE in %d,%d\n", x, y);
                        goto X2316;
X2234:  
                        sprintf(fmtbuf,"Outbreak of plague in %s", xytoa(x, y, (int)sect.sct_owned));
                        wu(0, sect.sct_owned, fmtbuf);
                        getnat(sectowner);
X2316:  
                        nreport(sect.sct_owned, N_OUT_PLAGUE, 0);
                        break;
                case PLG_EXPOSED:
                        sprintf(fmtbuf,"%s gets the plague in %s", nat.nat_cnam, xytoa(x, y, 0));
                        wu(cnum, 0, fmtbuf);
                        getnat(sectowner);
                        sect.sct_p_time = 0;
                        break;
                }       
                if( (sect.sct_p_time & 0177) == 0 ) {
                        sect.sct_p_stage--;
                        sect.sct_p_time = ((rand()>>2) & 037) + 32;
                }
        } else {
                dtemp = (sect.sct_civil + sect.sct_milit) / 254.;
                dtemp *= (nat.nat_t_level + sect.sct_ore + 100.);
                dtemp /= (nat.nat_r_level + sect.sct_effic + sect.sct_mobil + 100.);
                if( dtemp - 1. > (rand() % 32768)/100 ) {
                        sect.sct_p_stage = PLG_EXPOSED;
                }
        }
        plgue = sect.sct_p_stage;
        q = dchr[sctdes].d_mcst;
        q &= 0377;
        movease = 1000. / ((q * 80) + 21 - sect.sct_effic);
        dp = &sect.sct_c_use;
        i = 0;
        goto X4270;
X2734:  
        if( sctdes != S_XCHNG ) goto X2756;
        if( i < 2 ) goto X2756;
/*
*	If this is the destination sector in a workforce (civ or mil)
*	delivery, don't deliver workforce.  This is the recursion stopper.
*/
        goto X4316;
	if( selup & UP_WFDEL && ( i == 0 || i == 1 ) ) goto X4266;
X2756:  
        if( (*(dp + i) & 017) != 0 ) goto X2776;
        goto X4266;
X2776:  
        if( 02 != *(dp + i) ) goto X3016;
        goto X4266;
X3016:  
        dx = (*(dp + i)<<12);
        dx = (dx>>14) + x;
        dy = (*(dp + i)<<14);
        dy = (dy>>14) + y;
        thresh = (*(dp + i)>>1) & 0170;
        dtemp = (movease / ip->i_lbs) * (sect.sct_mobil / 2);
        q = (thresh < *(&sect.sct_civil + i)) ? *(&sect.sct_civil + i) - thresh : 0;
        q = (dtemp < q) ? dtemp : q;
        if( q >  0 ) goto X3260;
        goto X4266;
X3260:  
        funit = ip->i_pkg[pkgs];
        amt = funit * q;
        shipper = sect.sct_owned;
/*
*	If we're delivering workforce (civilians or military), we have to
*	get the destination sector with the special UP_WFDEL flag set.
*	This tells getsect to call update with the same flag.  This will
*	cause the destination sector to be updated to within 4 time units
*	of curup.  This will prevent the re-use of civilians along a civilian
*	delivery chain (the "rolling civilians" bug), or re-use of military
*	to increase sector efficiencies.
*/
	if( i == 0 || i == 1 ) {
		if( getsect(dx, dy, UP_WFDEL) == -1 ) goto X3404;
		getnat(sectowner);
	} else {
		if( getsect(dx, dy, UP_NONE) == -1 ) goto X3404;
	}
        if( getsect(dx, dy, UP_NONE) == -1 ) goto X3404;
        if( shipper == sect.sct_owned ) goto X3612;
        if( sect.sct_chkpt != 0 ) goto X3612;
X3404:  
        if( owner != 0 ) goto X3430;
        if( shipper != cnum ) goto X3502;
        if( quiet != 0 ) goto X3502;
X3430:  
        printf("%s delivery walkout between %d,%d & %d,%d\n", ip->i_name, x, y, dx, dy);
X3502:  
        amt = 0;
        goto X4154;
X3512:  
        if( sect.sct_desig != S_GMINE ) goto X3532;
        if( sctdes != S_GMINE ) goto X3552;
X3532:  
        if( sect.sct_desig == S_GMINE ) goto X3622;
        if( sctdes != S_GMINE ) goto X3622;
X3552:  
        if( owner == 0 ) goto X3502;
        if( quiet != 0 ) goto X3502;
        printf("Ore mover's union dispute in %d,%d\n", dx, dy);
        goto X3502;
X3612:  
        if( ip->i_mnem == 'o' ) goto X3512;
X3622:  
        tunit = ip->i_pkg[dchr[sect.sct_desig].d_pkg];
        if( (amt / tunit) + *(&sect.sct_civil + i) <= 127 ) goto X4012;
        if( owner == 0 ) goto X3766;
        if( verbose == 0 ) goto X3766;
        printf("Delivery backlog (%s) -- %d,%d to %d,%d\n", ip->i_name, x, y, dx , dy);
X3766:  
        amt = (127 - *(&sect.sct_civil + i)) * tunit;
X4012:  
        unit = (funit == tunit) ? funit : funit * tunit;
        amt = (amt / unit) * unit;
        *(&sect.sct_civil + i) += (amt / tunit);
        if( plgue != PLG_INFECT ) goto X4136;
        if( sect.sct_p_stage != 0 ) goto X4136;
        sect.sct_p_stage = PLG_EXPOSED;
X4136:  
        putsect(dx, dy);
X4154:  
        getsect(x, y, UP_NONE);
        q = amt / funit;
        *(&sect.sct_civil + i) -= q;
        sect.sct_mobil =  sect.sct_mobil - ((ip->i_lbs / movease) * q);
X4266:  
        i++;
X4270:  
        ip = &ichr[i + 12];
        if( ip->i_mnem == '\0' ) goto X4316;
        goto X2734;
X4316:  
        if( sect.sct_effic >= 60 ) goto X4332;
        goto X6440;
X4332:  
        if( broke == 0 ) goto X4344;
        goto X6440;
X4344:  
        i = sect.sct_prdct;
        switch( sctdes ) {
        case S_BANK:
                q = 18;
                q = ichr[q].i_pkg[pkgs];
                cashout -= ((((long)sect.sct_gold * q * dt) >> 3) & 017777);
                break;
        case S_CAPIT:
        case S_RADAR:
        case S_WETHR:
                cashout += dt;
                break;
        case S_TECH:
        case S_RSRCH:
                cashout += dt;
                i = i / prdcst;
                if( i != 0 ) goto X4604;
                goto X5136;
X4604:  
                if( owner == 0 ) goto X4666;
                if( quiet != 0 ) goto X4666;
                printf("%s made in %d, %d\n", ichr[prdtyp].i_name, x, y);
X4666:  
                if( sctdes != S_TECH ) goto X4716;
                nat.nat_t_level += i;
                goto X4734;
X4716:  
                nat.nat_r_level += i;
X4734:  
                putnat(sectowner);
                goto X5136;
        case S_ARMSF:
        case S_AMMOF:
        case S_AIRPT:
                 i = (127 - *(&sect.sct_civil + prdtyp - 12) > i / prdcst) ? i / prdcst : 127 - *(&sect.sct_civil + prdtyp - 12);
                if( i == 0 ) goto X5116;
                if( owner == 0 ) goto X5116;
                if( quiet != 0 ) goto X5116;
                printf("%s built in %d,%d\n", ichr[prdtyp].i_name, x, y);
X5116:  
                *(&sect.sct_civil + prdtyp - 12) += i;
X5136:  
                sect.sct_prdct -= prdcst * i;
                cashout += prdcst * i;
        case S_HARBR:
        case S_BHEAD:
                q = (sect.sct_ore > (sect.sct_effic * iwork / 100) ) ? sect.sct_effic * iwork / 100 : sect.sct_ore;
                if( 127 >= sect.sct_prdct + q ) goto X5350;
                if( owner == 0 ) goto X5334;
                if( quiet != 0 ) goto X5334;
                printf("Production bottleneck in %d,%d\n", x, y);
X5334:  
                q = 127 - sect.sct_prdct;
X5350:  
                sect.sct_ore -= q;
                sect.sct_prdct += q;
                if( sect.sct_contr != 0 ) {
                        cashout -= ((sect.sct_contr / 100.) * ichr[prdtyp].i_bid) * sect.sct_prdct;
                        sect.sct_prdct = 0;
                }
                break;
        case S_GMINE:
                i = (127 - sect.sct_gold > (i / prdcst)) ? i / prdcst : 127 - sect.sct_gold;
                sect.sct_gold += i;
                sect.sct_prdct -= (prdcst * i);
                cashout += (prdcst * i);
                q = (sect.sct_ore > sect.sct_effic * iwork / 100) ? sect.sct_effic * iwork / 100 : sect.sct_ore;
                if( sect.sct_prdct + q <= 127 ) goto X5706;
                if( owner == 0 ) goto X5672;
                if( quiet != 0 ) goto X5672;
                printf("production overflow in %d,%d\n", x, y);
X5672:  
                q = 127 - sect.sct_prdct;
X5706:  
                sect.sct_ore -= q;
                sect.sct_prdct += q;
                if( sect.sct_contr == 0 ) goto X6030;
                cashout -= ((sect.sct_contr / 100.) * ichr[prdtyp].i_bid) * sect.sct_prdct;
                sect.sct_prdct = 0;
X6030:  
		if( sect.sct_ore + q > 127 ) q = 127 - sect.sct_ore;
                q = (sect.sct_gmin > sect.sct_gmin * iwork / 100) ? sect.sct_gmin * iwork / 100 : sect.sct_gmin;
                sect.sct_ore += q;
                sect.sct_gmin -= (q>>1);
                if( owner == 0 ) break;
                if( sect.sct_gmin != 0 ) break;
                if( quiet != 0 ) break;
                printf("Out of ore in %d,%d\n", x, y);
                break;
        case S_MINE:
                /* q = (float)sect.sct_miner * sect.sct_effic * iwork / 1.e4; */
q=(int)(((float)sect.sct_miner * (float)sect.sct_effic * (float)iwork) / (10000.0));
/****	printf("q= %d min %f eff %f iwork %d\n",(int)q,(float)sect.sct_miner,(float)sect.sct_effic, (int)iwork);  **** */
/* ****	printf("work is %g\n",work);  **** */
                if( 127 >= sect.sct_ore + q ) goto X6332;
                if( owner == 0 ) goto X6316;
                if( quiet != 0 ) goto X6316;
                printf("Ore ready to move in %d,%d\n", x, y);
X6316:  
                q = 127 - sect.sct_ore;
X6332:  
                sect.sct_ore += q;
                if( sect.sct_contr == 0 ) break;
                cashout -= ((sect.sct_contr / 100.) * ichr[prdtyp].i_bid) * sect.sct_ore;
                sect.sct_ore = 0;
        }
X6440:  
        if( owner == 0 || nstat == STAT_GOD ) {
                nat.nat_money -= cashout;
                putnat(sectowner);
        } else {
                dolcost += cashout;
        }
        return(1);
}
