/*
 * %W% (mrdch&amnnon) %G%
 */

# include "header"

/* if a restored game switched players i.d. */
extern  SWITCH_FLAG;
extern  nulluser;
extern int  no_new_year;
char    moreflg[2] = TWOZERO;
/* flag to indicate if player changed during game */
char    didchange[2] = TWOZERO;
int     changestat;
planet * curpln;

int     overstat (), retrieve (), pversion (),
        getoutofhere (), restorgame (), savegame (), takepl (),
        endgame (), quit (), give_up (), planetsit (), planetenq (),
        blackout (), assign (), feedpop (), takefrom (), moveto (),
        detectmv (), nodetect (), leaveat (), trading (), putalm (),
        init_disc (), killalm (), write_enemy (), build (),
        set_missile (), help (), menu (), next_year (), wizwiz (),
        no_new (), toggle_wizard (), changeplay ();

struct commands {
    char   *str;                /* the command name      */
    int     (*cfun) ();         /* the function that executes it   */
    char   *docname;            /* the file name that documents it */
    char   *exp;                /* a short explanation   */
};

struct commands ctab[] = {
    "os", overstat, "overstat",
    "Overall Situation.                  os || os [abcdefFikmnops]",
    "ps", planetsit, "plansit",
    "Planet Situation.                   ps [plid] [i]",
    "as", assign, "assign",
    "Assign people.                      as [plid] N (cfmbs) (cfmbs)",
    "en", planetenq, "planenq",
    "Enquire Planet.                     en [plid] [level] (afkmpst) T",
    "bo", blackout, "blackout",
    "Blackout a Planet.                  bo [plid] T",
    "tk", takefrom, "takefrom",
    "Take from.                          tk [plid] (k) || (N (tcfmbsv) )",
    "lv", leaveat, "leave_at",
    "Leave at.                           lv [plid] (k || K) || (N (tcfmbsv) )",
    "mv", moveto, "move_to",
    "Move to.                            mv [plid] -[plid] [>dr] NH [NH NH ...]",
    "bs", build, "build",
    "Build new ships.                    bs [plid] NH T || bs [plid] t T",
    "sm", set_missile, "set_missl",
    "Set missiles.                       sm [plid] NH",
    "dt", detectmv, "detect",
    "Detect movement / Paint ships.      dt/nd [plid] T",
    "nd", nodetect, "paint", 0,
    "lm", putalm, "putalm",
    "Lay ALM.                            lm [plid] N",
    "dm", killalm, "killalm",
    "Deactivate ALM.                     dm [plid] N",
    "fd", feedpop, "feedpop",
    "Feed population.                    fd T",
    "tr", trading, "trade",
    "Trade.                              tr T",
    "rt", retrieve, "retrieve",
    "Retrieve invested money.            rt T (tfbs) [plid]",
    "wr", write_enemy, "write",
    "Write to the enemy.                 wr [anything....+ return]",
    "mp", init_disc, "init_dis",
    "Refresh the screen.                 mp",
    "ny", next_year, "next_year",
    "Jump a year/Continue stoped game.   ny",
    "nn", no_new, "next_year",
    "Stop game for a while.              nn",
    "sv", savegame, "savegame",
    "Save / Restore Game.                sv/rs (file name)",
    "rs", restorgame, "restor", 0,
    "gu", give_up, "give_up",
    "Give up this game / Ask to quit.    gu/qt",
    "qt", quit, "quit", 0,
    "menu", menu, 0, 0,
    "help", help, "help", 0,
    "man", help, "help", 0,
    "version", pversion, 0, 0,
    "cp", changeplay, "chng_ply", "# Change player",
    "wz", wizwiz, 0, "# Get a lot of everything",
    "capt", takepl, 0, "# Take any planet you wish",
    "toggle", toggle_wizard, 0, "# Give up/regain wizardcy",
    "getout", getoutofhere, 0, "# Exit the game neatly and QUICK!!",
    0, 0, 0, 0
};

/*
 * this funstion sets the active terminal to the required .
 */
termn (n) {
    if (SWITCH_FLAG)            /* in a restored game, player switched */
        n = !n;
    if (n == 0) {
        term0 ();
    }
    else {
        term1 ();
    }
/* check if cp command was given */
    if (didchange[n]) {
        player = !player;
        changestat = 1;
    }
    else
        changestat = 0;
/* set the  current planet to the active player */
    if (!player)
        curpln = spntr;
    else
        curpln = apntr;
}

#define GETSTRMODE      1
#define CURSORMODE      2

/* in these buffers the input from both players is gathered */
char    bufs[2][100];
/* pointers into the buffers */
int     bufsp[2] = TWOZERO;
/* flags to the current position of player */
int     mode[2] = {
    CURSORMODE, CURSORMODE
};
/*
 * The main loop. Reads chars from the two processes, stores
 * them in bufs, and interprets the command. When a legal
 * command is found, the function associated with that command
 * is invoked. The nature of this program requires that ALL
 * the relevant information to a specific command to be known
 * at the time of it's execution. (Any busy wait loop will cause
 * stagnation for the SECOND player).
 */
parse () {
    struct commands *c = ctab;
    char   *ch;
    chan x;

 /* initialize first terminal */
    termn (0);
    if (ctrlinit () == 0);
    crmode ();
    noecho ();
    say ("Sir. If you feel somewhat lost, give the \"help\" command.");
    cleol (22, 11);

 /* initialize second terminal */
    termn (1);
    if (ctrlinit () == 0);
    crmode ();
    noecho ();
    say ("Sir. If you feel somewhat lost, give the \"help\" command.");
    cleol (22, 11);

/* enter main loop */
getcommand:
    for (;;) {
        c = ctab;
        (void) fflush (ttys[0]);
        (void) fflush (ttys[1]);
        if (readc (&x) != sizeof (x))
            bug ("read error on pipe.");
        termn (x.ichan);        /* set terminal to where it came from */
    /*
     *  First check if it's in "more" mode .
     *  In that case, flip back on request.
     */
        if (moreflg[player]) {
            if (x.c != ' ' && x.c != '5')
                goto getcommand;
            endmore ();
            putmsgs ('r');      /* regular printout */
            prteller ();
            goto getcommand;
        }
    /* esc char toggles the two stages */
        if (x.c == '\033') {    /* ascii esc */
            if (mode[player] == GETSTRMODE) {
                mode[player] = CURSORMODE;
                curse_map (curpln);
            }
            else {
                mode[player] = GETSTRMODE;
                curse_com (curpln);
                cleol (22, 11 + bufsp[player]);
            }
            goto getcommand;
        }
    /* in the map mode, movement and message command and given */
        if (mode[player] == CURSORMODE) {
            if (x.c == '-' || x.c == ' ') {/* delete messages stack */
                putmsgs ('-');  /* to advance rapidly */
                curse_map (curpln);
                goto getcommand;
            }
        /* good for ascii only. */
            if ((x.c > '9') || iscntrl (x.c)) {
                curse_com (curpln);
                mode[player] = GETSTRMODE;
                cleol (22, 11 + bufsp[player]);
            }
            else {
                changeloc (x.c);
                goto getcommand;
            }
        }
        if (mode[player] == GETSTRMODE) {
            if (x.c == '\n' || x.c == '\r') {
                mode[player] = CURSORMODE;
                bufs[player][bufsp[player]] = '\0';
                ch = bufs[player];
                bufsp[player] = 0;
                goto parse;
            }
            else                /* collect the chars */
                dogetstr (x.c);
        }
    }
/* find out which command was given */
parse:
    if (*ch) {
        if (Pause) {
        /* were having a pause. Execute harmless commands (no movements) */
            if (strncmp (ch, "mv", 2) == 0) {
                say ("As we take a break, I cannot move a single ship, sir.");
                cleol (22, 11);
                goto getcommand;
            }
        }
        while (c -> str != 0) {
            cleol (23, 0);
	    /* Next line called strcmpn(), which I never heard of */
            if (strncmp (ch, c -> str, strlen (c -> str)) == 0) {
            /*  enable quick return in case of error */
                if (setjmp (jparse) == 0)
                    (*c -> cfun) (ch + strlen (c -> str));
            /* update time */
                if (!Pause)
                    softclock (0);
                if (!moreflg[player]) {
                    cleol (22, 11);
                    putmsgs ('r');/* regular */
                    prteller ();
                }
                goto getcommand;
            }
            c++;
        }
        error ();
        c = ctab;
        goto getcommand;
    }
    goto getcommand;
}

extern int  month;              /* the x/100 of the year */

prteller () {
    char    s[10];

    cleol (19, 0);
    pr_at (19, 4, "Planet: %s", curpln -> pid);
    pos (19, 25);
    if (teller[player])
        print ("You have %d Tellers left, sir.", teller[player]);
    else
        print ("No money left, sir. So sorry..");
    if (month < 10)
        (void) sprintf (s, "%c%d", '0', month);
    else
        (void) sprintf (s, "%d", month);
    if (wants_newy[player])
        pr_at (19, 62, "NY");
    pr_at (19, 65, "Year : %d.%s", year, s);
    curse_map (curpln);
}

changeloc (dir)
char    dir;
{
    int     j;

 /* if the terminal has the capability, flip the page over */
    if (dir == '5')
        if (ttyc -> t_fl != 0) {
            flip ();
            more ();
            return;
        }

    if ((j = find_direct (dir)) < 0)
        return;                 /* illegal direction */
    if (curpln -> gate[j] == NULL)
        return;
    curpln = curpln -> gate[j];
    if (!player)
        spntr = curpln;
    else
        apntr = curpln;

    pr_at (19, 12, "%s", curpln -> pid);
 /* update position at screen */
    curse_map (curpln);
}

/*
 * This function prints a message and sleeps until a character
 * is striked at the terminal which executed more.
 *
 * The problem is that we can't wait to a character here becaue
 * we want to get a character from the other terminal.
 *
 * We can use the fact that all the functions
 * using more() call it just before they return to parse.
 * More will simply set a flag to tell parse that we are in
 * more mode.
 */
more () {
    so (23, 56, "Hit SPACE to continue");
    (void) fflush (tty);
    moreflg[player] = 1;
}

endmore () {
    cleol (23, 0);              /* clear the "more" message */
    moreflg[player] = 0;
    flip ();
}

/*
 * Get online help. All the commands that are listed above
 * have help files associated with them. In case that information
 * is required about some general issues, the requested file is
 * searced. Actually, all the manual can be obtained in that
 * fashion.
 */
help (s)
char   *s;
{
    FILE * docfile;
    char    nwcm[200];
    struct commands *c = ctab;

    skipwhite (s);
    if (*s == '\0') {           /* no patrameters were given */
        strcpy (nwcm, "help");
        help (nwcm);            /* call recursivly with arg  */
        return;
    }

    if (*s == '-') {            /* a general information was requested */
        (void) sprintf (nwcm, "%s%s.doc", ONLINE, ++s);
        goto show_doc;
    }

    while (c -> str != 0) {     /* didn't reach the last command */
        if (strncmp (s, c -> str, strlen (c -> str)) == 0) {
            if (c -> docname == 0)
                break;
            (void) sprintf (nwcm, "%s%s.doc", ONLINE, c -> docname);
    show_doc: docfile = fopen (nwcm, "r");
            if (docfile == NULL) {
                say ("I think you have the wrong name, sir. Try \"help\".");
                return;
            }
            flip ();
            clear ();
            while (fgets (nwcm, 200, docfile) != NULL)
                print ("%s\r", nwcm);
            (void) fclose (docfile);
            more ();
            return;
        }
        c++;
    }
    say ("But sir !!! I don't understand that command. Try \"menu\".");
}

/*
 * Extract from the command structure the information concerning
 * the syntax of the commands.
 */
menu (s)
char   *s;
{
    struct commands *c = ctab;
    int     wiz = 0;

    skipwhite (s);
    if (*s == 'w') {
        assert_wizard ();
        wiz = 1;
    }

    flip ();
    clear ();
    if (!wiz)
        print ("<plid = planet id> <N = no.> <T = Tellers> <H = ship type> <dr = direction>");
    else
        print ("This commands are only yours to give, dear lord.\n\n\r\r");
    while (c -> str != 0) {
        if (c -> exp != 0) {
            if (!wiz) {
                if (*c -> exp != '#')
                    print ("\r\n%s", c -> exp);
            }
            else {
                if (*c -> exp == '#')
                    print ("\r\n%s:\t\t%s", c -> str, c -> exp);
            }
        }
        c++;
    }
    more ();
}

error () {
    say ("Sorry sir, but I don't understand you!!");
    curse_map (curpln);
}

/*
 * This function terminates the game. If it is calles with
 * a winner, the principal facts of the game will be recorded
 * on the galaxy score file. If there is no winner, the game
 * will just end gracefully.
 */
# include       "score.h"
endgame (mode)
int     mode;
{
    int     resf;
    char   *ctime ();
    struct score    sc;
    time_t time ();

/* force the printout of games played -null */
    if (nulluser && !iswiz[1])
        mode = 1;

    if (mode == -1)
        goto noscore;

    resf = open (GALSCOR, 1);
    if (resf == -1)
        goto noscore;

    (void) lseek (resf, 0L, 2);

    if (mode) {
        strcpy (sc.win, ply1), strcpy (sc.los, ply0);
    }
    else {
        strcpy (sc.win, ply0), strcpy (sc.los, ply1);
    }

    sc.played_at = time (0);
    sc.years = year;

    (void) write (resf, (char *) & sc, sizeof (sc));
    (void) close (resf);

noscore:
    term0 ();
    doend (mode);
    term1 ();
    doend (mode);
    if (mode != -1)
        sleep (7);
    (void) kill (0, 9);
}

doend (mode) {
    static char *argv[] = {
        "IGNORED",
        "-t"
    };

    clear ();
    if (mode != -1) {
        glxscore (2, argv);
    }
    ctrlreset ();
    (void) fflush (tty);
}

/* check if the capital was invaded */
check_end () {
    int     j = capitals[0] -> whos;

    if (j == capitals[1] -> whos)
        end_game (j);
}

/*
 * A suicidal command. When all is lost.
 */
give_up () {
    end_game (!player);
}

/*
 * This function enables two players to discontinue the game
 * withourt any of them loosing face. Of course, they both
 * must agree.
 */
quit () {
    if (wants_quit[player]) {
        wants_quit[player] = 0;
        say ("Canceling last quit request.");
        return;
    }
    if (!wants_quit[!player]) {
        wants_quit[player] = 1;
        say ("Waiting for him to agree....");
        dowrite_enemy ("I would like to quit. If offer accepted, give the \"qt\" command.", !player);
        return;
    }

    endgame (-1);
}

pversion () {
    extern char version[80];

    say (version);
}

/* STOP the time */
no_new () {
    if (nulluser || iswiz[player] == 1) {
        no_new_year = 1;
        return;
    }
 /* We want to have a break! */

    if (wants_pause[player]) {
        say ("Cancelling last pause request");
        wants_pause[player] = 0;
        return;
    }
    wants_pause[player] = 1;
    if (wants_pause[!player]) {
        Pause = 1;
        no_new_year = 1;
        dowrite_enemy ("Break activated, Sir!!!", player);
        dowrite_enemy ("Break activated, Sir!!!", !player);
        return;
    }
    say ("Waiting for him to agree.....");
    dowrite_enemy ("Hey, let's take a break OK ?", !player);
}

/*
 * If the two players ask for a new year - so it will happen.
 * wizard may force one.
 */
next_year () {
    if (nulluser || iswiz[player] == 1) {
        no_new_year = 0;
        Pause = 0;
        softclock (1);
        return;
    }

    if (wants_newy[player]) {
        wants_newy[player] = 0;
        if (Pause)
            dowrite_enemy ("Sorry. I still need time.", !player);
        say ("Canceling last new_year request.");
        return;
    }

    if (!wants_newy[!player]) {
        wants_newy[player] = 1;
        if (Pause) {
            dowrite_enemy ("Let's continue the game O.K. ?", !player);
            say ("Waiting for him to agree...");
        }
        return;
    }

    wants_newy[player] = 0;
    wants_newy[!player] = 0;
    no_new_year = 0;

    if (Pause) {
        Pause = 0;
        dowrite_enemy ("There we go again!!", player);
        dowrite_enemy ("There we go again!!", !player);
        softclock (2);
        return;
    }
    softclock (1);
}
