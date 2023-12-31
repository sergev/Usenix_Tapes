/*
 * #     #
 * #    #   #    #  #   ##  #    #   # 
 *		         #
 *
 * Exploring the dungeons of doom
 * Copyright (C) 1981 by Michael Toy, Ken Arnold, and Glenn Wichman
 * All rights reserved
 *
 * @(#)main.c	4.4 (NMT from Berkeley 5.2) 8/25/83
 */

#include <curses.h>
#ifdef	attron
#include <term.h>
#endif	attron
#include <signal.h>
#include <pwd.h>
#include "rogue.h"

/*
 * main:
 *	The main program, of course
 */
main(argc, argv, envp)
char **argv;
char **envp;
{
    register char *env;
    register struct passwd *pw;
    struct passwd *getpwuid();
    char *getpass(), *crypt();
    int quit(), exit(), lowtime;

#ifndef DUMP
    signal(SIGQUIT, exit);
    signal(SIGILL, exit);
    signal(SIGTRAP, exit);
    signal(SIGIOT, exit);
    signal(SIGEMT, exit);
    signal(SIGFPE, exit);
    signal(SIGBUS, exit);
    signal(SIGSEGV, exit);
    signal(SIGSYS, exit);
#endif

#ifdef WIZARD
    /*
     * Check to see if he is a wizard
     */
    if (argc >= 2 && argv[1][0] == '\0')
	if (strcmp(PASSWD, crypt(getpass("Wizard's password: "), "mT")) == 0)
	{
	    wizard = TRUE;
	    player.t_flags |= SEEMONST;
	    argv++;
	    argc--;
	}
#endif

    /*
     * get home and options from environment
     */
    if ((env = getenv("HOME")) != NULL)
	strcpy(home, env);
    else if ((pw = getpwuid(getuid())) != NULL)
	strcpy(home, pw->pw_dir);
    else
	home[0] = '\0';
    strcat(home, "/");

    strcpy(file_name, home);
    strcat(file_name, "rogue.save");

    if ((env = getenv("ROGUEOPTS")) != NULL)
	parse_opts(env);
    if (env == NULL || whoami[0] == '\0')
	if ((pw = getpwuid(getuid())) == NULL)
	{
	    printf("Say, who the hell are you?\n");
	    exit(1);
	}
	else
	    strucpy(whoami, pw->pw_name, strlen(pw->pw_name));
    if (env == NULL || fruit[0] == '\0')
	strcpy(fruit, "slime-mold");

    /*
     * check for print-score option
     */
    open_score();
    if (argc == 2 && strcmp(argv[1], "-s") == 0)
    {
	noscore = TRUE;
	score(0, -1);
	exit(0);
    }
    init_check();			/* check for legal startup */
    if (argc == 2)
	if (!restore(argv[1], envp))	/* Note: restore will never return */
	    exit(1);
    lowtime = (int) time(NULL);
#ifdef WIZARD
    if (wizard && getenv("SEED") != NULL)
	dnum = atoi(getenv("SEED"));
    else
#endif
	dnum = lowtime + getpid();
#ifdef WIZARD
    if (wizard)
	printf("Hello %s, welcome to dungeon #%d", whoami, dnum);
    else
#endif
	printf("Hello %s, just a moment while I dig the dungeon...", whoami);
    fflush(stdout);
    seed = dnum;

    init_player();			/* Set up initial player stats */
    init_things();			/* Set up probabilities of things */
    init_names();			/* Set up names of scrolls */
    init_colors();			/* Set up colors of potions */
    init_stones();			/* Set up stone settings of rings */
    init_materials();			/* Set up materials of wands */
    initscr();				/* Start up cursor package */
    setup();
    /*
     * Set up windows
     */
    hw = newwin(LINES, COLS, 0, 0);
#ifdef	attron
    idlok(stdscr, TRUE);
    idlok(hw, TRUE);
#endif	attron
#ifdef WIZARD
    noscore = wizard;
#endif
    new_level();			/* Draw current level */
    /*
     * Start up daemons and fuses
     */
    daemon(doctor, 0, AFTER);
    fuse(swander, 0, WANDERTIME, AFTER);
    daemon(stomach, 0, AFTER);
    daemon(runners, 0, AFTER);
    playit();
}

/*
 * endit:
 *	Exit the program abnormally.
 */
endit()
{
    fatal("Ok, if you want to exit that badly, I'll have to allow it\n");
}

/*
 * fatal:
 *	Exit the program, printing a message.
 */
fatal(s)
char *s;
{
    clear();
    move(LINES-2, 0);
    printw("%s", s);
    refresh();
    endwin();
    exit(0);
}

/*
 * rnd:
 *	Pick a very random number.
 */
rnd(range)
register int range;
{
    return range == 0 ? 0 : abs((int) RN) % range;
}

/*
 * roll:
 *	Roll a number of dice
 */
roll(number, sides)
register int number, sides;
{
    register int dtotal = 0;

    while (number--)
	dtotal += rnd(sides)+1;
    return dtotal;
}
#ifdef SIGTSTP
/*
 * tstp:
 *	Handle stop and start signals
 */
tstp()
{
    register int y, x;
    register int oy, ox;

    getyx(curscr, oy, ox);
    mvcur(0, COLS - 1, LINES - 1, 0);
    endwin();
    fflush(stdout);
    kill(0, SIGTSTP);
    signal(SIGTSTP, tstp);
    crmode();
    noecho();
    clearok(curscr, TRUE);
    wrefresh(curscr);
    getyx(curscr, y, x);
    mvcur(y, x, oy, ox);
    fflush(stdout);
    curscr->_cury = oy;
    curscr->_curx = ox;
}
#endif

/*
 * playit:
 *	The main loop of the program.  Loop until the game is over,
 *	refreshing things and looking at the proper times.
 */
playit()
{
    register char *opts;

    /*
     * set up defaults for slow terminals
     */

#ifndef	attron
/*HMS:    if (_tty.sg_ospeed <= B1200)	*/
    if ((_tty.c_cflag & CBAUD) <= B1200)
#else	attron
    if (baudrate() <= 1200)
#endif	attron
    {
	terse = TRUE;
	jump = TRUE;
    }
#ifndef	attron
    if (!CE)
#else	attron
    if (clr_eol)
#endif	attron
	inv_type = INV_CLEAR;

    /*
     * parse environment declaration of options
     */
    if ((opts = getenv("ROGUEOPTS")) != NULL)
	parse_opts(opts);


    oldpos = hero;
    oldrp = roomin(&hero);
    while (playing)
	command();			/* Command execution */
    endit();
}

/*
 * quit:
 *	Have player make certain, then exit.
 */
quit()
{
    register int oy, ox;

    /*
     * Reset the signal in case we got here via an interrupt
     */
    if (signal(SIGINT, quit) != quit)
	mpos = 0;
    getyx(curscr, oy, ox);
    msg("really quit?", (char *)NULL);
    if (readchar() == 'y')
    {
	signal(SIGINT, leave);
	clear();
	mvprintw(LINES - 2, 0, "You quit with %d gold pieces", purse);
	move(LINES - 1, 0);
	refresh();
	score(purse, 1);
	exit(0);
    }
    else
    {
	move(0, 0);
	clrtoeol();
	status();
	move(oy, ox);
	refresh();
	mpos = 0;
	count = 0;
    }
}

/*
 * leave:
 *	Leave quickly, but curteously
 */
leave()
{
#ifndef	attron
    if (!_endwin)
    {
	mvcur(0, COLS - 1, LINES - 1, 0);
	endwin();
    }
#else	attron
    endwin();
#endif	attron
    putchar('\n');
    exit(0);
}

/*
 * shell:
 *	Let him escape for a while
 */
shell()
{
    register int pid;
    register char *sh;
    int ret_status;

    /*
     * Set the terminal back to original mode
     */
    move(LINES-1, 0);
    refresh();
    endwin();
    putchar('\n');
    in_shell = TRUE;
    after = FALSE;
    sh = getenv("SHELL");
    fflush(stdout);
    /*
     * Fork and do a shell
     */
    while ((pid = fork()) < 0)
	sleep(1);
    if (pid == 0)
    {
	execl(sh == NULL ? "/bin/sh" : sh, "shell", "-i", 0);
	perror("No shelly");
	exit(-1);
    }
    else
    {
	int endit();

	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	while (wait(&ret_status) != pid)
	    continue;
	signal(SIGINT, quit);
	signal(SIGQUIT, endit);
	printf("\n[Press return to continue]");
	noecho();
	crmode();
	in_shell = FALSE;
	wait_for('\n');
	clearok(stdscr, TRUE);
    }
}
