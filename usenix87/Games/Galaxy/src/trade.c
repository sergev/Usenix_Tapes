/*
 * %W% (mrdch&amnnon) %G%
 */

# include "header"

trading (s)
char   *s;
{
    int     i;

    i = atoi (s);               /* take the amount specified */
    assert_money (i);           /* see if legal and available */
    skipword (s);               /* skip over it */
    assert_end (s);             /* see if it ends well */
    say ("I pray for a good trading year, my lord.");
    teller[player] -= i;        /* take his money */
    trade[player] += i;         /* and invest it in trade */
}
