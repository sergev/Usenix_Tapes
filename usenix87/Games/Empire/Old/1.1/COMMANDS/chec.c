#define D_UPDATE
#define D_SECTDES
#define D_NATSTR
#define D_SCTSTR
#define D_NSCSTR
#define D_FILES
#include        "empdef.h"
#include        <stdio.h>

chec()
{
        register        k;
        struct  nstr    nsct;

        cleared();
        if( snxtsct(&nsct, argp[1]) == -1 ) return(SYN_RETURN);
        k = 0;
        while( nxtsct(&nsct, UP_NONE) > 0 ) {
                if( owner == 0 ) continue;
                if( (k++ & 03) == 3 ) printf("\n");
                printf("%2d,%2d code:", nsct.n_x, nsct.n_y);
                fflush(stdout);
                sread(junk, 80);
                sect.sct_chkpt = atoi(junk);
                putsect(nsct.n_x, nsct.n_y);
        }
        return(NORM_RETURN);
}
