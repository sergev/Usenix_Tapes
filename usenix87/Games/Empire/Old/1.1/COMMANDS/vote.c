#define D_NEWSVERBS
#define D_TRTSTR
#define D_FILES
#include        "empdef.h"

vote()
{
        register        arg, a, b;
        char    *cp, *getstar(), *cname(), *ctime();
        int     c;
        long    now;

        time(&now);
        arg = onearg(argp[1], "Treaty number? ");
        if( arg == -1 ) return(SYN_RETURN);
        if( gettre(arg) == -1 ) {
                printf("There is no treaty #%d\n", arg);
                return(SYN_RETURN);
        }
        a = trty.trt_cna;
        b = (trty.trt_cnb > -trty.trt_cnb) ? trty.trt_cnb : -trty.trt_cnb;
        if( a != cnum && b != cnum ) {
                printf("You are not involved in treaty #%d", arg);
                return(SYN_RETURN);
        }
        if( now >  trty.trt_exp ) {
                printf("Treaty #%d has already expired.\n", arg);
                return(SYN_RETURN);
        }
        if( trty.trt_cnb > 0 ) {
                printf("Treaty #%d is already in effect", arg);
                return(SYN_RETURN);
        }
        c = (a == cnum) ? b : a;
        sprintf(fmtbuf,"Vote on proposed treaty with %s? ", cname(c));
        cp = getstar(argp[2], fmtbuf);
        switch( *cp ) {
        case 'y':
                if( trty.trt_cnb != -cnum ) break;
                trty.trt_cnb = cnum;
                sprintf(fmtbuf,"Treaty #%d accepted by country #%d", arg, cnum);
                wu(0, c, fmtbuf);
                nreport(cnum, N_SIGN_TRE, c);
                break;
        case 'n':
                trty.trt_cna = trty.trt_cnb = 0;
                sprintf(fmtbuf,"Treaty #%d refused", arg);
                wu(0, c, fmtbuf);
                break;
        default:
                printf("Your vote will be registered as an abstention\n");
                sprintf(fmtbuf,"Treaty %d considered by country #%d", arg, cnum);
                wu(0, c, fmtbuf);
                return(NORM_RETURN);
        }
        puttre(arg);
        if( trty.trt_cna > 0 && 
            trty.trt_cnb > 0 ) {
                printf("The treaty is in effect until %s", ctime(&trty.trt_exp));
                return(NORM_RETURN);
        }
        if( trty.trt_cna == 0 ) {
                printf("The treaty is now void\n");
                return(NORM_RETURN);
        }
        printf("The treaty is still pending\n");
        return(NORM_RETURN);
}
