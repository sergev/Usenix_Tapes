/*
**	1stlatex.w	1st_word to latex conversion program
**	file : 1ststyle.c	handles style information
**	version 1.0
**
**	Jerome M. Lang
**	Public Domain
**	7 March 1985.
**
**	doStyle(), handles style information
*/
/*
**	1986-04-28	: added support for bold math characters.
*/

#include "1stlatex.h"

/* style types */
#define BOLD		0x0001
#define LIGHT		0x0002
#define	ITALICS 	0x0004
#define UNDERLINE	0x0008
#define	SUPERSCRIPT	0x0010
#define SUBSCRIPT	0x0020

static int	stylelevel = 0;
static int	oldstyle   = 0;
static char	*stk[15];
static char	*brace="}";
static char	*math="}$}";

doStyle( ch )
int	ch;
{

	/*	Remove all previous style consideration
	** 	It is a bit dangerous
	** 	to do it this way.
        */

	if ( (oldstyle & ITALICS) & ~(ch & ITALICS) )
	{
		outstr( "\\/");
	}

	while( stylelevel > 0 )
	{
		outstr(stk[--stylelevel]);
	}

	underline = FALSE;			/* we can start paragraph now */

	if ( ch & BOLD )
	{
		outstr( "{\\bf ");		/* bold face */
		stk[stylelevel++]=brace;
	}

	if ( ch & LIGHT & (~BOLD) & (~ITALICS) )
	{
		outstr( "{\\sl ");		/* slanted */
		stk[stylelevel++]=brace;
	}

	if ( ch & ITALICS & (~BOLD) & (~LIGHT) )
	{
		outstr( "{\\it ");		/* italics */
		stk[stylelevel++]=brace;
	}

	if ( ch & LIGHT & ITALICS & (~BOLD) )
 	{
		outstr( "{\\sf ");		/* sans serif */
		stk[stylelevel++]=brace;
	}


	if ( ch & UNDERLINE )
	{
		outstr( "\\underline{");	/* underline */
		stk[stylelevel++]=brace;
		underline = TRUE;
	}

	if ( ch & SUPERSCRIPT )
	{
		outstr( "{$^{");			/* superscript */
		stk[stylelevel++]=math;
	}

	if ( ch & SUBSCRIPT )
	{
		outstr( "{$_{");			/* subscript */
		stk[stylelevel++]=math;
	}

	oldstyle = ch;
}

boolean
isbold()
{
	return( 0!=(oldstyle & BOLD));
}
