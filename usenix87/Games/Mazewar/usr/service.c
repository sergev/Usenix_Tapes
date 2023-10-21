#ifndef lint
static char rcsid[] = "$Header: service.c,v 1.1 84/08/25 17:11:30 chris Exp $";
#endif

#include <stdio.h>
#include <curses.h>
#include "../h/defs.h"
#include "../h/struct.h"
#include "../h/extern.h"

#ifndef CTRL
#define CTRL(c)		('c' & 037)
#endif

/*
 *  Read and parse keyboard input.  All possible functions
 *  are stored in an array of function pointers which is
 *  accessed by the routine input_act();
 */

service()
{
	char 		buf[BUFSIZ];
	register int	count;
	register char 	*cp;
	extern 		(*input_act())();
	register	(*act)();

	/*
	 * We have to remember to read all the characters currently
	 * on the input queue.  The select in work assures us that
	 * there is at least on character on the queue so that it
	 * won't block.
	 */
	
	count = read(fileno(stdin), buf, sizeof buf);
	if (count < 0) {
		perror("Read error on stdin");
		return(-1);
	}

	buf[count] = '\0';

	for (cp = buf; *cp; cp++) {
		act = input_act(*cp);
#ifdef DEBUG_2
		fprintf(stderr, "service: input_act('%c') = 0x%x\n",
		    *cp, act);
#endif
		if ((int)act != NULL)
			act(*cp);
	}
	return(count);
}

act_beep()
{
	char	beep = CTRL(g);

	if (write(2, &beep, 1) < 0)
		perror("Write error to stderr");
}

act_redraw(input)
{
	if (ascii)
		(void) wrefresh(curscr);
#ifdef sun
	else
		screen_redraw();
#endif
	return(0);
}

act_forward(input)
{
	peek = 0;
	forward();
	return(0);
}

act_shoot(input)
{
	if (keyok <= 0) {
		keyok = 1;
		fire();
	}
	
	return(0);
}

act_quit(input)
{
	quit(0);
}

act_180turn(input)
{
	peek = 0;
	bturn();

	return(0);
}

act_left(input)
{
	peek = 0;
	lturn();
	return(0);
}

act_right(input)
{
	peek = 0;
	rturn();

	return(0);
}

pact_left(input)
{
	peek = 3;

	return(0);
}

pact_stop(input)
{
	peek = 0;

	return(0);
}

pact_right(input)
{
	peek = 1;

	return(0);
}

act_back_up(input)
{
	peek = 0;
	back();

	return(0);
}

/* Clean up and quit */
quit(status)
{

	unsetioctls();
	sendquit();

#ifdef sun
	if (onsun) {
		endpix();
	} else {
#else
	{
#endif
		move(LINES - 1, 0);
		clrtoeol();
		refresh();

		endwin();
	}

	exit(0);
}
