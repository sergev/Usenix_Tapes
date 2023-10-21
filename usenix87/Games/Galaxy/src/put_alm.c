/*
 * %W% (mrdch&amnnon) %G%
 */

# include "header"

putalm (s)
char   *s;
{
    planet * pp;
    int     i,
            nalms;

    pp = getpl (s);
    assert_player (pp);         /* check valid planet and owner */
    if (planet_popul (pp) == 0) {
        say ("But sir, there is nobody there to do it.");
        return;
    }
    nalms = atoi (s);
    assert_number (nalms);      /* valid no.?            */
    skipword (s);
    assert_end (s);             /* ends gracefully?      */
    i = ALMCOST * nalms;
    assert_money (i);           /* hase enough money?    */
    pp -> alms += nalms;
    teller[player] -= i;
    say ("Beware, sir, we'll better get the hell out of here!!");
}
