#define D_SHPSTR
#define D_FILES
#include        "empdef.h"

extern int      xflg, wflg, mflg;

ships()
{
        register        i, j, k;
        char    *cp, *getstri();

        shipf = open(shipfil, O_RDWR);
X30:    
        xflg = wflg = mflg = 0;
        cp = getstri("#? ");
        if( *cp != '\0' ) goto X106;
        close(shipf);
        return;
X106:   
        i = atoi(cp);
        k = i * sizeof(ship);
        lseek(shipf, (long)k, 0);
        j = read(shipf, &ship, sizeof(ship));
        if( j >= sizeof(ship) ) goto X216;
        printf("Only %d bytes in that one...\n", j);
X216:   
        printf("Ship #%d\n", i);
        bytefix("own", &ship.shp_own, 0);
        bytefix("type", &ship.shp_type, 0);
        bytefix("effc", &ship.shp_effc, 0);
        wordfix("xp", &ship.shp_xp, 0);
        wordfix("yp", &ship.shp_yp, 0);
        bytefix("fleet", &ship.shp_fleet, 0);
        bytefix("crew", &ship.shp_crew, 0);
        bytefix("shels", &ship.shp_shels, 0);
        bytefix("gun", &ship.shp_gun, 0);
        bytefix("plns", &ship.shp_plns, 0);
        bytefix("or", &ship.shp_or, 0);
        bytefix("gld", &ship.shp_gld, 0);
        bytefix("spric", &ship.shp_spric, 0);
        wordfix("mbl", &ship.shp_mbl, 0);
        wordfix("lstp", &ship.shp_lstp, 0);
        if( mflg != 0 ) goto X624;
        goto X30;
X624:   
        lseek(shipf, (long)k, 0);
        write(shipf, &ship, sizeof(ship));
        printf("Rewritten\n");
        goto X30;
}
