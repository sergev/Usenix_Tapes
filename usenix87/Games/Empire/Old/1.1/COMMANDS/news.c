#define D_NATSTAT
#define D_NWSSTR
#define D_RPTSTR
#define D_FILES
#include        "empdef.h"

news()
{
        register        i, j, n;
        char    *ctime(), *cname();
        int     page;
        long    then, now;
        struct  nwsstr  item[64];

        head();
        time(&now);
        if( argp[1] == 0 ) goto X166;
        if( argp[1][0] == '\0' ) goto X166;
        if( (then = 86400L * atoi(argp[1])) > 328840. ) goto X166;
        then = now - then;
        goto X212;
X166:   
        then = now - 328840.;
X212:   
        printf("\nThe details of Empire news since %s", ctime(&then));
        page = 1;
X254:
        printf("\n\t=== page %d ===\n", page);
        lseek(newsf, 0L, 0);
        n = 0;
        goto X1054;
X326:
        if( page == rpt[nws.nws_vrb].r_newspage ) goto X354;
        goto X1054;
X354:   
        if( nws.nws_when >= then ) goto X406;
        goto X1054;
X406:   
        j = n;
        i = 0;
        goto X420;
X414:   
        j = i;
X416:   
        i++;
X420:   
        if( i >= n ) goto X672;
        if( item[i].nws_ntm == 0 ) goto X414;
        if( nws.nws_vrb != item[i].nws_vrb ) goto X416;
        if( nws.nws_when - item[i].nws_when >  900 ) goto X416;
        if( nws.nws_when - item[i].nws_when + 900 < 0 ) goto X416;
        if( nws.nws_ano != item[i].nws_ano ) goto X416;
        if( nws.nws_vno != item[i].nws_vno ) goto X416;
        item[i].nws_ntm++;
X672:   
        if( i < n ) goto X1054;
        if( j != n ) goto X734;
        if( n != 64 ) goto X732;
        j = freport(item, n);
        goto X734;
X732:   
        n++;
X734:   
        item[j].nws_ano = nws.nws_ano;
        item[j].nws_vrb = nws.nws_vrb;
        item[j].nws_vno = nws.nws_vno;
        item[j].nws_ntm = 1;
        item[j].nws_when = nws.nws_when;
X1054:  
        if( read(newsf, &nws, sizeof(nws)) != sizeof(nws) ) goto X1112;
        goto X326;
X1112:  
        if( freport(item, n) != -1 ) goto X1112;
        page++;
        if( page > 3 ) goto X1156;
        goto X254;
X1156:  
        return;
}

freport(ip, n)
struct  nwsstr  *ip;
int     n;
{
        register        i, j;
        long    first;
        short   flag;

        time(&first);
        flag = 0;
        for( i=0; i < n; i++ ) {
                if( ip[i].nws_ntm <= 0 ) continue;
                if( ip[i].nws_when >= first ) continue;
                j = i;
                first = ip[j].nws_when;
                flag = 1;
        }
        if( flag == 0 ) return(-1);
        preport(&ip[j]);
        return(j);
}

preport(np)
struct  nwsstr  *np;
{
        register char   *cp;
        register        i;
        char    buf[128], *ctime(), *copy(), *cname();

        printf("%-16.16s  ", ctime(&np->nws_when));
        cp = buf;
        if( (rand() & 3) != 0 ) goto X212;
        if( np->nws_ntm <= 1 ) goto X212;
        if( np->nws_ntm < NUMNUMNAMES ) {
                sprintf(fmtbuf,"%s times ", numnames[np->nws_ntm]);
                cp = copy(fmtbuf, cp);
        } else {
                sprintf(fmtbuf,"%d times ", np->nws_ntm);
                cp = copy(fmtbuf, cp);
        }
        np->nws_ntm = 1;
X212:   
        cp = copy(cname(np->nws_ano), cp);
        *cp++ = ' ';
        if( np->nws_vrb < 1 || np->nws_vrb > n_max_verb ) {
        np->nws_vrb = 0;
        }
        sprintf(fmtbuf,rpt[np->nws_vrb].r_newstory, cname(np->nws_vno));
        cp = copy(fmtbuf, cp);
        if( np->nws_ntm <= 1 ) goto X500;
        if( np->nws_ntm < NUMNUMNAMES ) {
                sprintf(fmtbuf," %s times", numnames[np->nws_ntm]);
                cp = copy(fmtbuf, cp);
        } else {
                sprintf(fmtbuf," %d times", np->nws_ntm);
                cp = copy(fmtbuf, cp);
        }
X500:   
        if( cp - buf > 61 ) {
                for( i=61; --i > 40; ) {
                        if( buf[i] == ' ' ) break;
                }
                buf[i] = '\0';
                printf("%s\n\t\t  %s\n", buf, &buf[i+1]);
        } else {
                printf("%s\n", buf);
        }
        np->nws_ntm = 0;
        return;
}

head()
{
        register        i, j;
        register struct histstr *h;
        char    *cp, *ctime(), *cname();
        int     k, scoop, n, nexti, nextj;
        long    now, news_per, news_age;
        struct histstr  {
                int     h_past;
                int     h_recent;
        }       hist[MAX_MAXNOC][MAX_MAXNOC];

        time(&now);
        if( argp[1] == 0 || 
            argp[1][0] == '\0' || 
            (news_per = 86400L * atoi(argp[1])) > 328840. ) {
                news_per = 328840.;
        }
        printf("\n        -=[  EMPIRE NEWS  ]=-\n");
        printf("::::::::::::::::::::::::::::::::::::::::::::::::::\n");
        printf("!       \"All the news that fits, we print.\"      !\n");
        printf("::::::::::::::::::::::::::::::::::::::::::::::::::\n");
        printf("       %s", ctime(&now));
        for( i=0; i < maxnoc; i++ ) {
                for( j=0; j < maxnoc; j++ ) {
                        hist[i][j].h_past = hist[i][j].h_recent = 0;
                }
        }
        lseek(newsf, 0L, 0);
        while( read(newsf, &nws, sizeof(nws)) == sizeof(nws) ) {
                news_age = now - nws.nws_when;
                if( news_age > news_per ) continue;
                if( nws.nws_ano == nws.nws_vno ) continue;
                if( (i = rpt[nws.nws_vrb].r_good_will) == 0 ) continue;
                if( news_age > (news_per>>1) ) {
                        hist[nws.nws_ano][nws.nws_vno].h_past += i;
                } else {
                        hist[nws.nws_ano][nws.nws_vno].h_recent += i;
                }
        }
        n = 0;
X640:   
        scoop = 9;
        for( i=0; i < maxnoc; i++ ) {
                for( j=0; j < maxnoc; j++ ) {
                        h = &hist[i][j];
                        k = abs(h->h_recent / 2);
                        if( k > scoop ) {
                                scoop = k;
                                nexti = i;
                                nextj = j;
                        }
                        k = abs(h->h_recent - h->h_past);
                        if( k > scoop ) {
                                scoop = k;
                                nexti = i;
                                nextj = j;
                        }
                }
        }
        printf("\n");
        if( scoop < 10 ) goto X2154;
X1052:  
        h = &hist[nexti][nextj];
        if( nstat == STAT_GOD ) {
                printf("%-3d=>%3d  ", h->h_past, h->h_recent);
        }
        k = (abs(h->h_past) > abs(h->h_recent)) ? 1 : 0;
        k += (h->h_past >= 0) ? 2 : 0;
        k += (h->h_recent >=  0) ? 4 : 0;
        switch( k ) {
        case 0:
                cp = "Carnage being wreaked by %s on %s continues unabated!";
                break;
        case 1:
                if( h->h_recent < -10 ) {
                        cp = "Further %s agression against %s";
                } else {
                        cp = "Peace talks may succeed between %s & %s";
                }
                break;
        case 2:
                if( h->h_recent < -12 ) {
                        if( h->h_past > 8 ) {
                                cp = " ! WAR !  A complete reversal of prior %s -- %s relations";
                        } else {
                                if( h->h_recent < -20 ) {
                                        cp = "%s wreaks havoc on %s!";
                                } else {
                                        cp = "VIOLENCE ERUPTS! -- %s wages war on %s";
                                }
                        }
                } else {
                        cp = "Breakdown in communication between %s & %s";
                }
                break;
        case 3:
                if( h->h_past > 10 ) {
                        cp = "FLASH!    %s turns on former ally, %s!";
                } else {
                        cp = "%s aggravates rift with %s";
                }
                break;
        case 4:
                if( h->h_recent > 10 ) {
                        cp = "%s enters new era of cooperation with %s";
                } else {
                        cp = "%s \"makes friends\" with %s";
                }
                break;
        case 5:
                if( h->h_recent > 5 ) {
                        cp = "%s willing to bury the hatchet with %s";
                } else {
                        if( h->h_past < -16 ) {
                                cp = "Tensions ease as %s attacks on %s seem at an end";
                        } else {
                                cp = "%s seems to have forgotten earlier disagreement with %s";
                        }
                }
                break;
        case 6:
                cp = "%s good deeds further growing alliance with %s";
                break;
        case 7:
                if( h->h_recent - h->h_past < -20 ) {
                        cp = "Honeymoon appears to be over between %s & %s";
                } else {
                        cp = "Friendly relations between %s & %s have cooled somewhat";
                }
                break;
        }
        h->h_past = h->h_recent = 0;
        printf(cp, cname(nexti), cname(nextj));
        printf("\n");
        h = &hist[nextj][nexti];
        k = h->h_recent - h->h_past;
        if( k <= -scoop/2 ||
            k >= scoop/2 ) {
                k &= 03;
                if( k == 0 )            printf("\tMeanwhile\n");
                else if( k == 1 )       printf("\tOn the other hand\n");
                else if( k == 2)        printf("\tAt the same time\n");
                else                    printf("\tAlthough\n");
                k = nexti;
                nexti = nextj;
                nextj = k;
                goto X1052;
        }
        n++;
        if( n < 5 ) goto X640;
X2154:  
        if( n <= 1 ) {
                printf("Relative calm prevails.\n");
        }
        return;
}
