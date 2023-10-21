/**			showmsg_cmd.c			**/

/** This is an interface for the showmsg command line.  The possible
    functions that could be invoked from the showmsg command line are
    almost as numerous as those from the main command line and include
    the following;

	   |    = pipe this message to command...
	   !    = call Unix command
	   <    = scan message for calendar info
	   b    = bounce (remail) message
	   d    = mark message for deletion
	   e    = edit entire mailbox
	   f    = forward message
	   g    = group reply
	   j,n  = move to body of next message
	   k    = move to body of previous message
	   p    = print this (all tagged) message
	   r    = reply to this message
	   s    = save this message to a maibox/folder 
	   t    = tag this message
	   x    = Exit Elm NOW

    all commands not explicitly listed here are returned as unprocessed
    to be dealt with at the main command level.

    This function returns 0 if it dealt with the command, or the command
    otherwise.
**/

#include "headers.h"

process_showmsg_command(command)
char command;
{
	switch (command) {
	  case '|' : clear_bottom_of_screen();
		     PutLine0(LINES-3,0,"Command: pipe");
		     softkeys_off();
		     (void) pipe();	/* do pipe regardless */
		     softkeys_on();
		     return(0);		/* must have new screen */

	  case '!' : clear_bottom_of_screen();
		     PutLine0(LINES-3,0,"Command: system call");
		     softkeys_off();
		     (void) subshell();	/* do shell regardless */
		     softkeys_on();
		     return(0);		/* must have new screen */

	  case '<' : 
#ifdef ENABLE_CALENDAR
		     scan_calendar();
#else
		     error("can't scan for calendar entries!");
#endif
		    break;

	  case 'b' : clear_bottom_of_screen();
		     PutLine0(LINES-3,0,"Command: bounce message");
		     remail();
		     return(0);		/* must have new screen */

	  case 'd' : delete(TRUE);
		     break;

	  case 'e' : edit_mailbox();
		     return(0);		/* must have new screen */

	  case 'f' : clear_bottom_of_screen();
		     PutLine0(LINES-3,0,"Command: forward message");
		     (void) forward();
		     return(0);		/* must have new screen */

	  case 'g' : clear_bottom_of_screen();
		     PutLine0(LINES-3,0,"Command: group reply");
		     (void) reply_to_everyone();
		     return(0);		/* must have new screen */

	  case 'j' :
	  case 'n' : if (current < message_count)
		       show_msg(++current);
		     return(0);	

	  case 'k' : if (current > 0)
		       show_msg(--current);
		     return(0);


	  case 'p' : printmsg();	
		     break;

	  case 'r' : clear_bottom_of_screen();
		     PutLine0(LINES-3,0,"Command: reply to message");
		     (void) reply();
		     return(0);		/* must have new screen */

	  case 's' : clear_bottom_of_screen();
		     PutLine0(LINES-3,0,"Command: save message");
		     (void) save();
		     break;

	  case 't' : tag_message();	
		     break;

	  case 'x' : leave();

	  case '\n':
	  case '\r': return(0);		/* avoid <return> looping */

	  default  : return(command);	/* couldn't deal with it! */
	}

	return(1);	/* done with it! */
}

clear_bottom_of_screen()
{
	/** clear the last 4 lines of the screen... **/

	MoveCursor(LINES-4, 0);
	CleartoEOS();
}
