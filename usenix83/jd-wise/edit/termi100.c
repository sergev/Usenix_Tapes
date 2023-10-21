#include "term.h"
struct termchar t={
	0,		/* start dummy */
	80,		/* number of columns */
	24,		/* number of lines */
	INFOTON,	/* terminal type code */
	{
		' ',	/* offset */
		curev,	/* line,col are reversed */
		},
	"",		/* start dummy */
	"\033f%c%c",	/* cursor addressing */
	"\014",		/* clear screen */
	"\033g",	/* clear all tabs */
	"\0332",	/* clear tab */
	(char *)0,	/* delete char */
	"12\033M",	/* delete line */
	"\033K",	/* erase to end of line */
	"\033J",	/* erase to end of page */
	"\033H",	/* home */
	(char*)0,	/* insert char */
	"12\033L",	/* insert line */
	" ",		/* map */
	"\0331",	/* set tab */
	"",		/* end dummy */
	};
