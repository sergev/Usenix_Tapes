#define D_UPDATE
#define D_NATSTAT
#define D_SECTDES
#define D_NEWSVERBS
#define D_NATSTR
#define D_SCTSTR
#define D_DCHRSTR
#define D_NSCSTR
#define D_FILES
#include        "empdef.h"

gran()
{
        register        k, number;
        char    cnbuf[32], *cname(), *splur(), *copy();
        struct  nstr    nsct;

        cleared();
        if( snxtsct(&nsct, argp[1]) == -1 ) return(SYN_RETURN);
        k = natarg(argp[2], "Grant to which country? ");
        if( k == -1 ) return(SYN_RETURN);
        copy(cname(k), cnbuf);
        number = 0;
        goto X454;
X132:   
        if( owner == 0 ) goto X454;
        printf("Sector %d,%d ", nsct.n_x, nsct.n_y);
        if( neigh(nsct.n_x, nsct.n_y, k, 1) != 0 ) goto X324;
        if( nstat == STAT_GOD ) goto X324;
        printf(" not bounded by %s\n", cnbuf);
        goto X306;
X256:   
        if( nstat == STAT_GOD ) goto X344;
X266:   
        printf("is a %s\n", dchr[sect.sct_desig].d_name);
X306:   
        goto X454;
X324:   
        if( sect.sct_desig == S_CAPIT ) goto X266;
        if( sect.sct_desig == S_SANCT ) goto X256;
X344:   
        sect.sct_owned = k;
        if( sect.sct_civil != 0 ) goto X420;
        if( sect.sct_milit != 0 ) goto X420;
        if( nstat != STAT_GOD ) goto X420;
        sect.sct_civil = sect.sct_milit = sect.sct_effic = sect.sct_lstup = 100;
X420:   
        sect.sct_lstup = curup;
        putsect(nsct.n_x, nsct.n_y);
        printf("granted\n");
        number++;
X454:   
        if( nxtsct(&nsct, UP_OWN) != 0 ) goto X132;
        if( number > 0 ) {
                printf("%d sectors granted\n", number);
                nreport(cnum, N_GRANT_SECT, k);
                sprintf(fmtbuf,"Country #%d (%s) granted you %d sector%s.", cnum, cname(cnum), number, splur(number));
                wu(0, k, fmtbuf);
        } else {
                printf("No sectors granted\n");
        }
        return(NORM_RETURN);
}
