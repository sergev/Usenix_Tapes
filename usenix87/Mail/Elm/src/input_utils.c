/**			input_utils.c			**/

/** Mindless I/O routines for ELM 
	
    (C) Copyright 1985 Dave Taylor
**/

#include "headers.h"
#include <errno.h>

extern int errno;		/* system error number */

unsigned alarm();

#define special_char(c)		(c == ' ' || c == '\t' || c == '/' || c == ',' \
				 || c == '\0')

#define erase_a_char()		{ Writechar(BACKSPACE); Writechar(' '); \
			          Writechar(BACKSPACE); fflush(stdout); }

int
want_to(question, dflt, echo_answer)
char *question, dflt;
int  echo_answer;
{
	/** Ask 'question' at LINES-2, COLUMNS-40, returning the answer in 
	    lower case.  If 'echo_answer', then echo answer.  'dflt' is the 
	    default answer if <return> is pressed. (Note: 'dflt' is also what 
	    will be returned if <return> is pressed!)
	**/
	register char ch, cols;

	cols = (strlen(question) < 30)? COLUMNS-40 : COLUMNS-50;

	PutLine3(LINES-3, cols,"%s%c%c", question, dflt, BACKSPACE);
	fflush(stdout);
	fflush(stdin);
	ch = tolower(ReadCh());

	if (echo_answer && ch > (char) ' ') {
	  Writechar(ch);
	  fflush(stdout);
	}

	return(ch == '\n' || ch == '\r' ? dflt : ch);
}

int
read_number(ch)
char ch;
{
	/** Read a number, where 'ch' is the leading digit! **/
	
	char buff[SHORT_SLEN];
	int  num;

	buff[0] = ch;
	buff[1] = '\0';

	PutLine0(LINES-3, COLUMNS-40,"Set current message to :");
	if (optionally_enter(buff, LINES-3, COLUMNS-15, TRUE) == -1)
	  return(current);

	sscanf(buff,"%d", &num);
	return(num);
}

int
optionally_enter(string, x, y, append_current)
char *string;
int  x,y, append_current;
{
	/** Display the string on the screen and if RETURN is pressed, return 
	    it.  Otherwise, allow standard text input, including backspaces 
	    and such until RETURN is hit.  
	    If "append_current" is set, then leave the default string in 
	    place and edit AFTER it...assume 'x,y' is placing us at the
	    beginning of the string...
	    This routine returns zero unless INTERRUPT hit, then it returns
	    -1 and must be treated accordingly.
	    Added ^W and ^R support...
	    Also added that if x and y are < 0 don't try any cursor stuff
	**/

	char ch;
	register int index = 0, use_cursor_control;

	use_cursor_control = ((! mail_only) && x >= 0 && y >= 0);
	
	if (use_cursor_control)
	  PutLine1(x,y, "%s", string);	
	else
	  printf("%s", string);	

	CleartoEOLN();

	if (! append_current) 
	  if (use_cursor_control)
	    MoveCursor(x,y);
	  else
	    non_destructive_back_up(strlen(string));

	if (cursor_control)
	  transmit_functions(OFF);

	ch = getchar();

	if (ch == '\n' || ch == '\r') {
	  if (cursor_control)
	    transmit_functions(ON);
	  return(0);	/* we're done.  No change needed */
	}
	
	CleartoEOLN();

	index = (append_current? strlen(string) : 0);

	if (ch == kill_line) {
	  if (use_cursor_control)
	    MoveCursor(x,y);
	  else
	    back_up(index);
          CleartoEOLN();
	  index = 0;
	}
	else if (ch != backspace) {
	  Writechar(ch);
	  string[index++] = ch;
	}
	else if (index > 0) {
	  index--;
	  erase_a_char();
	}
	else {
	  Writechar(' ');
	  Writechar(BACKSPACE);
	  fflush(stdout);
	}

	do {
	  ch = getchar();

	  /* the following is converted from a case statement to
	     allow the variable characters (backspace, kill_line
	     and break) to be processed.  Case statements in
	     C require constants as labels, so it failed ...
	  */

	    if (ch == backspace) {
              if (index > 0) {
	        erase_a_char();
		index--;
	      }
	      else {
		Writechar(' ');
		Writechar(BACKSPACE);
	        fflush(stdout);
	      }
	    }
	    else if (ch == '\n' || ch == '\r') {
	      string[index] = '\0';
	      if (cursor_control)
	        transmit_functions(ON);
	      return(0);
	    }
	    else if (ch == ctrl('W')) {		/* back up a word! */
	      if (special_char(string[index]) && index > 0) {
		index--;
	        erase_a_char();
	      }
	      while (index > 0 && ! special_char(string[index])) {
		index--;
	        erase_a_char();
	      }
	    }
	    else if (ch == ctrl('R')) {
	      string[index] = '\0';
	      if (use_cursor_control) {
	        PutLine1(x,y, "%s", string);	
	        CleartoEOLN();
	      }
	      else
	        printf("\n%s", string);	
	    }
	    else if (ch == kill_line) {
	      if (use_cursor_control)
	        MoveCursor(x,y);
	      else
	        back_up(index+1);
              CleartoEOLN();
	      index = 0;
	    }
	    else if (ch == NULL) {
	      if (cursor_control)
	        transmit_functions(ON);
	      fflush(stdin); 	/* remove extraneous chars, if any */
	      string[0] = '\0'; /* clean up string, and... */
	      return(-1);
	    }
	    else {  /* default case */
		        
	      string[index++] = ch;
	      Writechar(ch);
	   }
	} while (index < SLEN);

	string[index] = '\0';

	if (cursor_control)
	  transmit_functions(ON);
	return(0);
}

int
pattern_enter(string, alt_string, x, y, alternate_prompt)
char *string, *alt_string, *alternate_prompt;
int  x,y;
{
	/** This function is functionally similar to the routine
	    optionally-enter, but if the first character pressed
	    is a '/' character, then the alternate prompt and string
	    are used rather than the normal one.  This routine 
	    returns 1 if alternate was used, 0 if not
	**/

	char ch;
	register index = 0;

	PutLine1(x, y, "%s", string);	
	CleartoEOLN();
	MoveCursor(x,y);

	if (cursor_control)
	  transmit_functions(OFF);

	ch = getchar();

	if (ch == '\n' || ch == '\r') {
	  if (cursor_control)
	    transmit_functions(ON);
	  return(0);	/* we're done.  No change needed */
	}
	
	if (ch == '/') {
	  PutLine1(x, 0, "%s", alternate_prompt);
	  CleartoEOLN();
	  (void) optionally_enter(alt_string, x, strlen(alternate_prompt)+1,
		 FALSE);
	  return(1);
	}

	CleartoEOLN();

	index = 0;

	if (ch == kill_line) {
	  MoveCursor(x,y);
          CleartoEOLN();
	  index = 0;
	}
	else if (ch != backspace) {
	  Writechar(ch);
	  string[index++] = ch;
	}
	else if (index > 0) {
	  index--;
	  erase_a_char();
	}
	else {
	  Writechar(' ');
	  Writechar(BACKSPACE);
	}

	do {
	  fflush(stdout);
	  ch = getchar();

	  /* the following is converted from a case statement to
	     allow the variable characters (backspace, kill_line
	     and break) to be processed.  Case statements in
	     C require constants as labels, so it failed ...
	  */

	    if (ch == backspace) {
              if (index > 0) {
		index--;
		erase_a_char();
	      }
	      else {
		Writechar(' ');
		Writechar(BACKSPACE);
	      }
	    }
	    else if (ch == '\n' || ch == '\r') {
	      string[index] = '\0';
	      if (cursor_control)
	        transmit_functions(ON);
	      return(0);
	    }
	    else if (ch == ctrl('W')) {
	      /* get to rightmost non-alpha */
	      if (special_char (string[index]) && index > 0)
		index--;
	      while (index > 0 && ! special_char(string[index])) {
		erase_a_char();
		index--;
	      }
	    }
	    else if (ch == ctrl('R')) {
	      string[index] = '\0';
	      if (!mail_only) {
	        PutLine1(x,y, "%s", string);	
	        CleartoEOLN();
	      }
	      else
	        printf("\n%s", string);	
	    }
	    else if (ch == kill_line) {
	      MoveCursor(x,y);
              CleartoEOLN();
	      index = 0;
	    }
	    else if (ch == NULL) {
	      if (cursor_control)
	        transmit_functions(ON);
	      fflush(stdin); 	/* remove extraneous chars, if any */
	      string[0] = '\0'; /* clean up string, and... */
	      return(-1);
	    }
	    else {  /* default case */
		        
	      string[index++] = ch;
	      Writechar(ch);
	   }
	} while (index < SLEN);

	string[index] = '\0';

	if (cursor_control)
	  transmit_functions(ON);
	return(0);
}

back_up(spaces)
int spaces;
{
	/** this routine is to replace the goto x,y call for when sending
	    mail without starting the entire "elm" system up... **/
	
	while (spaces--) {
	  erase_a_char();
	}
}

non_destructive_back_up(spaces)
int spaces;
{
	/** same as back_up() but doesn't ERASE the characters on the screen **/

	while (spaces--)
	  Writechar(BACKSPACE); 
        fflush(stdout); 
}

int
GetPrompt()
{
	/** This routine does a read/timeout for a single character.
	    The way that this is determined is that the routine to
	    read a character is called, then the "errno" is checked
	    against EINTR (interrupted call).  If they match, this
	    returns NO_OP_COMMAND otherwise it returns the normal
	    command.	
	**/

	int ch;

	if (timeout > 0) {
	  alarm((unsigned) timeout);
	  errno = 0;	/* we actually have to do this.  *sigh*  */
	  ch = ReadCh();
	  if (errno == EINTR) ch = NO_OP_COMMAND;
	  alarm((unsigned) 0);
	}
	else
	  ch = ReadCh();

	return(ch);
}
