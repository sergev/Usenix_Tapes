#define D_UPDATE
#define D_SECTDES
#define D_SCTSTR
#define D_DCHRSTR
#define D_ICHRSTR
#define D_NSCSTR
#define D_FILES
#include        "empdef.h"

cont()
{
        register char   *cp;
        char    *delp, *getstar(), *cname();
        int     prdbid, rnd, prdtyp;
        struct  ichrstr *ip;
        struct  nstr    nsct;

        if( snxtsct(&nsct, argp[1]) == -1 ) return(SYN_RETURN);
        while( nxtsct(&nsct, UP_OWN) > 0 ) {
                if( owner == 0 ) continue;
                prdtyp = (int)dchr[sect.sct_desig].d_ptyp;
                ip = &ichr[prdtyp];
                if( (prdbid = ip->i_bid) == 0 ) continue;
                delp = 0;
                if( (int)ip->i_del != 0 ) {
                        delp = (int)ip->i_del + (char *)&sect.sct_owned;
                }
                time(junk);
                rnd = ((((nsct.n_x + junk[0] + nsct.n_y) & 077777)) % 50) + 75;
                if( sect.sct_contr != 0 ) {
                        printf("Although you already have a contract, ");
                        if( delp != 0 ) *delp = 0;
                        sect.sct_contr = 0;
                }
                printf("%s offers $%.2f per ", cname(0), (rnd / 100.) * prdbid );
                if( sect.sct_desig == S_GMINE ||
                    sect.sct_desig == S_MINE ) {
                        printf("ton of ore");
                } else {
                        printf("unit of production");
                }
                sprintf(fmtbuf," in %d,%d.  Do you accept?", nsct.n_x, nsct.n_y);
                cp = getstar(argp[2], fmtbuf);
                if( *cp == 'y' ) {
                        sect.sct_contr = rnd;
                        if( delp != 0 ) *delp = 2;
                }
                putsect(nsct.n_x, nsct.n_y);
        }
        return(NORM_RETURN);
}
