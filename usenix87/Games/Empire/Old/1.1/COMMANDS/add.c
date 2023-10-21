#define D_NATSTR
#define D_NATSTAT
#define D_FILES
#include        "empdef.h"

add()
{
        register        i, cno;
        char    *cp, *getstri(), *mailbox();

X50:
        printf("New country number? ");
        for( i=0; i <= maxcno; i++ ) {
                if( getnat(i) <  0 ) break;
                if( nat.nat_cnam[0] == '\0' ) break;
                if( nat.nat_stat == STAT_DEAD ) break;
        }
        if( i <= maxcno ) {
                printf("(%d is unused) ", i);
        } else {
                printf("(they all seem to be used) ");
        }
        cno = atoi(getstri(""));
        if( cno <= 0 || cno > maxcno ) {
                printf("Invalid country number - try again..\n");
                goto X50;
        }
        if( getnat(cno) < 0 ) {
                printf("getnat(%d) < 0\n", cno);
                return(FAIL_RETURN);
        }
        copy(getstri("Country name? "), nat.nat_cnam);
        copy(getstri("Representative? "), nat.nat_pnam);
        nat.nat_stat = STAT_DEAD;
X256:   
        cp = getstri("Status? (visitor, new, normal or god) ");
        if( *cp == 'v' ) goto X566;
        if( *cp == 'n' ) goto X576;
X312:   
        if( *cp == 'n' ) goto X622;
X322:   
        if( *cp == 'g' ) goto X646;
        printf("Use at least two letters\n");
        goto X654;
X344:   
        nat.nat_btu = nat.nat_nuid = nat.nat_playing = 0;
        nat.nat_tgms = nat.nat_xcap = nat.nat_ycap = 0;
        for( i=0; i < 4; i++ ) {
                nat.nat_b[i].b_xl = nat.nat_b[i].b_xh = nat.nat_b[i].b_yl = nat.nat_b[i].b_yh = 0;
        }
        nat.nat_money = 0;
        time(&nat.nat_date);
        nat.nat_dayno = nat.nat_minused = 0;
        if( nat.nat_stat != STAT_NEW ) goto X716;
        creat(mailbox(cno), 0600);
        goto X716;
X566:   
        nat.nat_stat = STAT_VISITOR;
        goto X654;
X576:   
        if( *(cp+1) != 'e' ) goto X312;
        nat.nat_stat = STAT_NEW;
        goto X654;
X622:   
        if( *(cp+1) != 'o' ) goto X322;
        nat.nat_stat = STAT_NORMAL;
        goto X654;
X646:   
        nat.nat_stat = STAT_GOD;
X654:   
        if( nat.nat_stat != STAT_DEAD ) goto X666;
        goto X256;
X666:   
        if( nat.nat_stat == STAT_NEW ) goto X344;
        if( nat.nat_stat == STAT_VISITOR ) goto X344;
        printf("No special initializations done...\n");
X716:   
        if( putnat(cno) >= 0 ) goto X752;
        printf("putnat(%d) < 0\n", cno);
X752:   
        sprintf(fmtbuf,"Country #%d, (%s), added as `%s'",
                cno, nat.nat_cnam, cp);
        erlog(fmtbuf);
        return(NORM_RETURN);
}
