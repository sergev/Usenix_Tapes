/*
 * %W% (mrdch&amnnon) %G%
 */

# include "header"

# define SOULS   pp->inventar.popul     /* shorter */
# define M_SOULS pp->to_take.popul      /* shorter */

int     month = 0;
int     work_p[MAXPL][CLASES];  /* how many EFFECIVE people are */
extern int  no_new_year;

/*
 * This routine is called whenever a command finishes
 * execution. If time past is longer then defined, or
 * if explicitly asked for, a new year arrives.
 */
softclock (force)
int     force;
{
    static  time_t todays_time = 0;
    static  time_t last_year;

    if (no_new_year)
/* wizard asked that TIME will freeze */
        return;

/* This is the first time we call soft clock. */
    if (todays_time == 0) {
        (void) time (&todays_time);
        last_year = todays_time;
        return;
    }

    (void) time (&todays_time);

    if ( force == 2 ) {         /* We took a break! Restore time */
        last_year =  todays_time - (time_t) ((float) month * (float) YEARLENGTH/100.);
        return;
    }
    if (last_year + YEARLENGTH <= todays_time || force) {
        last_year = todays_time;
        newyear ();
    }
    month = (todays_time - last_year) * 100 / YEARLENGTH;
}

/*
 * A NEW YEAR is defined as one of the following situations :
 * 1- The allocated time had passed.
 * 2- When the TWO players have agreed to it.
 */

int     ncitizens[3],
        npopul[3];
static int  plan_own[MAXPL];

newyear () {
    int     i,
            j;
    planet * pp;

    dowrite_enemy ("Happy new year, dear sir.", 0);
    dowrite_enemy ("Happy new year, dear sir.", 1);

/* only in the FIRST year */
    if (year == 0)
        for (i = 0; i < MAXPL; i++) {
            pp = &pl[i];
            plan_own[i] = pl[i].whos;
            for (j = 0; j < CLASES; j++)
                work_p[i][j] = SOULS[j];
        }

    year++;                     /* this is a NEW year */

/* update EFFECTIVE no of people if planet changed hands */
    for (i = 0; i < MAXPL; i++) {
        pp = &pl[i];
        for (j = 0; j < CLASES; j++) {
            if (plan_own[i] != pl[i].whos)
                work_p[i][j] = 0;
/* update EFFECTIVE no of people if less then before. */
        if (SOULS[j] < work_p[i][j])
            work_p[i][j] = SOULS[j];
        }
    }
    for (i = 0; i < MAXPL; i++) {
        pp = &pl[i];
    /* see if any active alm was left */
        if (pl[i].alms)
            check_alm (i, pp);
    /* raise the popul. of citizens */
        if (work_p[i][CITI])
            inc_popul (i, pp);
    /* see if any knwledge gained */
        if (work_p[i][SCIE])
            check_know (i);
    /* see if mining was done */
        if (work_p[i][MINE] || work_p[i][SLAV])
            check_mining (i);
    /* see if building is in progress */
        if (pl[i].to_build[NSHIPS])
            check_build (i);
    /* see to it that detection devices loose effectiveness */
        if (pl[i].detect)
            reduce_detect (pp);
    /* see if it is espionaged. */
        check_esp (pp);
    }
    for (i = 0; i < MAXPL; i++)
        plan_own[i] = pl[i].whos;
    for (j = 0; j < 2; j++) {
        npopul[j] = count_popul (j);
        ncitizens[j] = count_citi (j);
        teller[j] += ncitizens[j];
        if (trade[j])
                do_trade (j);           /* add the money for trade */
        check_food (j);         /* see if enough food was given */
        clr_svqt (j);           /* clear all two players flags */
    }
/* update the population to the new situation */
    for (i = 0; i < MAXPL; i++) {
        pp = &pl[i];
        for (j = 0; j < CLASES; j++)
            work_p[i][j] = SOULS[j];
    }
}

/* count the effective citizenes */
count_citi (j) {
    int     i,
            nciti = 0;

    for (i = 0; i < MAXPL; i++)
        if (pl[i].whos == j)
            nciti += work_p[i][CITI];
    return (nciti);
}

/* clear save and quit flags */
clr_svqt (j) {
    wants_quit[j] = 0;
    wants_save[j] = 0;
    wants_restor[j] = 0;
    wants_newy[j] = 0;
}

/* increment the citizens population */
inc_popul (i, pp)
planet * pp;
{
    double  d;

    d = (double) work_p[i][CITI];
    d *= (PERC_POPUL) / 100.;
    SOULS[CITI] += (int) d;
}

/*
 * Chek for alm presence. If found and the the flag for this
 * planet is 0- raise it to 1 to indicate that it was just
 * now installed. If it is 1- destroy whatever is found there.
 * If there is none- put 0 in the corresponding flag.
 */
check_alm (i, pp)
planet * pp;
{
    static int  alm_flg[MAXPL]; /* flags to indicate alm presence */
    int     j;

    if (year == 1)
        for (j = 0; j < MAXPL; j++)
            alm_flg[j] = 0;
    if (pl[i].alms) {
        if (!alm_flg[i])
            alm_flg[i]++;
        else {
            for (j = 0; j < CLASES; j++) {
                SOULS[j] = 0;
                M_SOULS[j] = 0;
                work_p[i][j] = 0;
            }
        }
    }
    else
        alm_flg[i] = 0;
}

/* reduce the effectiveness of detection devices */
reduce_detect (pp)
planet * pp;
{
    pp -> detect = pp -> detect * (100 - REDUCE_RATE) / 100;
}

/* update the mining materials in the given planet */
check_mining (i)
int     i;
{
    static int  some_metal[MAXPL];/* accumulates metal */
    int     nminers;

    nminers = work_p[i][MINE] + work_p[i][SLAV];
    while (nminers >= MINING_FACTOR) {
        pl[i].inventar.metals++;
        nminers -= MINING_FACTOR;
    }
    if (nminers)
        some_metal[i] += nminers;
    if (some_metal[i] >= MINING_FACTOR) {
        some_metal[i] -= MINING_FACTOR;
        pl[i].inventar.metals++;
    }
}

/* see if building is in progress */
check_build (i)
int     i;
{
    char    s[450],
            s1[450];
    int     factor = 1,
            nbuild = 0,
            j = 0;
    int     shipcost;
    int     level = pl[i].to_build[LEVEL];
    int     nbuilders = work_p[i][BUIL];

    s[0] = s1[0] = '\0' ;
    while (level > j++)
        factor *= 2;
    shipcost = factor * SHIP_COST;

 /* BEFORE starting to build- is there still knowledge ?? */
    if (pl[i].inventar.know < level) {
        (void) sprintf (s, "The knowledge at %s is not enough, sir.", pl[i].pid);
        dowrite_enemy (s, pl[i].whos);
        return;
    }
    while (pl[i].to_build[NSHIPS]) {
        if (!(pl[i].inventar.metals >= factor)) {
            (void) sprintf (s, "The metal in %s was not enough to complete the work, sir.", pl[i].pid);
            break;
        }
        if (!(nbuilders >= shipcost)) {
            (void) sprintf (s, "There were not enough builders in %s to complete the work, sir.", pl[i].pid);
            break;
        }
        if (!(pl[i].to_build[BUILD_MONEY] >= shipcost)) {
            (void) sprintf (s, "We run out of building ships money in %s, sir.", pl[i].pid);
            break;
        }
        pl[i].inventar.metals -= factor;
        nbuilders -= shipcost;
        pl[i].to_build[BUILD_MONEY] -= shipcost;
        pl[i].to_build[NSHIPS]--;
        pl[i].ships[level]++;
        nbuild++;
    }
    if (nbuild) {
        (void) sprintf (s1, "At %s %d %c-type ships were build !!!", pl[i].pid, nbuild, 'A' + level);
        dowrite_enemy (s1, pl[i].whos);
    }
    if (*s)
        dowrite_enemy (s, pl[i].whos);
}

/* see if any knowledge gained */
check_know (i)
int     i;
{
    char    s[100];
    int     factor = 1,
            nscience = 0,
            j = 0;
    int     know_cost;
    int     level = pl[i].inventar.know + 1;
    static int  know_points[MAXPL];


    if (pl[i].inventar.know == MAXSHIPS -1 ) {
        (void) sprintf (s, "Sir!!! Your scintists at %s are unable to advance further." , pl[i].pid) ;
        dowrite_enemy (s, pl[i].whos) ;
        return;
    }
    if (plan_own[i] != pl[i].whos) {
        know_points[i] = 0;
    }
    while (level > ++j)
        factor *= 2;
    know_cost = factor * KNOW_FACTOR;
    nscience = work_p[i][SCIE];
    know_points[i] += nscience;
    if (know_points[i] >= know_cost) {
        pl[i].inventar.know++;
        (void) sprintf (s, "Sir!!! Your scientists at %s have gained %c-level knowledge!!!", pl[i].pid, level + 'A');
        dowrite_enemy (s, pl[i].whos);
        know_points[i] = 0;
    }
}

do_trade (j)                    /* add the results of last year trade */
int     j;
{
    int     i,
            temp;
    char    s[100];
    double      perc ;

    for ( i = 0 , temp = 0 ; i < MAXPL ; i++)
        if (pl[i].whos == j)
            temp++ ;    /* count the planets he controls */
    if ( (perc = (temp - 33) / 2) < 0 )
        perc = 0 ;
    perc += PERC_TRADE;
    perc += rand () % 5;
    temp = (int) ((double) trade[j] * (100.+  perc) / 100.);
    (void) sprintf (s, "We had %d trade profit last year, sir.",
                temp - trade[j]);
    dowrite_enemy (s, j);
    teller[j] += temp;
    trade[j] = 0;
}

/* check if enough food was provided */
check_food (j)
int     j;
{
    char    s[100];
    int     i,
            k,
            kind;
    planet * pp;

    npopul[j] -= ncitizens[j];
    k = food[j] * FEED_RATIO;   /* check how much was given */

    if (k >= npopul[j]) {
        food[j] -= npopul[j] / FEED_RATIO;
        k -= npopul[j];
        if (k < npopul[j]) {    /* next time they might die */
            (void) sprintf (s, "Sir, we are in danger that %d of our people will starve to death!!!", npopul[j] - k);
            dowrite_enemy (s, j);
        }
        return;
    }
    food[j] = 0;
    npopul[j] -= k;             /* that many we'll have to kill */
    (void) sprintf (s, "%d people died from lack of food, sir.", npopul[j]);
    dowrite_enemy (s, j);
    for (kind = SLAV; kind > CITI; kind--) {
        for (i = MAXPL - 1; i >= 0; --i) {
            pp = &pl[i];
            if (pp -> whos == j) {
                if (npopul[j] - SOULS[kind] > 0) {
                    npopul[j] -= SOULS[kind];
                    SOULS[kind] = 0;
                }
                else {
                    SOULS[kind] -= npopul[j];
                    npopul[j] = 0;
                }
                if (npopul[j] - M_SOULS[kind] > 0) {
                    npopul[j] -= M_SOULS[kind];
                    M_SOULS[kind] = 0;
                }
                else {
                    SOULS[kind] -= npopul[j];
                    npopul[j] = 0;
                }
            }
        }
    }
    if (npopul[j]) {            /* still to kill */
        kind = FIGT;
        for (i = MAXPL; i >= 0; --i) {
            pp = &pl[i];
            if (pp -> whos == j) {
                if (npopul[j] - SOULS[kind] > 0) {
                    npopul[j] -= SOULS[kind];
                    SOULS[kind] = 0;
                }
                else {
                    SOULS[kind] -= npopul[j];
                    npopul[j] = 0;
                }
                if (npopul[j] - M_SOULS[kind] > 0) {
                    npopul[j] -= M_SOULS[kind];
                    M_SOULS[kind] = 0;
                }
                else {
                    SOULS[kind] -= npopul[j];
                    npopul[j] = 0;
                }
            }
        }
    }
}

int     last_esp_cost;

check_esp (pp)
planet * pp;
{
    int     etype;

    if (pp -> whos == 2)        /* uninhabited planet! */
        return;

/* check all types of espionage */
    for (etype = 0; etype < ESPTYP; etype++)
        do_check_esp (pp, etype);
}

do_check_esp (pp, etype)
planet * pp;
{
    int     cpl = pp -> whos;
    int     tl1;
    int     elvl;

    tl1 = pp -> espion[!cpl][etype][0];

/* if no money was invested in this kind by the enemy */
    if (tl1 == 0)
        return;

/* if the investment wasn't succesfull, do not check further */
    if (!gave_info (pp, etype, tl1))
        return;

/* for each consecutive level, check if information was obtained */
    for (elvl = 1; elvl < ESPSIZ; elvl++) {
        if (!counter_esp (pp, etype, elvl - 1, cpl))
            return;

        if (!counter_esp (pp, etype, elvl, !cpl))
            return;
    }
}

counter_esp (pp, etype, lvl, ply)
planet * pp;
{
    char    s[100];
/* that's how much was invested in this counter espionage */
    int     tl = pp -> espion[ply][etype][lvl];

/* no money, no information */
    if (tl == 0)
        return (0);

/* reduce the money required by half */
    last_esp_cost /= 2;

/* not enough  money, no information */
    if (tl < last_esp_cost)
        return (0);

/* in counter espionage, only USED money are lost */
    pp -> espion[ply][etype][lvl] -= last_esp_cost;

    (void) sprintf (s,
            "Level %d information at planet's %s obtained by the enemy.\r\n"
            ,lvl, inftypes[etype]);

/* finally give the information */
    spy_msg (s, ply, pp, lvl);
    return (1);
}

/*
 * the main espionage routine, it only check for first
 * level espionage.
 */
gave_info (pp, etype, tlrs)
planet * pp;
{
    int     secur = pp -> secur;

/* the sum invested */
    last_esp_cost = tlrs;
/* the black_out factor, if present */
    last_esp_cost -= pp -> secur;
/* ALL the money is lost */
    pp -> espion[!pp -> whos][etype][0] = 0;
/* reduce black out */
    secur -= tlrs;
/* add the SPECIFIC cost */
    secur += espcost[etype];
/*
 * Each planet has an MIN_ESP initial blackout
 * without investing any money in blackout!
 */

    if (secur < 0) {
/*
 * We have to give him the info since the
 * blackout is less then the money invested in
 * espionage.
 */
        pp -> secur = 0;
        give_info (pp, etype);
        return (1);             /* info given */
    }
    pp -> secur -= tlrs;
    if (pp -> secur < 0)
        pp -> secur = 0;
    pp -> espion[!pp -> whos][etype][0] = 0;
    return (0);
}

/*
 * when deserving it, give the information required.
 */
give_info (pp, esptype)
planet * pp;
{
    char   *ptos ();
    char   *do_esp_pop ();
    char   *do_esp_force ();
    char   *do_esp_missile ();
    char    s[100];

    switch (esptype) {
        case ESPKIND:
            (void) sprintf (s, "Category: %s\r\n", ptos (pp));
            break;
        case ESPPOP:
            (void) sprintf (s, "Popul: %s\r\n", do_esp_pop (pp));
            break;
        case ESPKNOW:
            (void) sprintf (s, "Knowledge level is %c\r\n", pp -> inventar.know + 'A');
            break;
        case ESPMTL:
            (void) sprintf (s, "Metal: %d\r\n", pp -> inventar.metals);
            break;
        case ESPSHIP:
            (void) sprintf (s, "Force: %s\r\n", do_esp_force (pp));
            break;
        case ESPALM:
            (void) sprintf (s, "ALM installed: %d\r\n", pp -> alms);
            break;
        case ESPMSL:
            (void) sprintf (s, "Missile: %s\r\n", do_esp_missile (pp));
            break;
    }
    spy_msg (s, !pp -> whos, pp, 0);
}

/* returns a string with the kind of planet information */
char   *
        ptos (pp)
        planet * pp;
{
    char   *rindex ();
    char   *x;
    static char s[100];

    (void) strcpy (s, "");

    if (SOULS[FIGT])
        (void) strcat (s, "military/");
    if (SOULS[CITI])
        (void) strcat (s, "agricultural/");
    if (SOULS[MINE])
        (void) strcat (s, "mining/");
    if (SOULS[BUIL])
        (void) strcat (s, "industrial/");
    if (SOULS[SCIE])
        (void) strcat (s, "scientific/");

    x = rindex (s, '/');
    if (x != 0)
        *x = '\0';

    if (strcmp (s, "") == 0)
        (void) strcpy (s, "none");

    return (s);
}

char   *people[] = {
    "fight", "citizen", "scien", "build",
    "miner", "slave", 0
};

/* returns a string with the population information */
char   *
        do_esp_pop (pp)
        planet * pp;
{
    char   *rindex ();
    char   *x;
    int     i,
            j;
    static char    s[100];
    static char    s1[100];

    (void) strcpy (s, "");

    for (i = 0; i < CLASES; i++) {
        j = pp -> inventar.popul[i];
        if (j) {
            (void) sprintf (s1, "%d %s/", j, people[i]);
            (void) strcat (s, s1);
        }
    }
    x = rindex (s, '/');
    if (x != 0)
        *x = '\0';

    if (strcmp (s, "") == 0)
        (void) strcpy (s, "no population");
    return (s);
}

/* returns a string with the force information */
char   *
        do_esp_force (pp)
        planet * pp;
{
    int     i,
            j;
    static char    s[100];
    static char    s1[100];

    (void) strcpy (s, "");

    for (i = 0; i < MAXSHIPS; i++) {
        j = pp -> ships[i];
        if (j) {
            (void) sprintf (s1, "%c-%d ", i + 'A', j);
            (void) strcat (s, s1);
        }
    }

    if (strcmp (s, "") == 0)
        (void) strcpy (s, "no forces");
    return (s);
}

/* returns a string with the missile information */
char   *
        do_esp_missile (pp)
        planet * pp;
{
    int     i,
            j;
    static char    s[100];
    static char    s1[100];

    (void) strcpy (s, "");

    for (i = 0; i < MAXSHIPS; i++) {
        j = pp -> missile[i];
        if (j) {
            (void) sprintf (s1, "%c-%d ", i + 'A', j);
            (void) strcat (s, s1);
        }
    }

    if (strcmp (s, "") == 0)
        (void) strcpy (s, "no missiles");
    return (s);
}
