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
/*
 * Modified:
 *	30-Apr-86 Mic Kaczmarczik
 *	#define index to strchr if VAX C
 *
 */




/*
 *  LIBRARY FUNCTION
 *
 *	tgetflag   extract boolean termcap capability
 *
 *  KEY WORDS
 *
 *	termcap
 *
 *  SYNOPSIS
 *
 *	tgetflag(id)
 *	char *id;
 *
 *  DESCRIPTION
 *
 *	Returns TRUE if specified id is present in terminal
 *	entry, FALSE otherwise.
 *
 */

#include <stdio.h>
#ifdef VAXC
#define index strchr
#endif

#define TRUE 1
#define FALSE 0

extern char *_tcpbuf;		/* Termcap entry buffer pointer */



/*
 *  PSEUDO CODE
 *
 *	Begin tgetflag
 *	    Initialize pointer to the termcap entry buffer.
 *	    While there is a field to process
 *		Skip over the field separator character.
 *		If this is the entry we want then
 *		    If entry is identifier only then
 *			Return TRUE
 *		    Else
 *			Return FALSE
 *		    End if
 *		End if
 *	    End while
 *	    Return FALSE as default.
 *	End tgetflag
 *
 */

tgetflag(id)
char *id;
{
    char *bp;
    extern char *index();

    bp = _tcpbuf;
    while ((bp = index(bp,':')) != NULL) {
	bp++;
	if (*bp++ == id[0] && *bp != NULL && *bp++ == id[1]) {
	    if (*bp == NULL || *bp++ == ':') {
		return(TRUE);
	    } else {
		return(FALSE);
	    }
	}
    }
    return(FALSE);
}
