#include "term.h"
struct termchar t={
	0,		/* start dummy */
	80,		/* number of columns */
	24,		/* number of lines */
	HP2621,	/* terminal type code */
	{
		0,	/* offset */
		0,	/* line,col are reversed */
		},
	"",		/* start dummy */
	"\033&a%dy%dC",	/* cursor addressing */
	(char *)0,	/* ?clear screen */
	"\033g",	/* ?clear all tabs */
	"\0332",	/* ?clear tab */
	(char *)0,	/* ?delete char */
	"10\033M",	/* delete line */
	"\033K",	/* erase to end of line */
	"\033J",	/* erase to end of page */
	"\033&a0y0C",	/* home */
	(char*)0,	/* ?insert char */
	"10\033L",	/* insert line */
	" \033p\n \033L \033q\n \033M \033r\n \033K \033s\n \033P \
\033t\n \033A \033u\n \033D \033v\n \033C \033w\n \033B",	/* map */
	"\0333",	/* set tab */
	"",		/* end dummy */
	};
