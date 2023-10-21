/** 			mailmsg2.c			**/

/** Interface to allow mail to be sent to users.  Part of ELM  **/

/** (C) Copyright 1986, Dave Taylor 			       **/

#include "headers.h"
#include <errno.h>

extern int errno;

char *error_name(), *error_description(), *strip_parens();
char *strcat(), *strcpy();
char *format_long(), *strip_commas(), *tail_of_string(); 

unsigned long sleep();

#ifdef SITE_HIDING 
 char *get_ctime_date();
#endif
FILE *write_header_info();

extern char subject[SLEN], action[SLEN], reply_to[SLEN], expires[SLEN], 
	    priority [SLEN], to[VERY_LONG_STRING], cc[VERY_LONG_STRING],
            in_reply_to[SLEN], expanded_to[VERY_LONG_STRING], 
	    expanded_cc[VERY_LONG_STRING], user_defined_header[SLEN];
#ifdef ALLOW_BCC
extern char bcc[VERY_LONG_STRING], expanded_bcc[VERY_LONG_STRING];
#endif

int gotten_key = 0;

char *bounce_off_remote();

mail(copy_msg, edit_message, batch, form)
int  copy_msg, edit_message, batch, form;
{
	/** Given the addresses and various other miscellany (specifically, 
	    'copy-msg' indicates whether a copy of the current message should 
	    be included, 'edit_message' indicates whether the message should 
	    be edited and 'batch' indicates that the message should be read 
	    from stdin) this routine will invoke an editor for the user and 
	    then actually mail off the message. 'form' can be YES, NO, or
	    MAYBE.  YES=add "Content-Type: mailform" header, MAYBE=add the
	    M)ake form option to last question, and NO=don't worry about it!
	    Also, if 'copy_msg' = FORM, then grab the form temp file and use
	    that...
	**/

	FILE *reply, *real_reply; /* second is post-input buffer */
	char filename[SLEN], filename2[SLEN], fname[SLEN],
             very_long_buffer[VERY_LONG_STRING];
	char ch;
	register int retransmit = FALSE; 
	int      already_has_text = FALSE;		/* we need an ADDRESS */

	static int cancelled_msg = 0;

	dprint2(4,"\nMailing to '%s'(with%s editing)\n",
		  expanded_to, edit_message? "" : "out");
	
	/** first generate the temporary filename **/

	sprintf(filename,"%s%d",temp_file, getpid());

	/** if possible, let's try to recall the last message? **/

	if (! batch && copy_msg != FORM && user_level != 0)
	  retransmit = recall_last_msg(filename, copy_msg, &cancelled_msg, 
		       &already_has_text);

	/** if we're not retransmitting, create the file.. **/

	if (! retransmit)
	  if ((reply = fopen(filename,"w")) == NULL) {
	    dprint2(1,
               "Attempt to write to temp file %s failed with error %s (mail)\n",
		 filename, error_name(errno));
	    error2("Could not create file %s (%s)",filename,
		 error_name(errno));
	    return(1);
	  }

	if (batch) {
	  Raw(OFF);
	  if (isatty(fileno(stdin))) {
	    fclose(reply);	/* let edit-the-message open it! */
	    printf("To: %s\nSubject: %s\n", expanded_to, subject);
	    strcpy(editor, "none");	/* force inline editor */
	    if (no_editor_edit_the_message(filename)) {
	      return(0);	/* confused?  edit_the_msg returns 1 if bad */
	    }
	    if (verify_transmission(filename, &form) == 'f')
	      return(0);
	  }
	  else {
	    while (gets(very_long_buffer) != NULL) 
	      fprintf(reply, "%s\n", very_long_buffer);
	  }
	}

	if (copy_msg == FORM) {
	  sprintf(fname, "%s%d", temp_form_file, getpid());
	  fclose(reply);	/* we can't retransmit a form! */
	  if (access(fname,ACCESS_EXISTS) != 0) {
	    error("couldn't find forms file!");
	    return(0);
	  }
	  unlink(filename);
	  dprint2(4, "-- linking existing file %s to file %s --\n",
		  fname, filename);
	  link(fname, filename);
	  unlink(fname);
	}
	else if (copy_msg && ! retransmit)    /* if retransmit we have it! */
	  if (edit_message) {
	    copy_message(prefixchars, reply, noheader, FALSE);
	    already_has_text = TRUE;	/* we just added it, right? */
	  }
	  else
	    copy_message("", reply, noheader, FALSE);

	if (!batch && ! retransmit && signature && copy_msg != FORM) {
	  fprintf(reply, "\n--\n");	/* News 2.11 compatibility? */
	  if (chloc(expanded_to, '!') == -1 && chloc(expanded_to, '@') == -1) {
	    if (strlen(local_signature) > 0) {
	      if (local_signature[0] != '/')
	        sprintf(filename2, "%s/%s", home, local_signature);
	      else	
 	        strcpy(filename2, local_signature);
	      (void) append(reply, filename2);
	      already_has_text = TRUE;	/* added signature... */
	    }
	  }
	  else {
	    if (remote_signature[0] != '/')
	      sprintf(filename2, "%s/%s", home, remote_signature);
	    else	
 	      strcpy(filename2, remote_signature);
	    (void) append(reply, filename2);
	    already_has_text = TRUE;	/* added signature... */
	  }
	}

	if (! retransmit && copy_msg != FORM)
	  (void) fclose(reply);	/* on replies, it won't be open! */

	/** Edit the message **/

	if (edit_message)
	  create_readmsg_file();	/* for "readmsg" routine */

	ch = edit_message? 'e' : ' ';	/* drop through if needed... */

	if (! batch) {
	  do {
	    switch (ch) {
	      case 'e': if (edit_the_message(filename, already_has_text)) {
			  cancelled_msg = TRUE;
			  return(1);
			}
			break;
	      case 'h': edit_headers();					break;
	      default : /* do nothing */ ;
	    }

	    /** ask that silly question again... **/
  
	    if ((ch = verify_transmission(filename, &form)) == 'f') {
	      cancelled_msg = TRUE;
	      return(1);
	    }
	  } while (ch != 's');

	  if (form == YES) 
	    if (format_form(filename) < 1) {
	      cancelled_msg = TRUE;
	      return(1);
	    }

	  if ((reply = fopen(filename,"r")) == NULL) {
	      dprint2(1,
	    "Attempt to open file %s for reading failed with error %s (mail)\n",
                filename, error_name(errno));
	      error1("Could not open reply file (%s)", error_name(errno));
	      return(1);
	  }
	}
	else if ((reply = fopen(filename,"r")) == NULL) {
	  dprint2(1,
	    "Attempt to open file %s for reading failed with error %s (mail)\n",
             filename, error_name(errno));
	  error1("Could not open reply file (%s)", error_name(errno));
	  return(1);
	}

	cancelled_msg = FALSE;	/* it ain't cancelled, is it? */

	/** ask about bounceback if the user wants us to.... **/

	if (uucp_hops(to) > bounceback && bounceback > 0 && copy_msg != FORM) 
	  if (verify_bounceback() == TRUE) {
	    if (strlen(cc) > 0) strcat(expanded_cc, ", ");
	    strcat(expanded_cc, bounce_off_remote(to));
	  }

	/** grab a copy if the user so desires... **/

	if (auto_cc && !batch) 
	  save_copy(subject, expanded_to, expanded_cc, filename, to);

	/** write all header information into real_reply **/

	sprintf(filename2,"%s%d",temp_file, getpid()+1);
	
	/** try to write headers to new temp file **/

	dprint2(6, "Composition file='%s' and mail buffer='%s'\n", 
		    filename, filename2);

	if ((real_reply=write_header_info(filename2, expanded_to, expanded_cc,
#ifdef ALLOW_BCC
				 	  expanded_bcc,
#endif
			                  form == YES)) == NULL) {

	  /** IT FAILED!!  MEIN GOTT!  Use a dumb mailer instead! **/

	  dprint1(3,"** write_header failed: %s\n", error_name(errno));

	  if (cc[0] != '\0')  		/* copies! */
	    sprintf(expanded_to,"%s %s", expanded_to, expanded_cc);

	  sprintf(very_long_buffer, "( (%s -s \"%s\" %s ; %s %s) & ) < %s",
                  mailx, subject, strip_parens(strip_commas(expanded_to)), 
		  remove, filename, filename);

	  error1("Message sent using dumb mailer - %s", mailx);
	  sleep(2);	/* ensure time to see this prompt! */

	}
	else {
	  copy_message_across(reply, real_reply);

	  fclose(real_reply);

	  if (cc[0] != '\0')  				         /* copies! */
	    sprintf(expanded_to,"%s %s", expanded_to, expanded_cc);

#ifdef ALLOW_BCC
	  if (bcc[0] != '\0') {
	    strcat(expanded_to, " ");
	    strcat(expanded_to, expanded_bcc);
	  }
#endif

	  if (access(sendmail, EXECUTE_ACCESS) == 0 
#ifdef SITE_HIDING
	      && ! is_a_hidden_user(username))
#else
	     )
#endif
	    sprintf(very_long_buffer,"( (%s %s %s ; %s %s) & ) < %s",
                  sendmail, smflags, strip_parens(strip_commas(expanded_to)), 
		  remove, filename2, filename2);
	  else 				   /* oh well, use default mailer... */
            sprintf(very_long_buffer,"( (%s %s ; %s %s) & ) < %s", 
                  mailer, strip_parens(strip_commas(expanded_to)), 
		  remove, filename2, filename2);
	}
	
	fclose(reply);

	if (mail_only) {
	  printf("sending mail...");
	  fflush(stdout);
	}
	else {	
	  PutLine0(LINES,0,"sending mail...");
	  CleartoEOLN();
	}

	system_call(very_long_buffer, SH);

	if (mail_only) 
	  printf("\rmail sent!      \n\r");
	else 
	  set_error("Mail sent!");

	return(TRUE);
}

mail_form(address, subj)
char *address, *subj;
{
	/** copy the appropriate variables to the shared space... */

	strcpy(subject, subj);
	strcpy(to, address);
	strcpy(expanded_to, address);

	return(mail(FORM, NO, NO, NO));
}

int
recall_last_msg(filename, copy_msg, cancelled_msg, already_has_text)
char *filename;
int  copy_msg, *cancelled_msg, *already_has_text;
{
	/** If filename exists and we've recently cancelled a message,
	    the ask if the user wants to use that message instead!  This
	    routine returns TRUE if the user wants to retransmit the last
	    message, FALSE otherwise...
	**/

	register int retransmit = FALSE;

	if (access(filename, EDIT_ACCESS) == 0 && *cancelled_msg) {
	  Raw(ON);
	  CleartoEOLN();
	  if (copy_msg)
	    PutLine1(LINES-1,0,"Recall last kept message instead? (y/n) y%c",
		     BACKSPACE);
	  else
	    PutLine1(LINES-1,0,"Recall last kept message? (y/n) y%c", 
		     BACKSPACE);
	  fflush(stdout);
	  if (tolower(ReadCh()) != 'n') {
	    Write_to_screen("Yes",0);	
            retransmit++;
	    *already_has_text = TRUE;
	  }
	  else 
	    Write_to_screen("No",0);	

	  fflush(stdout);

	  *cancelled_msg = 0;
	}

	return(retransmit);
}

int
verify_transmission(filename, form_letter)
char *filename;
int  *form_letter;
{
	/** Ensure the user wants to send this.  This routine returns
	    the character entered.  Modified compliments of Steve Wolf 
	    to add the'dead.letter' feature.
	    Also added form letter support... 
	**/

	FILE *deadfd, *messagefd;
	char ch, buffer[LONG_SLEN], fname[SLEN];

	if (mail_only) {
	  if (isatty(fileno(stdin))) {
	    printf("\n\rAre you sure you want to send this? (y/n) ");
	    CleartoEOLN();
	    printf("y%c", BACKSPACE);
	    fflush(stdin);				/* wait for answer! */
	    fflush(stdout);
	    if (tolower(ReadCh()) == 'n') { 			/* >SIGH< */
	      printf("No\n\r\n\r");
	      /** try to save it as a dead letter file **/
	      
	      sprintf(fname, "%s/%s", home, dead_letter);

	      if ((deadfd = fopen(fname,"a")) == NULL) {
		dprint2(1,
		   "\nAttempt to append to deadletter file '%s' failed: %s\n\r",
		    fname, error_name(errno));
	        printf("Message not saved, Sorry.\n\r\n\r");
		return('f');
	      }
	      else if ((messagefd = fopen(filename, "r")) == NULL) {
		dprint2(1,"\nAttempt to read reply file '%s' failed: %s\n\r",
			filename, error_name(errno));
	        printf("Message not saved, Sorry.\n\r\n\r");
		return('f');
	      }
	
	      /* if we get here we're okay for everything, right? */

	      while (fgets(buffer, LONG_SLEN, messagefd) != NULL)
		fputs(buffer, deadfd);

	      fclose(messagefd);
	      fclose(deadfd);

	      printf("Message saved in file \"$HOME/%s\"\n\r\n\r", dead_letter);

	      return('f');	/* forget it! */
	    }
	    else
	      printf("Yes\n\r\n\r");
	    return('s');       /* send this baby! */
	  }
	  else
	    return('s');	/*    ditto       */
	}
	else if (check_first) {    /* used to check strlen(infile) > 0 too? */
reprompt:
	  MoveCursor(LINES,0);
	  CleartoEOLN();
	  ClearLine(LINES-1);

	  if (user_level == 0)
	    PutLine1(LINES-1,0, "Are you sure you want to send this? (y/n) y%c",
			   BACKSPACE);
	  else if (*form_letter == PREFORMATTED) 
	    PutLine1(LINES-1, 0, 
                 "Choose: edit H)eaders, S)end, or F)orget : s%c",
	         BACKSPACE);
	  else if (*form_letter == YES) 
	    PutLine1(LINES-1, 0, 
                 "Choose: E)dit form, edit H)eaders, S)end, or F)orget : s%c",
	         BACKSPACE);
	  else if (*form_letter == MAYBE) 
	    PutLine1(LINES-1, 0, 
        "Choose: E)dit msg, edit H)eaders, M)ake form, S)end, or F)orget : s%c",
	     BACKSPACE);
	  else
	    PutLine1(LINES-1, 0, 
	     "Please choose: E)dit msg, edit H)eaders, S)end, or F)orget : s%c",
	     BACKSPACE);

	  fflush(stdin);	/* wait for answer! */
	  fflush(stdout);
	  Raw(ON);	/* double check... testing only... */
	  ch = tolower(ReadCh());

	  if (user_level == 0) {
	    if (ch == 'n') {
	      set_error("message cancelled");
	      return('f');
	    }
	    else
	      return('s');
	  }

	  /* otherwise someone who knows what's happenin' so...  */

	  switch (ch) {
	     case 'f': Write_to_screen("Forget",0);
	               set_error(
          "Message kept - Can be restored at next F)orward, M)ail or R)eply ");
		       break;

	     case '\n' :
	     case '\r' :
	     case 's'  : Write_to_screen("Send",0);
			 ch = 's';		/* make sure! */
			 break;

	     case 'm'  : if (*form_letter == MAYBE) {
			   *form_letter = YES;
		           switch (check_form_file(filename)) {
			     case -1 : return('f');
			     case 0  : *form_letter = MAYBE;  /* check later!*/
				       error("No fields in form!");
				       return('e');
			     default : goto reprompt;
	                   }
			 }
			 else {
	                    Write_to_screen("%c??", 1, 07);	/* BEEP */
			    sleep(1);
		            goto reprompt;		/* yech */
	                 }
	     case 'e'  :  if (*form_letter != PREFORMATTED) {
			    Write_to_screen("Edit",0);
	 	            if (*form_letter == YES) 
			      *form_letter = MAYBE;
	                  }
			  else {
	                    Write_to_screen("%c??", 1, 07);	/* BEEP */
			    sleep(1);
		            goto reprompt;		/* yech */
	                 }
			 break;
	     case 'h'  : Write_to_screen("Headers",0);
			 break;

	     default   : Write_to_screen("%c??", 1, 07);	/* BEEP */
			 sleep(1);
		         goto reprompt;		/* yech */
	   }

	   return(ch);
	}
	else return('s');
}

FILE *
#ifdef ALLOW_BCC
 write_header_info(filename, long_to, long_cc, long_bcc, form)
 char *filename, *long_to, *long_cc, *long_bcc;
#else
 write_header_info(filename, long_to, long_cc, form)
 char *filename, *long_to, *long_cc;
#endif
int   form;
{
	/** Try to open filedesc as the specified filename.  If we can,
	    then write all the headers into the file.  The routine returns
	    'filedesc' if it succeeded, NULL otherwise.  Added the ability
	    to have backquoted stuff in the users .elmheaders file!
	**/

	static FILE *filedesc;		/* our friendly file descriptor  */

#ifdef SITE_HIDING
	char  buffer[SLEN];
	int   is_hidden_user;		/* someone we should know about?  */
#endif

	char  *get_arpa_date();

	if ((filedesc = fopen(filename, "w")) == NULL) {
	  dprint1(1,
	    "Attempt to open file %s for writing failed! (write_header_info)\n",
	     filename);
	  dprint2(1,"** %s - %s **\n\n", error_name(errno),
		 error_description(errno));
	  error2("Error %s encountered trying to write to %s", 
		error_name(errno), filename);
	  sleep(2);
	  return(NULL);		/* couldn't open it!! */
	}

#ifdef SITE_HIDING
	if ((is_hidden_user = is_a_hidden_user(username))) {
	  /** this is the interesting part of this trick... **/
	  sprintf(buffer, "From %s!%s %s\n",  HIDDEN_SITE_NAME,
		  username, get_ctime_date());
	  fprintf(filedesc, "%s", buffer);
	  dprint1(1,"\nadded: %s", buffer);
	  /** so is this perverted or what? **/
	}
#endif

	fprintf(filedesc, "To: %s\n", format_long(long_to, strlen("To:")));

	fprintf(filedesc,"Date: %s\n", get_arpa_date());

#ifndef DONT_ADD_FROM
# ifdef SITE_HIDING
	if (is_hidden_user)
	  fprintf(filedesc,"From: %s <%s!%s!%s>\n", full_username,
		  hostname, HIDDEN_SITE_NAME, username);
	else
# endif
# ifdef  INTERNET_ADDRESS_FORMAT
#  ifdef  USE_DOMAIN
	fprintf(filedesc,"From: %s <%s@%s%s>\n", full_username, 
		username, hostname, DOMAIN);
#  else
	fprintf(filedesc,"From: %s <%s@%s>\n", full_username,
		username, hostname);
#  endif
# else
	fprintf(filedesc,"From: %s <%s!%s>\n", full_username,
		hostname, username);
# endif
#endif

	fprintf(filedesc, "Subject: %s\n", subject);

	if (cc[0] != '\0')
	  fprintf(filedesc, "Cc: %s\n", format_long(long_cc, strlen("Cc: ")));

#ifdef ALLOW_BCC
	if (bcc[0] != '\0')
	 fprintf(filedesc, "Bcc: %s\n", format_long(long_bcc, strlen("Bcc: ")));
#endif

	if (strlen(action) > 0)
	    fprintf(filedesc, "Action: %s\n", action);
	
	if (strlen(priority) > 0)
	    fprintf(filedesc, "Priority: %s\n", priority);
	
	if (strlen(expires) > 0)
	    fprintf(filedesc, "Expiration-Date: %s\n", expires);
	
	if (strlen(reply_to) > 0)
	    fprintf(filedesc, "Reply-To: %s\n", reply_to);

	if (strlen(in_reply_to) > 0)
	    fprintf(filedesc, "In-Reply-To: %s\n", in_reply_to);

	if (strlen(user_defined_header) > 0)
	    fprintf(filedesc, "%s\n", user_defined_header);

	add_mailheaders(filedesc);

	if (form)
	  fprintf(filedesc, "Content-Type: mailform\n");

	fprintf(filedesc, "X-Mailer: Elm [version %s]\n\n", VERSION);

	return((FILE *) filedesc);
}

copy_message_across(source, dest)
FILE *source, *dest;
{
	/** copy the message in the file pointed to by source to the
	    file pointed to by dest.  **/

	int  crypted = FALSE;			/* are we encrypting?  */
	int  encoded_lines = 0;			/* # lines encoded     */
	char buffer[LONG_SLEN];			/* file reading buffer */

	while (fgets(buffer, LONG_SLEN, source) != NULL) {
	  if (buffer[0] == '[') {
	    if (strncmp(buffer, START_ENCODE, strlen(START_ENCODE))==0)
	      crypted = TRUE;
	     else if (strncmp(buffer, END_ENCODE, strlen(END_ENCODE))==0)
	      crypted = FALSE;
	     else if (strncmp(buffer, DONT_SAVE, strlen(DONT_SAVE)) == 0)
	      continue;	/* next line? */
	    }
	    else if (crypted) {
	      if (! gotten_key++)
	        getkey(ON);
	      else if (! encoded_lines++)
	        get_key_no_prompt();		/* reinitialize.. */
	     
	      encode(buffer);
	    }
	    fputs(buffer, dest);
	  }
}

int
verify_bounceback()
{
	/** Ensure the user wants to have a bounceback copy too.  (This is
	    only called on messages that are greater than the specified 
	    threshold hops and NEVER for non-uucp addresses.... Returns
	    TRUE iff the user wants to bounce a copy back.... 
	 **/

	if (mail_only) {
	  printf("Would you like a copy \"bounced\" off the remote? (y/n) ");
	  CleartoEOLN();
	  printf("n%c", BACKSPACE);
	  fflush(stdin);				/* wait for answer! */
	  fflush(stdout);
	  if (tolower(ReadCh()) != 'y') {
	    printf("No\n\r");
	    return(FALSE);
	  }
	  else
	    printf("Yes - Bounceback included\n\r");
	}
	else {
	  MoveCursor(LINES,0);
	  CleartoEOLN();
	  PutLine1(LINES,0, 
		"\"Bounce\" a copy off the remote machine? (y/n) y%c",
		BACKSPACE);
	  fflush(stdin);	/* wait for answer! */
	  fflush(stdout);
	  if (tolower(ReadCh()) != 'y') { 
	    Write_to_screen("No", 0);
	    fflush(stdout);
	    return(FALSE);
	  }
	  Write_to_screen("Yes!", 0);
	  fflush(stdout);
	}

	return(TRUE);
}
