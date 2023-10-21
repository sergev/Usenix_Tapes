#
/*
**      GOMOKU -- learning program
** Compile: cc -O -n -q gomoku.c -lP -lS -o gomoku; chmod 755 gomoku
** Copyright (c) 1976 by Peter S. Langston - Camb., Ma. 02140
*/

#include    <stdio.h>
#include    <cursor.h>

#define infofil "/sys/games/.../gom_txt"

char    *whatsccs "@(#)gomoku.c	1.2  last mod 8/26/79 -- (c) psl 1976";
char    memory[11][11];
char    in[80], crtflg 1;
char    dd[8];
int     hbd[441], cbd[441], hist[200][2];
int     bsize, bsiz1, bsiz2;
int     bestmov, bestval, bestnum;
int     movenum, movesleft, max1, max2, memf;

char    *gexclams[] {
        "Whee! ", "Oh boy! ", "Aha! ", "Heh, heh ", "Well... ", "","",""
};
char    *bexclams[] {
        "Oh NO! ", "Dum-ta-dum-dum ", "Uh oh...", "Ooops! ", "Eeeeek! ",
        "Well...","","","",""
};
char    code[] { 0,        /* 0:0 */
        -1,-1,-1,-1,
        1,2,3,4,        /* 1:1 1:2 1:3 1:4 */
        5,6,7,-1,       /* 2:1 2:2 2:3 */
        8,9,-1,-1,      /* 3:1 3:2 */
        10,-1,-1,-1,    /* 4:1 */
        };

main(argc, argv)
char    *argv[];
{
        register int i, j, k;
        int x, y, z, *bd, *obd;

	for (i = 0; i < 11; i++)
                memory[i][10] = memory[10][i] = 1;
        if ((memf = open("gomoku.mem",2)) > 0) {
                read(memf,memory,121);
                printf("*** ");
	} else
		memf = creat("gomoku.mem", 0644);
	printf("Welcome to Go-moku!\n");
        getmodes(argc, argv);
	time(hbd);
	srand(hbd[0]);
        info();
sizeask:
        printf("What size board would you like to play on? (19 max) ");
	if (fgets(in, 80, stdin) == NULL)
		exit(1);
        bsize = atoi(in);
        if (bsize > 19 || bsize < 1)
                goto sizeask;
        bsiz1 = bsize + 1;
        bsiz2 = bsize + 2;
playagain:
        movesleft = bsize * bsize;
	movenum = 0;
        dd[0] = 1;
        dd[1] = bsiz2 + 1;
        dd[2] = bsiz2;
        dd[3] = bsiz2 - 1;
        dd[4] = -1;
        dd[5] = -bsiz2 - 1;
        dd[6] = -bsiz2;
        dd[7] = -bsiz2 + 1;
        for (i = 0; i < bsiz2; i++) {
                hbd[i] = cbd[i] = hbd[i + bsiz1 * bsiz2]
                 = cbd[i + bsiz1 * bsiz2] = -99;
                hbd[bsiz2 * i] = cbd[bsiz2 * i] = -99;
                hbd[bsiz2 * i + bsiz1] = cbd[bsiz2 * i + bsiz1] = -99;
        }
        for (i = 1; i < bsiz1; i++)
                for (j=1; j<bsiz1; j++) {
                        k = i * bsiz2 + j;
                        hbd[k] = cbd[k] = 0;
                }
        board();
loopy:
        printf("\nYour move? ");
        if (crtflg)
		printf("    ");
	if (fgets(in, 20, stdin) == NULL) {
oops:           info();
                goto loopy;
        }
        if (in[0] == '?') {
                board();
                goto loopy;
        }
        y = in[2] < '0'?in[1] - '0' : (in[1] - '0') * 10 + in[2] - '0';
	x = in[0] - 'a' + 1;
	if (x < 1 || x > bsize || y < 0 || y > bsize)
                goto oops;
        x = x * bsiz2 + y;
        if (hbd[x] == -1 || hbd[x] == -2)
                goto oops;
        z = hbd[x];
        hbd[x] = -2;
        cbd[x] = -1;
        if (crtflg)
                moveat(x);
        hist[movenum][0] = x;
        hist[movenum++][1] = z;
        if (moves2win(z) == 1) {
                lose();
                goto again;
        }
        if (--movesleft == 0)
                goto draw;
        update(x, hbd, cbd);
        bestval = 99;
        for (i = bsiz2; i < bsiz1 * bsiz2; i++) {
                if (cbd[i] == -99)
                        continue;
                if ((x = moves2win(cbd[i])) && x < bestval) {
                        printf(gexclams[x-1]);
                        bestval = x;
                        bestmov = i;
                }
        }
        for (i = bsiz2; i < bsiz1 * bsiz2; i++) {
                if (hbd[i] == -99)
                        continue;
                if ((x = moves2win(hbd[i])) && x < bestval) {
                        printf(bexclams[x-1]);
                        bestval = x;
                        bestmov = i;
		}
        }
        if (bestval == 99)
                bestmov = miscmove();
        x = bestmov;
        z = cbd[x];
        hbd[x] = -1;
        cbd[x] = -2;
	printf("I'll move %c%d    \n", x / bsiz2 + 'a' - 1, x % bsiz2);
	if (crtflg) {
		printf("%s", cursor(CLEAR_LINE, 0));
                moveat(x);
	}
	if (bestval == 1 && moves2win(z) == 1)
		goto again;
        update(x, cbd, hbd);
        if (--movesleft > 0)
                goto loopy;
draw:
        printf("Gosh!  It's a draw!  I'm doing lots better, huh?\n");
again:
        printf("Let's play again, ok? ");
	*in = 'n';
	fgets(in, 80, stdin);
        if (*in != 'n') {
		printf("\n\nOkay\n");
		goto playagain;
        }
        printf("Well, it was fun...\n");
        unlink("gomok.log");
}

miscmove()
{
        register int i, misc;
        int x, y, n;

        x = y = n = 0;
        for (i = bsiz2; i < bsiz1 * bsiz2; i++) {
                if (cbd[i] == 0 || cbd[i] == -99)
                        continue;
                x =+ i / bsiz2;
                y =+ i % bsiz2;
                n++;
        }
        misc = (x / n) * bsiz2 + y / n;
        while (hbd[misc] < 0) {
                do {
                        i = dd[rand() >> 4 & 7];
                } while (hbd[misc + i] == -99);
                misc =+ i;
        }
        return(misc);
}

pattern(x, d, b)
int b[];
{
        register int i, j, hi;
        int lo, nst, bnst, n5p, x0;

        d = dd[d % 4];
        bnst = n5p = 0;
        for (hi = 1; hi < 5; hi++) {
                x0 = x + hi * d;
                if (b[x0] == -99 || b[x0] == -1)
                        break;
        }
        --hi;
        for (lo = 1; lo < 5; lo++) {
                x0 = x - lo * d;
                if (b[x0] == -99 || b[x0] == -1)
                        break;
        }
        --lo;
        if (hi + lo < 4)
                return(0);
        for (i = -lo; i < hi-3; i++) {
                x0 = x + i * d;
                if (b[x0 - d] == -2 || b[x0 + 5 * d] == -2)
                        continue;
                nst = 0;
                for (j = 0; j < 5; j++) {
                        if (b[x0] == -2)
                                nst++;
                        x0 =+ d;
                }
                if (!nst || nst < bnst)
                        continue;
                if (nst != bnst) {
                        bnst = nst;
                        n5p = 0;
                }
                n5p++;
        }
        return(code[bnst * 4 + n5p]);
}

update(x, a, b)
int a[], b[];
{
        register int i, j, d;
        int ddd, shift, mask, x0, q;

        for (ddd = 0; ddd < 4; ddd++) {
                d = dd[ddd];
                shift = ddd << 2;
                mask = ~(017 << shift);
                /* effect on opponent */
                for (i = -1; i > -5; --i) {
                        j = b[x + i * d];
                        if (j == -99 || j == -1)
                                break;
                }
                while (i++ < 4) {
                        j = b[x0 = x + i * d];
                        if (j == -99 || (j == -1 && i))
                                break;
                        if (-2 <= j && j <= 0)
                                continue;
                        q = pattern(x0, ddd, b) << shift;
                        j =& mask;
                        b[x0] = j | q;
                }
                /* effect on player */
                for (i = -1; i > -6; --i) {
                        j = a[x + i * d];
                        if (j == -99 || j == -1)
                                break;
                }
                while (i++ < 5) {
                        j = a[x0 = x + i * d];
                        if (j == -99 || j == -1)
                                break;
                        if (j == -2)
                                continue;
                        q = pattern(x0, ddd, a) << shift;
                        j =& mask;
                        a[x0] = j | q;
                }
        }
}

board()
{
        register int i, j;

	printf("%s  ", cursor(CLEAR, 0));
        for (j = 0; j < bsize; j++)
                printf(" %c", 'a' + j);
        printf("\n");
        for (i = 1; i < bsiz1; i++) {
                printf("%2d", i);
                for (j = 1; j < bsiz1; j++) {
                        switch (hbd[j * bsiz2 + i]) {
                        case -1: printf(" *");
                                break;
                        case -2: printf(" O");
                                break;
                        default: printf(" .");
                        }
                }
                printf(" %d\n", i);
        }
        printf("  ");
        for (j = 0; j < bsize; j++)
                printf(" %c", 'a'+j);
        printf("\n");
}

moveat(z)
{
        register int x, y;

        x = z / bsiz2;
        y = z % bsiz2;
        printf("%s%c", cursor(x + x + 1, y), hbd[z] == -1? '*' : 'O');
        printf("%s", cursor(1, bsiz2));
}

moves2win(v)
{
        register int d, q;

        if (!v || (v < 0 && v > -3))
                return(0);
        max1 = max2 = 0;
        for (d = 0; d < 4; d++) {
                if ((q = (v >> (d << 2)) & 017) <= max2)
                        continue;
                if (q > max1) {
                        max2 = max1;
                        max1 = q;
                } else
                        max2 = q;
        }
        return (memory[max1][max2]);
}

lose()
{
        register int i, j, x;

        printf("Oh boy!  You won!                       \n");
        for (i = movenum - 1; i >= 0; --i) {
                x = hist[i][0];
                if ((j = moves2win(hist[i][1])) == 0)
                        break;
		printf("I understand your move at");
		printf(" %c%d (%d)   \n", x / bsiz2 + 'a' - 1, x % bsiz2, j);
                sleep(1);
        }
	printf("But your move at %c%d", x / bsiz2 + 'a' - 1, x % bsiz2);
        printf(" was masterful!  I shall learn it.   \n");
        x = movenum - i;
        for (i = max1; i < 11; i++)
                for (j = max2; j < 11; j++)
                        if (!memory[i][j] || memory[i][j] > x)
                                memory[i][j] = x;
        seek(memf, 0, 0);
        write(memf, memory, 121);
}

getmodes(argc, argv)
char    *argv[];
{
        register char *cp;

	if (argc > 1 && argv[1][1] == 't')
                crtflg = 0;
}

info()
{
        printf("Instructions? ");
	if (fgets(in, 80, stdin) == NULL)
	    exit(1);
	if (*in != 'y')
                return;
	instruct(infofil);
        if (crtflg) {
		fgets(in, 20, stdin);
		printf("%s", cursor(CLEAR, 0));
        }
}
