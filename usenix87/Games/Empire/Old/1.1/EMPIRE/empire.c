#define D_FILES
#define D_TELSTR
#define D_NATSTR
#define D_NATSTAT
#define D_SCTSTR
#include        "empdef.h"
#include        <stdio.h>

char    cjunk[80], pjunk[80];
int     uid, port;

main(argc, argv)
int     argc;
char    **argv;
{
        register        i;
        int     legit, notgod, logport;
        char    *copy(), *cname();

        if( argc > 1 ) copy(argv[1], cjunk);
        if( argc > 2 ) copy(argv[2], pjunk);
        argv[argc-1][0] = (char)-1;
        argv[argc-1][1] = '\0';
        i = 15;
        while( --i > 2 ) close(i);
        notgod = (uid = myruid()) == privuid ? 0 : 1;
        port = ttyn(2);
        telf = open(downfil, O_RDONLY);
        if( telf >= 0 ) {
                read(telf, &tgm, (sizeof tgm));
                printf("Empire down for repairs as of %s", ctime(&tgm.tel_date));
                read(telf, junk, tgm.tel_length);
                printf("%s\n", junk);
                if( notgod != 0 ) exit(1);
                close(telf);
        }
        telf = open(upfil, O_RDONLY);
        if( telf >= 0 ) {
                read(telf, &tgm, (sizeof tgm));
                read(telf, junk, tgm.tel_length);
                printf("%s\n", junk);
                close(telf);
        }
        if( argc < 2 ) {
                prmptrd("What nation do you represent? ", cjunk, 25);
        }
        if( cjunk[0] == '\0' ) exit(1);
        if( argc < 3 ) {
                printf("Your name? ");
                fflush(stdout);
                pjunk[sread(pjunk, 24)-1] = '\0';
                printf("\n");
        }
        if( (natf = open(natfil, O_RDWR)) <= 0 ) {
                printf("Can't open nation file.\n");
                exit(3);
        }
        legit = 0;
        for( cnum=0; cnum < maxnoc; cnum++ ) {
                if( read(natf, &nat, (sizeof nat)) != (sizeof nat) ) break;
                if( cnum == 0 ) {
                        up_offset = nat.nat_up_off;
                }
                if( same(cjunk, nat.nat_cnam) == 0 ) continue;
                legit = 1;
                if( notgod != 0 &&
                    nat.nat_playing != 0 &&
                    port != nat.nat_playing ) {
                        logport = nat.nat_playing;
                        legit = 2;
                }
                if( nat.nat_stat == STAT_DEAD ) legit = 5;
                if( notgod != 0 &&
                    nat.nat_nuid != uid &&
                    nat.nat_stat != STAT_NEW &&
                    nat.nat_stat != STAT_VISITOR &&
                    nat.nat_nuid != 0 ) legit = 3;
                if( same(pjunk, nat.nat_pnam ) == 0 &&
                    nat.nat_stat != STAT_VISITOR ) legit = 4;
                if( legit == 1 ) ok();
        }
        switch( legit ) {
        case 0:
                printf("We have no '%s' in our records ...\n", cjunk);
                printf("If you wish to observe or apply see %s.\n", privname);
                exit(1);
        case 2:
                printf("You are already logged in on tty%c.\n", logport);
                exit(1);
        case 3:
                printf("Sorry, I don't recognize you, #%d\n", uid);
                exit(1);
        case 4:
                printf("You are not empowered to represent %s\n", cjunk);
                exit(1);
        case 5:
                printf("%s is a dead country.\n", cjunk);
                exit(1);
        }
}

ok()
{
        char    cnumbuf[2];

        if( nat.nat_stat == STAT_NEW ) new();
        nat.nat_playing = port;
        if( nat.nat_stat == STAT_VISITOR ) {
                printf("Type \"info\" or \"list\" for help.");
                printf("  Enjoy your visit.\n");
        } else {
                printf("\t-=O=-\n");
        }
        lseek(natf, (long)cnum * (sizeof nat), 0);
        write(natf, &nat, (sizeof nat));
        if( nat.nat_money <= 1000 ) {
                if( nat.nat_stat != STAT_VISITOR ) {
                        if( nat.nat_money > 0 ) {
                                printf("Your cash reserves stand at $%ld\n", nat.nat_money );
                        } else {
                                printf("You are broke!\n");
                                printf("  Mines & industries are on strike.\n");
                        }
                }
        }
        close(natf);
        cnum = cnum + 1;
        cnumbuf[0] = (char)cnum + '0';
        cnumbuf[1] = '\0';
        execl(emprog[1], "Empire", cnumbuf, "", "1", "\0377", 0);
        printf("Sorry, but the game is hiding... (%s missing)\n", emprog[1]);
        exit();
}

new()
{
        register        i, s;
        long    addr;
        struct  sctstr  s1, s2;

        if( (sectf = open(sectfil, O_RDWR)) < 0 ) {
                printf("Can't open sector file");
                exit(3);
        }
        time(&nat.nat_date);
        srand((unsigned)nat.nat_date);
        i = 0;
        do {
                s = rand()%(w_xsize*w_ysize - 1);
                addr = (long)s * (sizeof sect);
                lseek(sectf, addr, 0);
                read(sectf, &s1, (sizeof sect));
                read(sectf, &s2, (sizeof sect));
                if( s1.sct_owned != 0 ||
                    s1.sct_desig < 3  ||
                    s2.sct_owned != 0 ||
                    s2.sct_desig < 3 ) continue;
                s1.sct_owned = s2.sct_owned = cnum;
                s1.sct_desig = s2.sct_desig = 2;
                s1.sct_effic = s2.sct_effic = 20;
                s1.sct_ore = s2.sct_ore = s1.sct_milit = s2.sct_milit = s1.sct_civil = s2.sct_civil = 127;
                s1.sct_mobil = s2.sct_mobil = 100;
                s1.sct_lstup = s2.sct_lstup = nat.nat_date/1800. - up_offset;
                lseek(sectf, addr, 0);
                write(sectf, &s1, (sizeof sect));
                write(sectf, &s2, (sizeof sect));
                close(sectf);
                printf("You have been accepted as nation #%d!\n", cnum);
                nat.nat_stat = STAT_NORMAL;
                nat.nat_btu = 75 ;
                nat.nat_nuid = uid;
                nat.nat_tgms = 0;
                nat.nat_xcap = s % w_xsize;
                nat.nat_ycap = s / w_xsize;
                i = 0;
                do {
                        nat.nat_b[i].b_xl = 0;
                        nat.nat_b[i].b_xh = 1;
                        nat.nat_b[i].b_yh = 0;
                        nat.nat_b[i].b_yl = 0;
                } while( ++i < 4 );
                nat.nat_money = 5000;
                printf("Your country consists of 2 sectors");
                printf(" (sanctuaries at 0,0 & 1,0),\n");
                printf("containing 254 civilians, 254 military troops");
                printf(" and 254 tons of ore.\n");
                printf("Your country also has $%ld in \"cash\".\n", nat.nat_money);
                printf("For playing information type:\n");
                printf("\"command list\" and/or \"info\"");
                printf(" and/or \"info overview\"\n");
                printf("in response to the \"[75] Command : \" prompt\n");
                printf("\n\t\tGood Luck!\n");
                return;
        } while( ++i < 1000 );
        printf("The world is already too full...  sorry\n");
        exit(1);
}
