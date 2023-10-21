#define D_FILES
#define D_NATSTR
#include        "empdef.h"

int     gnnum;

getnat(num)
int     num;
{
        register        n;
        long    lseek();

        if( num <  0 || num > maxcno ) return(-1);
        if( lseek(natf, (long)num * sizeof(nat), 0) <  0L ) {
                printf("Getnat(%d) seek %.1o failure\n", num, num * sizeof(nat))
;
                return(-1);
        }
        n = read(natf, &nat, sizeof(nat));
        if( n != sizeof(nat) ) {
                printf("Getnat(%d) read failure %d # %d\n", num, n, sizeof(nat))
;
                return(-1);
        }
        gnnum = num;
        return(0);
}

putnat(num)
int     num;
{
        int     n;
        long    lseek();

        if( gnnum != num ) {
                sprintf(fmtbuf,"putnat(%d) followed getnat(%d)", num, gnnum);
                erlog(fmtbuf);
                printf("Program error!  Please save output & notify %s.\n", privname);
                return(-1);
        }
        if( num <  0 || num > maxcno ) return(-1);
        if( lseek(natf, (long)num * sizeof(nat), 0) < 0L ) {
                printf("Wseek 0%o error\n", num * sizeof(nat));
                return(-1);
        }
        if( (n = write(natf, &nat, sizeof(nat))) != sizeof(nat) ) {
                printf("Write 0%o/%d error\n", num * sizeof(nat), n);
                return(-1);
        }
        return(0);
}
