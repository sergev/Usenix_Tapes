/*
 * %W% (mrdch&amnnon) %G%
 */

# include "header"

feedpop (s)
char   *s;
{
    int     i,
            j;

    i = atoi (s);               /* see how much he wants to give */

    if (i <= 0) {
        say ("But sir, please be more generous !!!");
        return;
    }
    assert_money (i);           /* see if he has that much */
    skipword (s);
    assert_end (s);             /* does it end properly? */
    teller[player] -= i;        /* take his money */
    food[player] += i;          /* and give it to the poors */
    j = food[player] * FEED_RATIO;
    i = count_popul (player) - count_class (player, CITI);
    if (j > i)                  /* he gave them more then enough */
        say ("Thousands thanks, my lord !!");
    else
        say ("%d of your people might starve to death, my lord.", i - j);
}
