#include    "ffdef.h"
#include    <stdio.h>
/*
**      FIX -- Fast Food Fixup
** Compile: cc -O -q fix.c ffglb.o fflib.a -lP -lS -o fix
** Copyright (c) 1980, P. Langston, New York, N.Y.
*/

int     skipflg = 0;

main(argc, argv)
char	**argv;
{
        register char *cp;
        extern int done();

        int i;

	if (argv[1][0] != 'g' || argv[1][1] != '\0')
	    exit(2);
	argv[1][0] = '\0';
        bkpt(2);
loop:
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
        extern int done();

	printf("Fix? ");
	if (fgets(buf, sizeof buf, stdin) == NULL)
	    done(0);
	buf[strlen(buf) - 1] = '\0';
	skipflg = 0;
	argp = parse(buf);
        switch (**argp) {
        case '\0':
	case '\n':
            break;
        case 'b':
	    fixboard();
	    break;
        case 'c':
	    fixchain(argp);
            break;
	case 'm':
	    fixmisc();
	    break;
        case 'p':
	    fixplayer(argp);
	    break;
        case 'q':
            printf("Bye-bye\n");
            done(0);
        default:
            printf("?%s?\n", *argp);
        case '?':
	    printf("Current Fast Food Fix Commands are:\n");
            printf(" ? - Command list\n");
	    printf(" b - Board\n");
	    printf(" c - Chain\n");
	    printf(" m - Misc\n");
	    printf(" p - Player\n");
	    printf(" q - Quit\n");
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

fixmisc()
{
	register int i, j;
	char *cp;
	struct plot plts[LIM_X_SIZE * LIM_Y_SIZE];

	getmisc(SAFE);
	read(mfh, &plts, sizeof plts);
	if (misc.m_nextup != END_OF_GAME)
	    printf("Next update is scheduled for %s",
	     ctime(&misc.m_nextup));
	else
	    printf("The game has ended\n");
	lfix("m_nextup", &misc.m_nextup);
	bfix("m_satsun", &misc.m_satsun);
	bfix("m_lstchn", &misc.m_lstchn);
	ifix("m_lstplt", &misc.m_lstplt);
	printf("Plot list:\n");
	skipflg <<= 1;
	for (i = 0; i < x_size * y_size && skipflg == 0; i++) {
	    printf(" plts[%d]", i);
	    pfix("", &plts[i]);
	}
	skipflg >>= 1;
	printf("Update times:\n");
	skipflg <<= 1;
	for (i = 0; i < LIM_UPDATES && skipflg == 0; i++)
	    tfix("m_time", &misc.m_time[i]);
	skipflg >>= 1;
	seek(mfh, sizeof misc, 0);
	write(mfh, &plts, sizeof plts);
	putmisc();
	printf("Rewritten\n");
	return(0);
}

fixboard()
{
	register int i, j;
	char *cp;

	getboard(SAFE);
	for (;;) {
	    cp = getchrs("plot? ");
	    if (cp == NULL | *cp == '\0')
		break;
	    if (*cp < 'a' || *cp > 'z'
	     || cp[1] < '0' || cp[1] > '9')
		printf("   Format is letter-number, e.g. b6\n");
	    else {
		i = *cp++ - 'a';
		j = atoip(&cp);
		bfix(" val", &board[i][j]);
	    }
	}
	putboard();
	printf("Rewritten\n");
	return(0);
}

fixplayer(argp)
char    **argp;
{
        register char *cp;
        register int stat, luid;
	char buf[512], *pname;
        int fh, i;

	pname = getarg(argp[1], "player? (name or #number) ");
	if (*pname == '#') {
	    pnum = atoi(++pname);
	    if (getplyr(pnum, SAFE) == -1) {
		printf("I have no records of player #%d\n", pnum);
		return(-1);
	    }
	} else {
	    for (pnum = 0; pnum < num_players; pnum++) {
		if (getplyr(pnum, SAFE) == -1)
		    break;
		if (!equal(pname, plyr.p_name))
		    continue;
		if (plyr.p_uid == luid)
		    break;
	    }
	    if (pnum >= num_players) {
		printf("Can't find player `%s'\n", pname);
		return(-1);
	    }
	}
	sfix("Name", &plyr.p_name, PNAME_LENGTH);
	lfix("p_time", &plyr.p_time);
	lfix("p_money", &plyr.p_money);
	lfix("p_div", &plyr.p_div);
	ifix("p_uid", &plyr.p_uid);
	ifix("p_trans", &plyr.p_trans);
	if (skipflg == 0 && *getchrs("Plots? ") == 'y') {
	    for (i = 0; i < num_plots && skipflg == 0; i++) {
		printf(" plot[%d]", i);
		pfix("", &plyr.p_plot[i]);
	    }
	    skipflg = 0;
	}
	if (skipflg == 0 && *getchrs("Shares? ") == 'y') {
	    for (i = 1; i <= LIM_CHAINS && skipflg == 0; i++) {
		printf(" %c", 'A' - 1 + i);
		bfix(" shares", &plyr.p_shares[i]);
	    }
	}
	putplyr(pnum);
}

fixchain(argp)
char	**argp;
{
	register char *cname;
	register int i, cnum;
	int hn;

	cname = getarg(argp[1], "chain? (name or #number) ");
	if (*cname == '#') {
	    cnum = atoi(++cname);
	    if (getchain(cnum, SAFE) == -1) {
		printf("I have no records of chain #%d\n", cnum);
		return(-1);
	    }
	} else {
	    for (cnum = 1; cnum < LIM_CHAINS; cnum++) {
		if (getchain(cnum, SAFE) == -1)
		    break;
		if (equal(cname, chain.c_name))
		    break;
	    }
	    if (cnum >= LIM_CHAINS) {
		printf("Can't find chain `%s'\n", cname);
		return(-1);
	    }
	}
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
	sfix("c_name", &chain.c_name, CNAME_LENGTH);
	ifix("c_size", &chain.c_size);
	ifix("c_index", &chain.c_index);
	ifix("c_shares", &chain.c_shares);
	ifix("c_volume", &chain.c_volume);
	ifix("c_change", &chain.c_change);
	putchain(cnum);
	printf("Rewritten\n");
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

lfix(pp, lp)
char    *pp;
long    *lp;
{
	register char *cp;
	extern long atol();

	if (skipflg)
	    return;
	printf("%s? <%D> ", pp, *lp);
	cp = getchrs("");
	if (cp == NULL || *cp != '\0')
	    if (cp == NULL || (*cp == 'x' && cp[1] == '\0'))
		skipflg = 1;
	    else
		*lp = atol(cp);
}

ifix(pp, ip)
char    *pp;
int     *ip;
{
	register char *cp;

	if (skipflg)
	    return;
	printf("%s? <%d> ", pp, *ip);
	cp = getchrs("");
	if (cp == NULL || *cp != '\0')
	    if (cp == NULL || (*cp == 'x' && cp[1] == '\0'))
		skipflg = 1;
	    else
		*ip = atoi(cp);
}

bfix(pp, bp)
char    *pp, *bp;
{
	register char *cp;

	if (skipflg)
	    return;
	printf("%s? <%d> ", pp, *bp);
	cp = getchrs("");
	if (cp == NULL || *cp != '\0')
	    if (cp == NULL || (*cp == 'x' && cp[1] == '\0'))
		skipflg = 1;
	    else
		*bp = atoi(cp);
}

sfix(pp, sp, len)
char    *pp, *sp;
{
	register char *cp;

	if (skipflg)
	    return;
	printf("%s? <%s> ", pp, sp);
	cp = getchrs("");
	if (cp == NULL || *cp != '\0') {
	    if (cp == NULL || (*cp == 'x' && cp[1] == '\0'))
		skipflg = 1;
	    else {
		cp[len] = '\0';
		while (*sp++ = *cp++);
	    }
	}
}

tfix(pp, tp)
char    *pp;
struct  times *tp;
{
	char *cp;

	if (skipflg)
	    return;
	printf("%s? <%d:%d%d> ", pp, tp->t_hr, tp->t_min / 10, tp->t_min % 10);
	cp = getchrs("");
	if (cp == NULL || *cp != '\0') {
	    if (cp == NULL || (*cp == 'x' && cp[1] == '\0'))
		skipflg = 1;
	    else {
		tp->t_hr = atoip(&cp);
		if (*cp++ != ':') {
		    printf("  Format is #:#\n");
		    return(tfix(pp, tp));
		}
		tp->t_min = atoi(cp);
	    }
	}
}

pfix(pp, plp)
char    *pp;
struct  plot *plp;
{
	char *cp;

	if (skipflg)
	    return;
	printf("%s? <%c%d> ", pp, 'a' + plp->p_x, plp->p_y);
	cp = getchrs("");
	if (cp == NULL || *cp != '\0') {
	    if (cp == NULL || (*cp == 'x' && cp[1] == '\0'))
		skipflg = 1;
	    else {
		if (*cp < 'a' || *cp > 'z'
		 || cp[1] < '0' || cp[1] > '9') {
		    printf("   Format is letter-number, e.g. b6\n");
		    return(pfix(pp, plp));
		}
		plp->p_x = *cp++ - 'a';
		plp->p_y = atoi(cp);
	    }
	}
}
