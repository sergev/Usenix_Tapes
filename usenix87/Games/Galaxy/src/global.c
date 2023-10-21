/*
 * %W% (mrdch&amnnon) %G%
 */

# include "header"
# define PLANET_H       1000

/*
 * This file contains little functions, used globaly
 * in almost all the parsing functions.
 */
/* LINTLIBRARY */

/*VARARGS1*/
say (a, b, c, d, e, f, g, h, i)
char   *a;
{
    cleol (23, 10);
    fprintf (tty, a, b, c, d, e, f, g, h, i);
}

/*VARARGS1*/
msg (a, b, c, d, e, f, g, h, i)
char   *a;
{
    pos (21, 10);
    fprintf (tty, a, b, c, d, e, f, g, h, i);
}

/*VARARGS1*/
bug (a, b, c, d, e, f, g, h, i)
char   *a;
{
    fprintf (stderr, a, b, c, d, e, f, g, h, i);
    fprintf (stderr, "\r\n");
    (void) kill (getpid (), 9);
    endgame (-1);
}

/*VARARGS1*/
pr_at (line, col, a, b, c, d, e, f, g, h, i)
char   *a;
{
    pos (line, col);
    fprintf (tty, a, b, c, d, e, f, g, h, i);
}

/*VARARGS1*/
print (a, b, c, d, e, f, g, h, i)
char   *a;
{
    fprintf (tty, a, b, c, d, e, f, g, h, i);
}

disch (c)
char    c;
{
    putc (c, tty);
}

/* A function to skip * spaces, and point to the first NON space */
doskipwhite (s)
char  **s;
{
    while (isspace (**s) && **s) {
        skip++;
        (*s)++;
    }
}

/* A function to skip * NON spaces, and point to the first space */
doskipblack (s)
char  **s;
{
    while (!isspace (**s) && **s) {
        skip++;
        (*s)++;
    }
}

/* skip a word WITH it's boundries */
doskipword (s)
char  **s;
{
    doskipwhite (s);
    doskipblack (s);
    doskipwhite (s);
}

/* skip only NUMERICAL. point to the first NON numeric. */
doskipnum (s)
char  **s;
{
    doskipwhite (s);
    while (isnum (**s)) {
        skip++;
        (*s)++;
    }
}

/*
 * parse a planet name.  this set of functions should
 * return a pointer to the planet structure, given a
 * planet name.
 */
planet * planet_list[PLANET_H];

planet *
getid (s)
char   *s;
{
    planet * dogetid ();
    register    planet * pp;
    char    pid[4];

    skipwhite (s);
    strncpy (pid, s, 3);

    pp = dogetid (pid);
    if (pp != 0)
        skip += 3;              /* a legal name was found, 3 chars */

    return (pp);
}

/*
 * hash the planet's name, and if found return the pointer
 * to it.
 */
planet *
dogetid (s)
char   *s;
{
    int     hval = 0;
    planet * pl;

    hval = hash_planet (s);

    pl = planet_list[hval];

    if (pl == 0)
        return (0);

    if (strcmp (pl -> pid, s) != 0)
        return (0);
    else
        return (pl);
}

/*
 * This hash function should be perfect.  No collisions are allowed.
 */

hash_planet (s)
register char  *s;
{
    register    ac;

    switch (*s++) {
        case 'l':
            ac = 1;
            break;
        case 'c':
            ac = 2;
            break;
        case 'r':
            ac = 3;
            break;
        default:
            bug ("bad arg to hash_planet %s", s-1);
    }

    ac *= 16;
    ac += hexval (*s++);
    ac *= 16;
    ac += hexval (*s);

    return (ac % PLANET_H);
}

/*
 * the initialization function for planet's recognization
 */
init_getid () {
    register    i;
    register    hval;

    for (i = 0; i < MAXPL; i++) {
        hval = hash_planet (pl[i].pid);

        if (planet_list[hval] != 0)
            bug ("hash function not perfect (init_getid)");

        planet_list[hval] = &pl[i];
    }

    capitals[0] = getid ("l00");
    capitals[1] = getid ("r00");
}

/*
 * The main function that deals with the planet id
 * in a given string.
 */
planet *
dogetpl (s)
char  **s;
{
    register    planet * pp;

    doskipwhite (s);
    skip = 0;

    if (ispid (*s))             /* a possibly legal name was given */
        pp = getid (*s);        /* get the planet's pointer */
    else
        pp = curpln;

    *s += skip;

    assert_planet (pp);
    return (pp);
}

/*
 * Check if a given string starts with chars that
 * possibly represent a planet, and the two other
 * chars are legal continuations of such name.
 */
ispid (s)
char   *s;
{
    switch (*s++) {
        case 'c':
        case 'r':
        case 'l':
            break;

        default:
            return (0);
    }

    if (fisnum (*s++))          /* the first must be numeric */
        if (fishex (*s++))      /* the second can be hex */
            return (1);

    return (0);
}

fisnum (a) {
    return (isnum (a));
}

fishex (a) {
    return (ishex (a));
}

hexval (c)
int     c;
{
    if (isdigit (c))
        return (c - '0');
    else
        return (c - 'a' + 10);
}

/* check if the direction is one allowed */
find_direct (cs)
char    cs;
{
    int     j;

    switch (cs) {
        case ',':               /* counter-clockwise     */
            j = 8;
            break;
        case '.':               /* clockwise     */
            j = 9;
            break;
        case '8':               /* north         */
            j = 0;
            break;
        case '6':               /* west          */
            j = 2;
            break;
        case '2':               /* south         */
            j = 4;
            break;
        case '4':               /* east          */
            j = 6;
            break;
        case '9':               /* north-east    */
            j = 1;
            break;
        case '3':               /* south-east    */
            j = 3;
            break;
        case '1':               /* south-west    */
            j = 5;
            break;
        case '7':               /* north-west    */
            j = 7;
            break;
        default:
            j = -1;
    }
    return (j);
}

/*
 * This function checks if a given planet (parameter)
 * is TOTALY empty. Returnes 0 if so.
 */
chek_empty (pp)
planet * pp;
{
    int     i,
            j,
            k,
            flg = 0;

    for (i = 0; i < CLASES; i++) {
        if (pp -> inventar.popul[i])
            flg++;
        if (pp -> to_take.popul[i])
            flg++;
    }
    if (pp -> inventar.know)
        flg++;
    if (pp -> inventar.metals)
        flg++;
    if (pp -> to_take.know)
        flg++;
    if (pp -> to_take.metals)
        flg++;
    if (pp -> secur)
        flg++;
    if (pp -> alms)
        flg++;
    if (pp -> paint)
        flg++;
    if (pp -> detect)
        flg++;
    for (i = 0; i < 3; i++)
        if (pp -> to_build[i])
            flg++;
    if (pp -> reports)
        flg++;
    for (i = 0; i < MAXSHIPS; i++) {
        if (pp -> ships[i])
            flg++;
        if (pp -> missile[i])
            flg++;
    }
    for (i = 0; i < 2; i++)
        for (j = 0; j < ESPTYP; j++)
            for (k = 0; k < ESPSIZ; k++)
                if (pp -> espion[i][j][k])
                    flg++;
    return (flg);
}

/*
 * assert functions. One never returns to calling function
 * if condition is met.
 */

assert_money (n) {
    if (n <= 0) {
        if (iswiz[player] == 1)
            return;

        say ("That little money won't do, sir.");
        longjmp (jparse, 1);
    }
    if (iswiz[player] == 1)
        teller[player] += n;
    if (n > teller[player]) {
        say ("You don't have that much money, sir.");
        longjmp (jparse, 1);
    }
}

assert_end (s)
char   *s;
{
    skipwhite (s);
    if (*s) {
        say ("The command's end is not clear, sir.");
        longjmp (jparse, 1);
    }
}

assert_number (n) {
    if (iswiz[player] == 1)
        return;

    if (n == 0) {
        say ("That will not change much, sir.");
        longjmp (jparse, 1);
    }
    assert_negative (n);
}

assert_negative (n) {
    if (iswiz[player] == 1)
        return;

    if (n < 0) {
        say ("Negative number, sir ???");
        longjmp (jparse, 1);
    }
}

assert_planet (p)
planet * p;
{
    if (!p) {
        say ("I don't know that planet, sir!!");
        longjmp (jparse, 1);
    }
}

assert_player (p)
planet * p;
{
    assert_planet (p);
    if (iswiz[player] == 1)
        return;
    if (p -> whos != player) {
        say ("Unfortunately, the planet is not yet yours, sir.");
        longjmp (jparse, 1);
    }
}

assert_occup (ocup) {
    if (ocup < 0) {
        say ("I don't recognize the occupation, sir.");
        longjmp (jparse, 1);
    }
}

assert_loosing () {
    say ("So sorry, sir, we have lost them all...");
    longjmp (jparse, 1);
}

/* check if a given planet is a capital */
iscapital (pp)
planet * pp;
{
    if (strcmp (pp -> pid, "r00") == 0 || strcmp (pp -> pid, "l00") == 0)
        return (1);
    else
        return (0);
}

/*
 * this swich checks for the types of legal ocupations.
 * It also check for the planet type that was before,
 * and the planet-type that should be after change will
 * be made.
 */

which_class (cc)
char    cc;
{
    switch (cc) {
        case 'f':
            return (FIGT);
        case 'c':
            return (CITI);
        case 's':
            return (SCIE);
        case 'b':
            return (BUIL);
        case 'm':
            return (MINE);
        case 'v':
            return (SLAV);
        default:
            return (-1);        /* unrecognized */
    }
}

/*
 * Function to count the no of population in a given
 * planet.
 */
planet_popul (pp)
planet * pp;
{
    int     i = 0,
            j;

    for (j = 0; j < CLASES; j++)
        i += pp -> inventar.popul[j];
    return (i);
}

 /*
  *  returns the total no. of people from a predifined class.
  */
count_class (j, class)
int     j,
        class;
{
    int     i,
            ncount = 0;

    for (i = 0; i < MAXPL; i++)
        if (pl[i].whos == j) {
            ncount += pl[i].inventar.popul[class];
            ncount += pl[i].to_take.popul[class];
        }
    return (ncount);
}

 /*
  *  returns the total no. of population of a player.
  */
count_popul (j) {
    int     i,
            k,
            ncount = 0;

    for (i = 0; i < MAXPL; i++)
        if (pl[i].whos == j)
            for (k = 0; k < CLASES; k++) {
                ncount += pl[i].inventar.popul[k];
                ncount += pl[i].to_take.popul[k];
            }
    return (ncount);
}
