/*
 * %W% (mrdch&amnnon) %G%
 */

# include "header"

/*
 * In order to set new missiles, several requirements have to be met.
 * The planet on which it is to be build must have enough soldiers
 * on it.
 * The knowledge level on that planet has to be at least equal to the
 * level of missiles to be build.
 */

set_missile (s)
char   *s;
{
    planet * pp;
    int     j,
            factor,
            nmissiles,
            missile_type,
            money;

    pp = getpl (s);             /* get planet id */
    assert_player (pp);         /* verify existence + owner */
    skipwhite (s);
    nmissiles = atoi (s);       /* see how many missiles to build  */
    assert_negative (nmissiles);
    if (nmissiles) {            /* skip the no. chars */
        skipnum (s);            /* go to the missile type */
        skipwhite (s);
    }
    else
        nmissiles = 1;          /* if none given, assume 1 */
    if (*s < 'a')
        *s += ('a' - 'A');      /* transform type to l.c. */
    if ((*s < 'a') || (*s > 'a' + MAXSHIPS - 1)) {
        say ("The type of missile is not clear, sir!!");
        return;
    }
    missile_type = *s - 'a';
    if (pp -> inventar.know < missile_type) {
        say ("The knowledge there is insufficient, sir.");
        return;
    }
    s++;
    assert_end (s);
    factor = 1;
    j = missile_type;
    while (j-- > 0)
        factor *= 2;    /* calculate money for X  type missile. */
    j = factor * NCREW ;
    if (pp -> inventar.popul[FIGT] < j ) {
        say ("But sir, no fighters there to carry out this work!!");
        return;
    }
    money = factor * nmissiles * MISSILE_COST;
    assert_money (money);       /* see if he owns that much */
    teller[player] -= money;    /* take player's money */
    pp -> missile[missile_type] += nmissiles;
    say ("The missiles are ready. Prepare detection, sir!!");
}
