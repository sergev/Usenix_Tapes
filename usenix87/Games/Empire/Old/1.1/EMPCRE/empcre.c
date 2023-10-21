#define D_FILES
#define D_NATSTAT
#define D_NATSTR
#define D_SCTSTR
#define D_DCHRSTR
#include        "empdef.h"
#include        <stdio.h>

char    thispro;
struct  {
        int     f1;
        int     f2;
        int     f3;
} elev[4] = { 3, 8, 0, 1, 10, 0, 3, 42, 0, 0, 40, 0 };
int     testfla, tracefl, creamfl;
extern  double  up_offset;
char    powbuf[48];
struct  {
        int     x;
        int     y;
} volc[50], gold[50], river[50];


main(argc, argv)
int     argc;
char    *argv[];
{
        register int i, j, k;
        double  pslsin();
        char    *mailbox();
        int     nvolc, ngold, nriver, d, e, st, r;
        int     sep, type[23], elmax, elmin, elsc, elcnts[1024];
        int     rx, ry, nx, ny, dx, dy;
        int     stripsiz;
        long    now, sectsiz;
        struct  sctstr  scts[128];

        sectsiz = sizeof(sect);
        time(&now);
        srand((unsigned int)now);
        while( --argc > 0 ) {
                if( argv[argc][0] == '-' ) {
                        if( argv[argc][1] == 's' &&
                            argv[argc][2] == '=' ) {
                                strcpy(sectfil, &argv[argc][3]);
                                testfla++;
                        } else if( argv[argc][1] == 't' ) {
                                tracefl++;
                        } else if( argv[argc][1] == 'c' ) {
                                creamfl++;
                        } else goto usage;
                } else if( argv[argc][0] >= '0'  &&
                    argv[argc][0] <= '9' ) {
                        srand((unsigned int)atoi(argv[argc]));
                } else {
usage:                  printf("Usage: %s [randnum] [-s=sectfil] [-trace] [-cream]\n", argv[0]);
                        exit(2);
                }
        }
        up_offset =  now/1800L - 1; /* minimum curup is 1 */
        if( !testfla ) {
                if( (natf = make( natfil )) < 0 ) exit(1);
                printf("Name for Country 0? ");
                fflush(stdout);
                nat.nat_cnam[read(0, nat.nat_cnam, 20)-1] = '\0';
                printf("Representative's name? ");
                fflush(stdout);
                nat.nat_pnam[sread(nat.nat_pnam, 20)-1] = '\0';
                nat.nat_stat = STAT_GOD;
                nat.nat_btu = 127;
                nat.nat_nuid = getuid();
                nat.nat_money = 12345;
                nat.nat_up_off = up_offset;
                write(natf, &nat, sizeof(nat));
                printf("\nAll praise to %s!\n", nat.nat_cnam);
                for( i=0; i<20; i++ ) {
                        nat.nat_cnam[i] = nat.nat_pnam[i] = '\0';
                }
                nat.nat_stat = nat.nat_btu = 0;
                for( i=1; i<maxnoc; i++ ) {
                        write(natf, &nat, sizeof(nat));
                }
                close(natf);
        }
        nvolc = (w_xsize * w_ysize)/166 + 2;
        ngold = (w_xsize * w_ysize)/333 + 2;
        nriver = (w_xsize * w_ysize)/100 + 2;
        printf("Volcanos\n");
        sep = (w_xsize + w_ysize)/4;
        k = nvolc;
        while( --k >= 0 ) {
                j = 10;
vloop:          volc[k].x = (rand()>>2) % w_xsize;
                volc[k].y = (rand()>>2) % w_ysize;
                i = nvolc;
                while( --i > k ) {
                        if( sep <= cdst(&volc[i], &volc[k]) ) continue;
                        if( --j < 0 ) {
                                sep--;
                                j = 9;
                        }
                        goto vloop;
                }
                printf("V%d\t%2d,%d [sep = %d]\n", k, volc[k].x, volc[k].y, sep);
        }
        printf("gold strikes\n");
        k = ngold;
        while( --k >= 0 ) {
                gold[k].x = (rand()>>2) % w_xsize;
                gold[k].y = (rand()>>2) % w_ysize;
                printf("%2d,%2d\n", gold[k].x, gold[k].y);
        }
        if( (sectf = make(sectfil)) < 0 ) exit(1);
        close(sectf);
        printf("Determine sector types");
        sectf = open(sectfil, O_RDWR);
        stripsiz = w_xsize * sectsiz;
        elmax = 0;
        elmin = 32767;
        for( j=0; j < w_ysize; j++ ) {
                for( i=0; i < w_xsize; i++ ) {
                        d = rand() % 10;
                        for( k=0; k<=nvolc; k++ ) {
                                d += 1500 / (dst(i - volc[k].x, j - volc[k].y) + rand()%4 + 2);
                        }
                        e = pslsin(d*166) * 45.;
                        scts[i].sct_miner = rand() % 11 + e + 45;
                        e = rand() % 3;
                        for( k=0; k<=ngold; k++ ) {
                                e += 1500 / (dst(i - gold[k].x, j - gold[k].y) + rand()%2 + 2);
                        }
                        scts[i].sct_gmin = max127(e/12 + rand()%29);
                        scts[i].sct_lstup = d;
                        if( d > elmax ) elmax = d;
                        if( d < elmin ) elmin = d;
                }
                write( sectf, scts, stripsiz );
                printf("...");
                fflush(stdout);
        }
        printf("\n");
        i = elmax;
        i -= elmin;
        elsc = i/1024 + 1;
        lseek(sectf, 0L, 0);
        for( j=0; j < w_ysize; j++ ) {
                read(sectf, scts, stripsiz);
                for( i=0; i < w_xsize; i++ ) {
                        k = (scts[i].sct_lstup - elmin)/elsc;
                        elcnts[k]++;
                }
        }
        for( i=100, k=0; i>0; k++ ) {
                elev[k].f2 = i = i - elev[k].f2;
        }
        if( i != 0 ) {
                printf("Elevs account for %d%% rather than 100%\n", 100-i);
                exit(1);
        }
        for( k=0; elev[k].f2 != 0; k++ ) {
                i = ((elev[k].f2 * w_xsize)/10 * (unsigned short)w_ysize)/10;
                for( j=0; j <= elmax - elmin; j++ ) {
                        i -= elcnts[j];
                        if( i <= 0 ) {
                                elev[k].f3 = elsc * j + elmin;
                                break;
                        }
                }
        }
        lseek(sectf, 0L, 0);
        for( j=0; j < w_ysize; j++ ) {
                read(sectf, scts, stripsiz);
                for( i=0; i < w_xsize; i++ ) {
                        d = scts[i].sct_lstup;
                        for( k=0; elev[k].f3 > 0; k++ ) {
                                if( d > elev[k].f3 ) break;
                        }
                        st = elev[k].f1;
                        if( st == 1 || st == 0 ) {
                                scts[i].sct_miner = scts[i].sct_gmin = 0;
                        }
                        scts[i].sct_desig = st;
                        type[st]++;
                        if( tracefl ) {
                                printf("%c", " ^s-"[st]);
                        }
                }
                lseek(sectf, (long)(-stripsiz), 1);
                write(sectf, scts, stripsiz);
                if( tracefl ) printf("\n");
        }
        printf("\nRivers\n");
        k = nriver;
        while( --k >= 0 ) {
rloop:          river[k].x = (rand()>>2) % w_xsize;
                river[k].y = (rand()>>2) % w_ysize;
                i = nvolc;
                while( --i >= 0 ) {
                        if( cdst(&river[k], &volc[i]) <  5 ) break;
                }
                if( i < 0 ) goto rloop;
                printf("R%d\t%2d,%d\n", k, river[k].x, river[k].y);
        }
        for( k=nriver; k >= 0; k-- ) {
                nx = river[k].x;
                ny = river[k].y;
                e = 9999;
                i = 0;
makeriv:        getsect(rx=nx, ry=ny);
                type[sect.sct_desig]--;
                type[0]++;
                sect.sct_desig = 0;
                putsect(rx, ry);
                i++;
                for( dx = -1; dx <= 1; dx++ ) {
                        for( dy = -1; dy <= 1; dy++ ) {
                                if( dy != 0 || dx != 0 ) {
                                        getsect(rx+dx, ry+dy);
                                        if( sect.sct_lstup  <= e ) {
                                                if( sect.sct_desig == 0 ) continue;
                                                e = sect.sct_lstup;
                                                nx = rx + dx;
                                                ny = ry + dy;
                                        }
                                }
                        }
                }
                if( nx != rx || ny != ry ) goto makeriv;
                printf("River %d is %d long\n", k, i);
        }
        lseek(sectf, 0L, 0);
        for( j=0; j < w_ysize; j++ ) {
                for( i=0; i < w_xsize; i++ ) {
                        read(sectf, &sect, sizeof(sect));
                        sect.sct_lstup = 0;
                        lseek(sectf, -sectsiz, 1);
                        write(sectf, &sect, sizeof(sect));
                }
        }
        lseek(sectf, 0L, 0);
        read(sectf, &sect, sizeof(sect));
        sect.sct_desig = 2;
        sect.sct_effic = 99;
        sect.sct_civil = 127;
        lseek(sectf, 0L, 0);
        write(sectf, &sect, sizeof(sect));
        close(sectf);
        d = w_xsize * w_ysize;
        for( k=0; k < 23; k++ ) {
                if( type[k] <= 0 ) continue;
                printf("%3d %-16.16s (%.1f%%)\n", type[k],
                        dchr[k].d_name, (type[k] * 100.)/d );
        }
        if( testfla ) exit(1);
        close(make(shipfil));
        powf = make(powfil);
        write(powf, powbuf, sizeof(long));
        for( i=maxnoc; i >= 0; i-- ) {
                close(creat(mailbox(i), 0600));
                write(powf, powbuf, sizeof(powbuf));
        }
        close(powf);
        close(make(treatfil));
        close(make(loanfil));
        close(make(newsfil));
}

make(file)
char    *file;
{
        register        fh;

        if( !creamfl ) {
                if( (fh = open(file, O_RDONLY)) >= 0 ) {
                        printf("%s already exists.  Remove it to do creation\n", file);
                        exit(1);
                }
        }
        if( (fh = creat(file, 0600)) < 0 ) {
                printf("Creation of %s failed.\n", file);
        }
        return(fh);
}

cdst(p0, p1)
short   p0[], p1[];
{
        return(dst(p0[0]-p1[0], p0[1]-p1[1]));
}

dst(x, y)
int     x, y;
{
        register        int     dx, dy;

        dx = xwrap(x);
        dy = ywrap(y);
        if( dx < 0 ) dx = -dx;
        if( dy < 0 ) dy = -dy;
        if( dx <= dy ) return(dy + (dx>>1));
        return(dx + (dy>>1));
}

getsect(i, j)
int     i, j;
{
        register        k;
        long    addr;

        k = ((w_ysize + j + capy)%w_ysize) * w_xsize + (w_xsize + i + capx)%w_xsize;
        addr = (long)k * sizeof(sect);
        lseek(sectf, addr, 0);
        if( read(sectf, &sect, sizeof(sect)) >= sizeof(sect) ) return;
        printf("Ooops, couldn't read sector %d,%d\n", i, j);
        exit(3);
}

putsect(i, j)
int     i, j;
{
        register        k;
        long    addr;

        k = ((w_ysize+j+capy)%w_ysize)*w_xsize + (w_xsize+i+capx)%w_xsize;
        addr = (long)k * sizeof(sect);
        lseek(sectf, addr, 0);
        write(sectf, &sect, sizeof(sect));
        return;
}
