/*
 * %W% (mrdch&amnnon) %G%
 */

# include "header"

extern int  work_p[MAXPL][CLASES];/* change actual working class */

assign (s)
char   *s;
{
    planet * pp;
    int     i,
            focup,
            socup;
    char    fc,
            sc;

    pp = getpl (s);             /* identify planet               */
    assert_player (pp);         /* justify planet & owner        */
    i = atoi (s);               /* how many people are concerned */
    assert_number (i);          /* justify no.                   */
    skipword (s);               /* go to the first occupation    */
    fc = *s++;                  /* take the first char   */
    skipwhite (s);              /* go to the second      */
    sc = *s++;                  /* take the second       */
    assert_end (s);             /* see if end is clear   */
    if (fc == sc) {
        say ("It will not change much, my lord !!");
        return;
    }

    focup = which_class (fc);
    assert_occup (focup);       /* does it exist??       */
    if (pp -> inventar.popul[focup] < i) {
        say ("But sir, you don't have that many %s at %s!!!", ocup_name[focup], pp -> pid);
        return;
    }

    socup = which_class (sc);
    assert_occup (socup);       /* does the second exist?? */
    if (focup == SLAV || socup == SLAV) {
        say ("Slaves cannot be assigned or reassigned, my lord!!");
        return;
    }
 /* here perform the actual assignment */
    pp -> inventar.popul[focup] -= i;
    pp -> inventar.popul[socup] += i;
    say ("The people are reassigned and ready, my lord.");
    for (i = 0; i < MAXPL; i++)
        if (pp -> pid == pl[i].pid)/* find the corresponding no. */
            break;
    work_p[i][focup] = pp -> inventar.popul[focup];
    return;
}
