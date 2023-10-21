/*
 * %W% (mrdch&amnnon) %G%
 */

# include "header"

leaveat (s)
char   *s;
{
    planet * pp;
    int     i = 0,
            spl;
    char    cc;


    pp = getpl (s);             /* get planet id. */
    assert_player (pp);         /* see if legal + owner */
    skipwhite (s);
 /* in case of knowledge, no parameter needed */
    if (*s == 'k' || (*s == 'K')) {
        if (pp -> inventar.know > pp -> to_take.know) {
            say ("But sir !!! Why destroy the hard acquired knowledge??");
            return ;
        }
        pp -> inventar.know = pp -> to_take.know;
        if (*s == 'K')          /* clear moving knowledge */
            pp -> to_take.know = 0;
        say ("%c-type knowledge was left, sir !!!", 'A' + pp -> inventar.know);
        return;
    }
    i = atoi (s);               /* in any other case- numeric parameter
                                   needed */
    assert_number (i);          /* chek if legal no. */
    skipword (s);
    cc = *s++;                  /* take item he wants to leave on planet
                                */
    assert_end (s);             /* does he end gracefully?? */
    if (cc == 't') {
        if (pp -> to_take.metals < i) {
            say ("But you don't have that much metal there !!");
            return;
        }
/* perform metal transfer */
        pp -> to_take.metals -= i;
        pp -> inventar.metals += i;
        say ("%d A-type material was transferred, sir !!", i);
        return;
    }
    spl = which_class (cc);
    assert_occup (spl);         /* does it exist?? */
    if (pp -> to_take.popul[spl] < i) {
        say ("But sir, you don't have that many %s there!!!", ocup_name[spl]);
        return;
    }
/* perform human transfer */
    pp -> to_take.popul[spl] -= i;
    pp -> inventar.popul[spl] += i;
    say ("%d %s were left on planet, sir.", i, ocup_name[spl]);
    return;
}
