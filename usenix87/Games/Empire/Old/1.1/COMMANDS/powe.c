#define D_UPDATE
#define D_NATSTAT
#define D_SECTDES
#define D_NEWSVERBS
#define D_NATSTR
#define D_SCTSTR
#define D_SHPSTR
#define D_POWSTR
#define D_DCHRSTR
#define D_ICHRSTR
#define D_MCHRSTR
#define D_FILES
#include        "empdef.h"

powe()
{
        register struct sctstr  *s;
        char     *ctime(), *cname();
        long    ldround();
        int     i, j, ii, pkgs, readsize, ownr;
        long    now, ndate;
        float   pmax, *fp, *tp;
        struct  sctstr  sct[MAX_W_XSIZE];
        struct  powstr  pow[MAX_MAXNOC], *k;

        printf("     - = [   Empire Power Report   ] = -\n");
        powf = open(powfil, O_RDWR);
        read(powf, &ndate, sizeof(long));
        read(powf, pow, sizeof(pow));
        time(&now);
        if( 3600. <= now - ndate ) goto X210;
        if( nstat == STAT_GOD ) goto X210;
        goto X2632;
X210:
        i = 0;
        goto X306;
X216:
        fp = &pow[i].p_power;
        goto X256;
X242:
        *fp-- = 0.;
X256:
        if( fp >= &pow[i].p_sects ) goto X242;
        i++;
X306:
        if( i < maxnoc ) goto X216;
        lseek(sectf, 0L, 0);
        readsize = w_xsize * sizeof(sect);
        j = 0;
        goto X1336;
X362:
        read(sectf, sct, readsize);
        i = 0;
        goto X1316;
X420:
        if( sct[i].sct_owned != 0 ) goto X444;
        goto X1312;
X444:
        s = &sct[i];
        pkgs = dchr[s->sct_desig].d_pkg;
        k = &pow[s->sct_owned];
        k->p_sects += 1.;
        k->p_effic += s->sct_effic;
        ii = 12;
        k->p_civil += ichr[ii].i_pkg[pkgs] * s->sct_civil;
        ii = 13;
        k->p_milit += ichr[ii].i_pkg[pkgs] * s->sct_milit;
        ii = 14;
        k->p_shell += ichr[ii].i_pkg[pkgs] * s->sct_shell;
        ii = 15;
        k->p_guns += ichr[ii].i_pkg[pkgs] * s->sct_guns;
        ii = 16;
        k->p_plane += ichr[ii].i_pkg[pkgs] * s->sct_plane;
        ii = 17;
        k->p_ore += ichr[ii].i_pkg[pkgs] * s->sct_ore;
        ii = 18;
        k->p_gold += ichr[ii].i_pkg[pkgs] * s->sct_gold;
X1312:
        i++;
X1316:
        if( i >= w_xsize ) goto X1332;
        goto X420;
X1332:
        j++;
X1336:
        if( j >= w_ysize ) goto X1352;
        goto X362;
X1352:
        i = 0;
        goto X2002;
X1362:
        if( ship.shp_own != 0 ) goto X1374;
        goto X1776;
X1374:
        k = &pow[ship.shp_own];
        if( mchr[ship.shp_type].m_civil == 0 ) goto X1464;
        k->p_civil += ship.shp_crew;
        goto X1512;
X1464:
        k->p_milit += ship.shp_crew;
X1512:
        k->p_shell += ship.shp_shels;
        k->p_guns += ship.shp_gun;
        k->p_plane += ship.shp_plns;
        k->p_ore += ship.shp_or;
        k->p_gold += ship.shp_gld;
        k->p_ships += 1.;
        k->p_power += mchr[ship.shp_type].m_prdct/10.;
X1776:
        i++;
X2002:
        if( getship(i, &ship) == -1 ) goto X2032;
        goto X1362;
X2032:
        i = 1;
        goto X2060;
X2042:
        k->p_power = 0.;
X2054:
        i++;
X2060:
        if( i < maxnoc ) goto X2074;
        goto X2524;
X2074:
        getnat(i);
        k = &pow[i];
        if( nat.nat_stat < STAT_NOCAP ) goto X2042;
        k->p_money = nat.nat_money / 1000.;
        k->p_power += (k->p_civil + k->p_milit + k->p_shell) / 10.;
        k->p_power += (k->p_guns + k->p_sects*k->p_effic) / 3.;
        k->p_power += k->p_plane + k->p_ships;
        k->p_power += k->p_gold * 3. + k->p_money * 10.;
        tp = &pow[0].p_power;
        fp = &k->p_power;
X2452:
        fp--;
        if( fp >= (float *)k ) goto X2474;
        goto X2054;
X2474:
        tp--;
        *tp += *fp;
        goto X2452;
X2524:
        lseek(powf, 0L, 0);
        time(&ndate);
        write(powf, &ndate, sizeof(long));
        write(powf, pow, sizeof(pow));
X2632:
        close(powf);
        printf("      as of %s", ctime(&ndate));
        printf("\n\t   sects  eff  civil  milit  shell  guns plane  ore  gold ship  money\n");
        i = 0;
        goto X4766;
X2720:
        pmax = 0.;
        j = 1;
        goto X3022;
X2736:
        k = &pow[j];
        if( pmax >= k->p_power ) goto X3016;
        pmax = k->p_power;
        ii = j;
X3016:  
        j++;
X3022:  
        if( j < maxnoc ) goto X2736;
        if( pmax != 0 ) goto X3046;
        goto X5002;
X3046:  
        k = &pow[ii];
        if( ii == cnum || cnum == 0 ) {
                ownr = 1;
        } else {
                ownr = 0;
        }
        printf("\n%11.11s", cname(ii));
        if( ownr != 0 ) goto X3176;
        if( k->p_sects >= 23. ) goto X3226;
X3176:  
        printf("%5.0f", k->p_sects, 10);
        goto X3270;
X3226:  
        printf("%5ld", ldround(k->p_sects, 10));
X3270:  
        printf("%4.0f%%", k->p_effic / (k->p_sects + .1));
        if( ownr != 0 ) goto X3374;
        if( k->p_civil >= 299 ) goto X3424;
X3374:  
        printf("%7.0f", k->p_civil);
        goto X3472;
X3424:  
        printf("%7ld", ldround(k->p_civil, 100));
X3472:  
        if( ownr != 0 ) goto X3530;
        if( k->p_milit >= 299 ) goto X3560;
X3530:  
        printf("%7.0f", k->p_milit);
        goto X3626;
X3560:  
        printf("%7ld", ldround(k->p_milit, 100));
X3626:  
        if( ownr == 0 ) goto X3672;
        printf("%7.0f", k->p_shell);
        goto X3740;
X3672:  
        printf("%7ld", ldround(k->p_shell, 100));
X3740:  
        if( ownr == 0 ) goto X4004;
        printf("%6.0f", k->p_guns);
        goto X4052;
X4004:  
        printf("%6ld", ldround(k->p_guns, 10));
X4052:  
        if( ownr == 0 ) goto X4116;
        printf("%5.0f", k->p_plane);
        goto X4164;
X4116:  
        printf("%5ld", ldround(k->p_plane, 10));
X4164:  
        if( ownr != 0 ) goto X4222;
        if( k->p_ore >= 299 ) goto X4252;
X4222:  
        printf("%7.0f", k->p_ore);
        goto X4320;
X4252:  
        printf("%7ld", ldround(k->p_ore, 100));
X4320:  
        if( ownr == 0 ) goto X4364;
        printf("%5.0f", k->p_gold);
        goto X4432;
X4364:  
        printf("%5ld", ldround(k->p_gold, 10));
X4432:  
        if( ownr == 0 ) goto X4476;
        printf("%5.0f", k->p_ships);
        goto X4544;
X4476:  
        printf("%5ld", ldround(k->p_ships, 10));
X4544:  
        if( ownr != 0 ) goto X4600;
        if( k->p_money >= 13. ) goto X4630;
X4600:  
        printf("%6.0fk\n", k->p_money);
        goto X4676;
X4630:  
        printf("%6ldk\n", ldround(k->p_money, 10));
X4676:  
        if( nstat != STAT_GOD ) goto X4750;
        printf("%9.2f\n", k->p_power);
X4750:  
        k->p_power = 0.;
        i++;
X4766:  
        if( i >= maxcno ) goto X5002;
        goto X2720;
X5002:  
        printf("\t    ---- ---- ------ ------ ------ ----- ---- ------ ---- ---- -----\n");
        k = &pow[0];
        printf("worldwide  ");
        printf("%5ld", ldround(k->p_sects, 10));
        printf("%4.0f%%", k->p_effic / (k->p_sects + .01));
        printf("%7ld", ldround(k->p_civil, 100));
        printf("%7ld", ldround(k->p_milit, 100));
        printf("%7ld", ldround(k->p_shell, 100));
        printf("%6ld", ldround(k->p_guns, 10));
        printf("%5ld", ldround(k->p_plane, 10));
        printf("%7ld", ldround(k->p_ore, 100));
        printf("%5ld", ldround(k->p_gold, 10));
        printf("%5ld", ldround(k->p_ships, 10));
        printf("%6ldk\n", ldround(k->p_money, 10));
        return(NORM_RETURN);
}
