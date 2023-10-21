/*
 * %W% (mrdch&amnnon) %G%
 */

# include "header"
# define ENEMY_FIGT sp->inventar.popul[FIGT]

int     my_ships[MAXSHIPS];     /* # ships moved to attack */
movable taken;                  /* here we put all that we have taken */
int     fighters;               /* no. of attacking fighters     */
planet * fp, *sp;               /* the First and Second planets pointers */
int     past_plan;              /* how many planets did we pass */
int     attack_fail[] = {
    0, 0, 2, 8, 16, 25, 33, 37, 42, 44, 45, 46, 47, 48
};
int     this_paint;

moveto (s)
char   *s;
{
    planet * tmp;
    char    s1[100];
    int     i,
            j,
            n,
            ngate;
    double  dtct,
            nodtct,
            ftemp,
            addfact;
    char   *spotted_ships ();   /* procedure to transform vars of the * type
                                   int[MAXSHIPS] to string */

    for (j = 0; j < MAXSHIPS; j++)/* clean temp storage */
        my_ships[j] = 0;
    for (j = 0; j < CLASES; j++)
        taken.popul[j] = 0;
    taken.metals = 0;

    fp = getpl (s);             /* see where from */
    assert_player (fp);         /* is the first planet his own */
    skipwhite (s);              /* go to the second planet */

    if (*s != '-') {            /* the '-' needed to signify destination */
        say ("Destination not clear, sir !!");
        return;
    }
    else
        *s = ' ';               /* the function must have a ' ' at the start */
    sp = getpl (s);             /* identify the second planet */
    if (fp == sp) {
        say ("That will not change much, sir !!");
        return;
    }
    skipwhite (s);
    j = -1;
    if (*s == '>') {            /* a SPECIFIC direction was given */
        s++;
        skipwhite (s);
    /* check if the direction is one allowed */
        if ((j = find_direct (*s++)) < 0) {
            say ("The direction specified is unclear, sir!!!");
            return;
        }
/* check if this direction brings us to the destination */
        tmp = fp;
        for (i = 0; i < 17; i++)/* can't be more than that */
            if ((tmp = tmp -> gate[j]) == sp || tmp == NULL)
                break;
        if (tmp != sp) {
            say ("In this direction we can never reach there, sir!!!");
            return;
        }

    }
 /*
  * Search for the destination planet along the specified
  * route. You either fall at the galaxy end, meet yourself
  * again in the circle, or FIND it.
  */
    if (j == -1) {              /* no direction was specified */
        ngate = 0;
        for (i = 0; i < 10; i++) {
            tmp = fp;
            while ((tmp = tmp -> gate[i]))
                if (tmp == sp) {
                    ngate++;
                    j = i;
                    break;      /* see remark below */
                }
                else
                    if (tmp == fp)
                        break;
        }
        if (!ngate) {
            say ("I don't see any direct route to %s, sir.", sp -> pid);
            return;
        }
    /* If forcing the players to specify a route is desired. if (ngate > 1) {
       say ("There is more then one way to get to %s, sir.", sp -> pid);
       return; } */
    }
    ngate = j;
 /* start collecting the aramade that was ordered to fly */
    for (;;) {
        skipwhite (s);
        i = atoi (s);           /* take the no. of ships */
        assert_negative (i);    /* see if not negative */
        if (i) {                /* if any was specified */
            skipnum (s);        /* go over it */
            skipwhite (s);      /* and get the type */
        }
        else
            i = 1;              /* if none given, assume 1 */
        if (*s < 'a')
            *s += ('a' - 'A');  /* transform them to lower */
        if ((*s < 'a') || (*s > 'a' + MAXSHIPS - 1)) {
            say ("The type of ship is not clear, sir!!");
            return;
        }
        j = *s - 'a';
        my_ships[j] += i;
        s++;
        skipwhite (s);
        if (!*s)                /* check if EOL reached */
            break;
    }
    for (j = 0; j < MAXSHIPS; j++)
        if (my_ships[j] > fp -> ships[j]) {
            say ("But you don't have that many %c-type ships there, sir!!", j + 'A');
            return;
        }
    n = 0;                      /* calculate no of fighters to fly them */
    for (j = 0, i = 1; j < MAXSHIPS; j++, i *= 2)
        n += NCREW / i * my_ships[j];
    if (n > fp -> inventar.popul[FIGT]) {/* not enough */
        say ("But sir, there are not enough fighters to fly these ships !!");
        return;
    }

 /*
  * At this point it is clear that movement is possible.
  * Before doing anything else- put in a "movable"
  * structure everything ordered to be moved. Update the
  * source planet NOW, and the destination planet
  * only after it's nature is known.
  * Then check if the movement was discovered. If so-
  * notify the enemy.
  * If going to a friendly planet just do the house keeping.
  * Else -  it's WAR .
  */

 /* update no. of fighters on the source planet */

    fighters = n;
    fp -> inventar.popul[FIGT] -= n;
 /* update no. of ships */
    for (j = 0; j < MAXSHIPS; j++)
        fp -> ships[j] -= my_ships[j];
 /* deal with the knowledge */
    taken.know = fp -> to_take.know;
    fp -> to_take.know = 0;
 /* how much can he carry? */
    n = 0;
    for (j = 0; j < MAXSHIPS; j++)
        n += (j + 1) * my_ships[j];
    if (n >= fp -> to_take.metals) {/* enough room */
        taken.metals += fp -> to_take.metals;
        fp -> to_take.metals = 0;
    }
    else {
        taken.metals += fp -> to_take.metals - n;
        fp -> to_take.metals -= n;
    }
 /* see how many people wait */
    i = 0;
    for (j = 0; j < CLASES; j++)
        i += fp -> to_take.popul[j];
 /* and how many places */
    n = 0;
    for (j = 0; j < MAXSHIPS; j++)
        n += VISITORS * (j + 1) * my_ships[j];
    if (n >= i)                 /* if enough room */
        for (j = 0; j < CLASES; j++) {
            taken.popul[j] = fp -> to_take.popul[j];
            fp -> to_take.popul[j] = 0;
        }
 /* NOT enough room. transfer by importance */
    else
        for (j = 0; n > 0; j++) {
            if ((n - fp -> to_take.popul[j]) >= 0) {
                n -= fp -> to_take.popul[j];
                taken.popul[j] = fp -> to_take.popul[j];
                fp -> to_take.popul[j] = 0;
            }
            else {
                taken.popul[j] = n;
                fp -> to_take.popul[j] -= n;
                n = 0;
            }
        }

 /* here check if the movement was discovered */

    this_paint = fp -> paint;   /* keep the paint. */
    fp -> paint = 0;            /* NONE was left at home */

 /*
  * Calculate effective painting.
  * The rule when calculating the paint:
  *     More ships   - easier to detect ;
  *     Better ships - harder to detect.
  * The calculation is confined to the type of ship. i.e.
  * each ship type has it's own no. indicating how
  * difficult it is to detect it.
  * As the ships start their voyage, the paint fades at
  * the FADE_RATE per each planet past-by.
  * A new factor is then calculated.
  */
    addfact = 0;
    for (i = 0, ftemp = 1; i < MAXSHIPS; i++, ftemp /= 2)
        addfact += ftemp * my_ships[i];
    nodtct = this_paint / addfact;

    tmp = fp;
    past_plan = 0;

    do {
        past_plan++;            /* count the no. of planets we passed */
        tmp = tmp -> gate[ngate];/* continue along the line */
        this_paint = (double) this_paint * (100.- FADE_RATE) / 100.;
        addfact = 0;
        for (i = 0, ftemp = 1; i < MAXSHIPS; i++, ftemp /= 2)
            addfact += ftemp * my_ships[i];
        nodtct = this_paint / addfact;

        if (tmp -> whos != fp -> whos) {
            dtct = tmp -> detect;
            if (dtct > nodtct) {
                if (!any_force (tmp -> missile))
                    (void) sprintf (s1, "Sir!!! At %s we spotted %s enemy ships.", tmp -> pid, spotted_ships (my_ships));
                else {
                    (void) sprintf (s1, "Sir!!! At %s our %s missiles encountered ", tmp -> pid, spotted_ships (tmp -> missile));
                    (void) sprintf (s1, "%s%s enemy ships.", s1, spotted_ships (my_ships));
                }
                dowrite_enemy (s1, !player);
                missile_war (tmp);
                if (!any_force (my_ships)) {
                    (void) sprintf (s1, "Sir!!! Our missiles defeated the enemy at %s!!!", tmp -> pid);
                    dowrite_enemy (s1, !player);
                    assert_loosing ();
                }
            }
        }
    } while (tmp != sp);

    if (sp -> alms)             /* are there any alms ? */
        assert_loosing ();

/* *********************************************  */

    if ((fp -> whos == sp -> whos) || (sp -> whos == 2)) {
    /* forces remain in his domain or neutral captured */
    /* first move the ships */
        for (j = 0; j < MAXSHIPS; j++)
            sp -> ships[j] += my_ships[j];
    /* then the fighters */
        sp -> inventar.popul[FIGT] += fighters;
    /* is there knowledge to take? */
        if (sp -> to_take.know < taken.know)
            sp -> to_take.know = taken.know;
    /* take the materials */
        sp -> to_take.metals += taken.metals;
    /* take the people */
        for (j = 0; j < CLASES; j++)
            sp -> to_take.popul[j] += taken.popul[j];
        sp -> paint += this_paint;
        if (sp -> whos != fp -> whos) {
            say ("Congratulations, sir. We have captured that planet.");
            sp -> whos = fp -> whos;
            prepare_siege ();
            chek_siege (sp);
            return;
        }
    }
    else {
        if (ENEMY_FIGT)         /* any FIGHTERS there? */
            war ();
        if (!any_force (my_ships)) {
            (void) sprintf (s1, "Sir!!! Our forces have encountered and defeated the enemy at %s!!!", sp -> pid);
            dowrite_enemy (s1, !player);
            assert_loosing ();
        }
        capture (1, sp);
        sp -> paint += this_paint;
        prepare_siege ();
        chek_siege (sp);
        say ("Congratulations, sir!!. We have captured that planet.");
    }
    check_end ();
}

/* *********************************************  */

 /*
  * This is the WAR section.
  * The results are determined by the no. and level of the
  * ships involved in the battle, and also the distance
  * that the attacking ships have traveled to their destination.
  * The longer they traveled, the better are the chances for
  * the defending forces to defeat their enemy with little
  * or no lost to themselves.
  */
war () {
    int     i,
            j,
            k,
            n;
    int     his_ships[MAXSHIPS];
    int     perc_fail = attack_fail[past_plan],
            temp_fail;
    char    s1[100];

    for (i = 0; i < MAXSHIPS; i++)
        his_ships[i] = 0;       /* clean temporary storage */

    for (j = MAXSHIPS - 1, i = NCREW; j >= 0; j--, i /= 2)
        while (sp -> ships[j] && ENEMY_FIGT >= NCREW / i) {
            his_ships[j]++;
            sp -> ships[j]--;
            ENEMY_FIGT -= NCREW / i;
        }

    if (!any_force (his_ships))
        return;                 /* he might not have enough soldiers */

    for (j = 0; j < MAXSHIPS; j++)
        while (my_ships[j] && his_ships[j]) {/* any ships */
            my_ships[j]--;      /* I ALWAYS loose one */
            i = 100 - (rand () % 100);
            if (i > perc_fail)  /* if more then criterion */
                his_ships[j]--; /* He looses too */
        }

    if (!any_force (his_ships))
        return;

    for (j = 1, k = 2; j < MAXSHIPS; j++, k *= 2) {
        if (my_ships[j] > 0) {
            i = 0;
            n = k;
            while (i < j) {
                while (his_ships[i] >= n && my_ships[j]) {
                    my_ships[j]--;/* I loose */
                    temp_fail = 100 - (rand () % 100);
                    if (temp_fail > perc_fail)
                        his_ships[i] -= n;/* He too */
                }
                if (my_ships[j])
                    his_ships[i] = 0;
                i++;
                n /= 2;
            }
        }
        else
            if (his_ships[j] > 0) {
                i = 0;
                n = k;
                while (i < j) {
                    while (my_ships[i] >= n && his_ships[j]) {
                        my_ships[i] -= n;
                        temp_fail = 100 - (rand () % 100);
                        if (temp_fail > perc_fail)
                            his_ships[j]--;
                    }
                    if (his_ships[j])
                        my_ships[i] = 0;
                    i++;
                    n /= 2;
                }
            }
    }
    if (!any_force (his_ships))
        return;

 /* none left from My forces.  we're doomed */

    n = 0;              /* calculate no of fighters left to him */
    for (j = 0, i = 1; j < MAXSHIPS; j++, i *= 2) {
        sp -> ships[j] += his_ships[j];
        n += NCREW / i * his_ships[j];
    }

    ENEMY_FIGT += n;
    (void) sprintf (s1, "Sir!!! Our forces have encountered and defeated the enemy at %s!!!", sp -> pid);
    dowrite_enemy (s1, !player);
    assert_loosing ();
}

missile_war (pp)
planet * pp;
{
    int     i,
            j,
            k,
            n;
    char    s1[100];
    int     destroyed[MAXSHIPS];/* # of ships destroyed */

    if (!any_force (pp -> missile))
        return;                 /* the defender has no missiles there */

    for (j = 0; j < MAXSHIPS; j++)
        destroyed[j] = 0;       /* all ships are intact (so far...) */

    for (j = 0; j < MAXSHIPS; j++)
        while (my_ships[j] && pp -> missile[j]) {/* any ships */
            my_ships[j]--;      /* I ALWAYS loose one */
            destroyed[j]++;     /* remember what I lost */
            pp -> missile[j]--; /* He looses too */
        }

    if (!any_force (pp -> missile)) {
    /* The battle has been fought, tell the owner of the missiles */
        (void) sprintf (s1, "Sir!!! Our missiles destroyed the enemy ships: %s.", spotted_ships (destroyed));
        dowrite_enemy (s1, !player);
        return;
    }

    for (j = 1, k = 2; j < MAXSHIPS; j++, k *= 2) {
        if (my_ships[j] > 0) {
            i = 0;
            n = k;
            while (i < j) {
                while (pp -> missile[i] >= n && my_ships[j]) {
                    my_ships[j]--;
                    destroyed[j]++;
                    pp -> missile[i] -= n;
                }
                if (my_ships[j])
                    pp -> missile[i] = 0;
                i++;
                n /= 2;
            }
        }
        else
            if (pp -> missile[j] > 0) {
                i = 0;
                n = k;
                while (i < j) {
                    while (my_ships[i] >= n && pp -> missile[j]) {
                        my_ships[i] -= n;
                        destroyed[j] += n;
                        pp -> missile[j]--;
                    }
                    if (pp -> missile[j]) {
                        destroyed[i] = my_ships[i];
                        my_ships[i] = 0;
                    }
                    i++;
                    n /= 2;
                }
            }
    }
 /* The battle has been fought, tell the owner of the missiles */
    (void) sprintf (s1, "Sir!!! Our missiles destroyed the enemy ships: %s.", spotted_ships (destroyed));
    dowrite_enemy (s1, !player);
}

/*
 * Here when landing on a unarmed enemy's teritory
 * or when winning a battle.
 */

capture (force, sp)
int     force;
planet * sp;
{
    int     i,
            j,
            k,
            m,
            n,
            n1,
            n2;

    sp -> whos = fp -> whos;    /* now it is MINE */
    free_reports (sp -> reports);/* the reports are not actual */
    sp -> secur = 0;            /* no security is valid */
    if (force) {
        sp -> paint = 0;        /* if paint was left-destroy */
        sp -> detect = 0;       /* if detection left-destroy */
        for (j = 0; j < MAXSHIPS; j++)
            sp -> missile[j] = 0;
    }
    for (j = 0; j < ESPSIZ; j++)
        for (m = 0; m < ESPTYP; m++)
            for (k = 0; k < 2; k++)
                sp -> espion[k][m][j] = 0;/* and no espionage yet */

 /* first move the forces */
    for (j = 0; j < MAXSHIPS; j++)
        sp -> ships[j] = my_ships[j];
 /* how much metal can he carry? */
    n = 0;
    for (j = 0; j < MAXSHIPS; j++)
        n += (j + 1) * my_ships[j];
    if (n >= taken.metals)      /* enough room */
        sp -> to_take.metals += taken.metals;
    else
        sp -> to_take.metals += n;
 /* when dealing with knowledge, it is ALWAYS transfered */
    if (force) {
        sp -> to_take.know = taken.know;
        sp -> inventar.know = taken.know;
    }
 /* now deal with the people */
    n2 = sp -> to_take.popul[SLAV];/* keep them. they were yours */
    sp -> to_take.popul[SLAV] = 0;
 /* add all the to_be_transfered people */
    for (j = 0, n1 = 0; j < CLASES; j++) {
        n1 += sp -> to_take.popul[j];
        sp -> to_take.popul[j] = 0;
    }
    sp -> to_take.popul[CITI] = n2;
    sp -> to_take.popul[SLAV] = n1;/* put them correctly now */
 /* see how many people wait */
    i = 0;
    for (j = 0; j < CLASES; j++)
        i += taken.popul[j];
 /* and how many places */
    n = 0;
    for (j = 0; j < MAXSHIPS; j++)
        n += VISITORS * (j + 1) * my_ships[j];
    if (n >= i)                 /* if enough room */
        for (j = 0; j < CLASES; j++) {
            sp -> to_take.popul[j] += taken.popul[j];
        }
 /* NOT enough room. transfer by importance */
    else
        for (j = 0; n > 0; j++) {
            if ((n - taken.popul[j]) >= 0) {
                n -= taken.popul[j];
                sp -> to_take.popul[j] += taken.popul[j];
            }
            else {
                sp -> to_take.popul[j] += n;
                n = 0;
            }
        }
 /* now deal with the people that are stationed on the planet */
    n2 = sp -> inventar.popul[SLAV];/* keep them. they were yours */
    sp -> inventar.popul[SLAV] = 0;
 /* add all the to_be_transfered people */
    for (j = 0, n1 = 0; j < CLASES; j++) {
        n1 += sp -> inventar.popul[j];
        sp -> inventar.popul[j] = 0;
    }
    sp -> to_take.popul[CITI] += n2;
    sp -> inventar.popul[SLAV] = n1;/* put them correectly now */

    n = 0;                      /* calculate no of fighters left to me */
    for (j = 0, i = 1; j < MAXSHIPS; j++, i *= 2)
        n += NCREW / i * my_ships[j];
    ENEMY_FIGT = n;
}

/* this recursive functions frees all the information that
 * was gathered about a captured planet. Naturly, it is not valid
 * anymore. */
free_reports (prvinfo)
info * prvinfo;
{
    if (prvinfo == 0)
        return;                 /* end of list. */
    free_reports (prvinfo -> next);
    ifree (prvinfo);
}

/* check to see if any forces left */
any_force (ship)
int    *ship;
{
    int     n,
            j;

    n = 0;
    for (j = 0; j < MAXSHIPS; j++)
        if (*ship++)
            n++;
    return (n);
}
/* this function prepares for the siege checking */
prepare_siege () {
    int     i;

    fighters = 0;
    taken.know = 0;
    taken.metals = 0;
    for (i = 0; i < CLASES; i++)
        taken.popul[i] = 0;
    for (i = 0; i < MAXSHIPS; i++)
        my_ships[i] = 0;
}

/*
 * This function checks to see if by capturing a planet
 * a siege was completed around one of the enemy's planets.
 * Each surrounding planet is scanned, and if it belongs to
 * the enenmy, it's surroundings is scanned. If all the
 * planets belong to the PLAYER, it is considered captured.
 * In contrast to direct taking by force, all except the ships
 * remain intact.
 */

chek_siege (pp)
planet * pp;
{
    char    s[100];
    int     i,
            j,
            np;
    planet * tmp1, *tmp2;

    for (i = 0; i < 10; i++)
        if ((tmp1 = pp -> gate[i]) && (tmp1 -> whos == !player)) {
            for (j = 0, np = 0; j < 10; j++)
                if ((tmp2 = tmp1 -> gate[j]) && (tmp2 -> whos != player))
                    np++;
            if (!np) {          /* he is totaly surrounded */
                capture (0, tmp1);
                (void) sprintf (s, "Sir!!! we have taken %s planet too!!!", tmp1 -> pid);
                dowrite_enemy (s, player);
            }
        }
}

/* This procedure transforms an integer array with ship counts in */
/* a string listing the amounts of the different ships            */
/* The array [3,2,0,1,0,0,...,0] would be transformed to the      */
/* string " 3A 2B 1D" .                                           */
char   *spotted_ships (ship)
int    *ship;
{
    static char s[100];         /* The list of ships; e.g. " 3A 2B 1D" */
    int     j;

    s[0] = '\0';
    for (j = 0; j < MAXSHIPS; j++) {
        if (*ship)
            (void) sprintf (s, "%s %d%c", s, *ship, 'A' + j);
        ship++;
    }
    return (s);
}
