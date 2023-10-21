/**			utils2.c			**/

/** More miscellanous utilities for the filter program and such...

    (C) Copyright 1986, Dave Taylor
**/

#include <stdio.h>
#include "defs.h"

#ifdef BSD
# include <pwd.h>
#endif

#ifdef NEED_GETHOSTNAME
#  include <sys/utsname.h>
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

#ifdef BSD

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

#ifndef NULL
# define NULL		0
#endif

#define DONE		0
#define ERROR		-1

char *optional_arg;			/* optional argument as we go */
int   opt_index;			/* argnum + 1 when we leave   */

int  _indx = 1, _argnum = 1;

int
getopt(argc, argv, options)
int argc;
char *argv[], *options;
{
	/** Returns the character argument next, and optionally instantiates 
	    "argument" to the argument associated with the particular option 
	**/
	
	char        *word, *strchr();

	if (_argnum >= argc) {	/* quick check first - no arguments! */
	  opt_index = argc;
	  return(DONE);
	}

	if (_indx >= strlen(argv[_argnum]) && _indx > 1) {
	  _argnum++;
	  _indx = 1;		/* zeroeth char is '-' */
	}

	if (_argnum >= argc) {
	  opt_index = _argnum; /* no more args */
	  return(DONE);
	}

	if (argv[_argnum][0] != '-') {
	  opt_index = _argnum;
	  return(DONE);
	}

        word = strchr(options, argv[_argnum][_indx++]);

	if (word == NULL)
	  return(ERROR);		/* Sun compatibility */

	if (word == NULL || strlen(word) == 0) 
	  return(ERROR);
	
	if (word[1] == ':') {

	  /** Two possibilities - either tailing end of this argument or the 
	      next argument in the list **/

	  if (_indx < strlen(argv[_argnum])) { /* first possibility */
	    optional_arg = (char *) (argv[_argnum] + _indx);
	    _argnum++;
	    _indx = 1;
	  }
	  else {				/* second choice     */
	    if (++_argnum >= argc) 
	      return(ERROR);			/* no argument!!     */

	    optional_arg = (char *) argv[_argnum++];
	    _indx = 1;
	  }
	}

	return((int) word[0]);
}
