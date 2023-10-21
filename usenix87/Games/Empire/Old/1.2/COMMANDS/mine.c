#define D_UPDATE
#define D_SECTDES
#define D_SHIPTYP
#define D_SCTSTR
#define D_SHPSTR
#define D_FILES
#include        "empdef.h"

mine()
{
        register        num, aship;

        aship = getshno(argp[1], "from ship? ", &ship);
        if( aship != -1 ) goto X46;
        printf("Usage: mine (ship)");
        return(SYN_RETURN);
X46:    
        if( cnum == ship.shp_own ) goto X70;
        printf("Not your ship");
        return(SYN_RETURN);
X70:    
        if( ship.shp_type != S_DES ) goto X106;
        if( ship.shp_shels != 0 ) goto X116;
X106:   
        printf("Can't mine from this one...");
        return(FAIL_RETURN);
X116:   
        if( getsect(ship.shp_xp, ship.shp_yp, UP_NONE) == -1 ) goto X166;
        if( sect.sct_desig == S_WATER ) goto X176;
        if( sect.sct_desig == S_BSPAN ) goto X176;
X166:   
        printf("You can't do that there!!");
        return(FAIL_RETURN);
X176:   
        sprintf(fmtbuf,"Drop how many mines? <max %d> ", ship.shp_shels);
        num = onearg(argp[2], fmtbuf);
        num = (num > ship.shp_shels) ? ship.shp_shels : num;
        goto X300;
X260:   
        printf("Splash...  ");
        sect.sct_shell++;
        ship.shp_shels--;
X300:   
        if( --num >= 0 ) goto X260;
        printf("\n");
        putsect(ship.shp_xp, ship.shp_yp);
        putship(aship, &ship);
        return(NORM_RETURN);
}
