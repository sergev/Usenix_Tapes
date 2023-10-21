#
/*
**      OXO -- Game of "Othello" aka "Reversi"
**          (c) psl 8/78
**      Compile: cc -O -n -q oxo.c -lS -o oxo
*/

#include    <stdio.h>

char    *whatsccs "@(#)oxo.c	2.4  last mod 7/4/79 -- (c) psl 1978";
#define INIT_LEVEL   5

#define MAX_BOARD_SIZE  18
#define DEF_BOARD_SIZE  10
#define MAX_NUM_CELLS   (MAX_BOARD_SIZE * MAX_BOARD_SIZE)
#define DEF_NUM_CELLS   DEF_BOARD_SIZE * DEF_BOARD_SIZE
#define MAX_MOVES       (MAX_BOARD_SIZE - 2) * (MAX_BOARD_SIZE - 2)
	/* special move codes (must be in the range -1:-112) */
#define PASS            -99
#define RE_INIT         -100
#define RE_MOVE         -101
#define BAD_MOVE        -102
#define RESIGN          -103

#define EDGE    3
#define EMPTY   2
#define BLACK   0
#define WHITE   1
#define TIE     -1

#define TIMEFILL    1
#define BS          010

/* cursor defines */
#define CLEAR      -1                             /* code for screen clear */
#define OPEN_LINE  -2                              /* code for insert line */
#define CLOSE_LINE -3                              /* code for delete line */
#define CLEAR_LINE -4               /* code to clear rest of line and <CR> */

struct  timestr {
    long    myusr, mysys;  /* N O T E : on many systems this line is "int" */
    long    kidusr, kidsys;
};

int     board_size  DEF_BOARD_SIZE;                     /* must be even */
int     num_cells   DEF_NUM_CELLS;
int     first_cell  DEF_BOARD_SIZE + 1;
int     last_cell   DEF_NUM_CELLS - DEF_BOARD_SIZE;

#define CF1         25              /* Outer corner fudge */
#define CF2         -1              /* Near outer corner fudge */
#define CF3         -9              /* Inner corner fudge */
#define CF4         0               /* Near inner corner fudge */
#define EF1         5               /* Outer edge fudge */
#define EF2         -1              /* Inner edge fudge */

char    fvals[MAX_NUM_CELLS] {
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0, CF1, CF2, EF1, EF1, EF1, EF1, CF2, CF1,   0,
	0, CF2, CF3, CF4, EF2, EF2, CF4, CF3, CF2,   0,
	0, EF1, CF4,   1,   1,   1,   1, CF4, EF1,   0,
	0, EF1, EF2,   1,   1,   1,   1, EF2, EF1,   0,
	0, EF1, EF2,   1,   1,   1,   1, EF2, EF1,   0,
	0, EF1, CF4,   1,   1,   1,   1, CF4, EF1,   0,
	0, CF2, CF3, CF4, EF2, EF2, CF4, CF3, CF2,   0,
	0, CF1, CF2, EF1, EF1, EF1, EF1, CF2, CF1,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
};

int     dir[] {
	-DEF_BOARD_SIZE,    -DEF_BOARD_SIZE + 1,
	1,                  DEF_BOARD_SIZE + 1,
	DEF_BOARD_SIZE,     DEF_BOARD_SIZE - 1,
	-1,                 -DEF_BOARD_SIZE - 1,
};
int     tdels[] {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 5, 7, 10, 20, 40, 0, 0,
};
char    timedel[128];
char    tty_type 'i', bdinit 0, msgflg 0;
char    blanks[] "                    ", *blnkep &blanks[20];
char    dispbd[MAX_NUM_CELLS];                   /* used by bdump for CRTs */
char    trace 0, quiet 0, secrep 0, hcolor BLACK, ccolor WHITE;
int     eval_level, report_level -1;
FILE    *infp, *outfp;

char    *mtoa();
int     *legalist();

main(argc, argv)
char *argv[];
{
	register char *cp;
        register int *llp;
	char bd[MAX_NUM_CELLS], buf[64];
	int i, j, next, best, moves[MAX_MOVES], movenum, alpha, beta;
	int spm, tpm_lo, tpm_hi, tavg;
	long int etime;
	struct timestr tstr;

	printf("Welcome to OXO (Othello / Reversi)\n");
	spm = 7;
	infp = stdin;
	outfp = 0;
	eval_level = 0;
	clearboard(bd);
	movenum = 0;
        while (--argc) {
	    switch (argv[argc][0]) {
	    case 'B':                           /* play "black" (go first) */
		hcolor = WHITE;
		ccolor = BLACK;
		break;
	    case 'b':                           /* non-standard board size */
		board_size = atoi(&argv[argc][1]) + 2;
		if (board_size & 1) {
		    printf("Board size must be even.\n");
		    exit(2);
		}
		setsize(board_size);
		break;
	    case 'd':                                             /* debug */
		trace = atoi(&argv[argc][1]);
		tty_type = '\0';
		break;
	    case 'I':                              /* initialize board pos */
		i = atom(&argv[argc][2]);
		if (bd[i] == EMPTY)
		    movenum++;
		bd[i] = argv[argc][1] == 'B'? BLACK : WHITE;
		break;
	    case 'i':                              /* take input from file */
		if ((infp = fopen(&argv[argc][1], "r")) == NULL) {
		    printf("Can't open `%s' for reading\n", &argv[argc][1]);
		    perror("");
		    exit(2);
		}
		break;
	    case 'l':                                    /* set eval level */
		eval_level = atoi(&argv[argc][1]);
		break;
	    case 'o':                                /* send moves to file */
		if ((outfp = fopen(&argv[argc][1], "a")) == NULL) {
		    printf("Can't open `%s' for append\n", &argv[argc][1]);
		    perror("");
		    exit(2);
		}
		break;
	    case 'q':                                /* quiet, (no boards) */
		quiet++;
		break;
	    case 'r':                                  /* set report level */
		report_level = atoi(&argv[argc][1]);
		tty_type = '\0';
		break;
	    case 's':                                   /* set secs / move */
		spm = atoi(&argv[argc][1]);
		secrep++;
		break;
	    case 'T':                        /* TTY (no cursor addressing) */
		tty_type = '\0';
		break;
            }
        }
	if (infp == stdin)
	    instruct();
	tpm_lo = spm * 60 * .85;
	tpm_hi = spm * 60 * 1.15;
	if (eval_level == 0) {
	    if (spm < 10)
		eval_level = 5;
	    else if (spm < 20)
		eval_level = 6;
	    else if (spm < 35)
		eval_level = 7;
	    else if (spm < 55)
		eval_level = 8;
	    else
		eval_level = 9;
	}
	if (secrep)
	    printf("min ticks is %d, max ticks is %d, initial level is %d\n",
	     tpm_lo, tpm_hi, eval_level);
	gtty(1, buf);
	cp = timedel;
	for (i = tdels[buf[0]]; --i >= 0; )
	    *cp++ = TIMEFILL;
	*cp++ = BS;
	*cp = '\0';
	etime = 0;

	if ((ccolor == BLACK && (movenum & 1) == 0)
	 || (ccolor == WHITE && (movenum & 1) == 1)) {
	    --eval_level;
	    goto cmove;
	}

	for (;;) {
	    if (!quiet)
		bdump(bd);
	    if (tty_type) {
		printf("%s", cursor(2, board_size +1)) ;
		printf("%s", cursor(OPEN_LINE, 0));
	    }
	    llp = legalist(hcolor, bd);        /* make list of legal moves */
	    if (*llp == 0) {
		if(*legalist(ccolor, bd) == 0)
		    game_end(bd);
		*llp = 0;
	    }
	    msgflg = 0;                                /* set by message() */
	    for (;;) {
		if (tty_type)
		    printf("%s", cursor(2, board_size + 1));
		printf("%2d: Your move? ", movenum);
		i = getmove(infp);
		if (i == BAD_MOVE)
		    message("Move format is letter-number. e.g. `a1'", "\n");
		else if (i == RE_INIT) {
		    bdinit = 0;
		    bdump(bd);
		} else if (i == RE_MOVE) {
		    if (movenum < 2) {
			message("You have no move to retract.","\n");
			continue;
		    }
		    movenum -= 2;
		    clearboard(bd);
		    for (i = 0; i < movenum; i++)
			if (moves[i] != PASS)
			    playmake(moves[i], i & 1, bd);
		   if (!quiet)
			bdump(bd);
		    if (tty_type) {
			printf("%s", cursor(2, board_size +1)) ;
			printf("%s", cursor(CLOSE_LINE, 0));
		    }
		} else if (i == PASS) {
		    if (*llp == 0)
			break;
		    message(mtoa(*llp), " would be a legal move so you can't pass.\n");
		} else if (i == RESIGN) {
		    if (infp != stdin)
			exit(0);
		    message("Do you really want to resign?", " ");
		    fgets(buf, 32, infp);
		    if (buf[0] == 'y')
			exit(0);
		} else if (bd[i] != EMPTY)
		    message(mtoa(i), " is illegal: not an empty spot.\n");
		else if (legal(i, hcolor, bd) == 0)
		    message(mtoa(i), " is illegal: no flip occurs.\n");
		else
		    break;                                   /* legal move */
	    }
	    if (msgflg && tty_type)
		printf("%s%s", cursor(0, board_size + 2), cursor(CLOSE_LINE, 0));
	    moves[movenum++] = i;
	    if (i != PASS) {
		playmake(i, hcolor, bd);
		if (!quiet)
		    bdump(bd);
	    }
    cmove:
	    best = 999;
	    alpha = -999;
	    beta = 999;
	    llp = legalist(ccolor, bd);            /* legal computer moves */
	    if (movenum == 0)                      /* computer moves first */
		llp[1] = 0;
	    if (*llp == 0) {
		if (tty_type)
		    printf("%s", cursor(20, board_size + 1));
		printf("I have to pass\n");
		moves[movenum++] = PASS;
		if (outfp != 0) {
		    fputs("pass\n", outfp);
		    fflush(outfp);
		}
		continue;
	    }
	    if (llp[1] == 0)                    /* if only 1 possible move */
		next = *llp;
	    else {
		best = -32767;
		for (next = -1; *llp; llp++) {
		    if (tty_type)
			printf("%s",
			 cursor((*llp % board_size) * 2, *llp / board_size));
		    j = cplay(bd, 1, *llp, alpha, beta);
		    if (report_level >= 0)
			printf("Move at %s scores %d\n", mtoa(*llp), j);
		    if (j > best) {
			best = j;
			next = *llp;
			if (j > alpha)
			    alpha = j;
		    }
		}
	    }
	    if (tty_type)
		printf("%s", cursor(22, board_size + 1));
	    printf("%d: I'll play at %s (%d)", movenum, mtoa(next), best);
	    if (outfp != 0) {
		fprintf(outfp, "%s\n", mtoa(next));
		fflush(outfp);
	    }
	    playmake(next, ccolor, bd);
	    moves[movenum++] = next;
	    j = eval_level;
	    i = etime;
	    times(&tstr);
	    etime = tstr.mysys + tstr.myusr;
	    i = etime - i;
	    if (i > 10 * 60)
		putc(07, stdout);
	    tavg = etime / ((movenum + 1) >> 1);
	    if ((tavg > tpm_hi && i > tpm_lo)
	     || (i > tpm_hi && tavg > tpm_lo)
	     && eval_level > 3)
		eval_level -= 1;
	    if ((tavg < tpm_lo && i < tpm_hi)
	     || (i < tpm_lo && tavg < tpm_hi)
	     && eval_level < 9)
		eval_level += 1;
	    if (secrep)
		printf(" et=%D act=%d avg=%d eval %d/%d\n",
		 etime, i, tavg, j, eval_level);
	}
}

clearboard(bd)
char    bd[];
{
	register char *bp;
	register int i;

	for (bp = &bd[num_cells]; --bp >= bd; )
	    *bp = EMPTY;
	for (i = 0; i < board_size; i++) {
	    bd[i*board_size] = bd[i*board_size+board_size-1] = EDGE;
	    bd[(board_size - 1) * board_size + i] = bd[i] = EDGE;
        }
        i = board_size;
	bp = &bd[num_cells / 2 - i / 2];
	bp[0] = bp[i - 1] = BLACK;
	bp[-1] = bp[i] = WHITE;
}

setsize()
{
	register int i;

	num_cells = board_size * board_size;
	first_cell = board_size + 1;
	last_cell = num_cells - board_size - 1;
	/* set up directions */
	dir[0] = -board_size;
	dir[1] = -board_size + 1;
	dir[2] = 1;
	dir[3] = board_size + 1;
	dir[4] = board_size;
	dir[5] = board_size - 1;
	dir[6] = -1;
	dir[7] = -board_size - 1;
	/* set up fudge-values && no-fudge-values */
	for (i = first_cell; i < last_cell; i++)
	    fvals[i] = 1;
	fudgit(fvals, first_cell, 2, 4);
	fudgit(fvals, 2 * (board_size - 1), 4, 6);
	fudgit(fvals, last_cell - 1, 6, 0);
	fudgit(fvals, (board_size - 1) * (board_size - 1), 0, 2);
}

fudgit(vp, corn, dir1, dir2)    /* put in fudge factors for a given corner */
char    *vp;
{
	register int i, j;
	int d1, d2;

	i = corn;
	d1 = dir[dir1];
	d2 = dir[dir2];
	vp[i] = CF1;
	vp[i + d1] = vp[i + d2] = CF2;
	vp[i + d1 + d2] = CF3;
	vp[i + d1 + d1 + d2] = vp[i + d2 + d2 - d1] = CF4;
	for (j = (board_size - 2) / 2; --j > 1; )
	    vp[i + j * d1] = vp[i + j * d2] = EF1;
	for (j = (board_size - 2) / 2; --j > 2; )
	    vp[i + j * d1 + d2] = vp[i + j * d2 - d1] = EF2;
}

cplay(bd, level, where, alpha, beta)                      /* computer play */
char    bd[];
{
	register int i, this;
	char brd[MAX_NUM_CELLS];
	int n, retval;

        if (trace > 0)
	    printf("%scplay(%d, %s, %d, %d)\n",
	     blnkep - level, level, mtoa(where), alpha, beta);
        copybd(bd, brd);
        if (where != PASS)
	    playmake(where, ccolor, brd);
        if (level >= eval_level)
	    return(feval(brd));
	/* try human responses */
        n = 0;
	retval = 999;
        for (this = first_cell; this < last_cell; this++) {
	    if (brd[this] == EMPTY && legal(this, hcolor, brd)) {
		n++;
		i = hplay(brd, level + 1, this, alpha, beta);
		if (level <= report_level)
		    printf("%sHuman @ %s, eval = %d\n",
		     blnkep - level, mtoa(this), i);
		if (i <= alpha) {
		    if (trace)
			printf("Cutoff, %d <= alpha (%d)\n", i, alpha);
		    return(i);
		}
		if (i < retval) {
		    retval = i;
		    if (i < beta)
			beta = i;
		}
	    }
        }
        if (n == 0) {
	    if (where != PASS)
		return(hplay(brd, level + 1, PASS, alpha, beta));
	    return(10 * nfeval(brd));
        }
        return(retval);
}

hplay(bd, level, where, alpha, beta)                         /* human play */
char    bd[];
{
	register int i, this;
	char brd[MAX_NUM_CELLS];
	int n, retval;

        if (trace > 0)
	    printf("%shplay(%d, %s, %d, %d)\n",
	     blnkep - level, level, mtoa(where), alpha, beta);
        copybd(bd, brd);
        if (where != PASS)
	    playmake(where, hcolor, brd);
        if (level >= eval_level)
	    return(feval(brd));
        n = 0;
	retval = -999;
        for (this = first_cell; this < last_cell; this++) {
	    if (brd[this] == EMPTY && legal(this, ccolor, brd)) {
		n++;
		i = cplay(brd, level + 1, this, alpha, beta);
		if (level <= report_level)
		    printf("%sComputer @ %s, eval = %d\n",
		     blnkep - level, mtoa(this), i);
		if (i >= beta) {
		    if (trace)
			printf("Cutoff, %d >= beta (%d)\n", i, beta);
		    return(i);
		}
		if (i > retval) {
		    retval = i;
		    if (i > alpha)
			alpha = i;
		}
	    }
        }
        if (n == 0) {
	    if (where != PASS)
		return(cplay(brd, level + 1, PASS, alpha, beta));
	    return(10 * nfeval(brd));
        }
        return(retval);
}

playmake(where, who, bd)
char    bd[];
{
        int who2, *dp, w;

        bd[where] = who;
	who2 = BLACK + WHITE - who;
        for (dp = & dir[8]; --dp >= dir; ) {
	    for (w = where; w += *dp; )
		if (bd[w] != who2)
		  break;
	    if (bd[w] == who)
		while ((w -= *dp) != where)
		    bd[w] = who;
        }
	if (trace > 1 && !quiet)
            bdump(bd);
}

legal(where, who, bd)
char    bd[];
{
        register int who2, *dp, w;

	who2 = BLACK + WHITE - who;
	for (dp = &dir[8]; --dp >= dir; ) {
	    for (w = where + *dp; bd[w] == who2; w += *dp);
	    if (bd[w] == who && w != where + *dp)
		return(1);
        }
        return(0);
}

copybd(a, b)
char    a[], b[];
{
        register int *ap, *bp;

	ap = &a[num_cells];
	bp = &b[num_cells];
        while (ap > a)
            *--bp = *--ap;
}

feval(bd)   /* evaluate board (with fudging) from computer's point of view */
char    bd[];
{
	register char *bp, *vp;
	register int score;
	int ccnt, hcnt;

	score = ccnt = hcnt = 0;
	vp = &fvals[last_cell];
	for (bp = &bd[last_cell]; --vp && --bp >= &bd[first_cell]; )
	    if (*bp == hcolor) {
		score -= *vp;
		hcnt++;
	    } else if (*bp == ccolor) {
		score += *vp;
		ccnt++;
	    }
	if (hcnt == 0 || ccnt == 0)
	    score =* 10;
        return(score);
}

nfeval(bd)    /* evaluate board (no fudging) from computer's point of view */
char    bd[];
{
	register char *bp;
	register int score;

	score = 0;
	for (bp = &bd[last_cell]; --bp >= &bd[first_cell]; )
	    if (*bp == hcolor)
		--score;
	    else if (*bp == ccolor)
		score++;
        return(score);
}

bdump(bd)
char    bd[];
{
        register int i, j, k;

        if (tty_type) {
	    if (bdinit)
		goto xymod;
	    printf("%s", cursor(CLEAR, 0));
        }
	printf("  ");
        for (j = 1; j < board_size - 1; j++)
	    printf("%c ", 'a' - 1 + j);
        for (i = 1; i < board_size - 1; i++) {
	    printf("\n%-2d", i - 1);
            for (j = 1; j < board_size - 1; j++) {
		k = i * board_size + j;
		printf("%c ", "XO+ "[bd[k]]);
		dispbd[k] = bd[k];
	    }
	    printf(" %d", i - 1);
        }
	printf("\n  ");
        for (j = 1; j < board_size - 1; j++)
	    printf("%c ", 'a' - 1 + j);
        printf("\n");
        bdinit = 1;
        return;
xymod:
        for (i = 1; i < board_size - 1; i++) {
            for (j = 1; j < board_size - 1; j++) {
		k = i * board_size + j;
		if (dispbd[k] != bd[k]) {
		    printf("%s", cursor(j + j, i));
		    if (dispbd[k] != EMPTY && bd[k] != EMPTY)
			printf("-%s\\%s|%s/%s-%s",
			 timedel, timedel, timedel, timedel, timedel);
		    printf("%c", "XO+ "[bd[k]]);
		    dispbd[k] = bd[k];
		}
	    }
        }
}

getmove(infp)
FILE    *infp;
{
        char buf[32];

	while (fgets(buf, 32, infp) == NULL)
	    sleep(10);
	if (infp != stdin)
	    printf("%s", buf);
	if (buf[0] == '!') {
	    signal(2, 1);
	    if (fork() == 0) {
		execl("/bin/sh", "-", 0);
		perror("/bin/sh");
		exit(3);
	    }
	    wait0();
	    signal(2, 0);
	    buf[0] = '?';
	}
	if (buf[0] == 'p' && buf[1] == 'a')
	    return(PASS);
	if (buf[0] == 'r' && buf[1] == 'e')
	    return(RESIGN);
	if (buf[0] == '?')
	    return(RE_INIT);
	if (buf[0] == '^')
	    return(RE_MOVE);
	return(atom(buf));
}

atom(buf)                         /* ascii to move (assumes letter-number) */
char    *buf;
{
	register int i;

	i = atoi(&buf[1]);
	if (buf[0] >= 'a' && buf[0] < 'a' - 2 + board_size
	 && i >= 0 && i < board_size - 2)
	    return((i + 1) * board_size + buf[0] - 'a' + 1);
	return(BAD_MOVE);
}

int     *
legalist(plyr, bd)
char bd[];
{
        register int i, *lp;
	static int list[MAX_NUM_CELLS];

        lp = list;
	for (i = last_cell; --i >= first_cell; )
	    if (bd[i] == EMPTY && legal(i, plyr, bd))
		*lp++ = i;
        *lp = 0;
        return(list);
}

char *
mtoa(n)
{
	register char *bp;
	register int i;
        static char buf[4];

	if (n == PASS)
	    return("pass");
	bp = buf;
	*bp++ = 'a' - 1 + (n % board_size);
	i = n / board_size - 1;
	if (i > 9) {
	    *bp++ = '0' + i / 10;
	    i =% 10;
	}
	*bp++ = '0' + i;
	*bp++ = '\0';
        return(buf);
}

game_end(bd)
char bd[];
{
        register int i;

	i = nfeval(bd);
	if (i < 0)
	    printf("Nice going; you win by %d\n", -i);
	else if (i > 0)
	    printf("I won by %d ... natch\n", i);
        else
	    printf("It's a tie!  Nice playing!\n");
        exit(0);
}

instruct()
{
        char buf[64];

	printf("Do you want instructions? ");
	fgets(buf, 64, infp);
        if (*buf == 'n')
	    return;
        printf("\n");
	printf("OXO is played on a square board, with little stones that are  white on  one\n");
        printf("side  and black on the other.  You are the white player, (your stones are indi-\n");
	printf("cated by 'X' on the board), and you play first.   My stones  are  indicated  by\n");
	printf("'O'  on the board.  The object of the game is to have your color predominate at\n");
        printf("the end.  Since play alternates you might think that there would  always  be  a\n");
        printf("tie,  BUT  the stones can change color by being 'flipped'.  This happens when a\n");
        printf("row of stones is bounded on both ends by enemy stones.\n");
        printf("                    a b c d e f g h\n");
        printf("If this is        0 + + + + + + + +\n");
	printf("the board         1 + + X + X + + +\n");
	printf("situation         2 + X O O O O + +\n");
	printf("                  3 + + + O + + + +\n");
        printf("\n");
	printf("                    a b c d e f g h      However, if I now play at d1\n");
	printf("and you play at   0 + + + + + + + +      (thereby flipping d2 back to O)\n");
	printf("g2 the board      1 + + X + X + + +      you'd could play c0 or e0 and\n");
	printf("becomes this      2 + X X X X X X +      flip d1; or c3, or e3, etc.\n");
	printf("                  3 + + + O + + + +\n");
	printf("If you can't make a move that will flip something you have to pass (type \"pass\"\n");
	printf("as  your  move).  If neither one of us can make a move the game has ended.\n");
	printf("                     Hit (RETURN) to continue: ");
	fgets(buf, 64, infp);
	printf("To redraw the board type \"?\".\n");
	printf("To generate a subshell type \"!\".\n");
	printf("To retract your previous move type \"^\".\n");
	printf("To resign type \"resign\".\n");
	printf("To have me move first run: oxo Black\n");
	printf("To play on a 10x10 board run: oxo b10\n");
	printf("                     Hit (RETURN) to continue: ");
	fgets(buf, 64, infp);
}


message(msg1, msg2)
char *msg1, *msg2;
{
        if (tty_type) {
	    printf("%s%s\n",
	     cursor(2, board_size + 1), cursor(CLEAR_LINE, 0));
	    if (msgflg == 0)
		printf("%s", cursor(OPEN_LINE, 0));
        }
	printf("%s%s", msg1, msg2);
        msgflg++;
}

/*
**      CURSOR -- X Y positioning routine for Interactive OWL terminals
**                (also misc other operations...)
**      (c) psl 8/78
*/

#define FF      014
#define CURADDR 017
#define LOPEN   020
#define LCLOSE  021

#define MULTC   026

#define LCLEAR  043

cursor(x, y)
int    x, y;
{
     register char *cp;
     static char cursbuf[4][4], next;

     next = (next + 1) & 3;
     cp = &cursbuf[next][4];
     *--cp = '\0';
     if (x < 0) {
	  if (x == CLEAR)
	       *--cp = FF;
	  else if (x == OPEN_LINE)
	       *--cp = LOPEN;
	  else if (x == CLOSE_LINE)
	       *--cp = LCLOSE;
	  else if (x == CLEAR_LINE) {
	       *--cp = LCLEAR;
	       *--cp = MULTC;
	  } else
	       *--cp = '?';
     } else {
	  *--cp = x + 040;
	  *--cp = y + 040;
	  *--cp = CURADDR;
     }
     return(cp);
}
