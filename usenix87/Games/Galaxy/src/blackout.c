/*
 * %W% (mrdch&amnnon) %G%
 */

# include "header"

blackout (s)
char   *s;
{
    planet * pp;
    int     i;

    pp = getpl (s);             /* get planet id                 */
    assert_player (pp);         /* see if legal planet + owner   */
    skipwhite (s);              /* go to the money part          */
    i = atoi (s);               /* take the amount               */
    assert_money (i);           /* see if there is enough of it */
    skipword (s);               /* skip that no.                 */
    assert_end (s);             /* does it end good??            */
    pp -> secur += i;           /* raise the BLACKOUT            */
    teller[player] -= i;        /* take his money                */
}
