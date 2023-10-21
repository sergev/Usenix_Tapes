/** 			showmsg.c			**/

/** This file contains all the routines needed to display the specified
    message.

   These routines (C) Copyright 1986 Dave Taylor 

   Modified 6/86 to use pager variable!!!   Hurrah!!!!
   Modified 7/86 to have secure pipes.. *sigh*
**/

#include "headers.h"
#include <ctype.h>
#include <errno.h>
#include <signal.h>

#ifdef BSD
# include <sys/wait.h>
# undef tolower
#endif

extern int errno;

char *error_name(), *strcat(), *strcpy();
void   _exit();

int    memory_lock = FALSE;	/* is it available?? */
int    pipe_abort  = FALSE;	/* did we receive a SIGNAL(SIGPIPE)? */

int
show_msg(number)
int number;
{
	/*** display number'th message.  Get starting and ending lines
	     of message from headers data structure, then fly through
	     the file, displaying only those lines that are between the
	     two!
		Returns non-zero iff screen was changed
	***/

	dprint0(8, "show_msg called\n");

	if (number > message_count) {
	  error1("Only %d messages!", message_count);
	  return(0);
	}
	else if (number < 1) {
	  error("you can't read THAT message!");
	  return(0);
	}

	clearit(header_table[number-1].status, NEW);   /* it's been read now! */

	memory_lock = FALSE;

	/* some explaination for that last one - We COULD use memory locking
	   to speed up the paging, but the action of "ClearScreen" on a screen
	   with memory lock turned on seems to vary considerably (amazingly so)
	   so it's safer to only allow memory lock to be a viable bit of
	   trickery when dumping text to the screen in scroll mode.
	   Philosophical arguments should be forwarded to Bruce at the 
	   University of Walamazoo, Australia, via ACSNet  *wry chuckle* */

	return(show_message(header_table[number-1].lines, 
	       header_table[number-1].offset,number));
}

int
show_message(lines, file_loc, msgnumber)
int lines, msgnumber;
long file_loc;
{
	/*** Show the indicated range of lines from mailfile
	     for message 'msgnumber' by using 'display'
	     Returns non-zero iff screen was altered.
	***/

	dprint3(9,"show_message(%d,%ld,%d)\n", lines, file_loc, msgnumber);

	if (fseek(mailfile, file_loc, 0) != 0) {
	  dprint2(1,"Error: seek %d bytes into file, errno %s (show_message)\n",
		  file_loc, error_name(errno));
	  error2("ELM failed seeking %d bytes into file (%s)",
		  file_loc, error_name(errno));	
	  return(0);
	}

	if (feof(mailfile))
	  dprint0(1,"\n*** seek put us at END OF FILE!!! ***\n");

	/* next read will get 'this' line so must be at end of previous */

	Raw(OFF);
	if (strcmp(pager,"builtin") == 0 || strcmp(pager,"internal") == 0)
	  display(lines, msgnumber);
	else
	  secure_display(lines, msgnumber);
	Raw(ON);
	if (memory_lock) EndMemlock();	/* turn it off!! */

	return(1);	/* we did it boss! */
}
	

/** This next one is the 'pipe' file descriptor for writing to the 
    pager process... **/

FILE   *output_pipe, *popen();

int
display(lines, msgnum)
int lines, msgnum;
{
	/** Display specified number of lines from file mailfile.
	    Note: This routine MUST be placed at the first line 
	    of the input file! 
	    Returns the same as the routine above (namely zero or one)
	**/

	char from_buffer[LONG_STRING], buffer[VERY_LONG_STRING], *full_month();

	int lines_displayed = 0;	
	int crypted = 0, gotten_key = 0;	/* encryption */
	int weed_header, weeding_out = 0;	/* weeding    */ 
	int mail_sent,				/* misc use   */
	    form_letter = FALSE,		/* Form ltr?  */
	    form_letter_section = 0,		/* section    */
	    builtin = FALSE;			/* our pager? */

	dprint3(4,"displaying %d lines from message %d using %s\n", 
		lines, msgnum, pager);

	ClearScreen();

	if (cursor_control) transmit_functions(OFF);

	pipe_abort = FALSE;

	builtin = (strcmp(pager, "builtin") == 0 || 
		   strcmp(pager,"internal") == 0);

	if (form_letter = (header_table[msgnum-1].status&FORM_LETTER)) {
	  if (filter)
	    form_letter_section = 1;	/* initialize to section 1 */
	}

	if (builtin) 
	  start_builtin(lines);
	else {
	  if ((output_pipe = popen(pager,"w")) == NULL) {
	    error2("Can't create pipe to %s [%s]", pager, 
		    error_name(errno));
	    dprint2(1,"\n*** Can't create pipe to %s - error %s ***\n\n",
	   	    pager, error_name(errno));
	    return(0);
	  }
	  dprint1(4,"Opened a write-only pipe to routine %s \n", pager);
	}

	if (title_messages) {

	  mail_sent = (strncmp(header_table[msgnum-1].from, "To:", 3) == 0);

	  tail_of(header_table[msgnum-1].from, from_buffer, FALSE);

	  sprintf(buffer, "\r%s #%d %s %s%s\t %s %s %s, %d at %s%s\n\r",
		   form_letter? "Form": "Message",
		    msgnum, mail_sent? "to" : "from", from_buffer,
		   (strlen(from_buffer) > 24? "\n\r": 
		     (strlen(from_buffer) > 16 ? "" : "\t")),
		   "Mailed",
     		   full_month(header_table[msgnum-1].month), 
		   header_table[msgnum-1].day, 
	           atoi(header_table[msgnum-1].year) + 1900,
	           header_table[msgnum-1].time,
		   filter? "": "\n\r\n\r");

	  if (builtin)
	    display_line(buffer);
	  else
	    fprintf(output_pipe, "%s", buffer);

	  if (! mail_sent && matches_weedlist("To:") && filter &&
	      strcmp(header_table[current-1].to,username) != 0 &&
	      strlen(header_table[current-1].to) > 0) {
	    sprintf(buffer, "\n\r(message addressed to %s)\n\r", 
		    header_table[current-1].to);
	    if (builtin)
	      display_line(buffer);
	    else
	      fprintf(output_pipe, "%s", buffer);
	  }
	
	  /** The test above is: if we didn't originally send the mail
	      (e.g. we're not reading "mail.sent") AND the user is currently
	      weeding out the "To:" line (otherwise they'll get it twice!)
	      AND the user is actually weeding out headers AND the message 
	      wasn't addressed to the user AND the 'to' address is non-zero 
	      (consider what happens when the message doesn't HAVE a "To:" 
	      line...the value is NULL but it doesn't match the username 
	      either.  We don't want to display something ugly like 
	      "(message addressed to )" which will just clutter up the 
	      display!).

	      And you thought programming was EASY!!!!
	  **/
	}

	weed_header = filter;	/* allow us to change it after header */

	while (lines > 0 && pipe_abort == FALSE) {

	    if (fgets(buffer, VERY_LONG_STRING, mailfile) == NULL) {
	      if (lines_displayed == 0) {

		/* AUGH!  Why do we get this occasionally???  */

	        dprint0(1,
	   	  "\n\n** Out of Sync!!  EOF with nothing read (display) **\n");
		dprint0(1,"** closing and reopening mailfile... **\n\n");

	        if (!builtin) pclose(output_pipe);	/* close pipe NOW! */

		if (mailfile != NULL)
		  fclose(mailfile);		/* huh? */

		if ((mailfile = fopen(infile, "r")) == NULL) {
		  error("Sync error: can't re-open mailbox!!");
		  show_mailfile_stats();
		  emergency_exit();
	        }
	        return(show_message(lines, 
			            header_table[msgnum-1].offset,
				    msgnum));
	      }
	      if (!builtin) 
	        pclose(output_pipe);
	      if (lines == 0 && pipe_abort == FALSE) {	/* displayed it all */
		if (!builtin)
	          PutLine0(LINES,0,"\rPress <return> to return to Elm: ");
	        else
		  printf("\n\r\n\rPress <return> to return to Elm: ");
		fflush(stdout);
	        Raw(ON);
	        (void) ReadCh();
	        Raw(OFF);
	      }
	      return(TRUE);
	    }

	    if (strlen(buffer) > 0) 
              no_ret(buffer);

	    if (strlen(buffer) == 0) {
	      weed_header = 0;		/* past header! */
	      weeding_out = 0;
	    }

	    lines--;
	    lines_displayed++;

	    if (form_letter && weed_header)
		/* skip it.  NEVER display random headers in forms! */;
	    else if (weed_header && matches_weedlist(buffer)) 
	      weeding_out = 1;	 /* aha!  We don't want to see this! */
	    else if (buffer[0] == '[') {
	      if (strcmp(buffer, START_ENCODE)==0)
	        crypted++;
	      else if (strcmp(buffer, END_ENCODE)==0)
	        crypted--;
	      else if (crypted) {
                encode(buffer);
	        show_line(buffer, builtin);
	      }
	      else
	        show_line(buffer, builtin);
	    } 
	    else if (crypted) {
	      if (! gotten_key++) getkey(OFF);
	      encode(buffer);
	      show_line(buffer, builtin); 
	    }
	    else if (weeding_out) {
	      weeding_out = (whitespace(buffer[0]));	/* 'n' line weed */
	      if (! weeding_out) 	/* just turned on! */
	        show_line(buffer, builtin);
	    } 
	    else if (form_letter && first_word(buffer,"***") && filter) {
	      strcpy(buffer,
"\n------------------------------------------------------------------------------\n");
	      show_line(buffer, builtin);	/* hide '***' */
	      form_letter_section++;
	    }
	    else if (form_letter_section == 1 || form_letter_section == 3)
	      /** skip this stuff - we can't deal with it... **/;
	    else
	      show_line(buffer, builtin);
	}

        if (cursor_control) transmit_functions(ON);

	if (! builtin) pclose(output_pipe);

	if (lines == 0 && pipe_abort == FALSE) {  	/* displayed it all! */
	  if (! builtin)
	    PutLine0(LINES,0,"\rPress <return> to return to Elm: ");
	  else
	    printf("\n\r\n\rPress <return> to return to Elm: ");
	  fflush(stdout);
	  Raw(ON);
	  (void) ReadCh();
	  Raw(OFF);
	}
	return(TRUE);
}

show_line(buffer, builtin)
char *buffer;
int  builtin;
{
	/** Hands the given line to the output pipe.  'builtin' is true if
	    we're using the builtin pager.  **/ 
	
	if (builtin) {
	  strcat(buffer, "\n\r");
	  pipe_abort = display_line(buffer);
	}
	else {
	  errno = 0;
	  fprintf(output_pipe, "%s\n", buffer);
	
	  if (errno != 0)
	    dprint1(1,"\terror %s hit!\n", error_name(errno));
	}
}

int
secure_display(lines, msgnumber)
int lines, msgnumber;
{
	/** This is the cheap way to implement secure pipes - spawn a
	    child process running under the old userid, then open the
	    pager and feed the message to it.  When the subprocess ends
	    (the routine returns) simply return.  Simple and effective.
	    (too bad it's this much of a hassle to implement secure
	    pipes, though - I can imagine it being a constant problem!)
	**/

	int pid, w;
#ifdef BSD
	union wait status;
#else
	int status;
#endif
	register int (*istat)(), (*qstat)();
	
#ifdef NO_VM		/* machine without virtual memory! */
	if ((pid = fork()) == 0) {
#else
	if ((pid = vfork()) == 0) {
#endif

	  setgid(groupid);	/* and group id		    */
	  setuid(userid);	/* back to the normal user! */

	  _exit(display(lines, msgnumber));
	}

	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);

	while ((w = wait(&status)) != pid && w != -1)
		;

	signal(SIGINT, istat);
	signal(SIGQUIT, qstat);

#ifdef BSD
	return(status.w_retcode);
#else
	return(status);
#endif
}
