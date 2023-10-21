#define D_UPDATE
#define D_SCTSTR
#define D_NATSTAT
#define D_NATSTR
#define D_FILES
#include        "empdef.h"
#include        <sys/types.h>
#include        <sys/stat.h>

nati()
{
        register        i;
        double  tfact();
        char     *ctime(), *getlogn();
        char    *mailbox(), *splur(), *cname();
        struct  stat    statbuf;

        if( getnat(cnum) != -1 ) goto X32;
        printf("Couldn't getnat\n");
        return(SYN_RETURN);
X32:    
        printf("\t%s Nation Report\t", nat.nat_cnam);
        prdate();
        printf("Nation status is ");
        if( nat.nat_stat == STAT_DEAD ) {
                printf("DEAD");
        } else if( nat.nat_stat == STAT_VISITOR ) {
                printf("VISITOR");
        } else if( nat.nat_stat == STAT_NEW ) {
                printf("NEW");
        } else if( nat.nat_stat == STAT_NOCAP ) {
                printf("IN FLUX (no capital)");
        } else if( nat.nat_stat == STAT_NORMAL ) {
                printf("ACTIVE");
        } else if( nat.nat_stat == STAT_GOD ) {
                printf("GOD");
        } else printf("?%d?", nat.nat_stat);
        printf("     Bureaucratic Time Units: %d\n", nat.nat_btu);
        printf("Last nation update occurred on %s", ctime(&nat.nat_date));
        if( nat.nat_stat <= STAT_NOCAP ) goto X444;
        getsect(0, 0, UP_OWN);
        getnat(cnum);
        printf("Capital is %d%% efficient and has %d civilian%s and %d military\n", sect.sct_effic, sect.sct_civil, splur(sect.sct_civil), sect.sct_milit);
X444:   
        if( nat.nat_nuid == 0 ) goto X504;
        printf("Representative's uid: %d  User-name: %s\n", nat.nat_nuid, getlogn(nat.nat_nuid));
        goto X514;
X504:   
        printf("No specific user-id\n");
X514:   
        stat(mailbox(cnum), &statbuf);
        i = statbuf.st_size;
        printf("Telegram file contains %d character%s\n", i, splur(i));
        for( i=0; i<4; i++ ) {
                printf("#%d  %d:%d,%d:%d    ", i, nat.nat_b[i].b_xl, nat.nat_b[i].b_xh, nat.nat_b[i].b_yl, nat.nat_b[i].b_yh);
        }
        printf("\n");
        printf("     The treasury has $%ld\n", nat.nat_money);
        printf("Technology level is %.2f       Research level is %.2f\n", nat.nat_t_level, nat.nat_r_level);
        printf("Technology factor : %.2f%%", tfact(cnum, 100.));
        printf("      Plague factor : %.2f%%\n", (nat.nat_t_level + 100.)/(nat.nat_r_level + 100.));
        printf("  Formal Relations with Other Countries:\n");
        for(i=0; i < maxnoc; i++ ) {
                if( cnum == i ) continue;
                if( getnat(i) == -1 ) continue;
                if( nat.nat_stat < STAT_NEW ) continue;
                switch( getrel(cnum, i) ) {
                case NEUTRAL:
                        printf("Neutral toward");
                        break;
                case ALLIED:
                        printf("Allied with");
                        break;
                case HOSTILE:
                        printf("Hostile toward");
                        break;
                case AT_WAR:
                        printf("At war with");
                        break;
                }
                printf(" %s (%d)\n", cname(i), i);
        }
        printf("  Other Countries Formal Relations with %s\n", cname(cnum));
        for( i=0; i < maxnoc; i++ ) {
                if( cnum == i ) continue;
                if( getnat(i) == -1 ) continue;
                if( nat.nat_stat < STAT_NEW ) continue;
                printf("%s -- ", cname(i));
                switch( getrel(i, cnum) ) {
                case NEUTRAL:
                        printf("neutral\n");
                        break;
                case ALLIED:
                        printf("allied\n");
                        break;
                case HOSTILE:
                        printf("hostile\n");
                        break;
                case AT_WAR:
                        printf("at war\n");
                        break;
                }
        }
        return(NORM_RETURN);
}
