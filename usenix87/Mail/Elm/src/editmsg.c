/**			editmsg.c			**/

/** This contains routines to do with starting up and using an editor (or two)
    from within Elm.  This stuff used to be in mailmsg2.c...

    (C) Copyright 1986, Dave Taylor
**/

#include "headers.h"
#include <errno.h>
#include <setjmp.h>
#include <signal.h>

extern int errno;

char *error_name(), *error_description(), *strcpy();
unsigned long sleep();

int
edit_the_message(filename, already_has_text)
char *filename;
int  already_has_text;
{
	/** Invoke the users editor on the filename.  Return when done. 
	    If 'already has text' then we can't use the no-editor option
	    and must use 'alternative editor' (e.g. $EDITOR or default_editor)
	    instead... **/

	char buffer[SLEN];
	register int stat, return_value = 0;

	buffer[0] = '\0';

	if (strcmp(editor, "builtin") == 0 || strcmp(editor, "none") == 0) {
	  if (already_has_text) 
	    sprintf(buffer, "%s %s", alternative_editor, filename);
	  else
	    return(no_editor_edit_the_message(filename));
	}

	PutLine0(LINES, 0, "invoking editor...");  fflush(stdout);
	if (strlen(buffer) == 0)
	  sprintf(buffer,"%s %s", editor, filename);

	Raw(OFF);

	chown(filename, userid, groupid);	/* file was owned by root! */

	if (cursor_control)
	  transmit_functions(OFF);		/* function keys are local */

	if ((stat = system_call(buffer, SH)) != 0) { 
	  dprint1(1,"System call failed with stat %d (edit_the_message)\n", 
		  stat);
	  error1("Can't invoke editor '%s' for composition", editor);
	  dprint2(1,"** %s - %s **\n", error_name(errno), 
		error_description(errno));
	  ClearLine(LINES-1);
	  sleep(2);
	  return_value = 1;
	}

	if (cursor_control)
	  transmit_functions(ON);		/* function keys are local */
	
	Raw(ON);

	return(return_value);
}

extern char to[VERY_LONG_STRING], cc[VERY_LONG_STRING], 
	    expanded_to[VERY_LONG_STRING], expanded_cc[VERY_LONG_STRING], 
            subject[SLEN];

int      interrupts_while_editing;	/* keep track 'o dis stuff         */
jmp_buf  edit_location;		        /* for getting back from interrupt */

#ifdef ALLOW_BCC
extern char bcc[VERY_LONG_STRING], expanded_bcc[VERY_LONG_STRING];
#endif

char *strip_commas();

int
no_editor_edit_the_message(filename)
char *filename;
{
	/** If the current editor is set to either "builtin" or "none", then
	    invoke this program instead.  As it turns out, this and the 
	    routine above have a pretty incestuous relationship (!)...
	**/

	FILE *edit_fd;
	char buffer[SLEN], editor_name[SLEN];
	int      edit_interrupt(), (*oldint)(), (*oldquit)();

	Raw(OFF);

	if ((edit_fd = fopen(filename, "a")) == NULL) {
	  error2("Couldn't open %s for appending [%s]", filename, 
		  error_name(errno));
	  sleep (2);
	  dprint1(1,"Error encountered trying to open file %s;\n", filename);
	  dprint2(1,"** %s - %s **\n", error_name(errno),
		    error_description(errno));
	  Raw(ON);
	  return(1);
	}

	/** file open...let's start accepting input! **/

	printf(
          "\nPlease enter message, ^D to end, or ~? <RETURN> for help;\n\n");

	oldint  = signal(SIGINT,  edit_interrupt);
	oldquit = signal(SIGQUIT, edit_interrupt);

	interrupts_while_editing = 0;

more_input:

	while (gets(buffer) != NULL) {

	  if (setjmp(edit_location) != 0) {
	    if (interrupts_while_editing > 1) {
	      Raw(ON);

	      (void) signal(SIGINT,  oldint);
	      (void) signal(SIGQUIT, oldquit);

	      if (edit_fd != NULL)	/* insurance... */
	        fclose(edit_fd);
	      return(1);
	    }
	    goto more_input;	/* read input again, please! */
	  }
	
	  interrupts_while_editing = 0;	/* reset to zero... */

	  if (strcmp(buffer, ".") == 0)
	    break;	/* '.' is as good as a ^D to us dumb programs :-) */
	  if (buffer[0] == TILDE) 
	    switch (tolower(buffer[1])) {
	      case '?' : tilde_help();	printf("(continue)\n"); goto more_input;
	      case '~' : move_left(buffer, 1); goto tilde_input;	/*!!*/

	      case 't' : get_with_expansion("To: ", to, expanded_to);	break;
#ifdef ALLOW_BCC
	      case 'b' : get_with_expansion("Bcc: ", bcc,expanded_bcc);	break;
#endif
	      case 'c' : get_with_expansion("Cc: ", cc, expanded_cc);	break;
	      case 's' : get_with_expansion("Subject: ", subject,NULL);	break;

	      case 'h' : get_with_expansion("To: ", to, expanded_to);	
	                 get_with_expansion("Cc: ", cc, expanded_cc);
#ifdef ALLOW_BCC
	                 get_with_expansion("Bcc: ", bcc,expanded_bcc);
#endif
	                 get_with_expansion("Subject: ", subject,NULL);	break;

	      case 'r' : read_in_file(edit_fd, (char *) buffer + 2);
		  	 goto more_input;
	      case 'e' : if (strlen(emacs_editor) > 0) 
	                   if (access(emacs_editor, ACCESS_EXISTS) == 0) {
	                     strcpy(buffer, editor);
			     strcpy(editor, emacs_editor);
	  		     fclose(edit_fd);
			     (void) edit_the_message(filename,0);
	   		     Raw(OFF);
			     strcpy(editor, buffer);
			     edit_fd = fopen(filename, "a");
		          printf("(continue entering message, ^D to end)\n");
	                     goto more_input;
		           }
		           else
	                     printf(
			  "(Can't find Emacs on this system!  continue)\n");
			 else
		            printf(
			"(Don't know where Emacs would be...continue)\n");	
			 goto more_input;

	       case 'v' : strcpy(buffer, editor);
			  strcpy(editor, default_editor);
			  fclose(edit_fd);
			  (void) edit_the_message(filename,0);
	   		  Raw(OFF);
			  strcpy(editor, buffer);
			  edit_fd = fopen(filename, "a");
		          printf("(continue entering message, ^D to end)\n");
	                  goto more_input;

	       case 'o' : printf("Please enter the name of the editor : ");
			  gets(editor_name);
	                  if (strlen(editor_name) > 0) {
	                    strcpy(buffer, editor);
			    strcpy(editor, editor_name);
			    fclose(edit_fd);
			    (void) edit_the_message(filename,0);
	 		    Raw(OFF);
			    strcpy(editor, buffer);
			    edit_fd = fopen(filename, "a");
		          printf("(continue entering message, ^D to end)\n");
	                    goto more_input;
		          }
	  		  printf("(continue)\n");
	                  goto more_input; 
		case '!' : if (strlen(buffer) < 3) 
			     (void) system_call(shell, USER_SHELL);
			   else
	                     (void) system_call((char *) buffer+2, USER_SHELL);
	    		   printf("(continue)\n");
			   goto more_input;
		 case 'm' : /* same as 'f' but with leading prefix added */
	         case 'f' : /* this can be directly translated into a
			       'readmsg' call with the same params! */
			    read_in_messages(edit_fd, (char *) buffer + 1);
			    goto more_input;
	         case 'p' : /* print out message so far.  Soooo simple! */
			    print_message_so_far(edit_fd, filename);
			    goto more_input;
		 default  : printf(
			     "(don't know what ~%c is.  Try ~? for help)\n",
				    buffer[1]);
	       }
	     else
tilde_input:
	       fprintf(edit_fd, "%s\n", buffer);
	};

	printf("<end-of-message>\n\n");

	Raw(ON);

	(void) signal(SIGINT,  oldint);
	(void) signal(SIGQUIT, oldquit);

	if (edit_fd != NULL)	/* insurance... */
	  fclose(edit_fd);

	return(0);
}

tilde_help()
{
	/* a simple routine to print out what is available at this level */

	printf(
"(Available 'tilde' commands at this point are;\n\
\n\
\t~?\tPrint this help menu\n\
\t~~\tAdd line prefixed by a single '~' character\n");

#ifdef ALLOW_BCC
	printf("\
\t~b\tChange the addresses in the Blind-carbon-copy list\n");
#endif

	printf("\
\t~c\tChange the addresses in the Carbon-copy list\n\
\t~e\tInvoke the Emacs editor on the message, if possible\n\
\t~f\tadd the specified list of messages, or current\n");
#ifdef ALLOW_BCC
	printf("\t~h\tchange all available headers (to,cc,bcc,subject)\n");
#endif
	printf("\
\t~m\tsame as '~f', but with the current 'prefix'\n\
\t~o\tInvoke a user specified editor on the message\n\
\t~p\tprint out message as typed in so far\n\
\t~r\tRead in the specified file\n\
\t~s\tChange the subject of the message\n\
\t~t\tChange the addresses in the To list\n\
\t~v\tInvoke the Vi visual editor on the message\n\
\t~!\texecute a unix command (or give a shell if no command)\n\
\n");

}

read_in_file(fd, filename)
FILE *fd;
char *filename;
{
	/** Open the specified file and stream it in to the already opened 
	    file descriptor given to us.  When we're done output the number
	    of lines we added, if any... **/

	FILE *myfd;
	char myfname[SLEN], buffer[LONG_SLEN];
	register int lines = 0;

	while (whitespace(filename[lines])) lines++;

	strcpy(myfname, (char *) filename + lines);

	if (strlen(myfname) == 0) {
	  printf("(no filename specified for file read!  Continue...)\n");
	  return;
	}

	if ((myfd = fopen(myfname,"r")) == NULL) {
	  printf("(Couldn't open file '%s' for reading!  Continue...)\n",
		 myfname);
	  return;
	}

	lines = 0;

	while (fgets(buffer, LONG_SLEN, myfd) != NULL) {
	  lines++;
  	  fputs(buffer, fd);
	  fflush(stdout);
	}

	fclose(myfd);

	printf("(added %d line%s from file %s.  Please continue...)\n",
		lines, plural(lines), myfname);
	return;
}

print_message_so_far(edit_fd, filename)
FILE *edit_fd;
char *filename;
{
	/** This prints out the message typed in so far.  We accomplish
	    this in a cheap manner - close the file, reopen it for reading,
	    stream it to the screen, then close the file, and reopen it
	    for appending.  Simple, but effective!

	    A nice enhancement would be for this to -> page <- the message
	    if it's sufficiently long.  Too much work for now, though.
	**/
	
	char buffer[LONG_SLEN];

	fclose(edit_fd);

	if ((edit_fd = fopen(filename, "r")) == NULL) {
	  printf("\nMayday!  Mayday!  Mayday!\n");
	  printf("\nPanic: Can't open file for reading!  Bail!\n");
	  emergency_exit();
	}

	printf("To: %s\n", format_long(to, 4));
	printf("Cc: %s\n", format_long(cc, 4));
#ifdef ALLOW_BCC
	printf("Bcc: %s\n", format_long(bcc, 5));
#endif
	printf("Subject: %s\n\n", subject);

	while (fgets(buffer, LONG_SLEN, edit_fd) != NULL)
	  printf("%s", buffer);

	fclose(edit_fd);

	if ((edit_fd = fopen(filename, "a")) == NULL) {
	  printf("Mayday!  Mayday!  Abandon Ship!  Aiiieeeeee\n");
	  printf("\nPanic: Can't reopen file for appending!\n");
	  emergency_exit();
	}

	printf("(continue entering message, please)\n\n");
}

read_in_messages(fd, buffer)
FILE *fd;
char *buffer;
{
	/** Read the specified messages into the open file.  If the
	    first character of "buffer" is 'm' then prefix it, other-
	    wise just stream it in straight...
	**/

	FILE *myfd, *popen();
	char  local_buffer[LONG_SLEN];
	register int lines = 0, add_prefix=0;

	add_prefix = (tolower(buffer[0]) == 'm');

	sprintf(local_buffer, "%s %s", readmsg, ++buffer);

	if ((myfd = popen(local_buffer, "r")) == NULL) {
	   printf("(can't get to 'readmsg' command.  Sorry...)\n");
	   return;	
	}

	while (fgets(local_buffer, LONG_SLEN, myfd) != NULL) {
	  lines++;
	  if (add_prefix)
	    fprintf(fd, "%s%s", prefixchars, local_buffer);
	  else 
	    fputs(local_buffer, fd);
	}

	pclose(myfd);

	if (lines == 0)
	  printf("(Couldn't add the requested message.   Continue)\n");
	else
	  printf("(added %d line%s to message...  Please continue)\n",
		lines, plural(lines));

	return;
}

get_with_expansion(prompt, buffer, expanded_buffer)
char *prompt, *buffer, *expanded_buffer;
{
	/** This is used to prompt for a new value of the specified field.
	    If expanded_buffer == NULL then we won't bother trying to expand
	    this puppy out!
	**/

	char mybuffer[VERY_LONG_STRING];

	printf(prompt);	fflush(stdout);	/* output! */

	strcpy(mybuffer, buffer);

	Raw(ON);
	optionally_enter(buffer, -1, -1, TRUE);	/* already data! */
	Raw(OFF);
	putchar('\n');

	if (strcmp(buffer, mybuffer) != 0 && expanded_buffer != NULL) 
	  build_address(strip_commas(buffer), expanded_buffer);

	return;
}

edit_interrupt()
{
	/** This routine is called when the user hits an interrupt key
	    while in the builtin editor...it increments the number of 
	    times an interrupt is hit and returns it.
	**/

	signal(SIGINT, edit_interrupt);
	signal(SIGQUIT, edit_interrupt);

	if (! interrupts_while_editing++)
	  printf("(Interrupt.  One more to cancel this letter...)\n\r");

	longjmp(edit_location);		/* get back */
}
