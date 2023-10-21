/**		reply.c		**/

/*** routine allows replying to the sender of the current message 

     (C) Copyright 1985, Dave Taylor
***/

#include "headers.h"
#include <errno.h>

#ifndef BSD
#  include <sys/types.h>
#  include <sys/utsname.h>
#endif

/** Note that this routine generates automatic header information
    for the subject and (obviously) to lines, but that these can
    be altered while in the editor composing the reply message! 
**/

char *strip_parens(), *get_token();

extern int errno;

char *error_name(), *strcat(), *strcpy();

int
reply()
{
	/** Reply to the current message.  Returns non-zero iff
	    the screen has to be rewritten. **/

	char return_address[LONG_SLEN], subject[SLEN];
	int  return_value, form_letter;

	form_letter = (header_table[current-1].status & FORM_LETTER);

	get_return(return_address);

	if (first_word(header_table[current-1].from, "To:")) {
	  strcpy(subject, header_table[current-1].subject);
	  if (form_letter)
	    return_value = mail_filled_in_form(return_address, subject);
	  else
	    return_value = send(return_address, subject, TRUE, NO);
	}
	else if (header_table[current-1].subject[0] != '\0') {
	  if ((strncmp("Re:", header_table[current-1].subject, 3) == 0) ||
	      (strncmp("RE:", header_table[current-1].subject, 3) == 0) ||
	      (strncmp("re:", header_table[current-1].subject, 3) == 0)) 
	    strcpy(subject, header_table[current-1].subject);
	  else {
	    strcpy(subject,"Re: ");
	    strcat(subject,header_table[current-1].subject); 
	  }
	  if (form_letter)
	    return_value = mail_filled_in_form(return_address, subject);
	  else
	    return_value = send(return_address, subject, TRUE, NO);
	}
	else
	  if (form_letter)
	    return_value = mail_filled_in_form(return_address, 
						"Filled in Form");
	  else
	    return_value = send(return_address, "Re: your mail", TRUE, NO);

	return(return_value);
}

int
reply_to_everyone()
{
	/** Reply to everyone who received the current message.  
	    This includes other people in the 'To:' line and people
	    in the 'Cc:' line too.  Returns non-zero iff the screen 
            has to be rewritten. **/

	char return_address[LONG_SLEN], subject[SLEN];
	char full_address[VERY_LONG_STRING];
	int  return_value;

	get_return(return_address);

	strcpy(full_address, return_address);	/* sender gets copy */
	
	get_and_expand_everyone(return_address, full_address);

	if (header_table[current-1].subject[0] != '\0') {
	  if ((strncmp("Re:", header_table[current-1].subject, 3) == 0) ||
	      (strncmp("RE:", header_table[current-1].subject, 3) == 0) ||
	      (strncmp("re:", header_table[current-1].subject, 3) == 0)) 
	    strcpy(subject, header_table[current-1].subject);
	  else {
	    strcpy(subject,"Re: ");
	    strcat(subject,header_table[current-1].subject); 
	  }
	  return_value = send(full_address, subject, TRUE, NO);
	}
	else
	  return_value = send(full_address, "Re: your mail", TRUE, NO);

	return(return_value);

}

int
forward()
{
	/** Forward the current message.  What this actually does is
	    to set auto_copy to true, then call 'send' to get the 
	    address and route the mail. 
	**/

	char subject[SLEN], address[VERY_LONG_STRING];
	int  original_cc, results, edit_msg = FALSE;

	original_cc = auto_copy;
	address[0] = '\0';

	if (header_table[current-1].status & FORM_LETTER)
	  PutLine0(LINES-3,COLUMNS-40,"<no editing allowed>");
	else {
	  edit_msg = (want_to("Edit outgoing message (y/n) ? ",'y',FALSE)!='n');
	  Write_to_screen("%s", 1, edit_msg? "Yes" : "No");
	}

	auto_cc = TRUE;			/* we want a copy */

	if (strlen(header_table[current-1].subject) > 0) {
	  strcpy(subject,header_table[current-1].subject); 
	  results = send(address, subject, edit_msg,
	    header_table[current-1].status & FORM_LETTER? 
	    PREFORMATTED : allow_forms);
	}
	else
	  results = send(address, "Forwarded Mail...", edit_msg, 
	    header_table[current-1].status & FORM_LETTER? 
	    PREFORMATTED : allow_forms);
	
	auto_copy = original_cc;

	return(results);
}

get_and_expand_everyone(return_address, full_address)
char *return_address, *full_address;
{
	/** Read the current message, extracting addresses from the 'To:'
	    and 'Cc:' lines.   As each address is taken, ensure that it
	    isn't to the author of the message NOR to us.  If neither,
	    prepend with current return address and append to the 
	    'full_address' string.
	**/

    char ret_address[LONG_SLEN], buf[LONG_SLEN], new_address[LONG_SLEN],
	 address[LONG_SLEN], comment[LONG_SLEN];
    int  in_message = 1, first_pass = 0, index;

    /** First off, get to the first line of the message desired **/

    if (fseek(mailfile, header_table[current-1].offset, 0) == -1) {
	dprint3(1,"Error: seek %ld resulted in errno %s (%s)\n", 
		 header_table[current-1].offset, error_name(errno), 
		 "get_and_expand_everyone");
	error2("ELM [seek] couldn't read %d bytes into file (%s)",
	       header_table[current-1].offset, error_name(errno));
	return;
    }
 
    /** okay!  Now we're there!  **/

    /** let's fix the ret_address to reflect the return address of this
	message with '%s' instead of the persons login name... **/

    translate_return(return_address, ret_address);

    /** now let's parse the actual message! **/

    while (in_message) {
      in_message = (int) (fgets(buf, LONG_SLEN, mailfile) != NULL);
      if (first_word(buf, "From ") && first_pass++ != 0) 
	in_message = FALSE;
      else if (first_word(buf, "To:") || first_word(buf, "Cc:") ||
	       first_word(buf, "CC:") || first_word(buf, "cc:")) {
	do {
	  no_ret(buf);

	  /** we have a buffer with a list of addresses, each of either the
	      form "address (name)" or "name <address>".  Our mission, should
	      we decide not to be too lazy, is to break it into the two parts.
	  **/
	      
	  if (!whitespace(buf[0]))
	    index = chloc(buf, ':')+1;		/* skip header field */
	  else
	    index = 0;				/* skip whitespace   */

	  while (break_down_tolist(buf, &index, address, comment)) {

	    if (okay_address(address, return_address)) {
	      sprintf(new_address, ret_address, address);
	      optimize_and_add(new_address, full_address);
	    }
	  }

          in_message = (int) (fgets(buf, LONG_SLEN, mailfile) != NULL);

	  if (in_message) dprint1(1,"> %s", buf);
	
	} while (in_message && whitespace(buf[0]));

      }
      else if (strlen(buf) < 2)	/* done with header */
	 in_message = FALSE;
    }
}

int
okay_address(address, return_address)
char *address, *return_address;
{
	/** This routine checks to ensure that the address we just got
	    from the "To:" or "Cc:" line isn't us AND isn't the person	
	    who sent the message.  Returns true iff neither is the case **/

	char our_address[SLEN];
	struct addr_rec  *alternatives;

	if (in_string(address, return_address))
	  return(FALSE);

	sprintf(our_address, "%s!%s", hostname, username);

	if (in_string(address, our_address))
	  return(FALSE);

	sprintf(our_address, "%s@%s", username, hostname);
	  
	if (in_string(address, our_address))
	  return(FALSE);

	alternatives = alternative_addresses;

	while (alternatives != NULL) {
	  if (in_string(address, alternatives->address))
	    return(FALSE);
	  alternatives = alternatives->next;
	}

	return(TRUE);
}
	    
optimize_and_add(new_address, full_address)
char *new_address, *full_address;
{
	/** This routine will add the new address to the list of addresses
	    in the full address buffer IFF it doesn't already occur.  It
	    will also try to fix dumb hops if possible, specifically hops
	    of the form ...a!b...!a... and hops of the form a@b@b etc 
	**/

	register int len, host_count = 0, i;
	char     hosts[MAX_HOPS][SLEN];	/* array of machine names */
	char     *host, *addrptr;

	if (in_string(full_address, new_address))
	  return(1);	/* duplicate address */

	/** optimize **/
	/*  break down into a list of machine names, checking as we go along */
	
	addrptr = (char *) new_address;

	while ((host = get_token(addrptr, "!", 1)) != NULL) {
	  for (i = 0; i < host_count && ! equal(hosts[i], host); i++)
	      ;

	  if (i == host_count) {
	    strcpy(hosts[host_count++], host);
	    if (host_count == MAX_HOPS) {
	       dprint1(2,
              "Error: hit max_hops limit trying to build return address (%s)\n",
		      "optimize_and_add");
	       error("Can't build return address - hit MAX_HOPS limit!");
	       return(1);
	    }
	  }
	  else 
	    host_count = i + 1;
	  addrptr = NULL;
	}

	/** fix the ARPA addresses, if needed **/
	
	if (chloc(hosts[host_count-1], '@') > -1)
	  fix_arpa_address(hosts[host_count-1]);
	  
	/** rebuild the address.. **/

	new_address[0] = '\0';

	for (i = 0; i < host_count; i++)
	  sprintf(new_address, "%s%s%s", new_address, 
	          new_address[0] == '\0'? "" : "!",
	          hosts[i]);

	if (full_address[0] == '\0')
	  strcpy(full_address, new_address);
	else {
	  len = strlen(full_address);
	  full_address[len  ] = ',';
	  full_address[len+1] = ' ';
	  full_address[len+2] = '\0';
	  strcat(full_address, new_address);
	}

	return(0);
}

get_return_name(address, name, shift_lower)
char *address, *name;
int   shift_lower;
{
	/** Given the address (either a single address or a combined list 
	    of addresses) extract the login name of the first person on
	    the list and return it as 'name'.  Modified to stop at
	    any non-alphanumeric character. **/

	/** An important note to remember is that it isn't vital that this
	    always returns just the login name, but rather that it always
	    returns the SAME name.  If the persons' login happens to be,
	    for example, joe.richards, then it's arguable if the name 
	    should be joe, or the full login.  It's really immaterial, as
	    indicated before, so long as we ALWAYS return the same name! **/

	/** Another note: modified to return the argument as all lowercase
	    always, unless shift_lower is FALSE... **/

	char single_address[LONG_SLEN];
	register int i, loc, index = 0;

	dprint2(6,"get_return_name called with (%s, <>, shift=%s)\n", 
		   address, onoff(shift_lower));

	/* First step - copy address up to a comma, space, or EOLN */

	for (i=0; address[i] != ',' && ! whitespace(address[i]) && 
	     address[i] != '\0'; i++)
	  single_address[i] = address[i];
	single_address[i] = '\0';

	/* Now is it an ARPA address?? */

	if ((loc = chloc(single_address, '@')) != -1) {	  /* Yes */

	  /* At this point the algorithm is to keep shifting our copy 
	     window left until we hit a '!'.  The login name is then
             located between the '!' and the first metacharacter to 
	     it's right (ie '%', ':' or '@'). */

	  for (i=loc; single_address[i] != '!' && i > -1; i--)
	      if (single_address[i] == '%' || 
	          single_address[i] == ':' ||
	          single_address[i] == '.' ||	/* no domains */
		  single_address[i] == '@') loc = i-1;
	
	  if (i < 0 || single_address[i] == '!') i++;

	  for (index = 0; index < loc - i + 1; index++)
	    if (shift_lower)
	      name[index] = tolower(single_address[index+i]);
	    else
	      name[index] = single_address[index+i];
	  name[index] = '\0';
	}
	else {	/* easier - standard USENET address */

	  /* This really is easier - we just cruise left from the end of
	     the string until we hit either a '!' or the beginning of the
             line.  No sweat. */

	  loc = strlen(single_address)-1; 	/* last char */

	  for (i = loc; single_address[i] != '!' && single_address[i] != '.' 
	       && i > -1; i--)
	     if (shift_lower)
	       name[index++] = tolower(single_address[i]);
	     else
	       name[index++] = single_address[i];
	  name[index] = '\0';
	  reverse(name);
	}
}

int
break_down_tolist(buf, index, address, comment)
char *buf, *address, *comment;
int  *index;
{
	/** This routine steps through "buf" and extracts a single address
	    entry.  This entry can be of any of the following forms;

		address (name)
		name <address>
		address
	
	    Once it's extracted a single entry, it will then return it as
	    two tokens, with 'name' (e.g. comment) surrounded by parens.
	    Returns ZERO if done with the string...
	**/

	char buffer[LONG_STRING];
	register int i, loc = 0;

	if (*index > strlen(buf)) return(FALSE);

	while (whitespace(buf[*index])) (*index)++;

	if (*index > strlen(buf)) return(FALSE);

	/** Now we're pointing at the first character of the token! **/

	while (buf[*index] != ',' && buf[*index] != '\0')
	  buffer[loc++] = buf[(*index)++];

	(*index)++;
	buffer[loc] = '\0';

	while (whitespace(buffer[loc])) 	/* remove trailing whitespace */
	  buffer[--loc] = '\0';

	if (strlen(buffer) == 0) return(FALSE);

	dprint1(5, "\n* got \"%s\"\n", buffer);

	if (buffer[loc-1] == ')') {	/*   address (name)  format */
	  for (loc = 0;buffer[loc] != '(' && loc < strlen(buffer); loc++)
		/* get to the opening comment character... */ ;

	  loc--;	/* back up to just before the paren */
	  while (whitespace(buffer[loc])) loc--;	/* back up */

	  /** get the address field... **/

	  for (i=0; i <= loc; i++)
	    address[i] = buffer[i];
	  address[i] = '\0';

	  /** now get the comment field, en toto! **/

	  loc = 0;

	  for (i = chloc(buffer, '('); i < strlen(buffer); i++)
	    comment[loc++] = buffer[i];
	  comment[loc] = '\0';
	}
	else if (buffer[loc-1] == '>') {	/*   name <address>  format */
	  dprint0(7, "\tcomment <address>\n");
	  for (loc = 0;buffer[loc] != '<' && loc < strlen(buffer); loc++)
		/* get to the opening comment character... */ ;
	  while (whitespace(buffer[loc])) loc--;	/* back up */

	  /** get the comment field... **/

	  comment[0] = '(';
	  for (i=1; i < loc; i++)
	    comment[i] = buffer[i-1];
	  comment[i++] = ')';
	  comment[i] = '\0';

	  /** now get the address field, en toto! **/

	  loc = 0;

	  for (i = chloc(buffer,'<') + 1; i < strlen(buffer) - 1; i++)
	    address[loc++] = buffer[i];
	
	  address[loc] = '\0';
	}
	else {
	  strcpy(address, buffer);
	  comment[0] = '\0';
	}

	dprint2(5,"-- returning '%s' '%s'\n", address, comment);

	return(TRUE);
}
