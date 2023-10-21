/*
 * %W% (mrdch&amnnon) %G%
 */

# include "header"

extern  char    didchange[2] ;
extern  int     changestat;
extern  int     nulluser;
int     no_new_year = 0;

/*
 * get a string. see if exists within the wizards file.
 */
iswizard (u)
char   *u;
{
    char    s[BUFSIZ];
    FILE * wizf = fopen (WIZFIL, "r");

    if (wizf == NULL)
        return (0);

    while (xgets (s, wizf)) {
        if (strcmp (s, u) == 0) {
            (void) fclose (wizf);
            return (1);         /* oh, boy, i'm a wizard! */
        }
    }

    (void) fclose (wizf);
    return (0);
}

/* gather chars into string, return upon CR or EOF */
xgets (s, f)
FILE * f;
char   *s;
{
    int     c;

    for (;;) {
        c = getc (f);
        if (c == '\n' || c == EOF) {
            *s = '\0';
            return (c == EOF ? 0 : 1);
        }
        else
            *s++ = c;
    }
}

/*
 * function that chakes if you're a wizard.
 */
assert_wizard () {
    if ( (iswiz[player] != 1) && !changestat) {
        say ("But sir, where is your wizard hat?");
        longjmp (jparse, 1);
    }
}

toggle_wizard () {
    if (iswiz[player] == 1) {
        iswiz[player]++ ;
        say("Sir! You are no longer a wizard.") ;
        return ;
    }
    if (iswiz[player] == 2) {
        iswiz[player]-- ;
        say ("Sir! You have been nominated wizard again.") ;
        return ;
    }
}

/*
 * If playing agains oneself, or wizard, give the asking side
 * the opportunity to become the other side.
 */
changeplay () {
    if (!nulluser)
        assert_wizard ();
    curse_map (curpln);
    if (changestat) {
        player = !player;
        curpln = apntr;
    }
    else
        curpln = spntr;
    curse_com (curpln);
    didchange[player] = !didchange[player];
}

/*
 * gives a lot. needs a wizard.
 */
wizwiz () {
    int     j;

    assert_wizard ();
    feedpop ("5000");

    for (j = 0; j < CLASES; j++) {
        curpln -> inventar.popul[j] += 1000;
    }
    curpln -> inventar.metals += 100;
    curpln -> inventar.know = 5;
    for (j = 1; j <= MAXSHIPS; j++) {
        curpln -> ships[j - 1] += j * 11;
        curpln -> missile[j - 1] += j * 11;
    }
}

/*
 * function that gives a planet to a player. Needs wizard to
 * work.
 */
takepl (s)
char   *s;
{
    planet * pp;

    assert_wizard ();
    pp = getpl (s);
    skipword (s);
    assert_end (s);
    pp -> whos = player;
    say ("Planet %s surrenders.", pp -> pid);
    check_end ();
}
/* A  quick exit for those who are permitted */
getoutofhere ()
{
    assert_wizard ();
    endgame (-1);
}

debuggingmode() {
        if(iswiz[0] || iswiz[1])
                return(1);
        else
                return(0);
}
