#include    "ffdef.h"
#include    <stdio.h>
/*
**      FF -- Fast Food mainline
** Compile: cc -O -q ff.c -lS -o ff
** Copyright (c) 1977, P. Langston, New York, N.Y.
*/

extern  char    *ctime();

main(argc, argv)
char	**argv;
{
        register char *cp;
        int i;
        long lstchng, get_board();
        extern int done();

        log_plyr(argc > 1? argv[1] : getchrs("Who are you? "));
        lstchng = 0;
	signal(1, &done);
	signal(3, &done);
        bkpt(2);
        sleep(1);
loop:
        lstchng = get_board(lstchng);
	for (i = FIRST_LOCK; i <= LAST_LOCK; i++)
	    if (locks[i] == LOCK_OK) {
                printf("%s left on\n", lockfile[i]);
                unlock(i);
            }
	if (locked(UPDATE_LOCK)) {
	    sleep(1);
	    if (locked(UPDATE_LOCK)) {
		printf("Update in progress\n");
		sleep(2);
		goto loop;
	    }
	}
        command();
        goto loop;
}

command()
{
	register char **argp;
	char buf[128];
        long now;
        extern int done();

        time(&now);
        now = misc.m_nextup - now;
        if (misc.m_nextup == END_OF_GAME)
	    printf("[END] ::: ");
        else if (now < 60)
            printf("[%2lds] ::: ", now);
        else if (now < 60*60)
            printf("[%2ldm] ::: ", now/60);
        else
            printf("[%2ldh] ::: ", now/3600);
	if (fgets(buf, sizeof buf, stdin) == NULL)
	    done(0);
	buf[strlen(buf) - 1] = '\0';
	argp = parse(buf);
        switch (**argp) {
        case '\0':
	case '\n':
            break;
        case 'b':
        case 'q':
            printf("Bye-bye\n");
            done(0);
        case 'c':
            chn_rept(argp);
            break;
	case 'e':
	    view_board(0);
            break;
        case 'f':
            fin_rept();
            break;
	case 'g':
	    graph(argp);
            break;
        case 'i':
            info(argp);
            break;
        case 'm':
            mkt_rept();
            break;
	case 'n':
	    name_change(argp);
            break;
        case 'p':
	    parameters();
	    break;
	case 'r':
	    reorder_plots(argp);
            break;
        case 's':
            stk_rept();
            break;
        case 't':
            trade(argp);
            break;
        case 'u':
            upd_rept();
            break;
        case 'v':
	    view_board(1);
            break;
        default:
            printf("?%s?\n", *argp);
        case '?':
            printf("Current Fast - Food Commands are:\n");
            printf(" ? - Command list\n");
	    printf(" b          Bye; same as q\n");
	    printf(" c          Chain Report\n");
	    printf(" e          Exhibit board (w/o new plots)\n");
	    printf(" f          Finance Report\n");
	    printf(" g          Graph chain or tycoon worth\n");
	    printf(" i [topic]  Information [on topic]\n");
	    printf(" m          Market Report\n");
	    printf(" n          Name change\n");
	    printf(" p          Parameters\n");
	    printf(" q          Quit (for a while, at least)\n");
	    printf(" r          Re-order Plots\n");
	    printf(" s          Stock Report\n");
	    printf(" t          Trade (buy & sell)\n");
	    printf(" u          Update report\n");
	    printf(" v          View all plots\n");
        }
}

char	*
getarg(arg, prompt)
char	*arg, *prompt;
{
        if (*arg != '\0')
            return(arg);
        return(getchrs(prompt));
}

log_plyr(pname)
char	*pname;
{
        register char *cp;
        register int stat, luid;
        char buf[512];
        int fh, i;

        if ((fh = open(upfil, 0)) < 0) {
            printf("Fast-Food is down\n");
            done(1);
        }
        write(1, buf, read(fh, buf, 512));
        close(fh);
	luid = myluid();
        stat = 0;
	for (pnum = 0; pnum < num_players; pnum++) {
            if (getplyr(pnum, SORRY) == -1)
                break;
            if (!equal(pname, plyr.p_name))
                continue;
            stat = 1;
            if (plyr.p_uid == luid)
                return(0);
        }
        if (stat == 0)
            printf("Never heard of you...\n");
        else if (stat == 1)
	    printf("You are not %s!\n", pname);
        done(1);
}

long
get_board(lstchng)
long	lstchng;
{
	register int up;
        long now;

        up = 0;
        do {
	    getmisc(SORRY);
            if (misc.m_nextup == END_OF_GAME)
                break;
            time(&now);
            if (now < misc.m_nextup)
                break;
	    up = update_board();
        } while (up > 0);
	getmisc(SORRY);
        if (misc.m_nextup != lstchng) {
	    getboard(SORRY);
            if (misc.m_nextup != END_OF_GAME)
                printf("Next update is scheduled for %s",
                 ctime(&misc.m_nextup));
            else
                printf("The game has ended\n");
        }
        return(misc.m_nextup);
}

update_board()
{
        int exstat, up;

        if (fork() == 0) {
            execl(upd_prog, "update", 0);
            perror(upd_prog);
            done(3);
        }
        up = 1;
        while (wait1(&exstat) != -1)
            if (exstat != 0)
                up = 0;
        getplyr(pnum, SORRY);
        return(up);
}

view_board(pflg)
{
        register int i, j, k;
        char *cp;
        int type;

        getplyr(pnum, SORRY);
        printf("  ");
	for (i = 0; i < x_size; i++)
            printf(" %c", 'a' + i);
        printf("\n");
	for (j = 0; j < y_size; j++) {
            printf("%2d ", j);
	    for (i = 0; i < x_size; i++) {
                type = board[i][j];
                if (type > 0) {
		    if (getchain(type, SORRY) == -1
		     || chain.c_size == 0)
			printf("? ");
		    else
			printf("%c ", chain.c_name[0]);
                } else if (type == -1)
                    printf("* ");
		else if (pflg == 1) {
		    for (k = num_plots; --k >= 0; ) {
                        if (plyr.p_plot[k].p_x == i
			 && plyr.p_plot[k].p_y == j) {
			    printf("%d ", k);
			    break;
			}
		    }
		    if (k < 0)
			printf("  ");
		} else
		    printf("  ");
            }
	    printf("%d\n", j);
        }
        printf("  ");
	for (i = 0; i < x_size; i++)
            printf(" %c", 'a' + i);
        printf("\n");
}

reorder_plots(argp)
char	**argp;
{
        register int i, maxp;
        char *cp;
        int p1, p2;

        if (argp[1][0] != '\0')
            cp = argp[1];
        do {
            if (argp[1][0] == '\0')
                cp = getchrs("Switch which pair? ");
            if (cp == 0 || *cp == 0)
                return;
            p1 = atoip(&cp);
            if (*cp++ != ',') {
                printf("Format is p1,p2\n");
                continue;
            }
            p2 = atoip(&cp);
            getplyr(pnum, SAFE);
	    for (maxp = 0; maxp < num_plots; maxp++)
                if (plyr.p_plot[maxp].p_xy == NO_PLOT_LEFT)
                    break;
	    if (p1 < 0 || p1 >= maxp || p2 < 0 || p2 >= maxp) {
		printf("range is 0 to %d\n", maxp - 1);
                unlock(PLAYER_LOCK);
                continue;
            }
            i = plyr.p_plot[p1].p_xy;
            plyr.p_plot[p1].p_xy = plyr.p_plot[p2].p_xy;
            plyr.p_plot[p2].p_xy = i;
            time(&plyr.p_time);
            putplyr(pnum);
        } while (argp[1][0] == '\0');
}

name_change(argp)
char    **argp;
{
	register char *cp;

	cp = getarg(argp[1], "New name? ");
	cp[PNAME_LENGTH - 1] = '\0';
	if (*cp == '\0')
	    printf("That would make your name blank!\n");
	else {
	    getplyr(pnum, SAFE);
	    copy(cp, plyr.p_name);
	    putplyr(pnum);
	}
}

parameters()
{
	register int i;
	register struct times *tp;

	printf("\t~~~~~ Fast Food Parameters ~~~~~\n");
	printf("There are %d plots arranged as a %d by %d rectangle.\n",
	 x_size * y_size, x_size, y_size);
	printf("There are a maximum of %d tycoons.\n", num_players);
	printf("Each tycoon holds the rights to %d plots.\n", num_plots);
	printf("The maximum number of concurrent chains is %d.\n", num_chains);
	printf("Updates happen at: ");
	for (i = 0; i < LIM_UPDATES; i++) {
	    tp = &misc.m_time[i];
	    if (tp->t_hr == 0 && tp->t_min == 0)
		break;
	    if (i % 8 == 5)
		printf("\n");
	    printf("%d:%d%d ", tp->t_hr, tp->t_min / 10, tp->t_min % 10);
	}
	if (misc.m_satsun)
	    printf("seven days a week.\n");
	else
	    printf("monday through friday.\n");
	printf("The maximum number of transactions per update is %d.\n",
	 trans_limit);
	printf("Chains larger than %d can not be merged.\n", safe_size - 1);
	printf("It costs $%d to develop a plot.\n", plot_cost);
	printf("The stock bonus for forming a new chain is %d shares.\n",
	 new_bonus);
	printf("The major holder's bonus in a merger is $%d.\n",
	 merge1_bonus);
	printf("The minor holder's bonus in a merger is $%d.\n",
	 merge2_bonus);
}

fin_rept()
{
        register int i, cnum;
        int maxnum;
	long stockval[LIM_PLAYERS], maxval, val[LIM_PLAYERS];

	printf("\t~~~~~ Fast Food Finance Report ~~~~~\n");
        printf(".................Tycoon   Cash    Stock    Net Worth  Last Div  Trans\n");
	for (i = 0; i < num_players; i++) {
            val[i] = -1;
            if (getplyr(i, SORRY) == -1 || plyr.p_name[0] == '\0')
                continue;
            stockval[i] = 0;
	    for (cnum = 1; cnum <= LIM_CHAINS; cnum++) {
                if (plyr.p_shares[cnum] <= 0)
                    continue;
		if (getchain(cnum, SORRY) != -1)
		    stockval[i] =+ plyr.p_shares[cnum] * price(&chain);
            }
            val[i] = stockval[i] + plyr.p_money;
        }
        for (;;) {
            maxval = -1;
	    for (i = 0; i < num_players; i++) {
                if (val[i] > maxval) {
                    maxval = val[i];
                    maxnum = i;
                }
            }
            if (maxval == -1)
                break;
            getplyr(maxnum, SORRY);
            printf("%23s  $%-7ld $%-7ld  $%-8ld $%-7ld   %d\n",
             plyr.p_name, plyr.p_money, stockval[maxnum], val[maxnum],
             plyr.p_div, plyr.p_trans);
            val[maxnum] = -1;
        }
}

chn_rept(argp)
char	**argp;
{
        register int i, cnum;
	int hn;

        cnum = chainum(getarg(argp[1], "Chain name? "));
        if (cnum == -1)
            return;
	printf("\t~~~~~ %s Chain Report ~~~~~\n", chain.c_name);
	printf("shares left: %d   price: $%ld   size: %d\n",
	 chain.c_shares, price(&chain), chain.c_size);
	holders(cnum, &hldr);                     /* sort into stock order */
        printf("............Stockholder Shares Value\n");
	for (hn = 0; hn < num_players; hn++) {
	    i = hldr[hn].h_pnum;
            if (getplyr(i, SORRY) != -1
             && plyr.p_name[0] != '\0'
             && plyr.p_shares[cnum] > 0) {
                printf("%23s %6d $%ld",
                 plyr.p_name, plyr.p_shares[cnum],
                 plyr.p_shares[cnum] * price(&chain));
		if (hldr[hn].h_maj) {
		    if (hldr[hn].h_maj > 1)
			printf("  (major holder / %d)\n", hldr[hn].h_maj);
		    else
			printf("  (major holder)\n");
		} else if (hldr[hn].h_min) {
		    if (hldr[hn].h_min > 1)
			printf("  (minor holder / %d)\n", hldr[hn].h_min);
		    else
			printf("  (minor holder)\n");
		} else
                    printf("\n");
            }
        }
}

stk_rept()
{
	register int i, hn;

        getplyr(pnum, SORRY);
	printf("\t~~~~~ %s Stock Report ~~~~~\n", plyr.p_name);
        printf("..................Chain Shares Value\n");
	for (i = 1; i <= LIM_CHAINS; i++) {
            if (plyr.p_shares[i] == 0)
                continue;
	    if (getchain(i, SORRY) == -1)
		continue;
	    holders(i, &hldr);                    /* sort into stock order */
            getplyr(pnum, SORRY);
            printf("%23s %6d $%-7ld",
	     chain.c_name, plyr.p_shares[i],
	     plyr.p_shares[i] * price(&chain));
	    for (hn = 0; hn < num_players; hn++)
		if (hldr[hn].h_pnum == pnum)
		    break;
	    if (hldr[hn].h_maj) {
		if (hldr[hn].h_maj > 1)
		    printf("  (major holder / %d)\n", hldr[hn].h_maj);
		else
		    printf("  (major holder)\n");
	    } else if (hldr[hn].h_min) {
		if (hldr[hn].h_min > 1)
		    printf("  (minor holder / %d)\n", hldr[hn].h_min);
		else
		    printf("  (minor holder)\n");
	    } else
		printf("\n");
        }
}

mkt_rept()
{
        register int cnum;
        double q;
        struct	chain	total;

	printf("\t~~~~~ Fast Food Market Report ~~~~~\n");
        printf("..................Chain Shares Size Price Volume Change\n");
        total.c_size = total.c_index = total.c_shares = 0;
        total.c_volume = total.c_change = 0;
        q = 0.;
	for (cnum = 1; cnum <= LIM_CHAINS; cnum++) {
            if (getchain(cnum, SORRY) == -1 || chain.c_size == 0)
                continue;
            printf("%23s %6d %4d $%-4ld %5d  ",
             chain.c_name, chain.c_shares, chain.c_size,
             price(&chain), chain.c_volume);
            if (chain.c_change >= 0)
                printf("  +%d\n", chain.c_change);
            else
                printf("  %d\n", chain.c_change);
            total.c_shares =+ chain.c_shares;
            total.c_size =+ chain.c_size;
            total.c_index =+ price(&chain);
            total.c_volume =+ chain.c_volume;
            total.c_change =+ chain.c_change;
            q =+ 1.;
        }
        if (q == 0.)
            return;
        printf("                        ------ ---- ----- ------   ---\n");
        printf("                Totals  %6d %4d $%-4.0f %5d  ",
	 total.c_shares, total.c_size, total.c_index / q, total.c_volume);
        if (total.c_change >= 0)
            printf("  +%d\n", total.c_change);
        else
            printf(" %d\n", total.c_change);
}

trade(argp)
char	**argp;
{
        register int cnum, n;
	char *cp, buf[128];
        int bos, inc;

        if (misc.m_nextup == END_OF_GAME) {
            printf("Sorry, the market is closed\n");
            return;
        }
        getplyr(pnum, SORRY);
	if (plyr.p_trans >= trans_limit) {
            printf("Sorry, you've already made %d transactions today.\n",
             plyr.p_trans);
            return;
        }
        cp = getarg(argp[1], "Buy or sell? ");
        if (*cp == 'b')
            bos = BUY;
        else if (*cp == 's')
            bos = SELL;
        else
            return;
        cnum = chainum(getarg(argp[2], "Chain name? "));
        if (cnum == -1)
            return;
        printf("  Chain %s\nshares left: %d  price: $%ld  size: %d\n",
         chain.c_name, chain.c_shares, price(&chain), chain.c_size);
        if (bos == BUY) {
            if (chain.c_shares <= 0) {
                printf("Sorry, no shares for sale\n");
                return;
            }
            n = plyr.p_money / price(&chain);
            n = min(n, chain.c_shares);
            if (n <= 0) {
                printf("Sorry, you can't afford this stock\n");
                return;
            }
	    n = min(n, trans_limit - plyr.p_trans);
        } else {
            if (plyr.p_shares[cnum] <= 0) {
                printf("Sorry, you don't have any shares\n");
                return;
            }
            n = plyr.p_shares[cnum];
        }
	cp = buf;
	sprintf(cp, "How many shares, %d max? ", n);
	cp = getarg(argp[3], cp);
        if ((n = min(n, atoi(cp))) == 0)
            return;
        inc = bos==BUY? n : -n;
        getplyr(pnum, SAFE);
        plyr.p_money =- inc * price(&chain);
        plyr.p_shares[cnum] =+ inc;
	if (bos == BUY)
	    plyr.p_trans =+ n;
        putplyr(pnum);
	if (getchain(cnum, SAFE) != -1) {
	    chain.c_shares =- inc;
	    chain.c_index =+ inc;
	    chain.c_volume =+ n;
	    chain.c_change =+ inc;
	    putchain(cnum);
	    n = plyr.p_shares[cnum];
	    printf("You now have %d share%s.\n", n, splur(n));
	} else
	    printf("Can't read chain %d\n", cnum);
}

graph(argp)
char	**argp;
{
	register int i, num;
	char *cp, line[64];
	int hfh;
	long nw, min, max;
	double vscale;
	struct histstr hist;

	hfh = open(histfil, 0);
	cp = getarg(argp[1], "Chains or tycoons? ");
	if (*cp == 'c') {
	    min = 9999;
	    max = -9999;
	    while (read(hfh, &hist, sizeof hist) == sizeof hist) {
		for (num = 1; num <= LIM_CHAINS; num++) {
		    if (hist.h_chn[num].hc_size != 0) {
			if (hist.h_chn[num].hc_price < min)
			    min = hist.h_chn[num].hc_price;
			if (hist.h_chn[num].hc_price > max)
			    max = hist.h_chn[num].hc_price;
		    }
		}
	    }
	    vscale = 64. / (max - min + 1.);
	    printf("%15D %61D\n", min, max);
	    printf("             |-----------------------------------");
	    printf("---------------------------|\n");
	    seek(hfh, 0, 0);
	    while (read(hfh, &hist, sizeof hist) == sizeof hist) {
		for (i = 64; --i >= 0; )
		    line[i] = ' ';
		for (num = 1; num <= LIM_CHAINS; num++) {
		    if (hist.h_chn[num].hc_size != 0) {
			i = (hist.h_chn[num].hc_price - min) * vscale;
			line[i] = 'A' + num - 1;
		    }
		}
		printf("%12.12s %64.64s\n", &ctime(&hist.h_date)[4], line);
	    }
	} else if (*cp == 't') {
	    min = 9999;
	    max = -9999;
	    while (read(hfh, &hist, sizeof hist) == sizeof hist) {
		for (num = 0; num < num_players; num++) {
		    nw = hist.h_tyc[num].ht_money + hist.h_tyc[num].ht_stock;
		    if (nw != 0) {
			if (nw < min)
			    min = nw;
			if (nw > max)
			    max = nw;
		    }
		}
	    }
	    vscale = 64. / (max - min + 1.);
	    printf("%15D %61D\n", min, max);
	    printf("             |-----------------------------------");
	    printf("---------------------------|\n");
	    seek(hfh, 0, 0);
	    while (read(hfh, &hist, sizeof hist) == sizeof hist) {
		for (i = 64; --i >= 0; )
		    line[i] = ' ';
		for (num = 0; num < num_players; num++) {
		    nw = hist.h_tyc[num].ht_money + hist.h_tyc[num].ht_stock;
		    if (nw != 0) {
			i = (nw - min) * vscale;
			line[i] = '0' + num;
		    }
		}
		printf("%12.12s %64.64s\n", &ctime(&hist.h_date)[4], line);
	    }
	    for (num = 0; num < num_players; num++) {
		getplyr(num, SORRY);
		printf("%2d - %-24.24s  ", num, plyr.p_name);
		if (num & 1)
		    printf("\n");
	    }
	}
	close(hfh);
	return;
}

chainum(cp)
char *cp;
{
        register int cnum;

	for (cnum = 1; cnum <= LIM_CHAINS; cnum++)
            if (getchain(cnum, SORRY) != -1
             && chain.c_size > 0
             && chain.c_name[0] == (cp[0] & ~040))
                return(cnum);
        printf("Chain \"%s\" not found\n", cp);
        return(-1);
}

upd_rept()
{
        register int i, fh;
        char buf[512];

	printf("\t~~~~~ Fast Food Update Report ~~~~~\n");
        fh = open(newsfil, 0);
        if (fh < 0)
            perror(newsfil);
        while ((i = read(fh, buf, 512)) > 0)
            write(1, buf, i);
        close(fh);
}

info(argp)
char	**argp;
{
        register char *cp;
        char junk[64];
        int buf[18];

        if (argp[1][0] == '\0') {
            printf("Information is currently available on:\n");
            if (fork() == 0)
		execl("/bin/ls", "ls", "-~.", infodir, 0);
            wait0();
            return;
        }
        cp = copy(infodir, junk);
        *cp++ = '/';
        copy(argp[1], cp);
        if (stat(junk, buf) < 0 || argp[1][0] == '.' || argp[1][0] == '/') {
            printf("There is no info on %s\n", argp[1]);
            return;
        }
        printf("Info on %s -- Last mod : %s", argp[1], ctime(&buf[16]));
        if (fork() == 0) {
            execl(nroffil, "nroff", "-s", nroffhd, junk, 0);
            printf("Sorry, %s is not execlable\n", nroffil);
            return;
        }
        while (wait0() >= 0);
}
