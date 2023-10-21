#define D_NATSTAT
#define D_NEWSVERBS
#define D_NATSTR
#define D_FILES
#include        "empdef.h"

chan()
{
        register char   *cp;
        register char   c;
        char     *getstar(), *copy(), *getlogn();
        int     ruid;

        cp = getstar(argp[1], "country name, representative or user? ");
        cleared();
        getnat(cnum);
        switch( *cp ) {
        case 'c':
                cp = getstar(argp[2], "New country name -- ");
                *(cp+19) = '\0';
                for( c=0; getnat(c) != -1; c++ ) {
                        if( same(cp, nat.nat_cnam) == 0 ) continue;
                        printf("Country #%d is already called `%s'!", c, nat.nat_cnam);
                        return(SYN_RETURN);
                }
                getnat(cnum);
                copy(cp, nat.nat_cnam);
                nreport(cnum, N_NAME_CHNG, 0);
                break;
        case 'r':
                cp = getstar(argp[2], "New representative name -- ");
                *(cp+19) = '\0';
                copy(cp, nat.nat_pnam);
                break;
        case 'u':
                ruid = myruid();
                if( nat.nat_nuid == 0 ) {
                        printf("Currently anyone");
                } else {
                        printf("Currently only %s", getlogn(nat.nat_nuid));
                }
                printf(" can use this country.\n");
                printf("You may reset it to only you (%s) or to anyone.\n", getlogn(ruid));
                cp = getstar(argp[2], "Which? (answer \"me\" or \"any\") ");
                if( *cp == 'm' ) {
                        nat.nat_nuid = ruid;
                        break;
                }
                if( *cp == 'a' ) {
                        nat.nat_nuid = 0;
                        break;
                }
                printf("Let's just leave it as is, then.\n");
                break;
        default:
                printf("Only \"country\", \"representative\" or \"user\" can change.");
                return(SYN_RETURN);
        }
        putnat(cnum);
        return(NORM_RETURN);
}
