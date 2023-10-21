/*
 * %W% (mrdch&amnnon) %G%
 */

# include "header"

planetsit (s)
char   *s;
{
    planet * pp;

    pp = getpl (s);
    skipwhite (s);              /* find the first non-white      */
    if (*s == 'i') {
        skipword (s);
        assert_end (s);
        ps_i (pp);
        return;
    }
    flip ();
    clear ();
    pos (0, 20);
    print ("Information available about: %s \r\n\n", pp -> pid);
 /* give full information to wizards or owner    */
    if (pp -> whos == player || iswiz[player])
        if (chek_empty (pp))
            print_info (pp);
        else
            print ("\r\n\n This planet is empty.");
    else {
        if (!count_spies (pp, player))
            print ("Sorry sir, no information.");
        else {
            print ("Your spies are assigned to:\r\n");
            dis_spies (pp, player, 3, 0);
        }
    }
    more ();
}

/* gives all the information about a given planet */
print_info (pp)
planet * pp;
{
    int     i,
            j,
            ps,
            cc,
            col;

    if (j = pp -> inventar.popul[FIGT])
        print ("There are %d Fighters ready.\r\n", j);

    if (j = pp -> inventar.popul[CITI])
        print ("Agricultural with %6d Citizens \r\n", j);

    if (j = pp -> inventar.popul[MINE])
        print ("Mining-planet with %6d Miners.\r\n", j);

    if (j = pp -> inventar.popul[BUIL])
        print ("Industrial with %6d Builders.\r\n", j);

    if (j = pp -> inventar.popul[SCIE])
        print ("Scientific with %6d Scientists.\r\n", j);

    if (j = pp -> inventar.popul[SLAV])
        print ("     It has %6d Slaves. \r\n", j);
    print ("\r\n");
    i = 0;
    for (j = 0; j < CLASES; j++)
        if (pp -> to_take.popul[j])
            i++;
    if (pp -> to_take.metals)
        i++;
    if (pp -> to_take.know)
        i++;
    if (i) {
        print ("Ready to be moved\r\n");
        for (j = 0; j < CLASES; j++)
            if ((i = pp -> to_take.popul[j]))
                print ("%6d %s.\r\n", i, ocup_name[j]);
        if ((i = pp -> to_take.metals))
            print ("%6d A-type material.\r\n", i);
        if ((cc = pp -> to_take.know))
            print ("   %c-type knowledge.", 'A' + cc);
    }

    ps = 2;
    col = 45;
    i = 0;
    for (j = 0; j < MAXSHIPS; j++)
        if (pp -> ships[j] || pp -> missile[j])
            i++;
    if (i) {
        pos (ps++, col + 3);
        print ("No. of ships and missiles:");
        for (i = 0; i < MAXSHIPS; i++) {
            if ((j = pp -> ships[i]) || pp -> missile[i]) {
                pos (ps++, col);
                print ("%c-type %4d ships %4d missiles.", i + 'A', j, pp -> missile[i]);
            }
        }
    }
    ps++;
    if (j = pp -> secur) {
        pos (ps++, col);
        print ("Blacked out with %d Tellers.", j);
    }
    if (j = pp -> detect) {
        pos (ps++, col);
        print ("Detect moves with %d Tellers.", j);
    }
    if (j = pp -> paint) {
        pos (ps++, col);
        print ("No detection paint %d Tellers.", j);
    }
    if (j = pp -> inventar.metals) {
        pos (ps++, col);
        print ("Metal for %d A-type HAWKs ready.", j);
    }
    if (j = pp -> inventar.know) {
        pos (ps++, col);
        print ("Knowledge of %c-type available.", j + 'A');
    }
    if (j = pp -> alms) {
        pos (ps++, col);
        print ("%d of ALMs installed.", j);
    }
    if (j = pp -> to_build[BUILD_MONEY]) {
        pos (ps++, col);
        print ("Building ships with %d tellers.", j);
    }
    if (j = pp -> to_build[NSHIPS]) {
        pos (ps++, col);
        print (" %d %c-type ships ordered.",
                j , pp -> to_build[LEVEL] + 'A');
    }
    if (count_spies (pp, player)) {
        ps++;
        pr_at (ps++, 30, "Your spies are assigned to:");
        dis_spies (pp, player, ps, 30);
    }
}

/*
 * gives the information about spies reports, gathered so far.
 */
ps_i (pp)
planet * pp;
{
    info * x;
    int     nmgs = 1;

/* count how many messages are on the line */
    if (count_msgs (pp -> reports, player) == 0) {
        say ("No messages, sir.");
        return;
    }
/* leave up to 24 message */
    scroll_msgs (&pp -> reports, player);
    flip ();
    clear ();
    pr_at (0, 20, "Information available about: %s\r\n ", pp -> pid);

    x = pp -> reports;

    while (x) {
        pos (nmgs, 0);
        if (x -> owner == player) {
            print ("%s.", x -> msg);
            nmgs++;
        }
        x = x -> next;
    }
    more ();
}

/* returns the no. of messages awaitaing */
count_msgs (hd, ply)
info * hd;
int     ply;
{
    int     ac = 0;

    while (hd) {
        if (hd -> owner == ply)
            ac++;
        hd = hd -> next;
    }
    return (ac);
}

count_spies (pp, ply)
planet * pp;
{
    int     i,
            j;

    for (i = 0; i < ESPTYP; i++)
        for (j = 0; j < ESPSIZ; j++)
            if (pp -> espion[ply][i][j])
                return (1);
    return (0);
}

extern char *inftypes[];

dis_spies (pp, ply, line, col)
planet * pp;
{
    int     i,
            j,
            flg;

    for (i = 0; i < ESPTYP; i++) {
        flg = 0;
        for (j = 0; j < ESPSIZ; j++)
            if (pp -> espion[ply][i][j])
                flg++;
        if (flg) {
            pr_at (line++, col, "%s:\t", inftypes[i]);
            for (j = 0; j < ESPSIZ; j++) {
                if (pp -> espion[ply][i][j])
                    print ("%d %d\t", j, pp -> espion[ply][i][j]);
            }
        }
    }
}
