/*Copyright (c) 1981 Gary Perlman  All rights reserved*/
#include "menu.h"
run (chosenmenu, chosen) struct MENU *chosenmenu;
	{
	char	syscommand[BUFSIZ];
	char	buf[BUFSIZ], *bufptr = buf;
	char	*strptr;
	int	i;
	if (!chosenmenu->program[chosen]) return;
	if (*chosenmenu->program[chosen] == '-')
		{
		internalrun (chosenmenu, chosen);
		return;
		}
	mvprintw (INFOLINE, 0, "COMMAND: %s %s", chosenmenu->program[chosen],
		chosenmenu->arguments[chosen]);
	clrtoeol ();
	if (docmode)
		{
		docmode = 0;
		strptr = chosenmenu->program[chosen];
		sprintf (variables[COMMAND].value, "man %s", strptr);
		mvprintw (RESPLINE, 0, "Getting documentation on %s, please wait", strptr);
		clrtoeol ();
		move (MENUBOTTOM, 0);
		refresh ();
		syscall (variables[COMMAND].value);
		GETRETURN
		display (menu);
		return;
		}
	if (chosenmenu->arguments[chosen])
		{
		if ((strptr = getargs (chosenmenu->arguments[chosen], NULL)) == NULL)
			{ noecho (); crmode (); return; }
		else sprintf (variables[COMMAND].value, "%s %s", 
			chosenmenu->program[chosen], strptr);
		}
	else sprintf (variables[COMMAND].value, "%s", chosenmenu->program[chosen]);
	clear (); refresh ();
	syscall (variables[COMMAND].value);
	if (!chosenmenu->nowait[chosen]) GETRETURN
	if (anchored)
	    {
	    anchored = 0;
	    display (menu = savemenu);
	    return;
	    }
	for (i = MAXCOM-1; i > 0; i--)
		variables[i].value = variables[i-1].value;
	variables[COMMAND].value = variables[MAXCOM-1].value;
	display (menu);
	}
