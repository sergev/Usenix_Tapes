#define D_NATSTR
#define D_FILES
#include        <stdio.h>
#include        "empdef.h"

cleared()
{
        if( getnat(cnum) == -1 ) return(-1);
        printf("I.D. check: Your name? ");
        fflush(stdout);
        sigsave();
        junk[sread(junk, 24)-1] = '\0';
        printf("\n");
        if( same(junk, nat.nat_pnam) != 0 ) return(0);
        printf("Eeek! An impostor!\n");
        bye();
}
