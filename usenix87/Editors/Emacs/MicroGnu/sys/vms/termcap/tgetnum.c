/************************************************************************
 *									*
 *			Copyright (c) 1982, Fred Fish			*
 *			    All Rights Reserved				*
 *									*
 *	This software and/or documentation is released for public	*
 *	distribution for personal, non-commercial use only.		*
 *	Limited rights to use, modify, and redistribute are hereby	*
 *	granted for non-commercial purposes, provided that all		*
 *	copyright notices remain intact and all changes are clearly	*
 *	documented.  The author makes no warranty of any kind with	*
 *	respect to this product and explicitly disclaims any implied	*
 *	warranties of merchantability or fitness for any particular	*
 *	purpose.							*
 *									*
 ************************************************************************
 */
/* Modified:
 * 30-Apr-86 Mic Kaczmarczik
 *	     Use ctype.h macros instead of the function isdigit().
 *	     #define index to strchr if VAXC
 */



/*
 *  LIBRARY FUNCTION
 *
 *	tgetnum   extract numeric option from termcap entry
 *
 *  KEY WORDS
 *
 *	termcap
 *	ce functions
 *
 *  SYNOPSIS
 *
 *	tgetnum(id)
 *	char *id;
 *
 *  DESCRIPTION
 *
 *	Returns numeric value of capability <id>, or -1 if <id>
 *	is not found.   Knows about octal numbers, which
 *	begin with 0.
 *
 */

#include <stdio.h>
#include <ctype.h>
#ifdef	VAXC
#define index strchr
#endif

extern char *_tcpbuf;		/* Termcap entry buffer pointer */



/*
 *  PSEUDO CODE
 *
 *	Begin tgetnum
 *	    Initialize pointer to the termcap entry buffer.
 *	    While there is a field to process
 *		Skip over the field separator character.
 *		If this is the entry we want then
 *		    If the entry is not a numeric then
 *			Return failure value.
 *		    Else
 *			Initialize value to zero.
 *			If number begins with zero then
 *			    Set accumulation base to 8.
 *			Else
 *			    Set accumulation base to 10.
 *			End if
 *			While there is a numeric character
 *			    Accumulate the value.
 *			End while
 *			Return value.
 *		    End if
 *		End if
 *	    End while
 *	    Return failure value.
 *	End tgetnum
 *
 */

tgetnum(id)
char *id;
{
    int value, base;
    char *bp;
    extern char *index();

    bp = _tcpbuf;
    while ((bp = index(bp,':')) != NULL) {
	bp++;
	if (*bp++ == id[0] && *bp != NULL && *bp++ == id[1]) {
	    if (*bp != NULL && *bp++ != '#') {
		return(-1);
	    } else {
		value = 0;
		if (*bp == '0') {
		    base = 8;
		} else {
		    base = 10;
		}
		while (isdigit(*bp)) {
		    value *= base;
		    value += (*bp++ - '0');
		}
		return(value);
	    }
	}
    }
    return(-1);
}
