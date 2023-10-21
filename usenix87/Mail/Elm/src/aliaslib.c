/**			aliaslib.c			**/

/** Library of functions dealing with the alias system...

    (C) Copyright 1986 Dave Taylor
 **/

#include "headers.h"

char *expand_group(), *get_alias_address(), *expand_system();
char *get_token(), *strpbrk();
long lseek();

char *get_alias_address(name, mailing, depth)
char *name;
int   mailing, depth;
{
	/** return the line from either datafile that corresponds 
	    to the specified name.  If 'mailing' specified, then
	    fully expand group names.  Depth specifies the nesting
	    depth - the routine should always initially be called
	    with this equal 0.  Returns NULL if not found   **/

	static char buffer[VERY_LONG_STRING];
	int    loc;

	if (strlen(name) == 0)
	  return( (char *) NULL);

	if (! read_in_aliases) {
	  read_alias_files();
	  read_in_aliases = TRUE;
	}

	if (user_files) 
	  if ((loc = find(name, user_hash_table, MAX_UALIASES)) >= 0) {
	    lseek(user_data, user_hash_table[loc].byte, 0L);
	    get_line(user_data, buffer);
	    if (buffer[0] == '!' && mailing)
	      return(expand_group(buffer, depth));
	    else if (strpbrk(buffer,"!@:") != NULL)	/* has a hostname */
#ifdef DONT_TOUCH_ADDRESSES
	      return((char *) buffer);
#else
	      return(expand_system(buffer, TRUE));
#endif
	    else
	      return((char *) buffer);
	  }
	 
	if (system_files) 
	  if ((loc = find(name, system_hash_table, MAX_SALIASES)) >= 0) {
	    lseek(system_data, system_hash_table[loc].byte, 0L);
	    get_line(system_data, buffer);
	    if (buffer[0] == '!' && mailing)
	      return(expand_group(buffer, depth));
	    else if (strpbrk(buffer,"!@:") != NULL)	/* has a hostname */
#ifdef DONT_TOUCH_ADDRESSES
	      return((char *) buffer);
#else
	      return(expand_system(buffer, TRUE));
#endif
	    else
	      return((char *) buffer);
	  }
	
	return( (char *) NULL);
}

char *expand_system(buffer, show_errors)
char *buffer;
int   show_errors;
{
	/** This routine will check the first machine name in the given path 
	    (if any) and expand it out if it is an alias...if not, it will 
	    return what it was given.  If show_errors is false, it won't 
	    display errors encountered...
	**/

	dprint2(6, "expand_system(%s, show-errors=%s)\n", buffer,
		onoff(show_errors));
	findnode(buffer, show_errors);

	return( (char *) buffer);
}
	      
char *expand_group(members, depth)
char *members;
int  depth;
{
	/** Given a group of names separated by commas, this routine
	    will return a string that is the full addresses of each
	    member separated by spaces. Depth is an internal counter
	    that keeps track of the depth of nesting that the routine
	    is in...it's for the get_token routine!  **/

	static char buffer[VERY_LONG_STRING];
	char   buf[LONG_STRING], *word, *address, *bufptr;
	char   *strcpy();

	strcpy(buf, members); 			/* parameter safety! */
	if (depth == 0) buffer[0] = '\0';	/* nothing in yet!   */
	bufptr = (char *) buf;			/* grab the address  */
	depth++;				/* one deeper!       */

	while ((word = get_token(bufptr, "!, ", depth)) != NULL) {
	  if ((address = get_alias_address(word, 1, depth)) == NULL) {
	    if (! valid_name(word)) {
	      dprint2(3, "Encountered illegal address %s (%s)\n",
			word, "expand_group");
	      error1("%s is an illegal address!", word);
	      return( (char *) NULL);
	    }
	    else if (strcmp(buffer, word) != 0)
	      sprintf(buffer, "%s%s%s", buffer,
		    (strlen(buffer) > 0)? ", ":"", word);
	  }
	  else if (strcmp(buffer, address) != 0)
	    sprintf(buffer,"%s%s%s", buffer, 
		    (strlen(buffer) > 0)? ", ":"", address);

	  bufptr = NULL;
	}

	return( (char *) buffer);
}

int
find(word, table, size)
char *word;
struct alias_rec table[];
int size;
{
	/** find word and return loc, or -1 **/
	register int loc;
	
	if (strlen(word) > 20) {
	  dprint2(3, "Too long alias name entered [%s] (%s)\n", word, "find");
	  error1("Bad alias name: %s.  Too long.\n", word);
	  return(-1);
	}

	loc = hash_it(word, size);

	while (strcmp(word, table[loc].name) != 0) {
	  if (table[loc].name[0] == '\0')
	    return(-1);
	  loc = (loc + 1) % size; 
	}

	return(loc);
}

int
hash_it(string, table_size)
char *string;
int   table_size;
{
	/** compute the hash function of the string, returning
	    it (mod table_size) **/

	register int i, sum = 0;
	
	for (i=0; string[i] != '\0'; i++)
	  sum += (int) string[i];

	return(sum % table_size);
}

get_line(fd, buffer)
int fd;
char *buffer;
{
	/* Read from file fd.  End read upon reading either 
	   EOF or '\n' character (this is where it differs 
	   from a straight 'read' command!) */

	register int i= 0;
	char     ch;

	while (read(fd, &ch, 1) > 0)
	  if (ch == '\n' || ch == '\r') {
	    buffer[i] = 0;
	    return;
	  }
	  else
	    buffer[i++] = ch;
}
