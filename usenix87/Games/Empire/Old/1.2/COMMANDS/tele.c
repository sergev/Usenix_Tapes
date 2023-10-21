#define D_NATSTAT
#define D_NEWSVERBS
#define D_NATSTR
#define D_FILES
#include        "empdef.h"

tele()
{
        register        arg;
        char    buf[512], *cname();

        arg = natarg(argp[1], "for which country? ");
        if( arg == -1 ) return(SYN_RETURN);
        getnat(arg);
        if( nat.nat_stat > STAT_VISITOR ) goto X114;
        printf("%s has no \"telegram privileges\".\n", cname(arg));
        return(SYN_RETURN);
X114:   
        if( getele(cname(arg), buf, argp[2]) >  0 ) goto X162;
        printf("Command ignored");
        return(SYN_RETURN);
X162:   
        wu(cnum, arg, buf);
        if( nstat == STAT_GOD ) goto X244;
        if( nat.nat_stat == STAT_GOD ) goto X244;
        nreport(cnum, N_SENT_TEL, arg);
X244:   
        return(NORM_RETURN);
}
