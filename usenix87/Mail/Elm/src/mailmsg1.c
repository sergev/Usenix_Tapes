/** 			mailmsg1.c			**/

/** Interface to allow mail to be sent to users.  Part of ELM  **/

/** (C) Copyright 1986, Dave Taylor 			       **/

#include "headers.h"

/** strings defined for the hdrconfg routines **/

char subject[SLEN], action[SLEN], reply_to[SLEN], expires[SLEN], priority[SLEN];
char to[VERY_LONG_STRING], cc[VERY_LONG_STRING], in_reply_to[SLEN];
char user_defined_header[SLEN];

char expanded_to[VERY_LONG_STRING], expanded_cc[VERY_LONG_STRING];

#ifdef ALLOW_BCC
char bcc[VERY_LONG_STRING], expanded_bcc[VERY_LONG_STRING];
#endif

char *format_long(), *strip_commas(), *tail_of_string(), *strcpy();
unsigned long sleep();

int
send(given_to, given_subject, edit_message, form_letter)
char *given_to, *given_subject;
int   edit_message, form_letter;
{
	/** Prompt for fields and then call mail() to send the specified
	    message.  If 'edit_message' is true then don't allow the
            message to be edited. 'form_letter' can be "YES" "NO" or "MAYBE".
	    if YES, then add the header.  If MAYBE, then add the M)ake form
	    option to the last question (see mailsg2.c) etc. etc. **/

	int  copy_msg = FALSE, is_a_response = FALSE;

	/* First: zero all current global message strings */

	cc[0] = action[0] = reply_to[0] = expires[0] = priority[0] = '\0';
#ifdef ALLOW_BCC
	bcc[0] = expanded_bcc[0] = '\0';
#endif
	in_reply_to[0] = expanded_to[0] = expanded_cc[0] = '\0';

	strcpy(subject, given_subject);		/* copy given subject */
	strcpy(to, given_to);			/* copy given to:     */

	/******* And now the real stuff! *******/

	copy_msg=copy_the_msg(&is_a_response); /* copy msg into edit buffer? */

	if (get_to(to, expanded_to) == 0)   /* get the To: address and expand */
	  return(0);

	/** are we by any chance just checking the addresses? **/

	if (check_only) {
	  printf("Expands to: %s\n", format_long(expanded_to, 12));
	  putchar('\r');	/* don't ask... */
	  leave();
	}

	/** if we're batchmailing, let's send it and GET OUTTA HERE! **/

	if (mail_only && strlen(batch_subject) > 0) { 
	  strcpy(subject, batch_subject);	/* get the batch subject */
	  return(mail(FALSE, FALSE, TRUE, form_letter));
	}

	display_to(expanded_to);	/* display the To: field on screen... */

	dprint1(3,"\nMailing to %s\n", expanded_to);
  
	if (get_subject(subject) == 0)	    /* get the Subject: field */
	  return(0);

	dprint1(4,"Subject is %s\n", subject);

	if (get_copies(cc, expanded_to, expanded_cc) == 0)
	  return(0);

	if (strlen(cc) > 0)
	  dprint1(4,"Copies to %s\n", expanded_cc);

	if (mail_only) 				/* indicate next step... */
	  printf("\n\r");
	else
	  MoveCursor(LINES,0);	/* you know you're hit <return> ! */

	/** generate the In-Reply-To: header... **/

	if (is_a_response)
	  generate_reply_to(current-1);

	/* and mail that puppy outta here! */

	mail(copy_msg, edit_message, FALSE, form_letter);
	
	return(edit_message);
}

get_to(to_field, address)
char *to_field, *address;
{
	/** prompt for the "To:" field, expanding into address if possible.
	    This routine returns ZERO if errored, or non-zero if okay **/

	if (strlen(to_field) == 0) {
	  if (user_level < 2) {
	    PutLine0(LINES-2, 0, "Send the message to: ");
	    (void) optionally_enter(to_field, LINES-2, 21, FALSE); 
	  }
	  else {
	    PutLine0(LINES-2, 0, "To: ");
	    (void) optionally_enter(to_field, LINES-2, 4, FALSE); 
	  }
	  if (strlen(to_field) == 0) {
	    ClearLine(LINES-2);	
	    return(0);
	  }
	  build_address(strip_commas(to_field), address); 
	}
	else if (mail_only) 
	  build_address(strip_commas(to_field), address); 
	else 
	  strcpy(address, to_field);
	
	if (strlen(address) == 0) {	/* bad address!  Removed!! */
	  if (! mail_only)
	    ClearLine(LINES-2);
	  return(0);
	}

	return(1);		/* everything is okay... */
}

get_subject(subject_field)
char *subject_field;
{
	/** get the subject and return non-zero if all okay... **/
	int len = 9;

	if (mail_only)
	  printf("Subject: ");
	else 
	  if (user_level == 0) {
	    PutLine0(LINES-2,0,"Subject of message: ");
	    len = 21;
	  }
	  else
	    PutLine0(LINES-2,0,"Subject: ");

	CleartoEOLN();

	if (optionally_enter(subject_field, LINES-2, len, TRUE) == -1) {
	  /** User hit the BREAK key! **/
	  MoveCursor(LINES-2,0); 	
	  CleartoEOLN();
	  error("mail not sent");
	  return(0);
	}

	if (strlen(subject_field) == 0) {	/* zero length subject?? */
	  if (mail_only) 
	    printf("\n\rNo subject - Continue with message? (y/n) n%c",
		  BACKSPACE);
	  else
	    PutLine1(LINES-2,0,"No subject - Continue with message? (y/n) n%c",
		  BACKSPACE);

	  if (tolower(ReadCh()) != 'y') {	/* user says no! */
	    if (mail_only) {
	      printf("\n\r\n\rMail Cancelled!\n\r");
	      return(0);
	    }
	    ClearLine(LINES-2);
	    error("mail not sent");
	    return(0);
	  }
	  else if (! mail_only) {
	    PutLine0(LINES-2,0,"Subject: <none>");
	    CleartoEOLN();
	  }
	}

	return(1);		/** everything is cruising along okay **/
}

get_copies(cc_field, address, addressII)
char *cc_field, *address, *addressII;
{
	/** Get the list of people that should be cc'd, returning ZERO if
	    any problems arise.  Address and AddressII are for expanding
	    the aliases out after entry! 
	    If 'bounceback' is nonzero, add a cc to ourselves via the remote
	    site, but only if hops to machine are > bounceback threshold.
	**/

	if (mail_only)
	  printf("\n\rCopies To: ");
	else
	  PutLine0(LINES-1,0,"Copies To: ");

	fflush(stdout);

	if (optionally_enter(cc_field, LINES-1, 11, FALSE) == -1) {
	  if (mail_only) {
	    printf("\n\r\n\rMail not sent!\n\r");
	    return(0);
	  }
	  ClearLine(LINES-2);
	  ClearLine(LINES-1);
	  
	  error("mail not sent");
	  return(0);
	}
	
	build_address(strip_commas(cc_field), addressII);

	if (strlen(address) + strlen(addressII) > VERY_LONG_STRING) {
	  dprint0(2,
		"String length of \"To:\" + \"Cc\" too long! (get_copies)\n");
	  error("Too many people.  Copies ignored");
	  sleep(2);
	  cc_field[0] = '\0';
	}

	return(1);		/* everything looks okay! */
}
	
int
copy_the_msg(is_a_response)
int *is_a_response;
{
	/** Returns True iff the user wants to copy the message being
	    replied to into the edit buffer before invoking the editor! 
	    Sets "is_a_response" to true if message is a response...
	**/

	int answer = FALSE;

	if (strlen(to) > 0 && !mail_only) {	/* predefined 'to' line! */
	  if (auto_copy) 
	    answer = TRUE;
	  else 
	    answer = (want_to("Copy message? (y/n) ", 'n', TRUE) == 'y');
	  *is_a_response = TRUE;
	}
	else
	  if (strlen(subject) > 0)  	/* predefined 'subject' (Forward) */
	    answer = TRUE;

	return(answer);
}

display_to(address)
char *address;
{
	/** Simple routine to display the "To:" line according to the
	    current configuration (etc) 			      
	 **/
	register int open_paren;

	if (mail_only)
	  printf("To: %s\n\r", format_long(address, 3));
	else {
	  if (names_only)
	    if ((open_paren = chloc(address, '(')) > 0) {
	      if (open_paren < chloc(address, ')')) {
	        output_abbreviated_to(address);
	        return;
	      } 
	     }
	  if (strlen(address) > 45) 
	    PutLine1(LINES-3, COLUMNS-50, "To: (%s)", 
		tail_of_string(address, 40));
	  else {
	    if (strlen(address) > 30) 
	      PutLine1(LINES-3, COLUMNS-50, "To: %s", address);
	    else
	      PutLine1(LINES-3, COLUMNS-50, "          To: %s", address);
	    CleartoEOLN();
	  }
	}
}

output_abbreviated_to(address)
char *address;
{
	/** Output just the fields in parens, separated by commas if need
	    be, and up to COLUMNS-50 characters...This is only used if the
	    user is at level BEGINNER.
	**/

	char newaddress[LONG_STRING];
	register int index, newindex = 0, in_paren = 0;

	index = 0;

	while (newindex < 55 && index < strlen(address)) {
	  if (address[index] == '(') in_paren++;
	  else if (address[index] == ')') { 
	    in_paren--;
	    if (index < strlen(address)-4) {
	      newaddress[newindex++] = ',';
	      newaddress[newindex++] = ' ';
	    }
	  }
	  
	  if (in_paren && address[index] != '(')
	      newaddress[newindex++] = address[index];
	     
	  index++;
	}

	newaddress[newindex] = '\0';

	if (strlen(newaddress) > 50) 
	   PutLine1(LINES-3, COLUMNS-50, "To: (%s)", 
	       tail_of_string(newaddress, 40));
	 else {
	   if (strlen(newaddress) > 30)
	     PutLine1(LINES-3, COLUMNS-50, "To: %s", newaddress);
	   else
	     PutLine1(LINES-3, COLUMNS-50, "          To: %s", newaddress);
	   CleartoEOLN();
	 }

	return;
}
