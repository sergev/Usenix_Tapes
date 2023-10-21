/**			help.c			**/

/*** help routine for ELM program 

     (C) Copyright 1985, Dave Taylor

***/

#include <ctype.h>

#ifdef BSD
# undef tolower
#endif

#include "headers.h"

help()
{
	/** Process the request for help [section] from the user **/

	char ch;		/* character buffer for input */
	char *s;		/* string pointer...	      */

	MoveCursor(LINES-7,0);
	CleartoEOS();

	Centerline(LINES-7, "ELM Help System");
	Centerline(LINES-5,
           "Press keys you want help for, '?' for a list, or '.' to end");

	PutLine0(LINES-3, 0, "Help on key: ");

	do {
	  MoveCursor(LINES-3, strlen("Help on key: "));
	  ch = tolower(ReadCh());
	  
	  if (ch == '.') return(0);	/* zero means footer rewrite only */

	  s = "Unknown command.  Use '?' for a list of commands...";

	  switch (ch) {

	    case '?': display_helpfile(MAIN_HELP);	return(1);

	    case '$': s =
  "$ = Force a resync of the current mailbox.  This will 'purge' deleted mail";
		break;

	    case '!': s = 
   "! = Escape to the Unix shell of your choice, or just to enter commands";
	       break;
	    case '@': s = 
       "@ = Debug - display a summary of the messages on the header page";
	       break;
	    case '|': s = 
 "| = Pipe the current message or tagged messages to the command specified";
	       break;
	    case '#': s = 
      "# = Debug - display all information known about current message";
	      break;
	    case '%': s = 
   "% = Debug - display the computed return address of the current message";
	       break;
	    case '*': s = "* = Go to the last message in the current mailbox";
	       break;
	    case '-': s = 
	       "- = Go to the previous page of messages in the current mailbox";
	       break;
	    case '=': s = 
                  "'=' = Go to the first message in the current mailbox";
	       break;
	    case ' ': 
	    case '+': s = 
		"+ = Go to the next page of messages in the current mailbox";
	       break;
	    case '/': s = "/ = Search for specified pattern in mailbox";
	       break;
	    case '<': s = 
	       "< = Scan current message for calendar entries (if enabled)";
	       break;
	    case '>': s = 
	       "> = Save current message or tagged messages to specified file";
	       break;
	    case '^': s = 
	       "^ = Toggle the Delete/Undelete status of the current message";
	       break;
	    case 'a': s = 
       "a = Enter the alias sub-menu section.  Create and display aliases";
	       break;
	    case 'b': s = 
    "b = Bounce (remail) a message to someone as if you have never seen it";
	       break;
	    case 'c': s = 
       "c = Change mailboxes, leaving the current mailbox as if 'quitting'";
	       break;
	    case 'd': s = "d = Mark the current message for future deletion";
	       break;
	    case ctrl('D') :
	s = "^D = Mark for deletion all messages with the specified pattern";
		break;
	    case 'e': s = 
       "e = Invoke the editor on the entire mailbox, resync'ing when done";
	       break;
	    case 'f': s = 
      "f = Forward the current message to someone, return address is yours";
	       break;
	    case 'g': s = 
 "g = Group reply not only to the sender, but to everyone who received msg";
	       break;
	    case 'h': s = 
	       "h = Display message with all Headers (ignore weedout list)";
	       break;
	    case 'j': s = 
       "j = Go to the next message.  This is the same as the DOWN arrow";
	       break;
	    case 'k': s = 
       "k = Go to the previous message.  This is the same as the UP arrow";
	       break;
	    case 'm': s = 
               "m = Create and send mail to the specified person or persons";
	       break;
	    case 'n': s = 
               "n = Read the current message, then move current to next messge";
	       break;
	    case 'o': s = "o = Go to the options submenu";
	       break;
	    case 'p': s = 
		"p = Print the current message or the tagged messages";
	       break;
	    case 'q': s = 
		"q = Quit the mailer, asking about deletion, saving, etc";
	       break;
	    case 'r': s = 
  "r = Reply to the message.  This only sends to the originator of the message";
	       break;
	    case 's': s = 
               "s = Save current message or tagged messages to specified file";
	       break;
	    case 't': s = 
               "t = Tag a message for further operations (or untag if tagged)";
	       break;
	    case ctrl('T') :
		s = "^T = tag all messages with the specified pattern";
		break;
	    case 'u': 
		s = "u = Undelete - remove the deletion mark on the message";
	       break;
	    case 'x': s = "x = Exit the mail system quickly";
	       break;
    	
	    case '\n':
	    case '\r': s = "<return> = Read the current message";
	       break;
    
	    case ctrl('L'): s = "^L = Rewrite the screen";	
	       break;
            case ctrl('?'):					    /* DEL */
	    case ctrl('Q'): s = "Exit the mail system quickly";
	       break;
	    default : if (isdigit(ch)) 
	            s = "<number> = Make specified number the current message";
	  }

	  ClearLine(LINES-1);
	  Centerline(LINES-1, s);

	} while (ch != '.');
	
	/** we'll never actually get here, but that's okay... **/

	return(0);
}

display_helpfile(section)
int section;
{
	/*** Help me!  Read file 'helpfile.<section>' and echo to screen ***/

	FILE *hfile;
	char buffer[SLEN];
	int  lines=0;

	sprintf(buffer, "%s/%s.%d", helphome, helpfile, section);
	if ((hfile = fopen(buffer,"r")) == NULL) {
	  dprint1(1,"Error: Couldn't open helpfile %s (help)\n", buffer);
	  error1("couldn't open helpfile %s",buffer);
	  return(FALSE);
	}
	
	ClearScreen();

	while (fgets(buffer, SLEN, hfile) != NULL) {
	  if (lines > LINES-3) {
	    PutLine0(LINES,0,"Press any key to continue: ");
	    (void) ReadCh();
	    lines = 0;
	    ClearScreen();
	    Write_to_screen("%s\r", 1, buffer);
	  }
	  else 
	    Write_to_screen("%s\r", 1, buffer);

	  lines++;
	}

        PutLine0(LINES,0,"Press any key to return: ");

	(void) ReadCh();
	clear_error();

	return(TRUE);
}
