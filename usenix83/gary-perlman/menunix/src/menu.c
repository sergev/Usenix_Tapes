/*Copyright (c) 1981 Gary Perlman  All rights reserved*/
#include "menu.defs"
main (argc, argv) char **argv;
	{
	initial (argc, argv);
	process ();
	}

#include <signal.h>
char	*getenv ();
initial (argc, argv) char **argv;
	{
	int	i;
	struct	sgttyb ttystat;
	extern	finish (), howquit ();
	char	*term	=getenv ("TERM");
	WINDOW	*subwin ();
	if (!movecursor (term))
		{
		printf ("This terminal (%s) can't run menus\n");
		exit (1);
		}
	initscr ();
	signal (SIGINT, howquit);
	uid = getuid ();
	gid = getgid ();
	gtty (fileno (stdout), &ttystat);
	ttyspeed = ttystat.sg_ospeed;
	pwd (pwdname);
	nonames = newdir ();
	if (argc > 1) strcpy (menudir, argv[1]);
	readvar ();
	rootmenu = menu->parent = menu = readmenu ("UNIX", "UNIX");
	stdmenu = readmenu ("CONTROL", "CONTROL");
	lmenu = subwin (stdscr, MAXOPTION+1, RIGHTMENU-1, 0, 0);
	filewin = subwin (stdscr, PAGESIZE+1, COLS/2, 0, COLS/2);
	history = subwin (stdscr, INFOLINE-HISTORY-1, 0, HISTORY, 0);
	timewin = subwin (stdscr, 1, 0, NOTICES, RIGHTMENU);
	display (menu);
	crmode (); noecho ();
	}

process ()
	{
	int	c;
	int	chosen;
	for (;;)
		{
		if ((c = timegetc (1)) == 0)
			{
			if (newmenu) leftdisplay (menu);
			printtime ();
			}
		else if (isdigit (c) && c != '0')
			(fileprocess (c));
		else if ((chosen = choose (c, stdmenu)) != OUT_OF_RANGE)
			run (stdmenu, chosen);
		else if ((chosen = choose (c, menu)) == OUT_OF_RANGE)
			{
			mvprintw (INFOLINE, 0, "BREAK to quit.  Type & for [CONTROL] commands");
			clrtoeol ();
			refresh ();
			printf ("");
			}
		else if (menu->nextmenu[chosen])
			{
			menu = menu->nextmenu[chosen];
			if (ttyspeed > B9600)
				leftdisplay (menu);
			else newmenu = 1;
			flipped = 0;
			}
		else run (menu, chosen);
		}
	}

finish ()
	{
	signal (SIGINT, SIG_IGN);
	alarm (0);
	clear ();
	refresh ();
	endwin ();
	exit (0);
	}

howquit ()
	{
	extern	finish (), howquit ();
	alarm (0);
	signal (SIGINT, SIG_IGN);
	if (trueval ("highlight")) standout ();
	mvprintw (INFOLINE, 0, "Type RETURN to return to menu, BREAK to exit");
	if (trueval ("highlight")) standend ();
	clrtoeol (); refresh ();
	crmode ();
	signal (SIGINT, finish);
	while (getchar () != '\n') printf ("");
	signal (SIGINT, howquit);
	move (INFOLINE, 0); clrtoeol (); refresh ();
	display (menu);
	process ();
	}

choose (ch, thismenu) struct MENU *thismenu;
	{
	int	i;
	int	chosen = OUT_OF_RANGE;
	for (i = 0; i < thismenu->noptions; i++)
		if (ch == thismenu->selector[i]) chosen = i;
	if (chosen == OUT_OF_RANGE) return (chosen);
	if (thismenu == stdmenu) return (chosen);
	for (i = 0; i < thismenu->noptions; i++)
		mvwprintw (lmenu, i + 1, 2, "  ");
	if (!newmenu) mvwprintw (lmenu, chosen + 1, 2, "<-");
	wrefresh (lmenu);
	return (chosen);
	}

