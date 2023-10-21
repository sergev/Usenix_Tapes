#define D_UPDATE
#define D_SECTDES
#define D_SHIPTYP
#define D_SCTSTR
#define D_SHPSTR
#define D_DCHRSTR
#define D_MCHRSTR
#define D_NSCSTR
#define D_FILES
#include        "empdef.h"

look()
{
        register        i, j;
        char     *cname();
        int     dx, dy;
        short   reported[512], *rep, nrep;
        int     ij, k;
        double  q, w, d, tfact();
        struct  nstr    nsct;
        struct  nbstr   nb;
        struct  {
                char    s_type;
                char    s_vrnge;
                short   s_xp;
                short   s_yp;
        } sh[128], *sp, *spmax;

        if( landorsea(argp[1]) != LAND ) goto X30;
        goto X2230;
X30:    
        if( snxtshp(&nb, argp[1], cnum, "from ship(s)? ") == -1 ) return(SYN_RETURN);
        spmax = &sh[128];
        for( sp = sh; sp < spmax; sp++ ) {
        if( nxtshp(&nb, &ship) == 0 ) break;
                sp->s_type = ship.shp_type;
                sp->s_vrnge = mchr[ship.shp_type].m_vrnge;
                sp->s_xp = ship.shp_xp;
                sp->s_yp = ship.shp_yp;
        }
        spmax = sp;
        i = 0;
        goto X1076;
X260:   
        if( ship.shp_own != 0 ) goto X272;
        goto X1074;
X272:   
        if( cnum != ship.shp_own ) goto X310;
        goto X1074;
X310:   
        j = 0;
        sp = sh;
X324:   
        if( sp >= spmax ) goto X630;
        dx = xwrap(sp->s_xp - ship.shp_xp);
        dy = ywrap(sp->s_yp - ship.shp_yp);
        w = (500 > wethr(ship.shp_xp, ship.shp_yp, 0) + (-300)) ? wethr(ship.shp_xp, ship.shp_yp, 0) + (-300) : 500;
        q = (float)mchr[ship.shp_type].m_visib * sp->s_vrnge * w / 8170.;
        if( q*q <  dx*dx + dy*dy ) goto X732;
        j |= 02;
X630:
        if( (j & 02) == 0 ) goto X1032;
        printf("%s (#%d) %s #%d @%d,%d\n", cname(ship.shp_own), ship.shp_own, mchr[ship.shp_type].m_name, i, ship.shp_xp, ship.shp_yp);
        goto X1074;
X732:   
        if( ship.shp_type != S_SUB ) goto X1020;
        if( sp->s_type != S_DES ) goto X1020;
        if( q * q * 36. <  dx*dx + dy*dy ) goto X1020;
        j |= 01;
X1020:  
        sp++;
        goto X324;
X1032:  
        if( (j & 01) == 0 ) goto X1074;
        printf("Snorkel at %d,%d\n", ship.shp_xp, ship.shp_yp);
X1074:  
        i++;
X1076:  
        if( getship(i, &ship) == -1 ) goto X1124;
        goto X260;
X1124:  
        nrep = 0;
        sp = sh;
        goto X2206;
X1146:  
        w = wethr(sp->s_xp, sp->s_yp, 0);
        j = sp->s_yp;
        j--;
        goto X2156;
X1226:  
        i = sp->s_xp;
        i--;
        goto X2132;
X1244:  
        if( (j - sp->s_yp) * (i - sp->s_xp) == 0 ) goto X1322;
        if( w >= 700. ) goto X1322;
        goto X2130;
X1322:  
        if( getsect(i, j, UP_NONE) != -1 ) goto X1350;
        goto X2130;
X1350:  
        if( sect.sct_desig != S_WATER ) goto X1362;
        goto X2130;
X1362:  
        ij = (i<<8) + j;
        rep = &reported[nrep];
X1416:  
        rep--;
        if( rep < &reported[0] ) goto X1450;
        if( ij != *rep ) goto X1416;
X1450:
        if( rep < &reported[0] ) goto X1470;
        goto X2130;
X1470:
        reported[nrep++] = ij;
        update(i, j, UP_ALL);
        if( sect.sct_owned != 0 ) {
                if( owner != 0 ) {
                        printf("Your ");
                } else {
                        printf("%s (#%d) ", cname(sect.sct_owned), sect.sct_owned);
                }
        } else {
                printf("Unowned ");
        }
        printf("%s ", dchr[sect.sct_desig].d_name);
        printf("%d%% eff", 
                (owner != 0) ? sect.sct_effic : round(sect.sct_effic, 10));
        if( owner != 0 ) goto X2050;
        k = round(sect.sct_civil, 10);
        if( k <= 0 ) goto X2004;
        printf(" with approx. %d civilians", k);
X2004:  
        k = round(sect.sct_milit, 10);
        if( k <= 0 ) goto X2104;
        printf(" with approx. %d troops", k);
        goto X2104;
X2050:  
        printf(" with %d civilians & %d troops", sect.sct_civil, sect.sct_milit);
X2104:  
        printf(" @%d, %d\n", i, j);
X2130:  
        i++;
X2132:  
        if( i >  sp->s_xp + 1 ) goto X2154;
        goto X1244;
X2154:  
        j++;
X2156:  
        if( j >  sp->s_yp + 1 ) goto X2200;
        goto X1226;
X2200:  
        sp++;
X2206:  
        if( sp >= spmax ) goto X2222;
        goto X1146;
X2222:  
        return(NORM_RETURN);
X2230:  
        if( snxtsct(&nsct, argp[1]) == -1 ) return(SYN_RETURN);
X2262:  
        if( nxtsct(&nsct, UP_OWN) <= 0 ) goto X2222;
        if( owner == 0 ) goto X2262;
        w = wethr(nsct.n_x, nsct.n_y, 0);
        d = ((w < 720.) ? ((w + (-480.)) / 250.) : (w / 730.)) * sect.sct_effic / 50.;
        if( sect.sct_desig != S_RADAR ) goto X2454;
        d = 3.5 * d;
X2454:  
        d = tfact(cnum, d);
        printf("%3d, %-3d efficiency %d%%, range %.1f\n", nsct.n_x, nsct.n_y, sect.sct_effic, d);
        if( d >= 1.0 ) goto X2604;
        if( sect.sct_desig == S_HARBR ) goto X2604;
        if( sect.sct_desig != S_BSPAN ) goto X2262;
X2604:  
        d *= d;
        i = 0;
X2620:  
        if( getship(i, &ship) == -1 ) goto X2262;
        if( ship.shp_own == 0 ) goto X3134;
        if( cnum == ship.shp_own ) goto X3134;
        dx = xwrap(ship.shp_xp - nsct.n_x);
        dy = ywrap(ship.shp_yp - nsct.n_y);
        j = dx*dx + dy*dy;
        if( j >  mchr[ship.shp_type].m_visib * d / 25. ) goto X3134;
        printf("%s (#%d)", cname(ship.shp_own), ship.shp_own);
        printf(" %s #%d @%d, %d\n", mchr[ship.shp_type].m_name, i, ship.shp_xp, ship.shp_yp);
X3134:  
        i++;
        goto X2620;
}
