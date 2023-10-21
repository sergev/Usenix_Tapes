/*
 * %W% (mrdch&amnnon) %G%
 */

# include "header"

char    s1[] = "We are on our way to a BETTER LAND... let GOD be with us next time.";
extern char moreflg[] ;

/*
 * When the game ends, this function is calles to show it
 * in a rather dramatic way.  The looser sees his galaxy vanishing,
 * while the winner gets a satisfactory view of the galaxy that
 * transforms to his original sign.
 */
end_game (winplay) {
    char    winc = '*'; /* the default winning sign */
    /* keeps the new coordinates of vanishing planets */
    int     newcord[2][MAXPL];
    int     i,
            j,
            m,
            captr;
    char   *ps1 = s1;
    int     line = 19,
            col;

    if (winplay)
        winc = '@';

/* notify the winner */
    termn (winplay);
    if (moreflg[winplay])
        endmore() ;
    cleol (19, 0);
    disch ('\007');
    say ("Sir!!! We won the battle!!! The enemy runs away!!!");
    disch ('\007');
    (void) fflush (tty);

/* notify the looser */
    termn (!winplay);
    if (moreflg[!winplay])
        endmore() ;
    disch ('\007');
    say ("Dear sir, we have lost everything. I wish you best luck next time...");
    disch ('\007');
    cleol (line, 0);

/* back to the winner */
    termn (winplay);
    so (19, 20, " Our forces are taking over the two Galaxies...");
    (void) fflush (tty);

/* and again the looser */
    termn (!winplay);
    for (i = 0; i < MAXPL; i++) {
        pos (pl[i].coord[0], pl[i].coord[1] + 1);
        newcord[0][i] = pl[i].coord[0];
        newcord[1][i] = pl[i].coord[1] + 1;
        pl[i].d_symbol[player] = winc;
        pl[i].d_symbol[!player] = winc;
    }
/* display the comforting line to the looser */
    col = 5;
    so (line--, col, ps1);
    col += 3;
    ps1 += 3;
/* make it succesivly shorter */
    s1[strlen (s1) - 3] = '\0';
    for (i = 0; i < MAXPL; i++) {
        pos (newcord[0][i], newcord[1][i]);
        disch (winc);
        pos (newcord[0][i], newcord[1][i] - 1);
        disch (' ');
        pos (newcord[0][i], newcord[1][i] + 1);
        disch (' ');
    }
    captr = 0;
    j = 20;
    while (j) {
        if (j < 14 && captr < MAXPL) {
            termn (winplay);
            for (m = 0; m < 7; m++) {
                curse_com (&pl[captr]);
                captr++;
            }
            (void) fflush (tty);
            termn (!winplay);
        }
        if (j % 2) {
            cleol (line, 0);
            so (line--, col, ps1);
            col += 3;
            ps1 += 3;
            s1[strlen (s1) - 3] = '\0';
        }
/* place the vanishing planets in their new position on screen */
        for (i = 0; i < MAXPL; i++) {
            pos (newcord[0][i], newcord[1][i]);
            disch (' ');
            if (newcord[0][i] > 8)
                newcord[0][i]--;
            else
                if (newcord[0][i] < 8)
                    newcord[0][i]++;
            if (newcord[1][i] - 39 >= 2)
                newcord[1][i] -= 2;
            else
                if (newcord[1][i] - 39 == 1)
                    newcord[1][i]--;
                else
                    if (39 - newcord[1][i] >= 2)
                        newcord[1][i] += 2;
                    else
                        if (39 - newcord[1][i] == 1)
                            newcord[1][i]++;
            pos (newcord[0][i], newcord[1][i]);
            disch (winc);
            (void) fflush (tty);
        }
        j--;
    }
    (void) fflush (tty);
    termn (winplay);
    endgame (winplay);
}
