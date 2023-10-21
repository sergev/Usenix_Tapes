/*Copyright (c) 1981 Gary Perlman  All rights reserved*/
#include "menu.h"
leftdisplay (menu) struct MENU *menu;
	{
	int i;
	extern	int flipped;
	if (menu == NULL) return;
	wclear (lmenu);
	if (trueval ("highlight")) wstandout (lmenu);
	mvwprintw (lmenu, 0, 0, "[%s]", menu->menuname);
	if (trueval ("highlight")) wstandend (lmenu);
	for (i = 0; i < menu->noptions; i++)
		{
		if (menu->nextmenu[i])
			{
			if (trueval ("highlight")) wstandout (lmenu);
			mvwprintw (lmenu, i+1, INDENT, "[%s]", menu->display[i]);
			if (trueval ("highlight")) wstandend (lmenu);
			}
		else
			{
			mvwprintw (lmenu, i+1, INDENT, menu->display[i]);
			wprintw (lmenu, " (%s)", menu->program[i]);
			}
		if (iscntrl (menu->selector[i]))
		mvwprintw (lmenu, i+1, 0,"^%c",menu->selector[i]-1+'A');
		else mvwaddch (lmenu, i+1, 0, menu->selector[i]);
		}
	wrefresh (lmenu);
	newmenu = 0;
	}

lastcomm ()
	{
	int	i;
	wclear (history);
	for (i = 1; ; i++)
		if (!*variables[i].value) continue;
		else if (ERR == mvwprintw (history, i-1, 0, "%c%s %s",
			varchar, variables[i].name, variables[i].value)) break;
	wrefresh (history);
	}

#include <time.h>
char	*months[] =	{ "January", "February", "March", "April",
			"May", "June", "July", "August",
			"September", "October", "November", "December" };

char	*days[] =	{ "Sunday", "Monday", "Tuesday", "Wednesday",
			"Thursday", "Friday", "Saturday"};
printtime ()
	{
	struct	tm *date;
	long	clock;
	int	hour;

	time (&clock);
	date = (struct tm *) localtime (&clock);
	if ((hour = date->tm_hour) > 12) hour %= 12;
	if (date->tm_sec == 0)
		checkmail (mailfile);
	mvwprintw (timewin, 0, 0, "%s, %s %d.  %d:%02d:%02d",
		days[date->tm_wday], months[date->tm_mon], date->tm_mday,
		hour, date->tm_min, date->tm_sec);
	wrefresh (timewin);
	}

checkmail (mailfile) char *mailfile;
	{
	struct	stat statbuf;
	if (stat (mailfile, &statbuf)) return;
	if (statbuf.st_size)
	    {
	    if (statbuf.st_atime > statbuf.st_mtime)
		mvprintw (NOTICES-1, RIGHTMENU, "You have mail");
	    else
		mvprintw (NOTICES-1, RIGHTMENU, "You have new mail");
	    printw (" (%d bytes)", statbuf.st_size);
	    }
	clrtoeol ();
	refresh ();
	}

display (menu) struct MENU *menu;
	{
	int i;
	if (menu == NULL) return;
	clear (); refresh ();
	printtime ();
	nonames = newdir ();
	page = vdir (page, nonames);
	leftdisplay (menu);
	lastcomm ();
	checkmail (mailfile);
	if (docmode)
		{
		mvprintw (NOTICES-3, RIGHTMENU, "Next selection get documentation");
		clrtoeol ();
		}
	if (anchored)
		{
		mvprintw (NOTICES-2, RIGHTMENU, "On a diversion from ");
		if (trueval ("highlight")) standout ();
		printw ("[%s]", savemenu->menuname);
		if (trueval ("highlight")) standend ();
		clrtoeol ();
		}
	mvwprintw (lmenu, 0, 0, "[%s]", menu->menuname);
	refresh ();
	}
