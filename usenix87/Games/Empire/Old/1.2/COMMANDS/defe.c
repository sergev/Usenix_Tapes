#define D_UPDATE
#define D_SECTDES
#define D_SCTSTR
#define D_DCHRSTR
#define D_NSCSTR
#define D_FILES
#include        "empdef.h"

defe()
{
        char    *splur();
        short   dx, dy;
        double  rng, tfact();
        struct  nstr    nsct;

        if( snxtsct(&nsct, argp[1]) == -1 ) return(SYN_RETURN);
        
        if( argp[2] != 0 ) {
                if( argp[2][0] == '%' ) {
                        cleared();
                        while( nxtsct(&nsct, UP_OWN) > 0 ) {
                                if( owner == 0 ) continue;
                                printf("%d,%d %c ", nsct.n_x, nsct.n_y, dchr[sect.sct_desig].d_mnem);
                                if( sect.sct_dfend == 0 ||
                                    (dx = sect.sct_dfend<<8,
                                     dx = (dx>>12),
                                     dy = sect.sct_dfend<<12,
                                     dy = (dy>>12),
                                   getsect(nsct.n_x + dx, nsct.n_y + dy, UP_NONE) == -1) || owner == 0 || sect.sct_desig != S_FORTR ) {
                                        printf("undefended\n");
                                } else {
                                        printf("defended by %d,%d ", nsct.n_x + dx, nsct.n_y + dy);
                                        rng = tfact(cnum, (float)((sect.sct_guns < 7) ? sect.sct_guns : 7) * sect.sct_effic * .01);
                                        printf("with %d gun%s & %d shell%s (range = %.1f)", sect.sct_guns, splur(sect.sct_guns), sect.sct_shell, splur(sect.sct_shell), rng);
                                        if( rng*rng < dx*dx + dy*dy ) {
                                                printf(" (out of range)\n");
                                        } else {
                                                printf("\n");
                                        }
                                }
                        }
                } 
        } else {
                if( getsno(argp[2], "Defend from fort at sector ") < 0 ) return(SYN_RETURN);
                rng = tfact(cnum, (float)((sect.sct_guns < 7) ? sect.sct_guns : 7));
                printf("Max defense range is %.1f\n", rng);
                if( sect.sct_effic < 100 ) {
                        printf(" with current effic of %d%% range is %.1f\n",
                                sect.sct_effic, sect.sct_effic * rng / 100.);
                }
                rng = rng*rng;
                while( nxtsct(&nsct, UP_OWN) >  0 ) {
                        if( owner == 0 ) continue;
                        dx = sx - nsct.n_x;
                        dy = sy - nsct.n_y;
                        if( rng <  dx*dx + dy*dy ) continue;
                        sect.sct_dfend = (dx<<4) + (dy & 017);
                        putsect(nsct.n_x, nsct.n_y);
                }
        }
        return(NORM_RETURN);
}
