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
 *	     #define index() to be strchr() if VAX C
 */




/*
 *  LIBRARY FUNCTION
 *
 *	tgetstr   extract string capability from termcap entry
 *
 *  KEY WORDS
 *
 *	termcap
 *
 *  SYNOPSIS
 *
 *	char *tgetstr(id,area)
 *	char *id;
 *	char **area;
 *
 *  DESCRIPTION
 *
 *	Gets the string capability for <id>, placing it in
 *	the buffer at *area, and advancing *area to point
 *	to next available storage.
 *
 *	For example, if the following capabilities are
 *	in the termcap file:
 *
 *		ZZ=zzzz
 *		YY=yyyyyy
 *		WW=www
 *
 *	then successive calls using YY, ZZ, and WW will
 *	build the following buffer:
 *
 *		yyyyyy0zzzz0www0
 *
 *	The first call will return a pointer to yyyyyy, the
 *	second will return a pointer to zzzz and the third
 *	will return a pointer to www.  Note that each
 *	string is null terminated, as are all C strings.
 *
 *	Characters preceded by the carot character (\136)
 *	are mapped into the corresponding control character.
 *	For example, the two character sequence ^A becomes
 *	a single control-A (\001) character.
 *
 *	The escape character is the normal C backslash and
 *	the normal C escape sequences are recognized, along
 *	with a special sequence for the ASCII escape character
 *	(\033).  The recognized sequences are:
 *
 *		\E   =>  '\033'  (ASCII escape character)
 *		\b   =>  '\010'  (ASCII backspace character)
 *		\f   =>  '\014'  (ASCII form feed character)
 *		\n   =>  '\012'  (ASCII newline/linefeed char)
 *		\r   =>  '\015'  (ASCII carriage return char)
 *		\t   =>  '\011'  (ASCII tab character)
 *		\ddd =>  '\ddd'  (arbitrary ASCII digit)
 *		\x   =>  'x'     (ordinary ASCII character)
 *
 */

#include <stdio.h>
#include <ctype.h>
#ifdef VAXC
#define index strchr
#endif

extern char *_tcpbuf;		/* Termcap entry buffer pointer */



/*
 *  PSEUDO CODE
 *
 *	Begin tgetstr
 *	    Initialize pointer to the termcap entry buffer.
 *	    While there is a field to process
 *		Skip over the field separator character.
 *		If this is the entry we want then
 *		    If the entry is not a string then
 *			Return NULL.
 *		    Else
 *			Transfer string and rtn pointer.
 *		    End if
 *		End if
 *	    End while
 *	    Return NULL
 *	End tgetstr
 *
 */

char *tgetstr(id,area)
char *id;
char **area;
{
    char *bp;
    extern char *index();
    char *decode();

    bp = _tcpbuf;
    while ((bp = index(bp,':')) != NULL) {
	if (*++bp == NULL)
		break;
	if (*bp++ == id[0] && *bp != NULL && *bp++ == id[1]) {
	    if (*bp != NULL && *bp++ != '=') {
		return(NULL);
	    } else {
		return(decode(bp,area));
	    }
	}
    }
    **area = NULL;
    bp = (*area)++;
    return(bp);
}



/*
 *  INTERNAL FUNCTION
 *
 *	decode   transfer string capability, decoding escapes
 *
 *  SYNOPSIS
 *
 *	static char *decode(bp,area)
 *	char *bp;
 *	char **area;
 *
 *  DESCRIPTION
 *
 *	Transfers the string capability, up to the next ':'
 *	character, or null, to the buffer pointed to by
 *	the pointer in *area.  Note that the initial
 *	value of *area and *area is updated to point
 *	to the next available location after the null
 *	terminating the transfered string.
 *
 *  BUGS
 *
 *	There is no overflow checking done on the destination
 *	buffer, so it better be large enough to hold
 *	all expected strings.
 *
 */



/*
 *  PSEUDO CODE
 *
 *	Begin decode
 *	    Initialize the transfer pointer.
 *	    While there is an input character left to process
 *		Switch on input character
 *		Case ESCAPE:
 *		    Decode and xfer the escaped sequence.
 *		    Break
 *		Case CONTROLIFY:
 *		    Controlify and xfer the next character.
 *		    Advance the buffer pointer.
 *		    Break
 *		Default:
 *		    Xfer a normal character.
 *		End switch
 *	    End while
 *	    Null terminate the output string.
 *	    Remember where the output string starts.
 *	    Update the output buffer pointer.
 *	    Return pointer to the output string.
 *	End decode
 *
 */

static char *decode(bp,area)
char *bp;
char **area;
{
    char *cp, *bgn;
    char *do_esc();

    cp = *area;
    while (*bp != NULL && *bp != ':') {
	switch(*bp) {
	case '\\':
	    bp = do_esc(cp++,++bp);
	    break;
	case '^':
	    *cp++ = *++bp & 037;
	    bp++;
	    break;
	default:
	    *cp++ = *bp++;
	    break;
	}
    }
    *cp++ = NULL;
    bgn = *area;
    *area = cp;
    return(bgn);
}



/*
 *  INTERNAL FUNCTION
 *
 *	do_esc    process an escaped sequence
 *
 *  SYNOPSIS
 *
 *	char *do_esc(out,in);
 *	char *out;
 *	char *in;
 *
 *  DESCRIPTION
 *
 *	Processes an escape sequence pointed to by
 *	in, transfering it to location pointed to
 *	by out, and updating the pointer to in.
 *
 */



/*
 *  PSEUDO CODE
 *
 *	Begin do_esc
 *	    If the first character is not a NULL then
 *		If is a digit then
 *		    Set value to zero.
 *		    For up to 3 digits
 *		        Accumulate the sum.
 *		    End for
 *		    Transfer the sum.
 *	        Else if character is in remap list then
 *		    Transfer the remapped character.
 *		    Advance the input pointer once.
 *	        Else
 *		    Simply transfer the character.
 *	        End if
 *	    End if
 *	    Return updated input pointer.
 *	End do_esc
 *
 */

static char *maplist = {
    "E\033b\bf\fn\nr\rt\t"
};

char *do_esc(out,in)
char *out;
char *in;
{
    int count;
    char ch;
    extern char *index();
    char *cp;

    if (*in != NULL) {
	if (isdigit(*in)) {
	    ch = 0;
	    for (count = 0; count < 3 && isdigit(*in); in++) {
		 ch <<= 3;
		 ch |= (*in - '0');
	    }
	    *out++ = ch;
	} else if ((cp = index(maplist,*in)) != NULL) {
	    *out++ = *++cp;
	    in++;
	} else {
	    *out++ = *in++;
	}
    }
    return(in);
}
