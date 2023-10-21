#include "term.h"
struct termchar t={
	0,		/* start dummy */
	80,		/* number of columns */
	24,		/* number of lines */
	VISUAL,		/* terminal type code */
	{
		' ',	/* offset */
		0,	/* line,col are reversed */
		},
	"",		/* start dummy */
	"\033Y%c%c",	/* cursor addressing */
	"\033v",	/* clear screen */
	"\033g",	/* clear all tabs */
	"\0332",	/* clear tab */
	(char *)0,	/* delete char */
	"\033M",	/* delete line */
	"\033K",	/* erase to end of line */
	"\033J",	/* erase to end of page */
	"\033H",	/* home */
	(char*)0,	/* insert char */
	"\033L",	/* insert line */
			/* map */
	" \033?q \033K \033?t \033M \033?u \033P \033?v \0332 \033?w \033L \033?x \033@ \033?y \0331 ",
	"\0331",	/* set tab */
	"",		/* end dummy */
	};
