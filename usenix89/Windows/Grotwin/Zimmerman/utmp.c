#ifndef lint
static char sccsid[] = "@(#)utmp.c  2.2  [ (C) Nigel Holder 1986 ]";
#endif

/**************************************
*
*	Author  :  Nigel Holder
*
*	Date    :  10 July 1986
*
*
*	Copyright (C) 1986  by Nigel Holder 
* 
*	   Permission to use this program is granted, provided it is not
*	sold, or distributed for direct commercial advantage, and includes
*	the copyright notice and this clause.
* 
*	  This program attempts to overcome the deficiences of 4.2 not
*	having any visible routines to update the utmp file like Sys V.
*	  Utmp is used by utilities such as who to find out who is
*	logged on.  On Sun 4.2, /etc/utmp can be written by
*	anyone, so no setuid etc. is required to alter it.
*
*	  Accessed only via utmp_insert(tty, id) and utmp_delete(tty)
*	to enable routines to be kept internal.  Uses host field
*	to indicate that pseudo tty is being used by grotwin (I know its
*	not what the field is meant for, but it looks better all the same).
*
*	  Getpwent() appears to hunk in approx 30 K of unnecessary code
*	to deal with remote hosts etc.
*
**************************************/


#include <stdio.h>
#include <utmp.h>
#include <pwd.h>


#define		UTMP_INSERT		( 1 )
#define		UTMP_DELETE		( 2 )

/*  max length of tty name entry in /etc/ttys file  */
#define		TTYS_MAXLEN		( 20 )


typedef		enum { FALSE, TRUE }	bool;



utmp_insert(tty, id)		/*  add entry to utmp  */

char	*tty, *id;

{
	return(utmp_update(tty, id, UTMP_INSERT));
}



utmp_delete(tty)		/*  delete entry from utmp  */

char	*tty;

{
	return(utmp_update(tty, "", UTMP_DELETE));
}




static
utmp_update(tty, id, mode)		/*  update utmp file  */

char	*tty, *id;
int	mode;

{
	static char	utmp[] = "/etc/utmp";

	struct utmp	entry;
	struct passwd	*getpwuid(), *passwd_entry;
	FILE		*fp, *fopen();
	long		*time();
	int		position, i;
	char		*getlogin(), *user_name;

	position = utmp_pos(tty);
	if (position == -1)   {
		return(-1);
	}
	if ((fp = fopen(utmp, "r+")) == NULL)   {
		return(-1);
	}

	if (mode == UTMP_INSERT)   {		/*  try to find user  */
		if ((user_name = getlogin()) == NULL)   {
			if ((passwd_entry = getpwuid(getuid())) == NULL)   {
				return(-1);		/*  give up  */
			}
			user_name = passwd_entry->pw_name;
		}

		/*******************
		*   In case strings are too long to fit utmp structure,
		*   copy max - 1 chars and null terminate at end.
		*******************/

		strncpy(entry.ut_name, user_name, sizeof(entry.ut_name) - 1);
		entry.ut_host[sizeof(entry.ut_name) - 1] = '\0';
		strncpy(entry.ut_host, id, sizeof(entry.ut_host) - 1);
		entry.ut_host[sizeof(entry.ut_host) - 1] = '\0';
	}
	else   {	/*  UTMP_DELETE  */
		for (i = 0 ; i < sizeof(entry.ut_name) ; ++i)   {
			entry.ut_name[i] = '\0';
		}
		for (i = 0 ; i < sizeof(entry.ut_host) ; ++i)   {
			entry.ut_host[i] = '\0';
		}
	}

	strncpy(entry.ut_line, tty, sizeof(entry.ut_line) - 1);
	entry.ut_line[sizeof(entry.ut_host) - 1] = '\0';
	(void) time(&(entry.ut_time));
	if (fseek(fp, (long) (sizeof(entry) * position), 0) != -1)   {
		(void) fwrite(&entry, sizeof(entry), 1, fp);
	}
	(void) fclose(fp);
	return(position);
}



utmp_pos(tty)				/*  like ttyslot  */

char	*tty;

{
	static char	ttys[] = "/etc/ttys";

	FILE	*fp, *fopen();
	int	position;
	bool	found;
	char	temp[TTYS_MAXLEN];

	if ((fp = fopen(ttys, "r")) == NULL)   {
		return(-1);
	}
	found = FALSE;
	for (position = 1 ; fgets(temp, TTYS_MAXLEN, fp) != NULL ; ++position) {
		temp[strlen(temp) - 1] = '\0';		/* remove trailing \n */
		if (strcmp(temp + 2, tty) == 0)   {
			found = TRUE;
			break;
		}
	}
	(void) fclose(fp);
	if (found == FALSE)   {
		return(-1);
	}
	return(position);
}
