/*
 * %W% (mrdch&amnnon) %G%
 */

# include "header"

overstat (s)
char   *s;
{
    planet * pp;
    int     i,
            j;
    int     all_popul[CLASES];  /* to count the entire population */
    int     myships[MAXSHIPS];  /* to count the ships inventory */
    int     mymissile[MAXSHIPS];/* to count the missile inventory */
    int     nplanets[PLKINDS + 1];/* to count planet's situation */
    int     nalms = 0;          /* to count the alms */
    int     nmetals = 0;        /* to count the metal digged */
    int     maxknow = -1;       /* the no. of planet with max know */
    char    cc;
    static char owner = 'O';
    static char black = '?';
    static char force = '!';
    static char specl = '+';
    static char alms  = '~';
    static char fight = '^';
    static char build = '&';
    static char dinfo = 'I';
    static char spies = 'S';
    static char detec = 'T';
    static char paint = '%';
    static char missl = 'M';

    skipwhite (s);
    if (*s == '\0') {
        flip ();
        clear ();
        for (i = 0; i < CLASES; i++)
            all_popul[i] = 0;
        for (i = 0; i < MAXSHIPS; i++) {
            myships[i] = 0;
            mymissile[i] = 0;
        }
        for (i = 0; i <= PLKINDS; i++)
            nplanets[i] = 0;
        for (i = 0, pp = &pl[0]; i < MAXPL; i++, pp = &pl[i])
            if (pp -> whos == player) {
                nplanets[PLKINDS]++;
                nalms += pp -> alms;/* add total alms */
                nmetals += pp -> inventar.metals;/* add total metals */
                if (maxknow == -1)
                    maxknow = i;
                if (pl[maxknow].inventar.know < pl[i].inventar.know)
                    maxknow = i;/* update max knowledge */

                if (pp -> inventar.popul[FIGT])
                    nplanets[FIGT]++;
                if (pp -> inventar.popul[CITI])
                    nplanets[CITI]++;
                if (pp -> inventar.popul[MINE])
                    nplanets[MINE]++;
                if (pp -> inventar.popul[BUIL])
                    nplanets[BUIL]++;
                if (pp -> inventar.popul[SCIE])
                    nplanets[SCIE]++;
                for (j = 0; j < CLASES; j++) {
                    all_popul[j] += pp -> inventar.popul[j];
                    all_popul[j] += pp -> to_take.popul[j];
                }
                for (j = 0; j < MAXSHIPS; j++) {
                    myships[j] += pp -> ships[j];
                    mymissile[j] += pp -> missile[j];
                }
            }

        pos (0, 28);
        print ("Overall situation report");
        pos (1, 28);
        print ("========================\r\n");
        print ("Population: %d\r\n", count_popul (player));
        for (j = 0; j < CLASES; j++)
            if (all_popul[j])
                print ("%6d %s\r\n", all_popul[j], ocup_name[j]);
        pos (11, 0);
        print ("Planets situation:\r\n");
        print ("%6d controlled planets\r\n", nplanets[PLKINDS]);
        print ("%6d Military bases.\r\n", nplanets[FIGT]);
        print ("%6d Agricultural planets.\r\n", nplanets[CITI]);
        print ("%6d Mining planets.\r\n", nplanets[MINE]);
        print ("%6d Industrial planets. \r\n", nplanets[BUIL]);
        print ("%6d Scientific planets.\r\n", nplanets[SCIE]);
        j = 3;
        pos (j++, 38);
        print ("No. of ships and missiles:");
        for (i = 0; i < MAXSHIPS; i++)
            if (myships[i] || mymissile[i]) {
                pos (j++, 35);
                print ("%c-type %4d ships %4d missiles.",
                        i + 'A', myships[i], mymissile[i]);
            }
        j++;
        pos (j++, 35);
        print ("Invested in trade %d Tellers.", trade[player]);
        pos (j++, 35);
        print ("Feeding the population with %d Tellers.",
                food[player]);
        pos (j++, 35);
        print ("Total of %d ALMs were laid.", nalms);
        pos (j++, 35);
        print ("Total of %d A-type metal quantities.", nmetals);
        pos (j++, 35);
        print ("The best knowledge is %c in planet %s.",
                'A' + pl[maxknow].inventar.know, pl[maxknow].pid);
        more ();
        return;
    }
/*
 * owner = 'O' ;
 * black = '?' ;
 * force = '!' ;
 * specl = '+' ;
 * alms  = '~' ;
 * fight = '^' ;
 * build = '&' ;
 * dinfo = 'I' ;
 * spies = 'S' ;
 * detec = 'T' ;
 * paint = '%' ;
 * missl = 'M' ;
 *
 * The chars used are: n  o  a  s  b  f  F  c  k  e  i  d  p  m
 * Their attributes:      O  ~  +  ?  !  ^  & A-G S  I  T  %  M
 */
    while ((cc = *s++)) {
        switch (cc) {
            case 'n':   /* clean the screen from all attributes */
                clean_attr ();
                break;
            case 'o':           /* show in reverse planets he owns */
                for (i = 0; i < MAXPL; i++)
                    if (pl[i].whos == player)
                        change_attr (owner, i);
                    else
                        check_attr (owner, i);
                break;
            case 'p':           /* show where detecting devices exist */
                for (i = 0; i < MAXPL; i++)
                    if (pl[i].whos == player)
                        if (pl[i].paint)
                            change_attr (paint, i);
                        else
                            check_attr (paint, i);
                break;
            case 'd':           /* show where detecting devices exist */
                for (i = 0; i < MAXPL; i++)
                    if (pl[i].whos == player)
                        if (pl[i].detect)
                            change_attr (detec, i);
                        else
                            check_attr (detec, i);
                break;

            case 'a':           /* show planets he owns with alm */
                for (i = 0; i < MAXPL; i++)
                    if (pl[i].whos == player)
                        if (pl[i].alms)
                            change_attr (alms, i);
                        else
                            check_attr (alms, i);
                break;
            case 's':           /* show special planets */
                for (i = 0; i < MAXPL; i++)
                    if (pl[i].whos == player) {
                        j = pl[i].inventar.popul[CITI] != 0;
                        j = j || (pl[i].inventar.popul[MINE] != 0);
                        j = j || (pl[i].inventar.popul[BUIL] != 0);
                        j = j || (pl[i].inventar.popul[SCIE] != 0);
                        if (j)
                            change_attr (specl, i);
                        else
                            check_attr (specl, i);
                    }
                break;
            case 'b':           /* show blanked planets */
                for (i = 0; i < MAXPL; i++)
                    if (pl[i].whos == player)
                        if (pl[i].secur)
                            change_attr (black, i);
                        else
                            check_attr (black, i);
                break;
            case 'f':           /* show where his forces are */
                for (i = 0; i < MAXPL; i++)
                    if (pl[i].whos == player) {
                        j = 0;
                        while (!pl[i].ships[j] && j < MAXSHIPS)
                                j++;
                        if (j < MAXSHIPS)
                            change_attr (force, i);
                        else
                            check_attr (force, i);
                    }
                break;
            case 'm':           /* show where his missiles are */
                for (i = 0; i < MAXPL; i++)
                    if (pl[i].whos == player) {
                        j = 0;
                        while (!pl[i].missile[j] && j < MAXSHIPS)
                                j++;
                        if (j < MAXSHIPS)
                            change_attr (missl, i);
                    }
                break;
            case 'F':           /* show where he has fighters */
                for (i = 0; i < MAXPL; i++)
                    if (pl[i].whos == player) {
                        if (pl[i].inventar.popul[FIGT])
                            change_attr (fight, i);
                        else
                            check_attr (fight, i);
                    }
                break;
            case 'c':   /* show where building of ships is done */
                for (i = 0; i < MAXPL; i++)
                    if (pl[i].whos == player) {
                        if (pl[i].to_build[BUILD_MONEY] || pl[i].to_build[NSHIPS])
                            change_attr (build, i);
                        else
                            check_attr (build, i);
                    }
                break;
            case 'k':   /* show other then A level of knowledge */
                for (i = 0; i < MAXPL; i++)
                    if (pl[i].whos == player) {
                        if (pl[i].inventar.know)
                            change_attr ('A' + pl[i].inventar.know, i);
                        else
                            if (pl[i].d_symbol[player] >= 'A' &&
                            pl[i].d_symbol[player] <= 'A' + MAXSHIPS)
                                change_attr (pl[i].symbol, i);
                    }
                break;
            case 'i':           /* show where you have messages */
                for (i = 0; i < MAXPL; i++)
                    if (count_msgs (pl[i].reports, player))
                        change_attr (dinfo, i);
                    else
                        check_attr (dinfo, i);
                break;
            case 'e':           /* show to which planet you have send
                                   spies */
                for (i = 0; i < MAXPL; i++)
                    if (count_spies (&pl[i], player))
                        change_attr (spies, i);
                    else
                        check_attr (spies, i);
                break;
            default:
                return;
        }
    }
}

clean_attr () {
    int     i;
    for (i = 0; i < MAXPL; i++)
        change_attr (pl[i].symbol, i);
}

change_attr (cc, i)
char    cc;
int     i;
{
    if (pl[i].d_symbol[player] != cc) {
        pl[i].d_symbol[player] = cc;
        pos (pl[i].coord[0], pl[i].coord[1] + 1);
        disch (cc);
    }
}

check_attr (cc, i)
char    cc;
{
    if (pl[i].d_symbol[player] == cc)
        change_attr (pl[i].symbol, i);
}
