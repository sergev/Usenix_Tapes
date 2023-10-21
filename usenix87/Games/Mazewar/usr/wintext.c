#ifndef lint
static char rcsid[] = "$Header: wintext.c,v 1.1 84/08/25 17:11:36 jcoker Exp $";
#endif

#include <stdio.h>
#include "../h/sunscreen.h"


/*
 *  Print the character at the given position on the pixrect.
 */

mvaddch(pr, x, y, c)
	struct pixrect	*pr;
{
	char	buf[2];

	buf[0] = c;
	buf[1] = '\0';

	return mvaddstr(pr, x, y, buf);
}


/*
 *  Put the string on the pixrect at the given coordianates.
 */

mvaddstr(pr, x, y, string)
	struct pixrect	*pr;
	char		*string;
{
	struct pr_prpos	where;

	where.pr = pr;
	where.pos.x = x;
	where.pos.y = y;
	
	pf_ttext(where, OP_CLEAR, pf_font, string);
}


/*
 *  Do a printf at the specified position on the standard
 *  screen pixrect.
 */

/*VARARGS4*/
mvprintf(pr, x, y, format, args)
	struct pixrect	*pr;
	char	*format;
{
	char	buf[BUFSIZ];
	FILE	str;

	str._flag = _IOWRT | _IOSTRG;
	str._cnt  = sizeof buf;
	str._ptr  = buf;
	str._file = -1;

	_doprnt(format, &args, &str);
	putc('\0', &str);

	return mvaddstr(pr, x, y, buf);
}
