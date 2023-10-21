/*
 *  Top - a top users display for Berkeley Unix
 *
 *  This file contains the routines that interface to termcap and stty/gtty.
 */

#include <stdio.h>
#include <sgtty.h>
#include "screen.h"
#include "boolean.h"

extern char *myname;

int putstdout();

int  scrolls;
int  hardcopy;
int  screen_length;
int  screen_width;
char ch_erase;
char ch_kill;
char smart_terminal;
char PC;
char *tgetstr();
char *tgoto();
char termcap_buf[1024];
char init_buf[1024];
char string_buffer[1024];
char home[15];
char lower_left[15];
char *clear_line;
char *clear_screen;
char *cursor_motion;
char *start_standout;
char *end_standout;
char *terminal_init;
char *terminal_end;
short ospeed;

static struct sgttyb old_settings;
static struct sgttyb new_settings;
static char is_a_terminal = No;

init_termcap()

{
    char *bufptr;
    char *PCptr;
    char *term_name;
    char *temp_ptr;
    char *getenv();
    int status;

    /* assume we have a smart terminal until proven otherwise */
    smart_terminal = Yes;

    /* now get terminal name and termcap entry */
    term_name = getenv("TERM");
    if ((status = tgetent(termcap_buf, term_name)) != 1)
    {
	if (status == -1)
	{
	    fprintf(stderr, "%s: can't open termcap file\n", myname);
	}
	else
	{
	    fprintf(stderr, "%s: no termcap entry for a `%s' terminal\n",
		    myname, getenv("TERM"));
	}

	/* pretend it's dumb and proceed */
	smart_terminal = No;
	return;
    }

    /* these immediately indicate a very stupid terminal */
    if (tgetflag("hc") || tgetflag("os"))
    {
	smart_terminal = No;
	return;
    }

    /* set up common terminal capabilities */
    if ((screen_length = tgetnum("li")) <= 0)
    {
	screen_length = smart_terminal = 0;
	return;
    }

    /* screen_width is a little different */
    if ((screen_width = tgetnum("co")) == -1)
    {
	screen_width = 79;
    }
    else
    {
	screen_width -= 1;
    }

    /* initialize the pointer into the termcap string buffer */
    bufptr = string_buffer;

    /* get necessary capabilities */
    if ((clear_line    = tgetstr("ce", &bufptr)) == NULL ||
	(clear_screen  = tgetstr("cl", &bufptr)) == NULL ||
	(cursor_motion = tgetstr("cm", &bufptr)) == NULL)
    {
	smart_terminal = No;
	return;
    }

    /* get some more sophisticated stuff -- these are optional */
    terminal_init  = tgetstr("ti", &bufptr);
    terminal_end   = tgetstr("te", &bufptr);
    start_standout = tgetstr("so", &bufptr);
    end_standout   = tgetstr("se", &bufptr);

    /* pad character */
    PC = (PCptr = tgetstr("pc", &bufptr)) ? *PCptr : 0;

    /* set convenience strings */
    strcpy(home, tgoto(cursor_motion, 0, 0));
    strcpy(lower_left, tgoto(cursor_motion, 0, screen_length - 1));

    /* if stdout is not a terminal, pretend we are a dumb terminal */
    if (gtty(1, &old_settings) == -1)
    {
	smart_terminal = No;
    }
}

init_screen()

{
    /* get the old settings for safe keeping */
    if (gtty(1, &old_settings) == 0)
    {
	/* copy the settings so we can modify them */
	new_settings = old_settings;

	/* turn on CBREAK and turn off character echo and tab expansion */
	new_settings.sg_flags |= CBREAK;
	new_settings.sg_flags &= ~(ECHO|XTABS);
	stty(1, &new_settings);

	/* remember the erase and kill characters */
	ch_erase = old_settings.sg_erase;
	ch_kill  = old_settings.sg_kill;

	/* remember that it really is a terminal */
	is_a_terminal = Yes;

	/* send the termcap initialization string */
	putcap(terminal_init);
    }
    else
    {
	/* not a terminal at all---consider it dumb */
	smart_terminal = No;
    }
}

end_screen()

{
    /* move to the lower left, clear the line and send "te" */
    if (smart_terminal)
    {
	putcap(lower_left);
	putcap(clear_line);
	putcap(terminal_end);
    }

    /* if we have settings to reset, then do so */
    if (is_a_terminal)
    {
	stty(1, &old_settings);
    }
}

reinit_screen()

{
    /* install our settings if it is a terminal */
    if (is_a_terminal)
    {
	stty(1, &new_settings);
    }

    /* send init string */
    if (smart_terminal)
    {
	putcap(terminal_init);
    }
}

standout(fmt, a1, a2, a3)

char *fmt;
int a1, a2, a3;

{
    if (smart_terminal)
    {
	putcap(start_standout);
	printf(fmt, a1, a2, a3);
	putcap(end_standout);
    }
    else
    {
	printf(fmt, a1, a2, a3);
    }
}

clear()

{
    if (smart_terminal)
    {
	putcap(clear_screen);
    }
}

/* This has to be defined as a subroutine for tputs (instead of a macro) */

putstdout(ch)

char ch;

{
    putchar(ch);
}

