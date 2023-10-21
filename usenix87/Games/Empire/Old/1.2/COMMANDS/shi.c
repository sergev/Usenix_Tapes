#define D_SHPSTR
#define D_MCHRSTR
#define D_NSCSTR
#define D_FILES
#include        "empdef.h"

shi()
{
        register        nships;
        char     *splur();
        struct  nbstr   nb;

        if( snxtshp(&nb, argp[1], cnum, "") == -1 ) return(SYN_RETURN);
        nships = 0;
        goto X370;
X64:    
        if( nships++ != 0 ) goto X104;
        printf("   #    type      x,y   f  eff c/m  sh gun pln ore gld  mu\n");
X104:   
        printf("%4d %-10.10s ", nb.nb_sno, mchr[ship.shp_type].m_name);
        printf("%3d,%-3d ", ship.shp_xp, ship.shp_yp);
        printf("%1c%4d%%", ship.shp_fleet, ship.shp_effc);
        printf("%4d%4d", ship.shp_crew, ship.shp_shels);
        printf("%4d%4d", ship.shp_gun, ship.shp_plns);
        printf("%4d%4d%4d\n", ship.shp_or, ship.shp_gld, ship.shp_mbl);
X370:   
        if( nxtshp(&nb, &ship) != 0 ) goto X64;
        if( nships != 0 ) goto X464;
        printf("%s: No ship(s)\n", (argp[1] == 0) ? "" : argp[1]);
        return(SYN_RETURN);
X464:   
        printf("%d ship%s\n", nships, splur(nships));
        return(NORM_RETURN);
}
