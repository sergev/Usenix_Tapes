/*
 * %W% (mrdch&amnnon) %G%
 */

# include "header"

/*
 * In order to build new ships, several requirements have to be met.
 * The planet on which it is to be build has to be assigned
 * to Industrial, with some buiders on it.
 * The knowledge level on that planet has to be at least equal to the
 * level of ships to be build. The material there has to be
 * at least equal to 1 ship needed for that level, and money should
 * be provided.
 */

build (s)
char   *s;
{
    planet * pp;
    int     j,
            factor,
            nships,
            ship_type,
            money;

    pp = getpl (s);             /* get planet id */
    assert_player (pp);         /* verify existence + owner */
    if (!pp -> inventar.popul[BUIL]) {
        say ("But sir, who will carry out this work??");
        return;
    }
    skipwhite (s);
    if (*s == 't') {            /* ONLY money is to be added */
        s++;
        skipwhite (s);          /* go to the money portion */
        money = atoi (s);       /* collect the money */
        assert_money (money);   /* does he own that much?? */
        skipword (s);
        assert_end (s);         /* chek now how it ends */
        pp -> to_build[BUILD_MONEY] += money;
        teller[player] -= money;
        factor = 1;
        j = pp -> to_build[LEVEL];
        while (j-- > 0)
            factor *= 2;
        if (pp -> to_build[BUILD_MONEY] >= factor * SHIP_COST)
            say ("The builders will start right away, sir !!");
        else
            say ("The money won't do for a single ship, sir !!");
        return;
    }
    nships = atoi (s);          /* see how many ships to build  */
    assert_negative (nships);
    if (nships) {               /* skip the no. chars */
        skipnum (s);            /* go to the ship type */
        skipwhite (s);
    }
    else
        nships = 1;             /* if none given, assume 1 */
    if (*s < 'a')
        *s += ('a' - 'A');      /* transform type to l.c. */
    if ((*s < 'a') || (*s > 'a' + MAXSHIPS - 1)) {
        say ("The type of ship is not clear, sir!!");
        return;
    }
    ship_type = *s - 'a';
    if (pp -> to_build[LEVEL] != ship_type && pp -> to_build[NSHIPS]) {
        say ("But sir, the builders are still working on the previous ships!!");
        return;                 /* CANNOT change LEVEL */
    }
    if (pp -> inventar.know < ship_type) {
        say ("The knowledge there is insufficient, sir.");
        return;
    }
    factor = 1;
    j = ship_type;
    while (j-- > 0)
        factor *= 2;            /* calculate material for X  type ship. */
    if (pp -> inventar.metals < factor) {
        say ("The material is insufficient for even one ship, sir.");
        return;
    }
    s++;
    skipwhite (s);              /* go to the money portion */
    money = 0;
    if (*s) {                   /* probably wants to add money */
        money = atoi (s);       /* collect it */
        assert_money (money);   /* see if he owns that much */
        skipword (s);           /* chek now how it ends */
        assert_end (s);
    }
    teller[player] -= money;    /* take player's money */
    pp -> to_build[BUILD_MONEY] += money;/* add to existing */
    pp -> to_build[LEVEL] = ship_type;
    pp -> to_build[NSHIPS] += nships;
    if (pp -> to_build[BUILD_MONEY] >= factor * SHIP_COST)
        say ("The builders will start right away, sir !!");
    else
        say ("The money won't do for a single ship, sir !!");
}
