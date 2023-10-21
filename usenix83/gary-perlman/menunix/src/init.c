/*Copyright (c) 1981 Gary Perlman  All rights reserved*/
#include "menu.h"
struct MENU *
readmenu (filename, header) char *filename; char *header;
	{
	char	*copy ();
	FILE	*ioptr;
	char	line[BUFSIZ];
	char	file[100];
	char	*p, *getvalue ();
	struct MENU *menu = (struct MENU *) calloc (1, sizeof (struct MENU));
	sprintf (file, "%s/%s", menudir, filename);
	if (menu == NULL)
		{ fprintf (stderr, "readmenu: out of space\n"); exit (1); }
	ioptr = xopen (file, "r");
	menu->menuname = copy (header);
	while (p = fgets (line, BUFSIZ, ioptr))
		{
		while (p = getvalue (p, menu));
		if (++menu->noptions > MAXOPTION)
			{
			fprintf (stderr, "readmenu: Too many options\n");
			exit (1);
			}
		}
	fclose (ioptr);
	return (menu);
	}
char *
getvalue (line, menu) char *line; struct MENU *menu;
	{
	char	valuebuf[BUFSIZ];
	char	designator, *value = valuebuf;
	*value = NULL;
	skipspace (line);
	if (!begin (*line++)) return (NULL);
	skipspace (line);
	designator = *line;
	while (!separator (*line))
		if (end (*line)) goto check;
		else line++;
	line++;
	while (!end (*line)) *value++ = *line++;
	*value = NULL;
	line++;
check:
	if (designator != 's')
		value = copy (valuebuf);
	switch (designator)
	    {
	    case 'd': menu->display[menu->noptions] = value; break;
	    case 's': menu->selector[menu->noptions] = *valuebuf; break;
	    case 'm': menu->nextmenu[menu->noptions]
		    = readmenu (value, menu->display[menu->noptions]);
		    menu->nextmenu[menu->noptions]->parent = menu;
		    break;
	    case 'p': menu->program[menu->noptions] = value; break;
	    case 'a': menu->arguments[menu->noptions] = value; break;
	    case 'w': menu->nowait[menu->noptions] = TRUE; break;
	    default:
		fprintf (stderr, "getvalue: bad designator '%c'\n", designator);
	    }
	return (line);
	}

readvar ()
	{
	char	line[BUFSIZ], *lineptr;
	char	namebuf[BUFSIZ], *nameptr;
	char	valuebuf[BUFSIZ], *valueptr;
	char	*interpolate ();
	char	*getenv ();
	FILE	*ioptr;
	int	i;
	for (i = 0; i < MAXCOM; i++)
		{
		variables[i].value = commandbuffer[i];
		variables[i].name  = copy (" ");
		*variables[i].name = '0' + i;
		}
	nvar = MAXCOM;
	variables[nvar].name	= copy ("dir");
	variables[nvar++].value	= pwdname;
	variables[nvar].name	= copy ("menu");
	variables[nvar++].value	= copy (menudir);
	variables[nvar].name	= copy ("home");
	variables[nvar++].value	= getenv ("HOME");
	variables[nvar].name	= copy ("user");
	variables[nvar++].value	= getenv ("USER");
	sprintf (line, "%s/%s", maildir, getenv ("USER"));
	variables[nvar].name	= copy ("mail");
	variables[nvar++].value	= mailfile = copy (line);
	sprintf (line, "%s/%s", getenv ("HOME"), ".menuvar");
	if (access (line, 4))
		sprintf (line, "%s/setup/menunix", getenv ("HOME"));
	if (ioptr = fopen (line, "r"))
	    {
	    while (fgets (line, BUFSIZ, ioptr))
		{
		lineptr = line;
		nameptr = namebuf; *nameptr = NULL;
		valueptr = valuebuf; *valueptr = NULL;
		skipspace (lineptr);
		while (isalnum (*lineptr))
			*nameptr++ = *lineptr++;
		*nameptr = NULL;
		skipspace (lineptr);
		if (*lineptr == '=') lineptr++;
		skipspace (lineptr);
		while (*lineptr != '\n')
			*valueptr++ = *lineptr++;
		*valueptr = NULL;
		variables[nvar].name = copy (namebuf);
		variables[nvar].value = copy (interpolate (valuebuf));
		nvar++;
		}
	    fclose (ioptr);
	    }
	variables[nvar].name	= copy ("editor");
	variables[nvar++].value = copy ("ex");
	variables[nvar].name	= copy ("printdotfiles");
	variables[nvar++].value	= copy ("true");
	variables[nvar].name	= copy ("shell");
	variables[nvar++].value	= copy (getenv ("SHELL"));
	}
