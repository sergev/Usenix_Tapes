/**			opt_utils.c			**/

/** This file contains routines that might be needed for the various
     machines that the mailer can run on.  Please check the Makefile
     for more help and/or information. 

     (C) Copyright 1986 Dave Taylor
**/

#include <stdio.h>
#include "headers.h"

#ifdef BSD
# include <pwd.h>
#endif

#ifdef NEED_GETHOSTNAME
#  include <sys/utsname.h>
#endif

#ifdef UTS
# include <sys/tubio.h>
# define  TTYIN		0		/* standard input */
#endif

#ifdef NEED_GETHOSTNAME

gethostname(hostname,size) /* get name of current host */
int size;
char *hostname;
{
	/** Return the name of the current host machine.  UTS only **/

	/** This routine compliments of Scott McGregor at the HP
	    Corporate Computing Center **/
     
	int uname();
	struct utsname name;

	(void) uname(&name);
	(void) strncpy(hostname,name.nodename,size-1);
	hostname[size] = '\0';

}

#endif

#ifdef UTS

int
isa3270()
{
	/** Returns TRUE and sets LINES and COLUMNS to the correct values
	    for an Amdahl (IBM) tube screen, or returns FALSE if on a normal
	    terminal (of course, next to a 3270, ANYTHING is normal!!) **/

	struct tubiocb tubecb;

	dprint0(3,"Seeing if we're a 3270...");

	if (ioctl(TTYIN, TUBGETMOD, &tubecb) == -1) {
	  dprint0(3,"We're not!\n");
	  return(FALSE);	/* not a tube! */
	}
	
	LINES   = tubecb->line_cnt - 1;
	COLUMNS = tubecb->col_cnt;
	
	dprint2(3,"We are!  %d lines and %d columns!\n",
		LINES, COLUMNS);
	return(TRUE);
}

#endif /* def UTS */

#ifdef BSD

cuserid(uname)
char *uname;
{
	/** Added for compatibility with Bell systems, this is the last-ditch
	    attempt to get the users login name, after getlogin() fails.  It
	    instantiates "uname" to the name of the user...
	**/

	struct passwd *password_entry, *getpwuid();

	password_entry = getpwuid(getuid());

	strcpy(uname, password_entry->pw_name);
}

/** some supplementary string functions for Berkeley Unix systems **/

strspn(source, keys)
char *source, *keys;
{
	/** This function returns the length of the substring of
	    'source' (starting at zero) that consists ENTIRELY of
	    characters from 'keys'.  This is used to skip over a
	    defined set of characters with parsing, usually. 
	**/

	register int loc = 0, key_index = 0;

	while (source[loc] != '\0') {
	  key_index = 0;
	  while (keys[key_index] != source[loc])
	    if (keys[key_index++] == '\0')
	      return(loc);
	  loc++;
	}

	return(loc);
}

strcspn(source, keys)
char *source, *keys;
{
	/** This function returns the length of the substring of
	    'source' (starting at zero) that consists entirely of
	    characters NOT from 'keys'.  This is used to skip to a
	    defined set of characters with parsing, usually. 
	    NOTE that this is the opposite of strspn() above
	**/

	register int loc = 0, key_index = 0;

	while (source[loc] != '\0') {
	  key_index = 0;
	  while (keys[key_index] != '\0')
	    if (keys[key_index++] == source[loc])
	      return(loc);
	  loc++;
	}

	return(loc);
}

char *strtok(source, keys)
char *source, *keys;
{
	/** This function returns a pointer to the next word in source
	    with the string considered broken up at the characters 
	    contained in 'keys'.  Source should be a character pointer
	    when this routine is first called, then NULL subsequently.
	    When strtok has exhausted the source string, it will 
	    return NULL as the next word. 

	    WARNING: This routine will DESTROY the string pointed to
	    by 'source' when first invoked.  If you want to keep the
	    string, make a copy before using this routine!!
	 **/

	register int  last_ch;
	static   char *sourceptr;
		 char *return_value;

	if (source != NULL)
	  sourceptr = source;
	
	if (*sourceptr == '\0') 
	  return(NULL);		/* we hit end-of-string last time!? */

	sourceptr += strspn(sourceptr, keys);	/* skip leading crap */
	
	if (*sourceptr == '\0') 
	  return(NULL);		/* we've hit end-of-string */

	last_ch = strcspn(sourceptr, keys);	/* end of good stuff */

	return_value = sourceptr;		/* and get the ret   */

	sourceptr += last_ch;			/* ...value 	     */

	if (*sourceptr != '\0')		/* don't forget if we're at END! */
	  sourceptr++;			   /* and skipping for next time */

	return_value[last_ch] = '\0';		/* ..ending right    */
	
	return((char *) return_value);		/* and we're outta here! */
}

char *strpbrk(source, keys)
char *source, *keys;
{
	/** Returns a pointer to the first character of source that is any
	    of the specified keys, or NULL if none of the keys are present
	    in the source string. 
	**/

	register int loc = 0, key_index = 0;

	while (source[loc] != '\0') {
	  key_index = 0;
	  while (keys[key_index] != '\0')
	    if (keys[key_index++] == source[loc])
	      return((char *) (source + loc));
	  loc++;
	}
	
	return(NULL);
}

char *strchr(buffer, ch)
char *buffer, ch;
{
	/** Returns a pointer to the first occurance of the character
	    'ch' in the specified string or NULL if it doesn't occur **/

	char *address;

	address = buffer;

	while (*address != ch) {
	  if (*address == '\0')
	    return (NULL);
	  address++;
	}

	return ( (char *) address);
}

#endif
