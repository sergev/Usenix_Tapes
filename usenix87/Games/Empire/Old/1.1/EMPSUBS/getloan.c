#define D_LONSTR
#define D_FILES
#include        "empdef.h"

getloan(i)
int     i;
{
        long    lseek();

        if( lseek(loanf, (long)i * (sizeof loan), 0) == -1L ||
             read(loanf, &loan, (sizeof loan)) != (sizeof loan) ) return(-1);
        return(0);
}

putloan(i)
int     i;
{
        long    lseek();

        lseek(loanf, (long)i * (sizeof loan), 0);
        return(write(loanf, &loan, (sizeof loan)));
}
