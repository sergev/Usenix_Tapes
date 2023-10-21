/*Copyright (c) 1981 Gary Perlman  All rights reserved*/
#include "menu.h"
WINDOW	*subwin ();
WINDOW	*feedback;
int	neveredited = 1;
char *
linedit (s, y, x) char *s;
    {
    char	*cursormode ();
    if (neveredited)
	{
	if ((feedback = subwin (stdscr, 1, 0, LINES-1, 0)) == NULL)
	    return (NULL);
	neveredited = 0;
	}
    return (cursormode (s, y, x, feedback));
    }

char *
cursormode (s, y, base_x, feedback) char *s; WINDOW *feedback;
	{
	extern	char	varchar;
	char	command = 0;
	char	saved_s[BUFSIZ];
	char	saved[2][BUFSIZ];
	int	x = 0;
	int	answer;
	int	max_x = strlen (s) - 1;
	int	other = 0;
	strcpy (saved_s, s);
	strcpy (saved[0], s);
	strcpy (saved[1], s);
	if (*s == NULL) command = 'I'; /* auto insert */
	else
	    {
	    mvwprintw (feedback, 0, 0,
		"Cursor mode: quit with 'q', insert with 'i', undo with 'u'");
	    wclrtoeol (feedback); wrefresh (feedback);
	    }
	move (y, base_x);
	ctrlprintw (s);
	clrtoeol ();
	move (y, x+base_x);
	refresh ();
	do	{
		strcpy (saved[other], s);
		switch (command)
			{
			case 0: break;
			case ' ': /* forward one character */
			case 'l':
			case 12:
			case '+':
				if (++x > max_x) printf ("");
				break;
			case '-': /* backward one character */
			case 'h':
			case 8:
				 if (--x < 0) printf ("");
				break;
			case '^':
			case 'H': /* all the way to left */
				x = 0;
				break;
			case '$':
			case 'L': /* all the way to right */
				x = max_x;
				break;
			case 'W': /* forward one word */
			case 'w':
				if (x == max_x)
					{
					printf ("");
					break;
					}
				while (isspace (s[x])) x++;
				while (x < max_x && !isspace (s[x])) x++;
				while (isspace (s[x])) x++;
				break;
			case 'B': /* backward one word */
			case 'b':
				if (x == 0)
					{
					printf ("");
					break;
					}
				while (x && !isspace (s[x])) x--;
				while (x && isspace (s[x])) x--;
				while (x && !isspace (s[x])) x--;
				if (isspace (s[x])) x++;
				break;
			case 'C': /*change to end of line */
				clrtoeol (); refresh ();
				s[x] = NULL;
				max_x = --x;
				if (max_x < 0) max_x = 0;
			case 'A': /* append after end */
				x = max_x;
			case 'a': /* append after cursor */
				other = !other;
				if (*s == NULL) x--;
				if ((answer = insert (s, y, base_x, feedback, x+1)) == RUN)
					return (s);
				x = answer - 1;
				max_x = strlen (s) - 1;
				break;
			case 'r': /* replace char */
				if (*s == NULL) printf ("");
				else	{
					other = !other;
					s[x] = getchar ();
					}
				break;
			case 'D':
			case '0':
				other = !other;
				s[x] = NULL;
				max_x = --x;
				break;
			case 'x': /* delete character */
				other = !other;
				strcpy (s+x, s+x+1);
				max_x--;
				break;
			case 'X': /* delete line  and auto insert */
				*s = NULL;
				move (y, base_x);
				clrtoeol ();
				refresh ();
			case 'I': /* insert before start */
				x = 0;
			case 'i': /* insert before  cursor */
				other = !other;
				if ((answer = insert (s, y, base_x, feedback, x)) == RUN)
					return (s);
				x = answer;
				max_x = strlen (s) - 1;
				break;
			case 'U': /* undo all stuff on line */
				other = !other;
				strcpy (s, saved_s);
				max_x = strlen (s) - 1;
				x = 0;
				break;
			case 'u': /* undo last change */
				other = !other;
				strcpy (s, saved[other]);
				max_x = strlen (s) - 1;
				x = 0;
				break;
			case 'Q':
				wclear (feedback);
				wrefresh (feedback);
				mvprintw (y, base_x, "Line-edit escaped");
				clrtoeol ();
				refresh ();
				return (NULL);
			default: printf ("");
			}
		if (max_x < 0) x = max_x = 0;
		else if (x > max_x) x = max_x;
		else if (x < 0) x = 0;
		move (y, base_x);
		ctrlprintw (s);
		clrtoeol ();
		move (y, x+base_x);
		refresh ();
		} while ((command = getchar ()) != 'q' && command != '\n');
	return (s);
	}

insert (s, y, base_x, feedback, x) char *s; WINDOW *feedback;
	{
	int	i;
	char	before_cursor[BUFSIZ], after_cursor[BUFSIZ];
	char	insert_cursor[BUFSIZ];
	char	*ptr = insert_cursor;
	int	max_x = strlen (s) - 1;
	before_cursor[0] = insert_cursor[0] = after_cursor[0] = NULL;
	mvwprintw (feedback, 0, 0,
	    "Adding text: quit with ESC, select files with '_'");
	wclrtoeol (feedback); wrefresh (feedback);
	if (*s)
		{
		sprintf (before_cursor, "%.*s", x, s);
		strcpy (after_cursor, s+x);
		}
	else max_x = 0;
	move (y, base_x);
	ctrlprintw (s);
	move (y, x+base_x);
	refresh ();
	while ((*ptr = getchar ()) != ESC && *ptr != '\n')
	    {
	    if (*ptr == modechar)
		{
		mvwprintw (feedback, 0, 0,
		    "Selecting files: quit with '_', select files by number");
		wclrtoeol (feedback);
		wrefresh (feedback);
		*ptr = NULL;
		while ((i = getfile ()) != MODECHANGE && i != RUN)
			{
			sprintf (ptr, "%s ", filent[i].f_name);
			i = strlen (filent[i].f_name) + 1;
			x += i;
			ptr += i;
			move (y, base_x);
			ctrlprintw (before_cursor);
			ctrlprintw (insert_cursor);
			ctrlprintw (after_cursor);
			clrtoeol ();
			move (y, x+base_x);
			refresh ();
			}
		mvwprintw (feedback, 0, 0, "Adding text: quit with ESC");
		wclrtoeol (feedback);
		wrefresh (feedback);
		}
	    else if (*ptr == '')
		    {
		    if (ptr > insert_cursor)
			    {
			    *ptr = NULL;
			    ptr--;
			    *ptr = NULL;
			    x--;
			    }
		    else
			    {
			    printf ("");
			    ptr = insert_cursor;
			    *ptr = NULL;
			    }
		    }
	    else if (*ptr == ESCAPE)
		    {
		    *ptr++ = getchar ();
		    *ptr = NULL;
		    x++;
		    }
	    else	{
		    ptr++;
		    *ptr = NULL;
		    x++;
		    }
	    move (y, base_x);
	    ctrlprintw (before_cursor);
	    ctrlprintw (insert_cursor);
	    ctrlprintw (after_cursor);
	    clrtoeol ();
	    move (y, x+base_x);
	    refresh ();
	    if (i == RUN) break;
	    }
	if (*ptr == '\n') i = RUN;
	*ptr = NULL;
	sprintf (s, "%s%s%s", before_cursor, insert_cursor, after_cursor);
	strcpy (s, interpolate (s));
	mvwprintw (feedback, 0, 0,"Cursor mode: quit with 'q', abort with 'Q'");
	wclrtoeol (feedback); wrefresh (feedback);
	if (i == RUN) return (RUN);
	return (x);
	}

getfile ()
	{
	char	c;
	int	i;
	for (;;)
		{
		c = getchar ();
		if (c == '+')
			page = vdir (++page, nonames);
		else if (c == '-')
			page = vdir (--page, nonames);
		else if (isdigit (c) && c != '0')
			if ((i = c - '1' + (page-1)*PAGESIZE) >= nonames)
				printf ("");
			else	{
				mvwprintw (filewin, 1+c-'1', 2, "<-");
				wrefresh (filewin);
				return (i);
				}
		else if (c == RETURN) return (RUN);
		else if (c == modechar) return (MODECHANGE);
		else printf ("");
		}
	}

ctrlprintw (s) char *s;
	{
	while (*s)
		{
		if (iscntrl (*s))
			{
			standout ();
			addch (*s-1+'A');
			standend ();
			}
		else addch (*s);
		s++;
		}
	}
