/*
 * %W% (mrdch&amnnon) %G%
 */

# include "header"

killalm (s)
char   *s;
{
    planet * pp;
    int     i,
            j,
            nalms;

    pp = getpl (s);             /* get planet id */
    skipwhite (s);              /* go to the no. of alms */
    nalms = atoi (s);           /* see how many he wants to kill */
    if (nalms <= 0) {
        say ("But sir, this will not change much !!");
        return;
    }
    skipword (s);               /* jump over that no. */
    assert_end (s);             /* see if it ending nicely */
    if (pp -> whos == player) {
        j = nalms * REMOVE_COST;
        assert_money (j);       /* see if he has that much */
        j = (nalms > pp -> alms) ? pp -> alms : nalms;
        teller[player] -= j * REMOVE_COST;/* take min. */
        pp -> alms -= j;
        if (pp -> alms < 0)
            pp -> alms = 0;
        if (pp -> alms) {
            say ("The planet is not yet clean, sir!!");
            return;
        }
        else {
            say ("It is perfectly safe now, sir.");
            return;
        }
    }
    else {                      /* remove from the enemy teritory */
        for (i = 0; i < 10; i++)
            if (pp -> gate[i] -> whos == player)
                break;
        if (i == 10) {
            say ("But sir, we don't have access to that planet!!");
            return;
        }
        i = nalms * ALM_KILL_COST;
        assert_money (i);       /* see if he has that much */
        teller[player] -= i;    /* take money IN ANY CASE !! */
        pp -> alms -= nalms;
        if (pp -> alms < 0)
            pp -> alms = 0;
    }
}
