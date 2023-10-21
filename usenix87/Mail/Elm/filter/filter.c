/**			filter.c			**/

/** This program is used as a filter within the users ``.forward'' file
    and allows intelligent preprocessing of mail at the point between
    when it shows up on the machine and when it is actually put in the
    mailbox.

    The program allows selection based on who the message is FROM, TO, or
    what the subject is.  Acceptable actions are for the program to DELETE
    the message, SAVE the message in a specified folder, FORWARD the message
    to a specified user, SAVE the message in a folder, but add a copy to the
    users mailbox anyway, or simply add the message to the incoming mail.

    Filter also keeps a log of what it does as it goes along, and at the
    end of each `quantum' mails a summary of actions, if any, to the user.

    Uses the files: $HOME/.filter for instructions to this program, and
    $HOME/.filterlog for a list of what has been done since last summary.

   (C) Copyright 1986, Dave Taylor
**/


#include <stdio.h>
#include <pwd.h>
#include <ctype.h>
#ifdef BSD
# include <sys/time.h>
#else
# include <time.h>
#endif
#include <fcntl.h>

#include "defs.h"

#define  MAIN_ROUTINE			/* for the filter.h file, of course! */
#include "filter.h"

main(argc, argv)
int argc;
char *argv[];
{
	FILE   *fd;				/* for output to temp file! */
	struct passwd  *passwd_entry, 
		       *getpwuid();		/* for /etc/passwd          */
	char filename[SLEN],			/* name of the temp file    */
	     buffer[LONG_SLEN];			/* input buffer space       */
	int  in_header = TRUE,			/* for header parsing       */
	     in_to     = FALSE,			/* are we on 'n' line To: ? */
	     c;					/* var for getopt routine   */

	/* first off, let's get the info from /etc/passwd */ 
	
	if ((passwd_entry = getpwuid(getuid())) == NULL) 
	  leave("Cannot get password entry for this uid!");

	strcpy(home, passwd_entry->pw_dir);
	strcpy(username, passwd_entry->pw_name);

	gethostname(hostname, sizeof(hostname));

	/* now parse the starting arguments... */

	while ((c = getopt(argc, argv, "aclnrSsv")) > 0) {
	  switch (c) {
	    case 'a' : audible = TRUE;				break;
	    case 'c' : clear_logs = TRUE;			break;
	    case 'l' : log_actions_only = TRUE;			break;
	    case 'r' : if (get_filter_rules() == -1)
		         fprintf(stderr,"Couldn't get rules!\n");
		       else
		         print_rules();
		       exit(0);
	    case 's' : if (get_filter_rules() == -1) exit(1);
	  	       show_summary(); 				exit(0);
	
	    case 'S' : long_summary = TRUE;	
	  	       show_summary(); 				exit(0);

	    case 'n' : show_only = TRUE;			break;
	    case 'v' : verbose = TRUE;				break;
	  }
	}

	if (c < 0) {
	  fprintf(stderr, "Usage: | filter [-nrv]\n\   or: filter [-c] -s\n");
          exit(1);
	}

	/* next, create the tempfile and save the incoming message */

	sprintf(filename, "%s.%d", filter_temp, getpid());

	if ((fd = fopen(filename,"w")) == NULL)
	  leave("Cannot open temporary file!");

	while (gets(buffer) != NULL) {
	  if (in_header) {

	    if (! whitespace(buffer[0])) 
		in_to = FALSE;

	    if (the_same(buffer, "From ")) 
	      save_from(buffer);
	    else if (the_same(buffer, "Subject:")) 
	      save_subject(buffer);
	    else if (the_same(buffer, "To:")) {
	      in_to++;
	      save_to(buffer);
	    }
	    else if (the_same(buffer, "X-Filtered-By:")) 
	      already_been_forwarded++;	/* could be a loop here! */
	    else if (strlen(buffer) < 2) 
	      in_header = 0;
	    else if (whitespace(buffer[0]) && in_to)
	      strcat(to, buffer);
	  }
	
          fprintf(fd, "%s\n", buffer);	/* and save it regardless! */
	  fflush(fd);
	  lines++;
	}

	fclose(fd);

	/** next let's see if the user HAS a filter file, and if so what's in
            it (and so on) **/

	if (get_filter_rules() == -1)
	  mail_message(username);
	else {
	  switch (action_from_ruleset()) {

	    case DELETE : if (verbose)
			    printf("%sfilter (%s): Message deleted\n",
				    BEEP, username);
			  log(DELETE);					break;

	    case SAVE   : if (save_message(rules[rule_choosen].argument2)) {
			    mail_message(username);
			    log(FAILED_SAVE);
			  }
			  else
		 	    log(SAVE);					break;

	    case SAVECC : if (save_message(rules[rule_choosen].argument2))
			    log(FAILED_SAVE);
			  else
		            log(SAVECC);					
			  mail_message(username);			break;

	    case FORWARD: mail_message(rules[rule_choosen].argument2);
			  log(FORWARD);					break;

	    case EXEC   : execute(rules[rule_choosen].argument2);
			  log(EXEC);					break;

	    case LEAVE  : mail_message(username);
			  log(LEAVE);					break;
	  }
	}

	(void) unlink(filename);	/* remove the temp file, please! */
	exit(0);	
}

save_from(buffer)
char *buffer;
{
	/** save the SECOND word of this string as FROM **/

	register int i, j;

	for (i=0; buffer[i] != ' '; i++) 	;	/* get to word     */

	for (i++, j=0; buffer[i] != ' ' && i < strlen(buffer); i++) 
	  from[j++] = buffer[i];			/* copy it and     */

	from[j++] = '\0';				/* Null terminate! */
}

save_subject(buffer)
char *buffer;
{
	/** save all but the word "Subject:" for the subject **/

	register int skip = 8;  /* skip "Subject:" initially */

	while (buffer[skip] == ' ') skip++;

	strcpy(subject, (char *) buffer + skip);
}

save_to(buffer)
char *buffer;
{
	/** save all but the word "To:" for the to list **/

	register int skip = 3;	/* skip "To:" initially */

	while (buffer[skip] == ' ') skip++;

	strcpy(to, (char *) buffer + skip);
}
