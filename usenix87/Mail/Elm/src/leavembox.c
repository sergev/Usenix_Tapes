/**			leavembox.c			**/

/** leave current mailbox, updating etc. as needed...
  
    (C) Copyright 1985, Dave Taylor
**/

#include "headers.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#define  ECHOIT 	1	/* echo on for prompting! */

/** added due to a bug in the 2.1 OS **/

struct utimbuf {
	time_t	actime;		/** access time **/
	time_t  modtime;	/** modification */
       };

extern int errno;

char *error_name(), *error_description(), *strcpy();
unsigned short getegid();
unsigned long sleep();

int
leave_mbox(quitting)
int quitting;
{
	/** Exit, saving files into mbox and deleting specified, or simply 
	    delete specified mail... If "quitting" is true, then output status 
            regardless of what happens.  Returns 1 iff mailfile was
	    changed (ie messages deleted from file), 0 if not, and -1 if new
	    mail has arrived in the meantime...
	**/

	FILE *temp;
	char outfile[SLEN], buffer[SLEN];
	struct stat buf;		/* stat command  */
	struct utimbuf times;		/* utime command */
	register int to_delete = 0, to_save = 0, i, mode = 00644,
		     pending = 0, number_saved = 0, last_sortby;
	char dflt;
	long bytes();

	dprint0(1,"\n\n-- leaving_mailbox --\n\n");

	if (message_count == 0) 
	  return(FALSE);	/* nothing changed */

	for (i = 0; i < message_count; i++)
	  if (ison(header_table[i].status, DELETED)) to_delete++;
	  else                                       to_save++;

	dprint2(2,"Count: %d to delete and %d to save\n", to_delete, to_save);

	if (mbox_specified == 0) 
	  update_mailtime();

	if (hp_softkeys && question_me) {
	  define_softkeys(YESNO);		/* YES or NO on softkeys */
	  softkeys_on();
	}

	if (always_del) 	/* set up the default answer... */
	  dflt = 'y';
	else 
	  dflt = 'n';

	if (question_me && to_delete)
	  if (to_save) {
	    fflush(stdin);
	    sprintf(buffer, "Delete message%s? (y/n) ", plural(to_delete));
	    if (want_to(buffer, dflt, ECHOIT) != 'y') {
	      if (mbox_specified == 0) unlock();	/* remove lock! */
	      dprint1(3,"\tDelete message%s? - answer was NO\n", 
			plural(to_delete));
	      error("Nothing deleted");
	      return(FALSE);	/* nothing was deleted! */
	    }
	  }
	  else if (! to_save) {		/* nothing to save!! */
	    fflush(stdin);
	    if (want_to("Delete all mail? (y/n) ", dflt, ECHOIT)!='y') {
	      if (mbox_specified == 0) unlock();	/* remove lock! */
	      dprint0(3,"Delete all mail? - answer was NO\n");
	      error("Nothing deleted");
	      return(FALSE);   /* nothing was deleted */
	    }
	  }

	if (always_leave) 
	  dflt = 'y';
	else
	  dflt = 'n';
	  
	/** we have to check to see what the sorting order was...so that
	    the order of saved messages is the same as the order of the
	    messages originally (a subtle point...) **/

	if (sortby != RECEIVED_DATE) {	/* what we want anyway! */
	  last_sortby = sortby;
	  sortby = RECEIVED_DATE;
	  sort_mailbox(message_count, FALSE);
	  sortby = last_sortby;
	}

	if (question_me && to_save && mbox_specified == 0) {
	  fflush(stdin);
	  if (want_to("Keep mail in incoming mailbox? (y/n) ",dflt, ECHOIT)
	      == 'y') 
	      if (to_delete)	    /* okay - keep undeleted as pending!    */
	        pending++;
	      else {	   	    /* gag! nothing to delete, don't save!  */	
	        unlock();	    /* remove mailfile lock!	   	    */
	        dprint0(3,"Keep mail in incoming mailbox? -- answer was YES\n");
	        error("Mailbox unchanged");
	        return(FALSE);	/* nothing changed! */
	      }
	}

	/** okay...now lets do it! **/

	if (to_save > 0) {
	  if (to_delete > 0)
	    sprintf(buffer ,"[%s %d message%s, and deleting %d]", 
	          pending? "keeping" : "storing", 
		  to_save, plural(to_save), to_delete);
	  else if (quitting)
	    sprintf(buffer,"[%s %s]",
	          pending? "keeping" : "storing",
		  to_save > 1? "all messages" : "message");
	  else
	    buffer[0] = '\0';	/* no string! */
	}
	else {
	  if (to_delete > 0)
	    sprintf(buffer, "[deleting all messages]");
	  else if (quitting)
	    sprintf(buffer, "[no messages to %s, and none to delete]",
	            pending? "keep" : "save");
	  else
	    buffer[0] = '\0';
	}

	dprint1(2,"Action: %s\n", buffer);

	error(buffer);

	if (! mbox_specified) {
	  if (pending) {                /* keep some messages pending! */
	    sprintf(outfile,"%s%d", temp_mbox, getpid());
	    unlink(outfile);
	  }
	  else if (mailbox_defined)	/* save to specified mailbox */
	    strcpy(outfile, mailbox);
	  else				/* save to $home/mbox */
	    sprintf(outfile,"%s/mbox", home);
	}
	else {
	  if (! to_delete) return(FALSE);	/* no work to do! */
          sprintf(outfile, "%s%d", temp_file, getpid());
	  unlink(outfile); /* ensure it's empty! */
	}

	if (to_save) {
	  if ((errno = can_open(outfile, "a"))) {
	    error1(
	         "Permission to append to %s denied!  Leaving mailbox intact\n",
		 outfile);
	    dprint2(1,
		 "Error: Permission to append to outfile %s denied!! (%s)\n",
		 outfile, "leavembox");
	    dprint2(1,"** %s - %s **\n", error_name(errno),
		 error_description(errno));
	    unlock();
	    return(0);
	  }
	  if ((temp = fopen(outfile,"a")) == NULL) {
	    if (mbox_specified == 0)
	      unlock();		/* remove mailfile lock! */
	    dprint1(1,"Error: could not append to file %s\n", 
		    outfile);
	    dprint2(1,"** %s - %s **\n", error_name(errno),
		 error_description(errno));
	    sprintf(buffer, "           Could not append to file %s!          ",
		    outfile);
	    Centerline(LINES-1, buffer);
	    emergency_exit();
	  }
  
	  for (i = 0; i < message_count; i++)
	    if (! (header_table[i].status & DELETED)) {
	      current = i+1;
	      if (! number_saved++) {
	        dprint2(2,"Saving message%s #%d, ", plural(to_save), current);
	      }
	      else {
		dprint1(2,"#%d, ", current);
	      }
	      copy_message("", temp, FALSE, FALSE);
	    }
	  fclose(temp);
	  dprint0(2,"\n\n");
	}

	/* remove source file...either default mailbox or original copy of 
           specified one! */

	/** let's grab the original mode and date/time of the mailfile 
	    before removing it **/

        if (stat(infile, &buf) == 0)
	  mode = buf.st_mode & 00777;
	else {
	  dprint2(1,"Error: errno %s attempting to stat file %s\n", 
		     error_name(errno), infile);
          error3("Error %s (%s) on stat(%s)", error_name(errno), 
		error_description(errno), infile);
	}

	fclose(mailfile);	/* close the baby... */
	
	if (mailfile_size != bytes(infile)) {
	  sort_mailbox(message_count, FALSE);	/* display sorting order! */
	  unlock();
	  error("New mail has just arrived - resyncing...");
	  return(-1);
	}
	unlink(infile); 	/* and BLAMO!        */

	if (to_save && (mbox_specified || pending)) {
	  if (link(outfile, infile) != 0) 
	    if (errno == EXDEV) { /** different file devices!  Use copy! **/
	      if (copy(outfile, infile) != 0) {
	        dprint2(1,"leavembox: copy(%s, %s) failed;",
			outfile, infile);
	        dprint2(1,"** %s - %s **\n", error_name(errno),
		     error_description(errno));
	        error("couldn't modify mail file!");
	        sleep(1);
	        sprintf(infile,"%s/%s", home, unedited_mail);
		if (copy(outfile, infile) != 0) {
	          dprint1(1,"leavembox: couldn't copy to %s either!!  Help;", 
			  infile);
	          dprint2(1,"** %s - %s **\n", error_name(errno),
		          error_description(errno));
	          error("something godawful is happening to me!!!");
		  emergency_exit();
	        }
		else {
	          dprint1(1,"\nWoah! Confused - Saved mail in %s (leavembox)\n", 
			  infile);
	          error1("saved mail in %s", infile);
	        }
	      }	
	    }
	    else {
	      dprint2(1,"link(%s, %s) failed (leavembox)\n", outfile, infile);
	      dprint2(1,"** %s - %s **\n", error_name(errno),
			error_description(errno));
	      error2("link failed! %s - %s", error_name(errno),
		error_description(errno));
	      emergency_exit();
	    }
	  unlink(outfile);
	}
	else if (keep_empty_files) {
	  sleep(1);
	  error1("..keeping empty mail file '%s'..", infile);
	  temp = fopen(infile, "w");
	  fclose(temp);
	  chmod(infile, mode);
	  chown(infile, userid, groupid);
	}

	if (mbox_specified == 0) {
	  if (mode != 00644) { /* if not the default mail access mode... */
	    if (! pending) { /* if none still being saved */
	      temp = fopen(infile, "w");
	      fclose(temp);
	    }
	    chmod(infile,mode);

	    /* let's set the access times of the new mail file to be
	       the same as the OLD one (still sitting in 'buf') ! */

	    times.actime = buf.st_atime;
	    times.modtime= buf.st_mtime;

	    if (utime(infile, &times) != 0) {
	      dprint0(1,"Error: encountered error doing utime (leavmbox)\n");
	      dprint2(1,"** %s - %s **\n", error_name(errno), 
		     error_description(errno));
	      error2("Error %s trying to change file %s access time", 
		     error_name(errno), infile);
	    }
	  }
	  unlock();	/* remove the lock on the file ASAP! */

	  /** finally, let's change the ownership of the default
	      outgoing mailbox, if needed **/

	  if (to_save) 
	    chown(outfile, userid, groupid);
	}

#ifdef SAVE_GROUP_MAILBOX_ID
	chown(infile, userid, getegid());	/** see the Config Guide **/
#else
	chown(infile, userid, groupid);		/**  file owned by user  **/
#endif

	return(to_delete);	
}

char lock_name[SLEN];

lock(direction)
int direction;
{
	/** Create lock file to ensure that we don't get any mail 
	    while altering the mailbox contents!
	    If it already exists sit and spin until 
               either the lock file is removed...indicating new mail
	    or
	       we have iterated MAX_ATTEMPTS times, in which case we
	       either fail or remove it and make our own (determined
	       by if REMOVE_AT_LAST is defined in header file

	    If direction == INCOMING then DON'T remove the lock file
	    on the way out!  (It'd mess up whatever created it!).
	**/

	register int iteration = 0, access_val, lock_fd;

	sprintf(lock_name,"%s%s.lock", mailhome, username);

	access_val = access(lock_name, ACCESS_EXISTS);

	while (access_val != -1 && iteration++ < MAX_ATTEMPTS) {
	  dprint1(2,"File '%s' currently exists!  Waiting...(lock)\n", 
		  lock_name);
	  if (direction == INCOMING)
	    PutLine0(LINES, 0, "\nMail being received!\twaiting...");
	  else
	    error1("Attempt %d: Mail being received...waiting", 
                   iteration);
	  sleep(5);
	  access_val = access(lock_name, ACCESS_EXISTS);
	}
	
	if (access_val != -1) {

#ifdef REMOVE_AT_LAST

	  /** time to waste the lock file!  Must be there in error! **/

	  dprint0(2,
	     "Warning: I'm giving up waiting - removing lock file(lock)\n");
	  if (direction == INCOMING)
	    PutLine0(LINES, 0,"\nTimed out - removing current lock file...");
	  else
	    error("Throwing away the current lock file!");

	  if (unlink(lock_name) != 0) {
	    dprint3(1,"Error %s (%s)\n\ttrying to unlink file %s (%s)\n", 
		     error_name(errno), error_description(errno), lock_name);
	    PutLine1(LINES, 0, 
		   "\n\rI couldn't remove the current lock file %s\n\r", 
		   lock_name);
	    PutLine2(LINES, 0, "** %s - %s **\n\r", error_name(errno),
		   error_description(errno));
	    if (direction == INCOMING)
	      leave();
	    else
	      emergency_exit();
	  }
	  
	  /* everything is okay, so lets act as if nothing had happened... */

#else

	  /* okay...we die and leave, not updating the mailfile mbox or
	     any of those! */
	  if (direction == INCOMING) {
	    PutLine1(LINES, 0, "\nGiving up after %d iterations...", iteration);
	    PutLine0(LINES, 0, 
		"Please try to read your mail again in a few minutes.\n");
	    dprint1(2,"Warning:bailing out after %d iterations...(lock)\n", 
		    iteration);
	    leave_locked(0);
	  }
	  else {
	    dprint1(2,"Warning: after %d iterations, timed out! (lock)\n", 
		    iteration);
	    leave(error("Timed out on lock file reads.  Leaving program"));
	  }

#endif
	}

	/* if we get here we can create the lock file, so lets do it! */

	if ((lock_fd = creat(lock_name, 0)) == -1) {
	  dprint2(1,"Can't create lock file: creat(%s) raises error %s (lock)\n", 
		  lock_name, error_name(errno));
	  if (errno == EACCES)
	    leave(error1(
                 "Can't create lock file!  I need write permission in %s!\n\r",
		  mailhome));
	  else {
	    dprint1(1,"Error encountered attempting to create lock %s\n", 
		  lock_name);
	    dprint2(1,"** %s - %s **\n", error_name(errno),
		  error_description(errno));
	    PutLine1(LINES, 0,
         "\n\rError encountered while attempting to create lock file %s;\n\r", 
		  lock_name);
	    PutLine2(LINES, 0, "** %s - %s **\n\r", error_name(errno),
		  error_description(errno));
	    leave();
	  }
	}
	close(lock_fd);	/* close it.  We don't want to KEEP the thing! */
}

unlock()
{
	/** Remove the lock file!    This must be part of the interrupt
	    processing routine to ensure that the lock file is NEVER
	    left sitting in the mailhome directory! **/

	(void) unlink(lock_name);
}
