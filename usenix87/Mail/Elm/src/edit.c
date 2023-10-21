/**			edit.c			**/

/** This routine is for allowing the user to edit their current mailbox
    as they wish. 

    (C) Copyright 1986 Dave Taylor
**/

#include "headers.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

extern int errno;

char   *error_name(), *error_description(), *strcpy();
long   bytes();
unsigned long sleep();

edit_mailbox()
{
	/** Allow the user to edit their mailbox, always resynchronizing
	    afterwards.   Due to intense laziness on the part of the
	    programmer, this routine will invoke $EDITOR on the entire
	    file.  The mailer will ALWAYS resync on the mailbox file
	    even if nothing has changed since, not unreasonably, it's
	    hard to figure out what occurred in the edit session...
	
	    Also note that if the user wants to edit their incoming
	    mailbox they'll actually be editing the tempfile that is
	    an exact copy.  More on how we resync in that case later
	    in this code.
	**/

	FILE     *real_mailbox, *temp_mailbox;
	char     filename[SLEN], buffer[LONG_SLEN], temp_infile[SLEN];
	struct   stat stat_buffer;
	int      loaded_stat_buffer = FALSE;

	PutLine0(LINES-1,0,"invoking editor...");

	if (mbox_specified == 0) {
	  sprintf(filename, "%s%s", temp_mbox, username);
	  chown(filename, userid, groupid);		/* make sure we can! */
	}
	else 
	  strcpy(filename, infile);

	/** now get and save the ownership and permissions... **/

	if (stat(infile, &stat_buffer)) {
	  error("Warning: couldn't 'stat' file, perms might get mangled");
	  sleep(2);
	}
	else
	  loaded_stat_buffer = TRUE;

	sprintf(buffer, "%s %s", alternative_editor, filename);

	Raw(OFF);

	if (system_call(buffer, SH) != 0) {
	  error1("Problems invoking editor %s!", alternative_editor);
	  Raw(ON);
	  sleep(2);
	  return(0);
	}

	Raw(ON);

	if (mbox_specified == 0) {	/* uh oh... now the toughie...  */

	  sprintf(temp_infile, "%s%s.temp", mailhome, username);
	  unlink(temp_infile);	/* remove it if it's there... */

	  if (bytes(infile) != mailfile_size) {

	     /* SIGH.  We've received mail since we invoked the editor
		on the mailbox.  We'll have to do some strange stuff to
	        remedy the problem... */

	     PutLine0(LINES, 0, "Warning: new mail received...");
	     CleartoEOLN();

	     if ((temp_mailbox = fopen(filename, "a")) == NULL) {
		dprint2(1, "Attempt to open %s to append failed! (%s)\n", 
			filename, "edit_mailbox");
	       set_error("Couldn't reopen tempfile.  Edit LOST!");
	       return(1);
	     }
	     /** Now let's lock the mailbox up and stream the new stuff 
		 into the temp file... **/

	     lock(OUTGOING);	
	     if ((real_mailbox = fopen(infile, "r")) == NULL) {
	       dprint2(1, 
		       "Attempt to open %s for reading new mail failed! (%s)\n",
 		 	infile, "edit_mailbox");
	       sprintf(buffer, "Couldn't open %s for reading!  Edit LOST!", 
		       infile);
	       set_error(buffer);
	       unlock();
	       return(1);
	     }
	     if (fseek(real_mailbox, mailfile_size, 0) != 0) {
	       dprint2(1, "Couldn't seek to end of infile (offset %ld) (%s)\n",
			mailfile_size, "edit_mailbox");
	       set_error("Couldn't seek to end of mailbox.  Edit LOST!");
	       unlock();
	       return(1);
	     }
	
	     /** Now we can finally stream the new mail into the tempfile **/

	     while (fgets(buffer, LONG_SLEN, real_mailbox) != NULL)
	       fprintf(temp_mailbox, "%s", buffer);

	     fclose(real_mailbox);
	     fclose(temp_mailbox);
 	   }
	   else
	     lock(OUTGOING); /* create a lock file if we're replacing mailbox */

	   /** link to the temporary mailbox in the mailhome directory... **/

	   if (link(filename, temp_infile) != 0) 
	     if (errno == EXDEV) {   /* attempt to link across file systems */
   	       if (copy(filename, temp_infile) != 0) {
	         error("Couldn't copy temp file to mailbox!");
	         unlock();					/* ciao!*/
	   	 emergency_exit();
	       }
	     }
	     else {
		Write_to_screen("\n\rCouldn't link %s to mailfile %s...\n\r",2,
			filename, temp_infile);
		Write_to_screen("** %s - %s **\n\r", 2,
			error_name(errno), error_description(errno));
	        emergency_exit();
	     }
	
	   /***  G U L P ... let's remove the incoming mail file... ***/
	     
	   unlink(infile);

	   /** and quickly now... **/

	   if (link(temp_infile, infile) != 0) {
	     Write_to_screen(
		    "\n\rCouldn't internally link %s to mailfile %s...\n\r",
		    2, temp_infile, infile);
	     Write_to_screen(
		    "\n\rYou'll need to check out %s for your mail...\n\r",
		    1, temp_infile);
	     Write_to_screen("** %s - %s **\n\r", 2,
		    error_name(errno), error_description(errno));
	     emergency_exit();
	   }

	   /** And let's remove the lock file!  We're DONE!!!  **/

	   unlock();
	   unlink(temp_infile);	/* remove the temp file too */
	   unlink(filename);	/* remove the temp file too */
	   error("edit changes incorporated into new mail...");
	}
	else
	  error("Resynchronizing with new version of mailbox...");

	sleep(2);
	resync();

	current = 1;		/* don't leave the user hanging! */

	/** finally restore the permissions... **/

	if (loaded_stat_buffer) {	/* if not, it's junk! */
	  chown(infile, stat_buffer.st_uid, stat_buffer.st_gid);
	  chmod(infile, stat_buffer.st_mode);
	}

	return(1);
}
