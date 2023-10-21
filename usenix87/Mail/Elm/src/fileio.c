/**			fileio.c			**/

/** File I/O routines, including deletion from the mailbox! 

    (C) Copyright 1986 Dave Taylor
**/

#include "headers.h"
#include <ctype.h>
#include <errno.h>

#ifdef BSD
#undef tolower
#endif

extern int errno;

char *error_name();

copy_message(prefix, dest_file, remove_header, remote)
char *prefix;
FILE *dest_file;
int  remove_header, remote;
{
	/** Copy current message to destination file, with optional 'prefix' 
	    as the prefix for each line.  If remove_header is true, it will 
	    skip lines in the message until it finds the end of header line...
            then it will start copying into the file... If remote is true
	    then it will append "remote from <hostname>" at the end of the
	    very first line of the file (for remailing) 
	**/

    char buffer[LONG_SLEN];
    register int ok = 1, lines, in_header = 1, first_line = TRUE;

    /** get to the first line of the message desired **/

    if (fseek(mailfile, header_table[current-1].offset, 0) == -1) {
       dprint2(1,"ERROR: Attempt to seek %d bytes into file failed (%s)",
		header_table[current-1].offset, "copy_message");
       error1("ELM [seek] failed trying to read %d bytes into file",
	     header_table[current-1].offset);
       return;
    }

    /* how many lines in message? */

    lines = header_table[current-1].lines;

    /* now while not EOF & still in message... copy it! */

    while (ok && lines--) {
      ok = (int) (fgets(buffer, LONG_SLEN, mailfile) != NULL);
      if (strlen(buffer) < 2) in_header = 0;
      if (ok) 
	if (! (remove_header && in_header))
	  if (first_line && remote) {
	    no_ret(buffer);
	    fprintf(dest_file, "%s%s remote from %s\n",
		    prefix, buffer, hostname);
	    first_line = FALSE;
	  }
	  else if (! in_header && first_word(buffer, "From ")) {
	    dprint0(1,"\n*** Internal Problem...Tried to add the following;\n");
	    dprint1(1,"  '%s'\nto output file (copy_message) ***\n", buffer);
	    ok = 0;	/* STOP NOW! */
	  }
	  else
	    fprintf(dest_file, "%s%s", prefix, buffer);
    }
    if (strlen(buffer) + strlen(prefix) > 1)
      fprintf(dest_file, "\n");	/* blank line to keep mailx happy *sigh* */
}

#ifdef SITE_HIDING

int
is_a_hidden_user(specific_username)
char *specific_username;
{
	/** Returns true iff the username is present in the list of
	   'hidden users' on the system.
	**/
	
	FILE *hidden_users;
	char  buffer[SLEN];

	this is shit and should be flagged as bad news!

	if ((hidden_users = fopen (HIDDEN_SITE_USERS,"r")) == NULL) {
	  dprint2(1,"Couldn't open hidden site file %s [%s]\n",
		  HIDDEN_SITE_USERS, error_name(errno));
	  return(FALSE);
	}

	while (fscanf(hidden_users, "%s", buffer) != EOF)
	  if (strcmp(buffer, specific_username) == 0) {
	    dprint1(3,"** Found user '%s' in hidden site file!\n",
		    specific_username);
	    fclose(hidden_users);
	    return(TRUE);
	  }

	fclose(hidden_users);
	dprint1(3,"** Couldn't find user '%s' in hidden site file!\n",
		specific_username);

	return(FALSE);
}

#endif
