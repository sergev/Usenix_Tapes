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
 *	30-Apr-86 Mic Kaczmarczik ...!ihnp4!seismo!ut-sally!ut-ngp!mic
 *		  Instead of using VAX C getenv("TERM"), which does not
 *		  return the value of logical name "TERM", translate the
 *		  logical name by hand.
 *	11-Oct-86 Mic Kaczmarczik ...!ihnp4!seismo!ut-sally!ut-ngp!mic
 *		  Support tc capability to allow the library to use standard
 *		  termcaps.  Rewrote tgetent to look for tc capability
 *		  and add new terminal definition to the caller's buffer. 
 *		  This makes it rather possible to overflow the caller's
 *		  buffer, but the library doesn't make any claim that it
 *		  won't overwrite the buffer anyway...
 */



/*
 *  LIBRARY FUNCTION
 *
 *	tgetent   load buffer with entry for specified terminal
 *
 *  KEY WORDS
 *
 *	termcap functions
 *	utility routines
 *
 *  SYNOPSIS
 *
 *	int tgetent(bp,name)
 *	char *bp;
 *	char *name;
 *
 *  DESCRIPTION
 *
 *	Extracts the entry for terminal <name> from the termcap file
 *	and places it in the character buffer <bp>.   It is currently
 *	assumed that bp is at least 1024 characters.  If the entry in
 *	the termcap file is larger than 1023 characters the excess
 *	characters will be discarded and appropriate status will
 *	be returned.
 *
 *	Also note that since bp is used by other termcap
 *	routines, the storage associated with the termcap entry
 *	cannot be freed until all termcap calls are completed.
 *
 *	Tgetent can be directed to look in a file other than
 *	the default (/etc/termcap) by defining an environment
 *	variable called TERMCAP to be the pathname of the desired
 *	termcap file.  This is useful for debugging new entries.
 *	NOTE: the pathname MUST begin with a '/' character.
 *
 *	Also, if the string assigned to TERMCAP does not begin with
 *	a '/' and if the environment variable TERM matches <name> then
 *	the string assigned to TERMCAP is copied to buffer <bp> 
 *	instead of reading a termcap file.
 *	
 *  RETURNS
 *
 *	-1  if the termcap file cannot be opened
 *	 0  if no entry in termcap file matches <name>
 *	 1  if extraction is successful with no errors
 *	 2  if extraction is successful but entry truncated
 *
 *  SEE ALSO
 *
 *	tgetnum   extract numeric type capability
 *	tgetflag  test boolean type capability
 *	tgetstr   get string value of capability
 *
 *  AUTHOR
 *
 *	Fred Fish
 *
 */

#include <stdio.h>

#define TRUE 1
#define FALSE 0
#define BUFSIZE 1024			/* Assumed size of external buffer */

#define NO_FILE	 -1			/* Returned if can't open file */
#define NO_ENTRY  0			/* Returned if can't find entry */
#define SUCCESS   1			/* Returned if entry found ok */
#define TRUNCATED 2			/* Returned if entry found but trunc */

#define DEFAULT_FILE "/etc/termcap"	/* default termcap filename */
#ifdef	VAXC
#define	index strchr
#endif

char *_tcpbuf;				/* Place to remember buffer pointer */
FILE *fp, *find_file();			/* Termcap file			*/
extern char *index();

/*
 *  PSEUDO CODE
 *
 *	Begin tgetent
 *	    Erase any previous buffer contents.
 *	    Remember the buffer pointer.
 *	    If termcap file is not found then
 *		If buffer was filled anyway then
 *		    Return SUCCESS.
 *		Else
 *		    Return NO_FILE.
 *		End if
 *	    Else
 *		Find entry associated with name
 *		While an entry was found and limit not reached
 *		    If no tc capability found Then
 *			Exit while loop with status = SUCCESS
 *		    Else
 *		        Call getent to get entry indicated by tc=
 *			If entry not found then
 *				Exit loop with status != SUCCESS
 *			End if
 *			Concatenate entry into buffer
 *		    End If
 *		End while
 *	    End if
 *	    Close termcap file
 *	    Return status code
 *	End tgetent
 *			
 */

int tgetent(bp,name)
char *bp;
char *name;
{
	char	*tc, *tcbufp, tcbuf[80], termbuf[BUFSIZE], *tgetstr();
	char	*bufp, *cp;		/* current start of buffer	*/
	int	limit = 10;		/* maximum nesting		*/
	int	status;			/* return from getent()		*/

	*bp = '\0';			/* clear buffer			*/
	_tcpbuf = bp;			/* save base of buffer		*/

	/* Look for termcap file.  If NULL, find_file may have found a	*/
	/* a valid termcap string in the environment variable TERMCAP.	*/
	/* If non-null, attempt to find the entry in the termcap file	*/

	if ((fp = find_file(bp)) == NULL) {
		if (*bp != NULL)
			return(SUCCESS);
		else
			return(NO_FILE);
	}
	status = getent(bp, name);/* look for main entry	*/

	/* Start looking for tc capabilities in the termcap.  If
	 * found, concatenate the entry for the new terminal to the
	 * current buffer and try again.  To avoid infinite loops,
	 * allow only 10 loops through this process.
	 */
	while ((status == SUCCESS) && limit--) {
		/* look for tc capability.  If none found, exit loop	*/
		tcbufp = tcbuf;
		if (((tc = tgetstr("tc",&tcbufp)) == NULL)
		      || (*tc == '\0')) {
			status = SUCCESS;/* no more tc= entries	*/
			break;
		}

		/* Attempt to get next entry. Exit loop if unsuccessful	*/
		if ((status = getent(termbuf, tcbuf)) != SUCCESS)
			break;

		/* Copy new entry into buffer, right at "tc="		 */
		for (bufp = bp; *bufp; bufp++)		/* find tc=	*/
			if ((*bufp=='t') && (bufp[1]=='c') && (bufp[2]=='='))
				break;
		if ((cp = index(termbuf,':')) == NULL)
			cp = termbuf;
		strcpy(bufp, cp + 1);
	}

	/* close termcap file and return the status	*/
	fclose(fp);
	return status;
}




/*
 *  INTERNAL FUNCTION
 *
 *	getent    find termcap entry in termcap file
 *
 *  KEY WORDS
 *
 *	internal functions
 *	getent
 *
 *  SYNOPSIS
 *
 *	static int getent(bp,name)
 *	char *bp;
 *	char *name;
 *
 *  DESCRIPTION
 *
 *	Getent is called by tgetent each time tgetent attempts to
 *	read a capability from the termcap database file.  Places
 *	the entry in the buffer pointed to by bp
 *
 *
 *  PSEUDOCODE
 *
 *	Begin Getent
 *	    Seek to beginning of termcap file
 *	    Clear buffer
 *	    While records left to process
 *		If this is entry is what we want then
 *		    If entry was truncated then
 *			Return TRUNCATED status
 *		    Else
 *			Return SUCCESS status.
 *		    End if
 *		End if
 *	    End while
 *	    Return NO_ENTRY status.
 *	End
 */

static int getent(bp,name)
char *bp;				/* Pointer to buffer (1024 char min) */
char *name;				/* Pointer to terminal entry to find */
{
	*bp = '\0';			/* clear buffer			*/
	lseek(fileno(fp), 0L, 0l);	/* rewind termcap file		*/

	while (fgetlr(bp,BUFSIZE,fp)) {
        	if (gotcha(bp,name)) {
			if (bp[strlen(bp)-1] != '\n') {
				return(TRUNCATED);
			} else {
				return(SUCCESS);
			}
		}
	}
	return(NO_ENTRY);
}



/*
 *  INTERNAL FUNCTION
 *
 *	find_file    find the termcap file and open it if possible
 *
 *  KEY WORDS
 *
 *	internal functions
 *	find_file
 *
 *  SYNOPSIS
 *
 *	static FILE *find_file(bp)
 *	char *bp;
 *
 *  DESCRIPTION
 *
 *	Attempts to locate and open the termcap file.  Also handles
 *	using the environment TERMCAP string as the actual buffer
 *	(that's why bp has to be an input parameter).
 *
 *	If TERMCAP is defined an begins with a '/' character then
 *	it is taken to be the pathname of the termcap file and
 *	an attempt is made to open it.  If this fails then
 *	the default termcap file is used instead.
 *
 *	If TERMCAP is defined but does not begin with a '/' then
 *	it is assumed to be the actual buffer contents provided
 *	that <name> matches the environment variable TERM.
 *
 *  BUGS
 *
 *	There is currently no way to be sure which termcap
 *	file was opened since the default will always be
 *	tried.
 *
 */



/*
 *  PSEUDO CODE
 *
 *	Begin find_file
 *	    If there is a TERMCAP environment string then
 *		If the string is not null then
 *		    If the string is a pathname then
 *			If that file is opened successfully then
 *			    Return its pointer.
 *			End if
 *		    Else
 *			If there is a TERM environment string then
 *			    If TERM matches <name> then
 *				Copy TERMCAP string to buffer.
 *				Return NULL for no file.
 *			    End if
 *			End if
 *		    End if
 *		End if
 *	    End if
 *	    Open default termcap file and return results.
 *	End find_file
 *
 */
#ifdef	VAXC
char *trnlnm();
#endif

static FILE *find_file(bp)
char *bp;
{
    FILE *fp, *fopen();
    char *cp, *ncp, *getenv(), vmsname[132];

    if ((cp = getenv("TERMCAP")) != NULL) {
	if (*cp != NULL) {
	    if (*cp == '/') {
		if ((fp = fopen(cp,"r")) != NULL) {
		    return(fp);
		}
	    } else {
#ifdef VAXC
		if ((ncp = trnlnm("TERM")) != NULL) {
#else
		if ((ncp = getenv("TERM")) != NULL) {
#endif
		    if (strcmp(cp,ncp) == 0) {
			strcpy(bp,cp);
			return((FILE *)NULL);
		    }
		}
	    }
	}
    }
    return(fopen(DEFAULT_FILE,"r"));
}



/*
 *  INTERNAL FUNCTION
 *
 *	gotcha   test to see if entry is for specified terminal
 *
 *  SYNOPSIS
 *
 *	gotcha(bp,name)
 *	char *bp;
 *	char *name;
 *
 *  DESCRIPTION
 *
 *	Tests to see if the entry in buffer bp matches the terminal
 *	specified by name.  Returns TRUE if match is detected, FALSE
 *	otherwise.
 *
 */



/*
 *  PSEUDO CODE
 *
 *	Begin gotcha
 *	    If buffer character is comment character then
 *		Return FALSE since remainder is comment
 *	    Else
 *		Initialize name scan pointer.
 *		Compare name and buffer until end or mismatch.
 *		If valid terminators for both name and buffer strings
 *		    Return TRUE since a match was found.
 *		Else
 *		    Find next non-name character in buffer.
 *		    If not an alternate name separater character
 *			Return FALSE since no more names to check.
 *		    Else
 *			Test next name and return results.
 *		    End if
 *		End if
 *	    End if
 *	End gotcha
 *
 */

gotcha(bp,name)
char *bp;
char *name;
{
    char *np;
 
    if (*bp == '#') {
	return(FALSE);
    } else {
	np = name;
	while (*np == *bp && *np != NULL) {np++; bp++;}
	if (*np == NULL && (*bp == NULL || *bp == '|' || *bp == ':')) {
	    return(TRUE);
	} else {
	    while (*bp != NULL && *bp != ':' && *bp != '|') {bp++;}
	    if (*bp != '|') {
		return(FALSE);
	    } else {
		return(gotcha(++bp,name));
	    }
	}
    }
}

