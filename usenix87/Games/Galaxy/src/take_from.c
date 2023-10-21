/*
 * %W% (mrdch&amnnon) %G%
 */

# include "header"

takefrom (s)
char   *s;
{
    planet * pp;
    int     i = 0,
            spl;
    char    cc;

    pp = getpl (s);
    assert_player (pp);
    skipwhite (s);
    if (*s == 'k') {            /* in case of knowledge, no parameter
                                   needed */
        if (pp -> inventar.know) {/* only if above A type */
            pp -> to_take.know = pp -> inventar.know;
            pp -> inventar.know = 0;
        /* DONT leave the knowledge there */
            say ("%c-type knowledge ready to be moved, sir !!!",
                    'A' + pp -> to_take.know);
            return;
        }
        say ("Planet blessed with little knowledge, sir.");
        return;
    }
    i = atoi (s);               /* in any other case- numeric parameter
                                   needed */
    assert_number (i);
    skipword (s);
    cc = *s++;
    assert_end (s);
 /*
  * for each of the following:
  * first check if enough available to be taken.
  * if not- print error massege and exit ;
  * if so- make the transportation to the wharehouse
  */
    if (cc == 't') {
        if (pp -> inventar.metals < i) {
            say ("But you don't have that much metal there !!");
            return;
        }
        pp -> inventar.metals -= i;
        pp -> to_take.metals += i;
        say ("%d A-type material ready to move, sir !!", i);
        return;
    }
    spl = which_class (cc);
    assert_occup (spl);
    if (pp -> inventar.popul[spl] < i) {
        say ("But sir, you don't have that much %s there !!", ocup_name[spl]);
        return;
    }
    pp -> inventar.popul[spl] -= i;
    pp -> to_take.popul[spl] += i;
    say ("%d %s are ready to move, sir !!", i, ocup_name[spl]);
    return;
}
