
/* RCS Info: $Revision: 1.2 $ on $Date: 86/04/02 10:33:25 $
 *           $Source: /ic4/faustus/src/spellfix/RCS/getbuf.c,v $
 * Copyright (c) 1985 Steve Procter
 */

#include <sys/ioctl.h>
#include <ctype.h>
#include <curses.h>

#define MAX(a,b)	((a) > (b) ? (a) : (b))
#define ISMETA(a)	((a) & 0x80)

char *
getbuf ()
{
    int     x;
    int     y;
    char    c;
    char    *s;
    char    *makeprint ();
    register    i;
    register    current = 0;
    register    offset = 0;
    static char string[BUFSIZ];
    struct sgttyb   sg, osg;
    struct ltchars  lt, olt;

    rewind (stdin);
    wrefresh (stdscr);
    getyx (stdscr, y, x);
    bzero (string, sizeof (string));

    ioctl (fileno (stdin), TIOCGETP, &sg);
    bcopy (&sg, &osg, sizeof (struct sgttyb));
    sg.sg_flags |= CRMOD;
    sg.sg_flags &= ~ECHO;
    ioctl (fileno (stdin), TIOCSETP, &sg);

    ioctl (fileno (stdin), TIOCGLTC, &lt);
    bcopy (&lt, &olt, sizeof (struct ltchars));

    while (((c = getchar ()) != '\015') && (c != '\n')) {
	c &= 0177;
	if (c == sg.sg_kill) {
	    current = 0;
	    offset = 0;
	    wmove (stdscr, y, x);
	    wclrtoeol (stdscr);
	    bzero (string, sizeof (string));
	}
	else if (c == sg.sg_erase) {
	    if (current <= 0) {
		current = 0;
		offset = 0;
		wmove (stdscr, y, x);
		wclrtoeol (stdscr);
		continue;
	    }
	    offset -= strlen (makeprint (string[--current]));
	    string[current] = NULL;
	    wmove (stdscr, y, x + offset);
	    wclrtoeol (stdscr);
	}
	else if (c == lt.t_werasc) {
	    if (current <= 1) {
		current = 0;
		offset = 0;
	    }
	    while ((current - 1 >= 0) && (string[current - 1] == ' ')) {
		string[--current] = NULL;
		offset--;
	    }
	    while ((current - 1 >= 0) && (string[current - 1] != ' ')) {
		current--;
		offset -= strlen (makeprint (string[current]));
		string[current] = NULL;
	    }
	    wmove (stdscr, y, x + offset);
	    wclrtoeol (stdscr);
	}
	else if (c == lt.t_rprntc) {
	    wmove (stdscr, y, x);
	    for (i = 0; i < current; i++) {
		addstr (makeprint (string[i]));
	    }
	}
	else {
	    if (isprint (c)) {
		string[current++] = c;
		wmove (stdscr, y, x + offset);
		waddch (stdscr, c);
		offset++;
	    }
	    else {
		s = makeprint (c);
		wmove (stdscr, y, x + offset);
		waddstr (stdscr, s);
		string[current++] = c;
		offset += strlen (s);
	    }
	}
	wrefresh (stdscr);
    }
    ioctl (fileno (stdin), TIOCSETP, &osg);
    ioctl (fileno (stdin), TIOCSLTC, &olt);
    return (string);
}

char *
makeprint (c)
char    c;
{
    static char r[5];

    bzero (r, sizeof (r));
    if (ISMETA(c)) {
	strcat (r, "^[");
	return (r);
    }
    if (iscntrl (c)) {
	sprintf (r, "^%c", (c | 0x40) & 0x7f);
	return (r);
    }
    r[0] = c;
    return (r);
}

