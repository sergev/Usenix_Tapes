#define D_SHPSTR
#define D_NSCSTR
#define D_FILES
#include        "empdef.h"

flee()
{
        register        i;
        char    fltnam, *splur();
        struct  nbstr   nb;

        fltnam = argp[1][0];
        if( fltnam < 'A' ) goto X36;
        if( fltnam <= 'z' ) goto X66;
X36:    
        if( fltnam !=  '~' ) goto X56;
        fltnam = ' ';
        goto X66;
X56:    
        printf("Specify fleet, (1 alpha char or `~')");
        return(SYN_RETURN);
X66:    
        if( snxtshp(&nb, argp[2], cnum, "Ships to be added? ") == -1 ) return(SYN_RETURN);
        i = 0;
        goto X170;
X142:   
        ship.shp_fleet = fltnam;
        putship(nb.nb_sno, &ship);
        i++;
X170:   
        if( nxtshp(&nb, &ship) != 0 ) goto X142;
        printf("%d ship%s added to fleet `%c'\n", i, splur(i), fltnam);
        return(NORM_RETURN);
}
