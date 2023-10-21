/**			pattern.c			**/

/**    General pattern matching for the ELM mailer.     

       (C) Copyright 1986 Dave Taylor
**/

#include <errno.h>

#include "headers.h"

static char pattern[SLEN] = { "" };
static char alt_pattern[SLEN] = { "" };

extern int errno;

char *error_name(), *shift_lower(), *strcpy();

meta_match(function)
int function;
{
	/** Perform specific function based on whether an entered string 
	    matches either the From or Subject lines.. 
	**/

	register int i, tagged=0, count=0;
	static char     meta_pattern[SLEN];

	PutLine1(LINES-3, strlen("Command: "), 
	     "%s messages that match pattern...", 
	     function==TAGGED?"Tag": function==DELETED?"Delete":"Undelete");

	if (function == TAGGED) {	/* are messages already tagged??? */
	  for (i=0; i < message_count; i++)
	    if (ison(header_table[i].status,TAGGED))
	      tagged++;

	  if (tagged) {
	    if (tagged > 2) 
	      PutLine0(LINES-2,0, "Some messages are already tagged");
	    else
	      PutLine0(LINES-2,0, "A message is already tagged");
	
	    Write_to_screen("- Remove tag%s? y%c", 2, plural(tagged),BACKSPACE);

	    if (tolower(ReadCh()) != 'n') {	/* remove tags... */
	      for (i=0; i < message_count; i++) {
	        clearit(header_table[i].status,TAGGED);
		show_new_status(i);
	      }
	    }
	  }
	}
	
	PutLine0(LINES-2,0, "Enter pattern: "); CleartoEOLN();

	optionally_enter(meta_pattern, LINES-2,strlen("Enter pattern: "),FALSE);

	if (strlen(meta_pattern) == 0) {
	  ClearLine(LINES-2);
	  return(0);
	}

	strcpy(meta_pattern, shift_lower(meta_pattern));   /* lowercase it */

	for (i = 0; i < message_count; i++) {
	  if (from_matches(i, meta_pattern)) {
	    if ((selected && header_table[i].status & VISIBLE) || ! selected) {
	      if (function == UNDELETE)
	        clearit(header_table[i].status, DELETED);
	      else
	        setit(header_table[i].status, function);
	      show_new_status(i);
	      count++;
	    }
	  }
	  else if (subject_matches(i, meta_pattern)) {
	    if ((selected && header_table[i].status & VISIBLE) || ! selected) {
	      if (function == UNDELETE)
	        clearit(header_table[i].status, DELETED);
	      else
	        setit(header_table[i].status, function);
	      show_new_status(i);
	      count++;
	    }
	  }
	}

	ClearLine(LINES-2);	/* remove "pattern: " prompt */
	
	if (count > 0)
	  error3("%s %d messsage%s", 
	         function==TAGGED? "tagged" : 
		   function==DELETED? "marked for deletion" : "undeleted",
		 count, plural(count));
	else
	  error1("no matches - no messages %s",
		 function==TAGGED? "tagged" : 
		   function==DELETED? "marked for deletion": "undeleted");

	return(0);
}
	  
int
pattern_match()
{
	/** Get a pattern from the user and try to match it with the
	    from/subject lines being displayed.  If matched (ignoring
	    case), move current message pointer to that message, if
	    not, error and return ZERO **/

	register int i;

	PutLine0(LINES-3,40,"/ = match anywhere in messages");
	
	PutLine0(LINES-1,0, "Match Pattern:");

	if (pattern_enter(pattern, alt_pattern, LINES-1, 16, 
	    "Match Pattern (in entire mailbox):"))
	  if (strlen(alt_pattern) > 0) {
	    strcpy(alt_pattern, shift_lower(alt_pattern));
	    return(match_in_message(alt_pattern));
	  }
	  else
	    return(1);
	  
	if (strlen(pattern) == 0) 
	  return(0);
	else
	  strcpy(pattern, shift_lower(pattern));

	for (i = current; i < message_count; i++) {
	  if (from_matches(i, pattern)) {
	    if (!selected || (selected && header_table[i].status & VISIBLE)) {
	      current = ++i;
	      return(1);
	    }
	  }
	  else if (subject_matches(i, pattern)) {
	    if (!selected || (selected && header_table[i].status & VISIBLE)) {
	      current = ++i;
	      return(1);
	    }
	  }
	}

	return(0);
}

int
from_matches(message_number, pat)
int message_number;
char *pat;
{
	/** Returns true iff the pattern occurs in it's entirety
	    in the from line of the indicated message **/

	return( in_string(shift_lower(header_table[message_number].from), 
		pat) );
}

int
subject_matches(message_number, pat)
int message_number;
char *pat;
{
	/** Returns true iff the pattern occurs in it's entirety
	    in the subject line of the indicated message **/

	return( in_string(shift_lower(header_table[message_number].subject), 
		pat) );
}

match_in_message(pat)
char *pat;
{
	/** Match a string INSIDE a message...starting at the current 
	    message read each line and try to find the pattern.  As
	    soon as we do, set current and leave! 
	    Returns 1 if found, 0 if not
	**/

	char buffer[LONG_STRING];
	int  message_number, lines, line;

	message_number = current-1;

	error("searching mailbox for pattern...");

	while (message_number < message_count) {

	  if (fseek(mailfile, header_table[message_number].offset, 0L) != 0) {

	    dprint3(1,"Error: seek %ld bytes into file failed. errno %d (%s)\n",
		      header_table[message_number].offset, errno, 
		     "match_in_message");
	    error2("ELM [match] failed looking %ld bytes into file (%s)",
		   header_table[message_number].offset, error_name(errno));
	    return(1);	/* fake it out to avoid replacing error message */
	  }

	  line = 0;
	  lines = header_table[message_number].lines;

	  while (fgets(buffer, LONG_STRING, mailfile) != NULL && line < lines) {
	
	    line++;

	    if (in_string(shift_lower(buffer), pat)) {
	      current = message_number+1; 
	      clear_error();
	      return(1);
	    }
	  }

	  /** now we've passed the end of THIS message...increment and 
	      continue the search with the next message! **/

	  message_number++;
	}

	return(0);
}
