
/*
 * Gprolog 1.4/1.5
 *
 * Barry Brachman
 * Dept. of Computer Science
 * Univ. of British Columbia
 * Vancouver, B.C. V6T 1W5
 *
 * .. {ihnp4!alberta, uw-beaver}!ubc-vision!ubc-cs!brachman
 * brachman@cs.ubc.cdn
 * brachman%ubc.csnet@csnet-relay.arpa
 * brachman@ubc.csnet
 */

#define NCOLS		5	/* default number of cols */

/*
 * Routine to print a table
 * Modified from 'ls.c' mods (BJB/83)
 * Arguments:
 *	base	- address of first entry
 *	num     - number of entries
 *	d_cols  - number of columns to use if > 0, "best" size if == 0
 *	width	- max line width if not zero
 *	prentry - address of the routine to call to print the string
 *	length  - address of the routine to call to determine the length
 *		  of string to be printed 
 *
 * prtable and length are called with the the address of the base and
 * an index
 */
prtable(base, num, d_cols, width, prentry, length)
char *base;
int num, d_cols;
int (*prentry)(), (*length)();
{
        register int c, j;
        register int a, b, cols, loc, maxlen, nrows, z;

        if (num == 0)
                return;
	maxlen = get_maxlen(base, num, length) + 1;
	if (d_cols > 0)
		cols = d_cols;
	else if (width == 0)
		cols = get_columns() / maxlen;
	else
		cols = width / maxlen;
	if (cols == 0)
		cols = NCOLS;
        nrows = (num - 1) / cols + 1;
        for (a = 1; a <= nrows; a++) {
                b = c = z = loc = 0;
                for (j = 0; j < num; j++) {
                        c++;
                        if (c >= a + b)
                                break;
                }
                while (j < num) {
                        (*prentry)(base, j);
			loc += (*length)(base, j);
                        z++;
                        b += nrows;
                        for (j++; j < num; j++) {
                                c++;
                                if (c >= a + b)
                                        break;
                        }
                        if (j < num) {
                                while (loc < z * maxlen) {
					printf(" ");
                                        loc++;
                                }
			}
                }
		printf("\n");
        }
}

static int
get_maxlen(base, num, length)
char *base;
int num;
int (*length)();
{
	register int i, len, max;

	max = (*length)(base, 0);
	for (i = 0; i < num; i++) {
		if ((len = (*length)(base, i)) > max)
			max = len;
	}
	return(max);
}

static int
get_columns()
{
	char *term, tbuf[2024];
	char *getenv();
	int tgetnum();

	if ((term = getenv("TERM")) == (char *) 0)
		return(0);
	if (tgetent(tbuf, term) <= 0)
		return(0);
	return(tgetnum("co"));
}
