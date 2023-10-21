#define D_SCTSTR
#define D_SECTDES
#define D_NATSTAT
#define D_NATSTR
#define D_COMSTR
#define D_UPDATE
#define D_FILES
#include "empdef.h"

#ifdef  BSD
#include        <sys/time.h>
#else
#include        <time.h>
#endif  BSD

#include        <stdio.h>

extern  int     thisprog;
extern int acce(), add(), assa(), atta(), boar(), buil(), cens(), chan(),
           chec(), coll(), cont(), coun(), decl(), defe(), deli(), desi(), diss(),
           enli(), fire(), flee(), fly(), fore(), gran(), head(), info(),
           ledg(), lend(), load(), look(), map(), mine(), move(), nati(),
           navi(), news(), offe(), powe(), rada(), rea(), real(), repa(),
           rout(), set(), shi(), spy(), tele(), tend(), torp(), trad(),
           trea(), turn(), upda(), vers(), vote(), weat();

struct comstr coms[] = {
"accept (loan)",        5,      2,      acce,   STAT_NORMAL,
"add [a new country]",  7,      0,      add,                    STAT_GOD,
"assault (sect) [from ship]",   3,      2,      assa,   STAT_NORMAL,
"attack (sect) [from sect]",    3,      2,      atta,   STAT_NORMAL,
"board (ship) [from ship]",     3,      2,      boar,   STAT_NORMAL,
"build [ships in] (sects) <type>",      4,      2,      buil,   STAT_NORMAL,
"bye [log-off]",        9,      0,      (int (*)())100, STAT_DEAD,
"census [on] (sects)",  1,      0,      cens,   STAT_NOCAP,
"change country/representative/user",   2,      0,      chan,   STAT_NOCAP,
"checkpoint (sects)",   2,      2,      chec,   STAT_NORMAL,
"collect [on] (loan)",  5,      2,      coll,   STAT_NORMAL,
"command list [brief]", 10,     0,      (int (*)())3,   STAT_DEAD,
"contract [in] (sector)",       5,      2,      cont,   STAT_NORMAL,
"country roster",       1,      0,      coun,   STAT_VISITOR,
"declare ally/neutral/war (cno/cname)", 2,      2,      decl,   STAT_NOCAP,
"defend (sects)",       2,      2,      defe,   STAT_NORMAL,
"deliver (item) (sects) <thresh/->",    1,      1,      deli,   STAT_NOCAP,
"designate (sects)",    1,      1,      desi,   STAT_NOCAP,
"dissolve [government]",        7,      0,      diss,   STAT_NOCAP,
"enlist [in] (sects)",  2,      2,      enli,   STAT_NORMAL,
"execute [commands from] file", 10,     0,      (int (*)())5,   STAT_NOCAP,
"fire [on] (sect/ship)",        3,      2,      fire,   STAT_NORMAL,
"fixup [nations, sectors, etc]",        98,     0,      (int (*)())100, STAT_GOD,
"fleetadd (fleet) (ships/fleet)",       4,      1,      flee,   STAT_NORMAL,
"fly [mission from] (sect/ship)",       7,      2,      fly,    STAT_NORMAL,
"forecast [weather in] (sects)",        1,      1,      fore,   STAT_NOCAP,
"grant (sects) [to country]",   2,      1,      gran,   STAT_NOCAP,
"headlines <days> [from \"news\"]",     2,      0,      head,   STAT_VISITOR,
"info [on] (topic)",    1,      0,      info,   STAT_VISITOR,
"ledger [loan report]", 5,      0,      ledg,   STAT_NOCAP,
"lend [money to] (cno/cname)",  5,      2,      lend,   STAT_NOCAP,
"list of commands",     10,     0,      (int (*)())3,   STAT_DEAD,
"load (ships/fleet)",   4,      2,      load,   STAT_NORMAL,
"lookout [from] (ships/fleet/sects)",   4,      2,      look,   STAT_NOCAP,
"map [from] (sect)",    1,      1,      map,    STAT_NOCAP,
"mine [from] (ship)",   4,      2,      mine,   STAT_NORMAL,
"move c/m/s/g/p/o/b",   1,      2,      move,   STAT_NORMAL,
"nation [report]",      1,      0,      nati,   STAT_NOCAP,
"navigate (ships/fleet)",       4,      2,      navi,   STAT_NORMAL,
"newspaper <days>",     2,      0,      news,   STAT_VISITOR,
"offer [treaty to] (cno/cname)",        2,      2,      offe,   STAT_NOCAP,
"power report", 2,      0,      powe,   STAT_VISITOR,
"radar [from] (ships/fleet/sect)",      4,      2,      rada,   STAT_NOCAP,
"read telegrams",       1,      0,      rea,    STAT_NOCAP,
"realm (number) <(sects)>",     1,      0,      real,   STAT_NOCAP,
"repay (loan)", 5,      2,      repa,   STAT_NORMAL,
"route [delivery] (item) (sects)",      1,      1,      rout,   STAT_NOCAP,
"set [price] (ships/fleet/sects) (item)",       5,      2,      set,    STAT_NOCAP,
"shell [spawn a subshell]",     99,     0,      (int (*)())100, STAT_NOCAP,
"ship report [on] (ships/fleet/sects)", 4,      1,      shi,    STAT_NOCAP,
"spy [on] (sects)",     2,      1,      spy,    STAT_NOCAP,
"telegram [to] (cno/cname) <file>",     1,      0,      tele,   STAT_NOCAP,
"tend (ship)",  4,      2,      tend,   STAT_NORMAL,
"torpedo (ship)",       3,      2,      torp,   STAT_NORMAL,
"trade [report] <land | naval>",        5,      2,      trad,   STAT_NOCAP,
"treaty report",        2,      1,      trea,   STAT_NOCAP,
"turn [the game] (on|off)",     7,      0,      turn,   STAT_GOD,
"unload (ship/fleet)",  4,      2,      load,   STAT_NORMAL,
"update (sects) <quiet | verbose>",     1,      1,      upda,   STAT_NOCAP,
"version [identification]",     7,      0,      vers,   STAT_VISITOR,
"vote [on] (treaty)",   2,      2,      vote,   STAT_NORMAL,
"weather [map for] (sects)",    1,      0,      weat,   STAT_NOCAP,
0,      0,      0,      0,      0,
};

main(argc, argv)
int     argc;
char    **argv;
{
        register        n;
        int port, bye();
        struct  tm      *nowtime;
        long now;
        double timu, init_na();
        char    *copy();

/*
        setbuf(stdout, (char *)NULL);
*/
        argv[0] = (char *)-1;
        if( (cnum = argv[1][0] - '0' - 1) < 0 ) goto X44;
        if( cnum <= maxcno ) goto X64;
X44:    
        printf("Sorry, Empire problems\n");
        exit(2);
X64:    
        if( (redirin = argv[3][0] - '0' - 1) >= 0 ) goto X114;
        exit(-1);
X114:   
        init_fi();
        if( getnat(0) != -1 ) goto X144;
        exit(3);
X144:   
        up_offset = nat.nat_up_off;
        time(&now);
        curup = now/1800. - up_offset;
        n = now & 077777;
        srand((unsigned)n);
        timu = init_na();
        port = ttyn();
        ntused = 0;
        dolcost = 0.;
        proto = 0;
X270:   
        signal(1, bye);
        signal(3, bye);
/* The command just completed; switch stdout back to its origin if necessary */
        if( proto == 0 ) goto X366;
        fflush(stdout); /* because we're using printf */
        close(1);
        dup(savfd1);
        close(savfd1);
        proto = 0;
X366:   
        if( getnat(cnum) != -1 ) goto X414;
        exit(-1);
X414:   
        if( dolcost == 0. ) goto X606;
        if( dolcost <= 100. ) goto X472;
        printf("That just cost you $%.2f\n", dolcost);
X472:   
        if( dolcost >= -100. ) goto X540;
        printf("You just made $%.2f\n", -dolcost);
X540:   
        nat.nat_money -= dolcost;
        putnat(cnum);
        dolcost = 0.;
X606:   
        time(&now);
        nowtime = (struct tm *)localtime(&now);
        ncomstat = nstat;
        if( (nowtime->tm_yday & 0177) == nat.nat_dayno ) goto X742;
        nat.nat_dayno = nowtime->tm_yday & 0177;
        nminused = nat.nat_minused = 0;
        putnat(cnum);
        lasttime = now;
X742:   
        n = (now - lasttime)/60.;
        lasttime += n * 60.;
        nminused += n;
        if( nminused < m_m_p_d ) goto X1030;
        if( nstat == STAT_NORMAL ) goto X1040;
X1030:  
        ncomstat = nstat;
        goto X1052;
X1040:  
        ncomstat = nstat - 1;
X1052:  
        wait((int *) 0);
        if( argc <= 0 ) goto X1130;
        if( argv[2][0] == '\0' ) goto X1130;
        argc = 0;
        copy(argv[2], combuf);
        goto X1650;

X1130:  
        ntime -= ntused;
        ntused = 0;
        if( (n = open(downfil, O_RDONLY)) < 0 ) goto X1240;
        close(n);
        if( nstat == STAT_GOD ) goto X1230;
        execl(emprog[0], "\0177", "empire", 0);
        goto X1240;

X1230:  
        printf("The game is down\n");
X1240:  
        if( port == nat.nat_playing ) goto X1306;
        if( nat.nat_playing == 0 ) {
                printf("Welcome back to the shell (so many levels!)\n");
        } else {
                printf("Oooops! you're on the wrong port!\n");
        }
        exit(-1);
X1306:  
        if( nat.nat_tgms == 0 ) goto X1422;
        if( nat.nat_tgms != 1 ) goto X1332;
        printf("You have a new telegram waiting ...\n");
        goto X1406;

X1332:  
        if( nat.nat_tgms >= 21 || nat.nat_tgms < 0 ) goto X1376;
        printf("You have %s new telegrams waiting ...\n", numnames[nat.nat_tgms]);
        goto X1406;

X1376:  
        printf("You have several new telegrams waiting ...\n");
X1406:  
        nat.nat_tgms = 0;
        putnat(cnum);
X1422:  
        nat.nat_btu = ntime;
        nstat = nat.nat_stat;
        if( nstat != STAT_NOCAP ) goto X1456;
        printf("You lost your capital... better designate one\n");
X1456:  
        if( putnat(cnum) != -1 ) goto X1504;
        exit(-1);
X1504:  
        n = getcomm(combuf);
        if( n != -1 ) goto X1572;
        if( redirin != 0 ) goto X1540;
        bye();
        goto X1572;

X1540:  
        close(0);
        dup(redirin);
        close(redirin);
        redirin = 0;
X1572:  
        if( n >= 1 ) goto X1604;
        goto X270;

X1604:  
        if( n != -1 ) goto X1616;
        goto X270;

X1616:  
        if( redirin == 0 ) goto X1650;
        printf("%s\n", combuf);
X1650:  
        if( parse(combuf) == -1 ) goto X1706;
        if( dispatc() <  0 ) goto X1706;
        goto X270;
X1706:  
        printf("Try \"command list\" or \"info commands\"\n");
        goto X270;
}

getcomm(combuf)
char *combuf;
{
        register char *cp;
        register int i;
        char buf[128], prompt[64];

        if( redirin == 0 ) {
                cp = "Command";
        } else {
                cp = "Execute";
        }
        sprintf(fmtbuf,"\n[%d:%d] %s : ", nminused, ntime, cp);
        copy(fmtbuf, prompt);
        do {
                if ((i = prmptrd(prompt, buf, 80)) <= 0)
                        return (-1);
        } while (i < 2);
        cp = copy(buf, combuf);
        return (cp - combuf);
}

init_fi()
{

        sectf = open(sectfil, O_RDWR);
        natf = open(natfil, O_RDWR);
        newsf = open(newsfil, O_RDWR);
        shipf = open(shipfil, O_RDWR);
        trtf = open(treatfil, O_RDWR);
        loanf = open(loanfil, O_RDWR);
}

double
init_na()
{
        register int i;
        long now, previous;
        double timu, q;

        if (getnat(cnum) == -1)
                exit(-1);

        capx = nat.nat_xcap;
        capy = nat.nat_ycap;
        if (getsect(0, 0, UP_OWN) == -1)
                exit(-1);

/*
        Update nation btus, tlevel, and rlevel only
        if 0,0 is owned and is a capital. 
        Have to do another getnat because getsect calls update which
        can change nat struct.
*/
        time(&now);
        getnat(cnum);
        if( owner == 0 ||
          ( sect.sct_desig != S_CAPIT &&
            sect.sct_desig != S_SANCT )) {
                if( nat.nat_stat == STAT_NORMAL ) {
                        printf("Your capital is missing!\n");
                        nat.nat_stat = STAT_NOCAP;
                }
                timu = 0.;
                nat.nat_date = now;
        } else if( sect.sct_desig == S_CAPIT || cnum == 0 ) {
                if( sect.sct_lstup == 0 ) {
                        sect.sct_lstup = curup;
                }
                putsect(0, 0);
                q = ((sect.sct_civil * sect.sct_effic) / 4.5e6) + 1.e-7;
                previous = nat.nat_date;
                i = (now - previous) * q;
                nat.nat_date = (i / q) + previous + .99;
                timu = i;
                q = 1. - ((nat.nat_date - previous) / 8.64e6);
                if( q < 0 ) q = 0;
                nat.nat_t_level = q * nat.nat_t_level;
                nat.nat_r_level = q * nat.nat_r_level;
                nat.nat_btu = (i + nat.nat_btu > 256.) ? 256 : i + nat.nat_btu;
        } else if( sect.sct_desig == S_SANCT ) {
                nat.nat_date = now;
        }
        putnat(cnum);
        ntime = nat.nat_btu;
        nstat = nat.nat_stat;
        i = 0;
        do {
                nrealm[i].b_xl = nat.nat_b[i].b_xl;
                nrealm[i].b_xh = nat.nat_b[i].b_xh;
                nrealm[i].b_yl = nat.nat_b[i].b_yl;
                nrealm[i].b_yh = nat.nat_b[i].b_yh;
        } while( ++i < 4 );
        nminused = nat.nat_minused & 0377;
        lasttime = now;
        if( nat.nat_money < 0 ) broke = 1;
        for( i = 0; i < maxnoc; i++ ) {
                if( i == cnum ) continue;
                if( getnat(i) == -1 ) continue;
                capxof[i] = nat.nat_xcap - capx;
                capyof[i] = nat.nat_ycap - capy;
        }
        return(timu);
}

dispatc()
{
        register int    n, comprog, (*comaddr)();
        char    *cp;
        int     retcode, cost, fmtlen;

        if( (n = comtch(argp[0])) < 0 ) {
                if( n == -2 ) {
                        printf("\"%s\" is ambiguous -- ", combuf);
                } else {
                        printf("\"%s\" is not a legal command ", combuf);
                        if( nstat != ncomstat ) printf("now ");
                        printf("\n\t");
                }
                return(-1);
        }
        comprog = coms[n].c_prog;
        comaddr = coms[n].c_addr;
        cost = coms[n].c_cost;
        if( ntime < cost && cost > 0 ) {
                printf("You don't have the BTU's, bozo\n");
                return(0);
        }
        if( comprog == thisprog  ) {
                ntused += cost;
/* If stdout has been redirected, output command  */
                if( proto != 0 ) {
                        printf("\n%s\n", combuf);
                }
                if( (retcode = (*comaddr)()) == 2 ) {
                        ntused -= cost;
                }
                return(0);
        }
        switch( comaddr ) {
        case 2:
        case 4:
        default:
                call(comprog);
                break;
        case 1:
                printf("Unimplemented command\n");
                break;
        case 3:
                explain();
                break;
        case 5:
                redirin = dup(0);
                close(0);
                if( (n = open(argp[1], O_RDONLY)) == 0 ) break;
                perror(argp[1]);
                dup(redirin);
                close(redirin);
                redirin = 0;
                break;
        }
        return(0);
}

parse(cmndp)
char    *cmndp;
{
        register char   *cp, *bp, *ap;
        int     i, nargs;
        static char     argbuf[128];

        cp = cmndp;
        ap = argbuf;
        condarg = "";
        nargs = 0;
        goto X222;
X36:    
        cp++;
        goto X222;
X42:    
        argp[i++] = 0;
        goto X260;
X62:    
        if( 16 <= nargs ) goto X252;
        argp[nargs++] = ap;
        if( *cp != '"' ) goto X126;
        i = *cp++;
        goto X134;
X126:   
        i = ' ';
X134:   
        bp = ap;
        goto X152;
X140:   
        if( i == *cp ) goto X156;
        *ap++ = *cp++;
X152:   
        if( *cp != '\0' ) goto X140;
X156:   
        *ap++ = 0;
        if( *bp != '>' ) goto X200;
        redirec(bp + 1);
        goto X216;
X200:   
        if( *bp != '?' ) goto X222;
        condarg = bp + 1;
X216:   
        nargs--;
X222:   
        if( *cp == ' ' ) goto X36;
        if( *cp == '\0' ) goto X252;
        if( *cp != '|' ) goto X62;
        pipe_to(cp + 1);
X252:   
        i = nargs;
X260:   
        if( 16 >  i ) goto X42;
        return(nargs - 1);
}

pipe_to(string)
char    *string;
{
        register char   *cp;
        int     pfh[2];

        cp = string;
        goto X20;
X16:    
        cp++;
X20:    
        if( *cp == ' ' ) goto X16;
        if( *cp == '\0' ) {
                printf("Incorrect pipe syntax, see `info syntax'\n");
                return;
        }
        if( pipe(pfh) <  0 ) {
                printf("pipe failed...\n");
                return;
        }
        switch( fork() ) {
        case 0:
                close(0);
                dup(pfh[0]);
                close(pfh[1]);
                execl("/bin/sh", "sh", "-c", cp, 0);
                printf("Oops! pipe execl failed\n");
                exit(3);
        case -1:
                printf("pipe fork() failed...\n");
        default:
/*
        Close the reader, then attach the output pipe to stdout.
        Save the current stdout for when the command terminates.
*/
                close(pfh[0]);
                proto = pfh[1];
                savfd1 = dup(1);
                close(1);
                dup(proto);
                close(proto);
        }
}

redirec(file)
char    *file;
{
        register        char    *cp;

/* There should be more complete analysis here of various error conditions */
        cp = file;
        if( *cp != '>' ) goto X146;
        if( (proto = open(++cp, O_WRONLY)) >= 0 ) goto X124;
        if( fork() != 0 ) goto X100;
        resetui();
        if( access(cp, 0) != 0 ) {
                umask(0);
                close(creat(cp, 0622)); /* create it if it didn't exist */
        } else {
                chmod(cp, 0622); /* allow writing if disallowed before */
        }
        exit(0);
X100:   
        wait((int *) 0);
        proto = open(cp, O_WRONLY);
X124:   
        lseek(proto, 0L, 2);
        goto X266;
X146:   
        if( fork() != 0 ) goto X242;
        resetui();
        unlink(cp);
        umask(0);
        close(creat(cp, 0622));
        exit(0);
X242:   
        wait((int *) 0);
        proto = open(cp, O_WRONLY);
X266:   
/*
        Save current stdout for when the command terminates.
        Attach current stdout to the newly opened file.
*/
        if( proto >= 0 ) {
                savfd1 = dup(1);
                close(1);
                dup(proto);
                close(proto);
                return;
        }
        sprintf(fmtbuf,"Can't open '%s'", cp);
        perror(fmtbuf);
        proto = 0;
        return(0);
}

call(prog)
int     prog;
{
        register        i;
        char    *argv[4], cnumbuf[2], redirbuf[2];
        int     cp;
        long    now;
        static int      cnum1, redirin1;
        double  timu;

        if( getnat(cnum) == -1 ) {
                printf("Nation file trouble...\n");
                exit(-1);
        }
        sigsave();
        if( prog == 9 ) {
                nat.nat_playing = 0;
        }
        time(&now);
        timu = ntime - ntused;
        if( timu >= -32767. ) goto X146;
        timu = -32767.;
X146:   
        if( timu <= 256. ) goto X166;
        timu = 256.;
X166:   
        if( timu >= 0 ) goto X212;
        nat.nat_btu = timu - .5;
        goto X216;
X212:   
        nat.nat_btu = timu;
X216:   
        i = ((now - lasttime + 30.) / 60.) + nminused;
        nat.nat_minused = (i < 256) ? i : 255;
        putnat(cnum);
/*
        We're going to switch modules, so switch stdout back to the
        original connection if it was redirected to a pipe or a file.
        The command will get re-parsed, and the redirection will happen
        again in the new module.
*/
        if( proto ) {
                close(1);
                dup(savfd1);
                close(savfd1);
                proto = 0;
        }
        i = 3;
        do {
                if( i == redirin ) continue;
                close(i);
        } while( ++i < 15 );
        cnum1 = cnum + 1;
        cnumbuf[0] = '0' + cnum1;
        cnumbuf[1] = '\0';
        redirin1 = redirin + 1;
        redirbuf[0] = '0' + redirin1;
        redirbuf[1] = '\0';
        if( prog <= 0 ) goto X532;
        if( 7 <  prog ) goto X532;
        execl(emprog[prog], "Empire", cnumbuf, combuf, redirbuf, "\0377", 0);
        printf("We got a prog missing\007! (#%d,%s)\n", prog, combuf);
        perror(emprog[prog]);
        call(9);
        return;
X532:   
        if( 9 != prog ) goto X556;
        printf("Bye-bye\n");
        exit(0);
X556:   
        if( 98 == prog ) goto X576;
        if( 99 != prog ) return;
X576:   
        if( fork( ) != 0 ) goto X740;
        if( 99 != prog ) goto X672;
        printf("Toodles...type '^D' to return to Empire\n");
        fflush(stdout);
        resetui();
        execl("/bin/sh", "Empshell", shllrg1, 0);
        goto X720;
X672:   
        execl(empfix, "empfix", "\0377", 0);
X720:   
        printf("Exec failed\n");
        exit(3);
X740:   
        if( wait((int *) 0) != -1 ) goto X740;
        printf("Welcome back to Empire.\n");
        cnumbuf[0] = '0' + cnum1;
        cnumbuf[1] = '\0';
        redirbuf[0] = '0' + redirin1;
        redirbuf[1] = '\0';
        argv[0] = emprog[thisprog];
        argv[1] = cnumbuf;
        argv[2] = "";
        argv[3] = redirbuf;
        main(4, argv);
        exit(3);
}

bye()
{
        return(call(9));
}

explain()
{
        register char   flipflop, *cform;
        register        i;

        printf("\t\tCurrent EMPIRE Command List\n");
        printf("\t\t------- ------ ------- ----\n");
        if( nstat > 0 ) {
                printf("Initial number is cost in B.T.U. units.\n");
                printf("Text enclosed in brackets is comment");
                printf(" rather than part of the command.\n");
                if( nstat > 1 ) {
                        printf("Items in parentheses have the following meanings:\n");
                        printf("  (cname) :: country name\n");
                        printf("  (cno) :: country number\n");
                        printf("  (fleet) :: fleet designation, or area\n");
                        printf("  (item) :: commodity such as \"ore\", \"guns\", etc\n");
                        printf("  (loan) :: loan number\n");
                        printf("  (sect) :: sector coordinates in the form:   x,y\n");
                        printf("  (sects) :: sector(s) in the form:");
                        printf("   lox:hix,loy:hiy/d ?cond&cond&...  \n");
                        printf("  (ship) :: one ship number\n");
                        printf("  (ships) :: ship number(s) separated with '/'\n");
                }
        }
X222:   
        flipflop = 0;
        for( i=0; (cform = coms[i].c_form) != 0; i++ ) {
                if( ncomstat <  coms[i].c_permit ) continue;
                flipflop ^= 1;
                if( flipflop != 0 ) {
                        printf("%d %-34.34s", coms[i].c_cost, cform);
                } else {
                        printf("%d %s\n", coms[i].c_cost, cform);
                }
        }
        if( flipflop != 0 ) {
                printf("\n");
        }
        printf("For further info on command syntax see \"info syntax\".\n");
        return;
}
