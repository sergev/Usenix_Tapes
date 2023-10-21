/**			actions.c			**/

/** RESULT oriented routines *chuckle*.  These routines implement the
    actions that result from either a specified rule being true or from
    the default action being taken.

    (C) Copyright 1986, Dave Taylor
**/

#include <stdio.h>
#include <pwd.h>
#include <ctype.h>
#include <fcntl.h>

#include "defs.h"
#include "filter.h"

mail_message(address)
char *address;
{
	/** Called with an address to send mail to.   For various reasons
	    that are too disgusting to go into herein, we're going to actually
	    open the users mailbox and by hand add this message.  Yech.
	    NOTE, of course, that if we're going to MAIL the message to someone
	    else, that we'll try to do nice things with it on the fly...
	**/

	FILE *pipefd, *tempfd, *mailfd;
	int  attempts = 0, ret, in_header = TRUE, line_count = 0;
	char tempfile[SLEN], mailbox[SLEN], lockfile[SLEN],
	     buffer[VERY_LONG_STRING];

	if (verbose && ! log_actions_only)
	  printf("%sfilter (%s): Mailing message to %s\n", 
		  BEEP, username, address);

	if (! show_only) {
	  sprintf(tempfile, "%s.%d", filter_temp, getpid());

	  if ((tempfd = fopen(tempfile, "r")) < 0) {
	    fprintf(stderr, "%sfilter (%s): Can't open temp file %s!!\n", 
		    BEEP, username, tempfile);
	    exit(1);
	  }
	 	
	  if (strcmp(address, username) != 0) {	/* mailing to someone else */
	    
	    if (already_been_forwarded) {	/* potential looping! */
	      if (contains(from, username)) {
	        fprintf(stderr, 
	"%sfilter (%s): Filter loop detected!  Message left in file %s.%d\n", 
			BEEP, username, filter_temp, getpid());
	        exit(0);
	      }
	    }

	    sprintf(buffer, "%s %s %s", sendmail, smflags, address);

	    if ((pipefd = popen(buffer, "w")) == NULL) {
	      fprintf(stderr, "%sfilter (%s): popen %s failed!\n", 
		      BEEP, buffer);
	      sprintf(buffer, "((%s %s %s ; %s %s) & ) < %s &",
		      sendmail , smflags, address, remove, tempfile, tempfile);
	      system(buffer);
	      return;
	    }

	    fprintf(pipefd, "Subject: \"%s\"\n", subject);
	    fprintf(pipefd, "From: The Filter of %s@%s <%s>\n", 
		    username, hostname, username);
	    fprintf(pipefd, "To: %s\n", address);
	    fprintf(pipefd, "X-Filtered-By: filter, version %s\n\n", VERSION);

	    fprintf(pipefd, "-- Begin filtered message --\n\n");
	
	    while (fgets(buffer, LONG_SLEN, tempfd) != NULL)
	      if (already_been_forwarded && in_header)
	        in_header = (strlen(buffer) == 1? 0 : in_header);
	      else
	        fprintf(pipefd," %s", buffer);

	    fprintf(pipefd, "\n-- End of filtered message --\n");
	    fclose(pipefd);
	    fclose(tempfd);
	
	    return;		/* YEAH!  Wot a slick program, eh? */
	  
	  }
	  
	  /** else it is to the current user... **/

	  sprintf(mailbox,  "%s%s", mailhome, username);
	  sprintf(lockfile,  "%s%s.lock", mailhome, username);

	  while ((ret=creat(lockfile, 0777)) < 0  && attempts++ < 10) 
	    sleep(2);	/* wait two seconds, okay?? */

	  if (ret < 0) {
	    fprintf(stderr, "%sfilter (%s): Couldn't create lockfile %s\n",
		    BEEP, username, lockfile);
	    strcpy(mailbox,"[due to lock not being allowed]");
	    /* doing that copy will make sure the next 'open' fails... */
	  }

	  if (mailbox[0] == '[' || (mailfd=fopen(mailbox,"a")) == NULL) {

	    fprintf(stderr, "%sfilter (%s): Can't open mailbox %s!\n",
			BEEP, username, mailbox);

	    sprintf(mailbox, "%s/%s", home, EMERGENCY_MAILBOX);
	    if ((mailfd=fopen(mailbox, "a")) == NULL) {
	      fprintf(stderr,"%sfilter (%s): Can't open %s either!!\n",
		      BEEP, username, mailbox);

	      sprintf(mailbox,"%s/%s", home, EMERG_MBOX); 
	      if ((mailfd = fopen(mailbox, "a")) == NULL) {
	         fprintf(stderr,"%sfilter (%s): Can't open %s either!!!!\n",
		      BEEP, username, mailbox);
	         fprintf(stderr, 
		         "%sfilter (%s): I can't open ANY mailboxes!  Augh!!\n",
			 BEEP, username);
	         fclose(tempfd);
		 unlink(lockfile);
	         leave("Cannot open any mailbox");	/* DIE DIE DIE DIE!! */
	      }
	      else
	        fprintf(stderr,"%sfilter (%s): Using %s as emergency mailbox\n",
			BEEP, username, mailbox);
	    }
	    else
	      fprintf(stderr,"%sfilter (%s): Using %s as emergency mailbox\n",
		      BEEP, username, mailbox);
	  }

	  while (fgets(buffer, sizeof(buffer), tempfd) != NULL) {
	    line_count++;
	    if (the_same(buffer, "From ") && line_count > 1)
	      fprintf(mailfd, ">%s", buffer);
	    else
	      fputs(buffer, mailfd);
	  }

	  fclose(mailfd);
	  unlink(lockfile);	/* blamo! */
	  fclose(tempfd);
	}
}

save_message(foldername)
char *foldername;
{
	/** Save the message in a folder.  Use full file buffering to
	    make this work without contention problems **/

	FILE  *fd, *tempfd;
	char  filename[SLEN], buffer[LONG_SLEN];

	if (verbose)
	  printf("%sfilter (%s): Message saved in folder %s\n", 
		  BEEP, username, foldername);
	
	if (!show_only) {
	  sprintf(filename, "%s.%d", filter_temp, getpid());

	  if ((fd = fopen(foldername, "a")) == NULL) {
	    fprintf(stderr, 
		 "%sfilter (%s): can't save message to requested folder %s!\n",
		    BEEP, username, foldername);
	    return(1);
	  }

	  if ((tempfd = fopen(filename, "r")) == NULL) {
	     fprintf(stderr, 
		     "%sfilter (%s): can't open temp file for reading!\n",
		     BEEP, username);
	     return(1);
	  }

	  while (fgets(buffer, sizeof(buffer), tempfd) != NULL)
	    fputs(buffer, fd);
	
	  fclose(fd);
	  fclose(tempfd);
	}

 	return(0);
}

execute(command)
char *command;
{
	/** execute the indicated command, feeding as standard input the
	    message we have.
	**/

	char buffer[LONG_SLEN];

	if (verbose)
	  printf("%sfilter (%s): Executing %s\n", 
		  BEEP, username, command);

	if (! show_only) {
	  sprintf(buffer, "%s %s.%d | %s", cat, filter_temp, getpid(), command);
	  system(buffer);
	}
}
