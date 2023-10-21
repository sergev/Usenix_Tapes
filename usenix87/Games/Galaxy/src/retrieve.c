/*
 * %W% (mrdch&amnnon) %G%
 */

# include "header"

/*
 * This function enables recovering from real stupid typing
 * mistakes. It could happen that when giving a command
 * involving money, too many '000...' will be typed.
 * Retrieving that money ( or part of it ) is made possible.
 * To avoid as much as possible the influence of such command
 * on the tactics one employs, it is NOT general, and it costs
 * some percentenge of the money asked to be retrieved.
 */

# define PERC_LOST 15

retrieve (s)
char   *s;
{
    planet * pp;
    int     i,
            money;
    double  d;

    skipwhite (s);
    money = atoi (s);
    assert_number (money);
    skipword (s);
    skipwhite (s);
    switch (*s++) {
        case 'f':               /* food */
            assert_end (s);
            if (food[player] < money) {
                say ("But you don't have that much there, sir!!!");
                return;
            }
            i = count_popul (player) - count_class (player, CITI);
            i = i / FEED_RATIO; /* that much they consume */
            money -= i;
            if (money <= 0) {
                say ("This much was consumed already, sir.");
                return;
            }
            food[player] -= money;
            d = (double) money * ((double) PERC_LOST + (rand () % 5)) / 100.;
            money -= (int) d;
            break;
        case 't':               /* trade */
            assert_end (s);
            if (trade[player] < money) {
                say ("But you don't have that much there, sir!!!");
                return;
            }
            trade[player] -= money;
            d = (double) money * ((double) PERC_LOST + (rand () % 5)) / 100.;
            money -= (int) d;
            break;
        case 's':               /* ships */
            pp = getpl (s);     /* get planet id */
            assert_end (s);
            assert_player (pp); /* verify existence + owner */
            if (pp -> to_build[BUILD_MONEY] < money) {
                say ("But you don't have that much there, sir!!!");
                return;
            }
            pp -> to_build[BUILD_MONEY] -= money;
            d = (double) money * ((double) PERC_LOST + (rand () % 5)) / 100.;
            money -= (int) d;
            break;
        case 'b':               /* blackout */
            pp = getpl (s);     /* get planet id */
            assert_end (s);
            assert_player (pp); /* verify existence + owner */
            if (pp -> secur < money) {
                say ("But you don't have that much there, sir!!!");
                return;
            }
            pp -> secur -= money;
            d = (double) money * ((double) PERC_LOST + (rand () % 5)) / 100.;
            money -= (int) d;
            break;
        default:
            say ("Source of retrivel is not clear, sir.");
            return;
    }
    if (money == 0) {
        say ("Sorry sir. Couldn't get anything out of it.");
        return;
    }
    teller[player] += money;
    say ("Menaged to retrieve only %d Tellers, sir.", money);
}
