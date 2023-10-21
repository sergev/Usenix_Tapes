/*Copyright (c) 1981 Gary Perlman  All rights reserved*/
#include "menu.h"
#define skipspace(ptr) while (isspace (*ptr)) ptr++;
internalrun (chosenmenu, chosen) struct MENU *chosenmenu;
    {
    char	syscommand[BUFSIZ];
    char	buf[BUFSIZ], *bufptr = buf;
    char	*strptr;
    char	comm = chosenmenu->program[chosen][1];
    int	i;
    if (docmode && comm != 'd')
	{
	docmode = 0;
	docinternal (comm);
	flipped = 1;
	return;
	}
    switch (comm)
	{
	case '@':
		image ();
		return;
	case 'f':
		if (flipped) leftdisplay (menu);
		else leftdisplay (stdmenu);
		flipped = !flipped;
		return;
	case 'p':
		popchar = chosenmenu->selector[chosen];
		flipped = 0;
		menu = menu->parent;
		if (ttyspeed > B9600)
			leftdisplay (menu);
		else newmenu = 1;
		return;
	case 'a':
		if (anchored)
			{
			menu = savemenu;
			newmenu = 1;
			if (ttyspeed > B9600)
			    leftdisplay (menu);
			move (NOTICES-2, RIGHTMENU); clrtoeol (); refresh ();
			}
		else	{
			savemenu = menu;
			mvprintw (NOTICES-2, RIGHTMENU, "On a diversion from ");
			if (trueval ("highlight")) standout ();
			printw ("[%s]", savemenu->menuname);
			if (trueval ("highlight")) standend ();
			refresh ();
			menu = rootmenu;
			if (ttyspeed > B9600)
			    leftdisplay (menu);
			else newmenu = 1;
			}
		anchored = !anchored;
		flipped = 0;
		return;
	case 'u':
		unixchar = chosenmenu->selector[chosen];
		if (anchored)
		    {
		    move (NOTICES-2, RIGHTMENU);
		    clrtoeol ();
		    refresh ();
		    }
		anchored = flipped = 0;
		menu = rootmenu;
		if (ttyspeed > B9600)
		    leftdisplay (menu);
		else newmenu = 1;
		return;
	case 'c':
		mvprintw (INFOLINE, 0, chosenmenu->display[chosen]);
		clrtoeol ();
		if (cd (getargs (chosenmenu->arguments[chosen], NULL)))
			{
			mvprintw (INFOLINE,0, "Can't change directory");
			clrtoeol ();
			refresh ();
			}
		else page = vdir (page=1, nonames = newdir ());
		return;
	case '0':
		dotdotchar = chosenmenu->selector[chosen];
		cd ("..");
		page = vdir (page=1, nonames=newdir ());
		return;
	case '+':
		page = vdir (++page, nonames);
		return;
	case '-':
		page = vdir (--page, nonames);
		return;
	case 'r': display (menu); flipped = 0; return;
	case 'd':
		if (docmode = !docmode)
		  mvprintw (NOTICES-3, RIGHTMENU, "Next selection gets documentation");
		else
		  mvprintw (NOTICES-3, RIGHTMENU, "Next selection runs program");
		clrtoeol ();
		refresh ();
		return;
	case 's': 
		shellchar = chosenmenu->selector[chosen];
		mvprintw (INFOLINE, 0, chosenmenu->display[chosen]);
		clrtoeol ();
		strptr = getargs (chosenmenu->arguments[chosen], NULL);
		if (strptr == NULL) return;
		while (isspace (*strptr)) strptr++;
		if (*strptr != NULL)
		    strcpy (variables[COMMAND].value, alias (strptr, "|;"));
		syscall (variables[COMMAND].value);
		GETRETURN
		display (menu);
		return;
	case 'v':
		varchar = chosenmenu->selector[chosen];
		mvprintw (INFOLINE, 0, "Setting variable");
		clrtoeol ();
		refresh ();

		if ((strptr = getargs ("{name}", NULL)) == NULL)
			return;
		else strptr = copy (strptr);
		skipspace (strptr);
		for (i = 0; i < nvar; i++)
			if (!strcmp (strptr, variables[i].name)) break;
		if (i == nvar)
		    {
		    if (nvar+1 > MAXVAR)
			    {
			    mvprintw (INFOLINE, 0,"No room for more variables");
			    clrtoeol ();
			    refresh ();
			    return;
			    }
		    mvprintw (INFOLINE, 0, "Setting new variable");
		    variables[i].name = copy (strptr);
		    }
		else
			mvprintw (INFOLINE, 0, "Changing old variable");
		clrtoeol ();
		refresh ();
		if ((strptr = getargs ("{value}", variables[i].value)) == NULL)
			return;
		else strptr = copy (strptr);
		skipspace (strptr);
		variables[i].value = strptr;
		mvprintw (INFOLINE, 0, "%c%s=%s",
			varchar, variables[i].name, variables[i].value);
		clrtoeol ();
		refresh ();
		lastcomm ();
		if (i == nvar) nvar++;
		return;
	case 'i':
		mvprintw (INFOLINE, 0, chosenmenu->display[chosen]);
		clrtoeol ();
		strptr = getargs (chosenmenu->arguments[chosen], NULL);
		if (*strptr)
		    {
		    mvprintw (INFOLINE, 0, interpolate (strptr));
		    clrtoeol ();
		    refresh ();
		    }
		else
		    {
		    FILE *popen(), *ioptr;
		    if (ioptr = popen ("/usr/ucb/more", "w"))
			{
			clear (); refresh ();
			nocrmode (); echo ();
			for (i = 0; i < nvar; i++)
			    fprintf (ioptr, "%10s=%s\n",
				variables[i].name, variables[i].value);
			pclose (ioptr);
			GETRETURN
			clear (); refresh ();
			crmode (); noecho ();
			display (menu);
			}
		    }
		return;
	default:
		mvprintw (INFOLINE, 0, "Unknown internal command\n");
		clrtoeol ();
		refresh ();
		return;
	}
    }

docinternal (command) char command;
	{
	FILE	*fopen (), *ioptr;
	char	*getval (), *menu = getval ("menu");
	char	docfile[BUFSIZ];
	char	line[BUFSIZ];
	sprintf (docfile, "%s/../doc/internal", menu);
	move (HISTORY, 0); clrtobot (); move (HISTORY, 0); refresh ();
	if (ioptr = fopen (docfile, "r"))
		{
		while (fgets (line, BUFSIZ, ioptr))
			if (*line == command)
				fputs (line+1, stdout);
		fclose (ioptr);
		}
	GETRETURN
	display (stdmenu);
	}
