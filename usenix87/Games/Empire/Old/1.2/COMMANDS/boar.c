#define D_NEWSVERBS
#define D_TRTYCLAUSE
#define D_SHPSTR
#define D_MCHRSTR
#define D_FILES
#include        "empdef.h"
#include        <stdio.h>

boar()
{
        register        i;
        char     *splur();
        int     vship, aship, at, vt, vnum;
        int     dx, dy, adef, dam, n;
        double  d, tfact(), seagun();
        struct  shpstr  as, vs;

        vship = getshno(argp[1], "Victim ship? ", &vs);
	if( vship == -1 || vs.shp_own == 0 ) {
                printf("No such ship");
                return(SYN_RETURN);
        }
        vnum = vs.shp_own;
        if( trechk(cnum, vnum, SEAATT) == -1 ) return(FAIL_RETURN);
        aship = getshno(argp[2], "Boarding party from ship #", &as);
        if( aship == -1 ||
            cnum != as.shp_own ) {
                printf("Not yours");
                return(SYN_RETURN);
        }
        if( as.shp_xp != vs.shp_xp ||
            as.shp_yp != vs.shp_yp ) {
                printf("Not adjacent");
                return(SYN_RETURN);
        }
        at = (as.shp_crew > mchr[as.shp_type].m_milit) ? mchr[as.shp_type].m_milit : as.shp_crew;
        if( at != 0 ) goto X350;
        printf("No military on %s #%d", mchr[as.shp_type].m_name, aship);
        return(FAIL_RETURN);
X350:   
        i = wethr(vs.shp_xp, vs.shp_yp, 0);
        if( i >= 700 ) goto X426;
        printf("Barometer @%.0f;  Seas too rough to board...", i);
        return(SYN_RETURN);
X426:   
        adef = seadef(as.shp_type);
        i = 0;
        goto X1276;
X452:   
        if( vnum == ship.shp_own ) goto X470;
        goto X1274;
X470:   
        if( ship.shp_fleet == vs.shp_fleet ) goto X504;
        goto X1274;
X504:   
        if( ship.shp_shels != 0 ) goto X516;
        goto X1274;
X516:   
        if( ship.shp_gun != 0 ) goto X530;
        goto X1274;
X530:   
        d = (mchr[ship.shp_type].m_frnge * ship.shp_effc) / 200.;
        if( (d = tfact(vnum, d)) != 0 ) goto X622;
        goto X1274;
X622:   
        dx = xwrap(ship.shp_xp - vs.shp_xp);
        dy = ywrap(ship.shp_yp - vs.shp_yp);
        if( d*d <  dx*dx + dy*dy ) goto X1274;
        n = (ship.shp_gun < ship.shp_shels) ? ship.shp_gun : ship.shp_shels;
        printf("Incoming shell%s!\n", splur(n));
        nreport(vnum, N_FIRE_BACK, cnum);
        dam = shelldam(seagun(ship.shp_type, ship.shp_effc, n), adef);
        shipdam(&as, dam);
        putship(aship, &as);
        if( as.shp_own != 0 ) goto X1274;
        sprintf(fmtbuf,"Country #%d lost %s #%d trying to board %s #%d", cnum, mchr[as.shp_type].m_name, aship, mchr[vs.shp_type].m_name, vship);
        wu(0, vnum, fmtbuf);
        nreport(cnum, N_SHP_LOSE, vnum);
        return(NORM_RETURN);
X1274:  
        i++;
X1276:  
        if( getship(i, &ship) == -1 ) goto X1324;
        goto X452;
X1324:  
        sigsave();
        getship(aship, &as);
        getship(vship, &vs);
        at = (as.shp_crew > mchr[as.shp_type].m_milit) ? mchr[as.shp_type].m_milit : as.shp_crew;
        vt = (vs.shp_crew > mchr[vs.shp_type].m_milit) ? mchr[vs.shp_type].m_milit : vs.shp_crew;
        as.shp_crew -= at;
        vs.shp_crew -= vt;
        putship(aship, &as);
        putship(vship, &vs);
        goto X1646;
X1606:  
        printf("@");
        if( --at <= 0 ) goto X1704;
X1624:  
        if( (01010 & rand()) != 0 ) goto X1646;
        fflush(stdout);
        sleep(1);
X1646:  
        if( rand() % 32768 > 20000 ) goto X1666;
        if( vt >  0 ) goto X1606;
X1666:  
        printf("!");
        if( --vt >  0 ) goto X1624;
X1704:  
        getship(aship, &as);
        getship(vship, &vs);
        if( at <= vt ) goto X2170;
        printf("\nBoarding sucessful\n");
        sprintf(fmtbuf,"Country #%d boarded %s #%d", cnum, mchr[vs.shp_type].m_name, vship);
        wu(0, vnum, fmtbuf);
        nreport(cnum, N_BOARD_SHIP, vnum);
        vs.shp_own = cnum;
        vs.shp_fleet = ' ';
        vs.shp_spric = 0;
        vt = ((at>>1) < mchr[vs.shp_type].m_milit) ? (at>>1) : mchr[vs.shp_type].m_milit;
        at -= vt;
        goto X2326;
X2170:  
        printf("\nYou have been repelled...\n");
        sprintf(fmtbuf,"Country #%d %s #%d tried to board %s #%d", cnum, mchr[as.shp_type].m_name, aship, mchr[vs.shp_type].m_name, vship);
        wu(0, vnum, fmtbuf);
        nreport(cnum, N_SHP_LOSE, vnum);
X2326:  
        as.shp_crew = at;
        vs.shp_crew = vt;
        putship(aship, &as);
        putship(vship, &vs);
        return(NORM_RETURN);
}
