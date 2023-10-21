#include        "ffdef.h"

#define	COUNTED_NEIGHBOR	-2

#define LOG0(f)                 pr(f)
#define LOG1(f,a1)              sprintf(fbuf,f,a1),pr(fbuf)
#define LOG2(f,a1,a2)           sprintf(fbuf,f,a1,a2),pr(fbuf)
#define LOG3(f,a1,a2,a3)        sprintf(fbuf,f,a1,a2,a3),pr(fbuf)
#define LOG4(f,a1,a2,a3,a4)     sprintf(fbuf,f,a1,a2,a3,a4),pr(fbuf)
#define LOG5(f,a1,a2,a3,a4,a5)  sprintf(fbuf,f,a1,a2,a3,a4,a5),pr(fbuf)

char    fbuf[256];      /* formatting buf for LOGs */
int	nfh;		/* news file handle */
int     onfh;           /* old news file handle */

struct  player  p[LIM_PLAYERS];
struct  chain   c[LIM_CHAINS + 1];

main(argc, argv)
char **argv;
{
        register int i, j;
        char buf[512];
	int next, num, numok, *ip;
	long min, now;
        struct times *tp;
        struct {
	    int     l_pnum;
	    long    l_time;
	} list[LIM_PLAYERS];

        signal(1, 1);
        signal(2, 1);
        signal(3, 1);
	for (i = 0; i++ < 25; ) {
	    num = 0;                                /* count lock failures */
	    if (lock(UPDATE_LOCK) == LOCK_FAIL)
		done(1);
	    for (i = FIRST_LOCK; i <= LAST_LOCK; i++) {
		if (lock(i) == LOCK_FAIL) {
		    LOG1("Update has trouble locking %s\n", lockfile[i]);
                    num++;
                }
            }
	    if (num == 0)                                 /* get them all? */
		break;
	    for (i = FIRST_LOCK; i <= LAST_LOCK; i++)
		if (i != UPDATE_LOCK)
		    unlock(i);                    /* avoid deadly embraces */
	    sleep(1);
	}
	if (num > 0) {
	    printf("Update couldn't get all locks and gave up\n");
	    done(1);
	}
	getmisc(SAFE);
        time(&now);
	if (now < misc.m_nextup || misc.m_nextup == END_OF_GAME)
	    done(1);                                   /* no update needed */
        nfh = noisyopen(newsfil, 0);
	onfh = noisyopen(onwsfil, 1);
	seek(onfh, 0, 2);
        while ((i = read(nfh, buf, 512)) > 0)
	    write(onfh, buf, i);         /* copy last update into old news */
	close(onfh);
        close(nfh);
        nfh = creat(newsfil, 0600);
        if (nfh < 0) {
            perror(newsfil);
            done(3);
        }
	getboard(SAFE);                                   /* read in board */
        cfh = noisyopen(chainfil, 2);
	read(cfh, &c, sizeof c);                 /* read entire chain file */
        pfh = noisyopen(plyrfil, 2);
	read(pfh, &p, sizeof p);                /* read entire player file */
	dohist();                         /* make pre-update history entry */
/* Sort players into playing order */
        num = 0;
	for (pnum = 0; pnum < num_players; pnum++) {
            if (p[pnum].p_name[0] == '\0')
                continue;
            list[num].l_pnum = pnum;
            list[num].l_time = p[pnum].p_time;
            num++;
        }
	LOG3("\n%d player%s ready to play as of %s",
	 num, splur(num), ctime(&misc.m_nextup));
/* Go through players in order and make their plays */
        if (num == 0)
	    numok = 1;                    /* avoid ending game prematurely */
        else
            numok = 0;
        for (;;) {
            min = 2000000000l;
	    for (i = 0; i < num; i++) {
                if (list[i].l_time < min) {
                    min = list[i].l_time;
                    next = i;
                }
            }
            if (min == 2000000000l)
                break;
            list[next].l_time = 2000000000l;
            numok += tryfor(list[next].l_pnum);
        }
/* figure out time for next update */
next_update:
        ip = localtime(&now);
	num = 1441;                           /* just over a day (minutes) */
	for (i = 0; i < LIM_UPDATES; i++) {
            tp = &misc.m_time[i];
            if (tp->t_hr == 0 && tp->t_min == 0)
                continue;
            j = (tp->t_hr - ip[2]) * 60 + tp->t_min - ip[1];
	    if (j <= 0)
                j += 1440;
            if (j < num) {
		num = j;                                        /* minutes */
		next = ip[0];                             /* extra seconds */
            }
        }
        misc.m_nextup = now + num * 60l - next;
	if (misc.m_satsun == 0) {                /* no updates on weekends */
            ip = localtime(&misc.m_nextup);
	    if (ip[6] == 6) {                               /* if saturday */
                now = misc.m_nextup + 1441 * 60l;
                goto next_update;
	    } else if (ip[6] == 0) {                          /* if sunday */
                now = misc.m_nextup + 60l;
                goto next_update;
            }
        }
/* write it all out */
        if (numok == 0)
	    game_end();
	putmisc();
	putboard();
	for (i = 1; i <= LIM_CHAINS; i++) {
	    c[i].c_volume = c[i].c_change = 0;
	    c[i].c_index = 100;
	}
        seek(cfh, 0, 0);
        write(cfh, &c, sizeof c);
        seek(pfh, 0, 0);
        write(pfh, &p, sizeof p);
	dohist();                        /* make post-update history entry */
        done(0);
}

tryfor(pn)
{
        register int i, j;
        int cnum, future;
        long div;

	LOG2(">>> %s  plots last rearranged %s",
	 p[pn].p_name, ctime(&p[pn].p_time));
	if (p[pn].p_money < plot_cost) {
	    LOG0("Not enough cash to develop a new plot\n");
	    if (p[pn].p_plot[0].p_xy == NO_PLOT_LEFT)
		future = 0;
	    else
		future = 2;
            goto dividends;
        }
	for (i = 0; i < num_plots; i++) {
            if (p[pn].p_plot[i].p_xy == NO_PLOT_LEFT)
                continue;
            if (play(pn, &p[pn].p_plot[i]) != -1) {
		for (j = i + 1; j < num_plots; j++)
		    p[pn].p_plot[j - 1].p_xy = p[pn].p_plot[j].p_xy;
		putmisc();
		getplot(&p[pn].p_plot[num_plots - 1]);
		p[pn].p_money -= plot_cost;
                break;
	    }
	    LOG2("%c%d -- illegal\n",
	     'a' + p[pn].p_plot[i].p_x, p[pn].p_plot[i].p_y);
        }
	if (i >= num_plots) {
	    LOG0("Tough luck, no move\n");
            future = 0;
	} else if (p[pn].p_plot[0].p_xy == NO_PLOT_LEFT)
            future = 0;
	else
            future = 1;
dividends:
        div = 0;
	for (cnum = 1; cnum <= LIM_CHAINS; cnum++) {
            if (p[pn].p_shares[cnum] <= 0)
                continue;
            div += price(&c[cnum]) * p[pn].p_shares[cnum] / 10l;
        }
	LOG1("  Dividend income $%D\n", div);
        p[pn].p_div = div;
        p[pn].p_money += div;
        p[pn].p_trans = 0;
	if (future == 2)                         /* if broke at the moment */
            if (div > 0)
		return(1);            /* will have money in the future [?] */
	    else
		return(0);                             /* no income either */
        return(future);
}

play(pn, pltp)
struct	plot	*pltp;
{
        register int i, j;
	char list[4], connect[LIM_CHAINS + 1], loose;
        int conchain, ntype, toobigs, maxsize, cnum;

	for (i = 1; i <= LIM_CHAINS; i++)
            connect[i] = 0;
        loose = conchain = 0;
	for (i = 0; i < 4; i++) {
            ntype = neightype(pltp, i);
            if (ntype == SINGLE_STORE)
                loose++;
            if (ntype <= 0)
                continue;
            if (connect[ntype] == 0)
                list[conchain++] = ntype;
            connect[ntype]++;
        }
        if (conchain > 1) {
/* merge chains ---     biggest triumphs unless >1 too big, then return -1 */
            toobigs = 0;
            maxsize = 0;
	    for (i = 0; i < conchain; i++) {
                j = howmany(list[i]);
		if (j >= safe_size)
                    toobigs++;
                if (j > maxsize) {
                    maxsize = j;
                    cnum = list[i];
                }
            }
            if (toobigs > 1)
                return(-1);
            board[pltp->p_x][pltp->p_y] = cnum;
				    /* recalculate size of resulting chain */
            c[cnum].c_size = contig(pltp, cnum);
	    LOG4("%c%d -- MERGE into %s (now %d large)\n",
	     'a' + pltp->p_x, pltp->p_y, c[cnum].c_name, c[cnum].c_size);
	    for (i = 0; i < conchain; i++) {
                if (list[i] == cnum)
                    continue;
		LOG1(" %s merged out\n", c[list[i]].c_name);
		holder_bonus(list[i]);                 /* hand out bonuses */
		trade_in(list[i], cnum, pn);           /* dispose of stock */
                c[list[i]].c_size = 0;
            }
        } else if (conchain == 1) {
/* add to old chain */
            cnum = list[0];
            board[pltp->p_x][pltp->p_y] = cnum;
				    /* recalculate size of resulting chain */
            c[cnum].c_size = contig(pltp, cnum);
	    LOG4("%c%d -- Add to %s (now %d large)\n",
	     'a' + pltp->p_x, pltp->p_y, c[cnum].c_name, c[cnum].c_size);
        } else if (loose > 0) {
/* make new chain (if poss) */
	    i = 0;                                /* count existing chains */
	    for (cnum = 1; cnum <= LIM_CHAINS; cnum++)
		if (c[cnum].c_size > 0)
		    i++;
	    if (i >= num_chains)                     /* max number already */
		return(-1);
	    for (cnum = misc.m_lstchn + 1; c[cnum].c_size > 0; cnum++)
		if (cnum > LIM_CHAINS)
		    cnum = 0;
	    misc.m_lstchn = cnum;
            board[pltp->p_x][pltp->p_y] = cnum;
            makechain(pltp, cnum);
	    c[cnum].c_shares -= new_bonus;
	    p[pn].p_shares[cnum] += new_bonus;
	    LOG4("%c%d -- Create new chain %s (%d large)\n",
	     'a' + pltp->p_x, pltp->p_y, c[cnum].c_name, c[cnum].c_size);
        } else {
            board[pltp->p_x][pltp->p_y] = SINGLE_STORE;
	    LOG2("%c%d -- Single store\n", 'a' + pltp->p_x, pltp->p_y);
        }
        return(0);
}

makechain(pltp, cnum)
{
	c[cnum].c_name[0] = 'A' + cnum - 1;      /* better safe than sorry */
        c[cnum].c_size = contig(pltp, cnum);
        c[cnum].c_index = 100;
        c[cnum].c_shares = 100;
        c[cnum].c_volume = c[cnum].c_change = 0;
}

contig(pltp, cnum)                 /* find all plots contiguous with pltp. */
struct  plot    *pltp;  /* make them all chain 'cnum' and return the count */
{
        register int i, j, k;
	struct plot list[LIM_X_SIZE * LIM_Y_SIZE];
        struct plot *pp, curplot, *neigh();

        k = 1;
        list[0].p_xy = pltp->p_xy;
        do {
            curplot.p_xy = list[--k].p_xy;
	    for (i = 0; i < 4; i++) {
                pp = neigh(&curplot, i);
                if (pp == 0
                 || board[pp->p_x][pp->p_y] == 0
                 || board[pp->p_x][pp->p_y] == COUNTED_NEIGHBOR)
                    continue;
                board[pp->p_x][pp->p_y] = COUNTED_NEIGHBOR;
                list[k++].p_xy = pp->p_xy;
            }
        } while (k > 0);
	return(howmany(cnum));    /* count the size and invert the numbers */
}

howmany(cnum)
{
        register int i, j, count;

        count = 0;
	for (j = 0; j < y_size; j++) {
	    for (i = 0; i < x_size; i++) {
		if (board[i][j] == cnum || board[i][j] == COUNTED_NEIGHBOR) {
                    board[i][j] = cnum;
                    count++;
                }
            }
        }
        return(count);
}

struct	plot	*
neigh(pltp, dir)
struct	plot	*pltp;
{
        static struct plot nplt;

        nplt.p_xy = pltp->p_xy;
        switch (dir) {
	case 0:                                                      /* -y */
            if (nplt.p_y <= 0)
                return(0);
            --nplt.p_y;
            break;
	case 1:                                                      /* +y */
	    if (nplt.p_y >= y_size - 1)
                return(0);
            nplt.p_y++;
            break;
	case 2:                                                      /* -x */
            if (nplt.p_x <= 0)
                return(0);
            --nplt.p_x;
            break;
	case 3:                                                      /* +x */
	    if (nplt.p_x >= x_size - 1)
                return(0);
            nplt.p_x++;
            break;
        }
        return(&nplt);
}

neightype(pltp, i)
struct	plot	*pltp;
{
        pltp = neigh(pltp, i);
        if (pltp == 0)
            return(0);
        return(board[pltp->p_x][pltp->p_y]);
}

holder_bonus(cnum)
{
	register int i, j, hn;

	LOG0("  Major & Minor Holder's Bonuses:\n");
	seek(pfh, 0, 0);              /* since holders() needs latest info */
	write(pfh, &p, sizeof p);
	if (holders(cnum, &hldr) == 0) {
	    LOG0("  No holders -- No bonuses\n");
            return;
        }
	for (hn = 0; hn < num_players; hn++) {
	    i = hldr[hn].h_pnum;
	    j = 0;
	    if (hldr[hn].h_maj > 0)
		j += merge1_bonus / hldr[hn].h_maj;
	    if (hldr[hn].h_min > 0)
		j += merge2_bonus / hldr[hn].h_min;
	    if (j > 0)
		LOG2("\t%s gets $%d\n", p[i].p_name, j);
	}
}

trade_in(cha, chb, pn)
{
        register int i, num;
	char trades[LIM_PLAYERS];
	int skipped;
	long green, doleach;

	for (i = num_players; --i >= 0; )
	    trades[i] = 0;
	if (chb == -1)                             /* no trades, just sell */
            goto sell;
	num = c[chb].c_shares;          /* trade in cha for chb at 2 for 1 */
        skipped = 0;
	for (i = pn; num > 0; i++) {
	    if (i >= num_players)
                i = 0;
            if (p[i].p_shares[cha] < 2)
		if (skipped++ >= num_players)
                    break;
                else
                    continue;
            p[i].p_shares[cha] -= 2;
            p[i].p_shares[chb] += 1;
            --num;
            skipped = 0;
	    trades[i]++;
	}
        c[chb].c_shares = num;
sell:                                /* sell back all remaining cha shares */
	doleach = price(&c[cha]);
	for (i = 0; i < num_players; i++) {
	    if (trades[i])
		LOG5("  %d %s traded for %d %s by %s\n", trades[i] * 2,
		 c[cha].c_name, trades[i], c[chb].c_name, p[i].p_name);
	    if (p[i].p_shares[cha] > 0) {
		green = p[i].p_shares[cha] * doleach;
		LOG4("  %d %s sold back by %s for $%D\n",
		 p[i].p_shares[cha], c[cha].c_name, p[i].p_name, green);
		p[i].p_money += green;
		p[i].p_shares[cha] = 0;
	    }
        }
}

pr(string)                      /* put string in news file and also on tty */
{
        register char *cp;

	for (cp = string; *cp++; );
	cp -= string + 1;
        write(1, string, cp);
        write(nfh, string, cp);
}

noisyopen(file, mode)
char *file;
{
        register int fh;

        fh = open(file, mode);
        if (fh < 0) {
            perror(file);
            done(3);
        }
        return(fh);
}

game_end()
{
        register int i;

	misc.m_nextup = END_OF_GAME;
	LOG0("The game has ended; no more plots can be developed\n");
	LOG0("\nFinal bonuses are as follows:\n");
	for (i = 1; i <= LIM_CHAINS; i++) {
            if (c[i].c_size <= 0)
                continue;
	    LOG1("%s - ", c[i].c_name);
            holder_bonus(i);
	    trade_in(i, -1, 0);                        /* sell back shares */
        }
}

dohist()             /* save misc info on players & chains in history file */
{
	int hfh, cnum;
	long sum;
	struct histstr hist;

	hfh = noisyopen(histfil, 2);
	seek(hfh, 0, 2);
	time(&hist.h_date);
	for (cnum = 1; cnum <= LIM_CHAINS; cnum++) {
	    hist.h_chn[cnum].hc_size = c[cnum].c_size;
	    hist.h_chn[cnum].hc_shares = c[cnum].c_shares;
	    hist.h_chn[cnum].hc_price = price(&c[cnum]);
	}
	for (pnum = 0; pnum < num_players; pnum++) {
	    hist.h_tyc[pnum].ht_money = p[pnum].p_money;
	    sum = 0;
	    for (cnum = 1; cnum <= LIM_CHAINS; cnum++)
		if (p[pnum].p_shares[cnum] > 0)
		    sum += (long) p[pnum].p_shares[cnum]
		     * hist.h_chn[cnum].hc_price;
	    hist.h_tyc[pnum].ht_stock = sum;
	}
	write(hfh, &hist, sizeof hist);
	close(hfh);
}
