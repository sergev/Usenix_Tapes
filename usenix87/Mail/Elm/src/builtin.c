/**			builtin.c		**/

/** This is the built-in pager for displaying messages while in the Elm
    program.  It's a bare-bones pager with precious few options. The idea
    is that those systems that are sufficiently slow that using an external
    pager such as 'more' is too slow, then they can use this!

    Added the following functionality;

	<number>s	skip <number> lines
	<number>f	forward a page or <number> pages
	/pattern	skip forward to pattern		

    (C) Copyright 1986, Dave Taylor
**/

#include "headers.h"
#include <ctype.h>

#define  BEEP		007		/* ASCII Bell character */

#ifdef BSD
#  undef tolower
#endif

int	lines_put_on_screen = 0,    /* number of lines displayed per screen */
	lines_displayed = 0,	    /* Total number of lines displayed      */
	total_lines_to_display,	    /* total number of lines in message     */
	lines_to_ignore = 0;	    /* for 'f' and 's' functions...         */

start_builtin(lines_in_message)
int lines_in_message;
{
	/** clears that screen and resets it's internal counters... **/

	dprint1(8,"displaying %d lines from message using internal pager\n",
		lines_in_message);

	lines_displayed = 0;
	lines_put_on_screen = 0;
	lines_to_ignore = 0;

	total_lines_to_display = lines_in_message;
}

int
display_line(line)
char *line;
{
	/** Display the given line on the screen, taking into account such
	    dumbness as wraparound and such.  If displaying this would put
	    us at the end of the screen, put out the "MORE" prompt and wait
	    for some input.   Return non-zero if the user terminates the
	    paging (e.g. 'q') or zero if we should continue...
	**/
	
	register int lines_needed, okay_char, ch, len = 12, iteration = 0;
	char     pattern[SLEN];

	if (lines_to_ignore != 0) {
	   if (--lines_to_ignore <= 0) {
	     putchar('\n');
	     lines_to_ignore = 0;
	   }
	   return(0);
	}

	lines_needed = (int) (printable_chars(line)/COLUMNS); /* wraparound */

	for (ch = 0; ch < strlen(line); ch++)
	  if (line[ch] == '\n') lines_needed++;

	if (lines_needed + lines_put_on_screen > LINES-1) {
	  StartBold();
	  if (user_level == 0) {
	    Write_to_screen(
               "You've read %d%%: press <space> for more, or 'q' to return", 1,
		(int) (100.0 * (
		   (float) lines_displayed / (float) total_lines_to_display)));
	    len = 59;
	  }
	  else if (user_level == 1) {
	    Write_to_screen( 
		"More (%d%%) Press <space> for more, 'q' to return", 1, 
		(int) (100.0 * (
		   (float) lines_displayed / (float) total_lines_to_display)));
	    len = 49;
	  }
	  else 
	    Write_to_screen(" More (%d%%)", 1, 
		(int) (100.0 * (
		   (float) lines_displayed / (float) total_lines_to_display)));

	  EndBold();
	  okay_char = FALSE;
	  do {
	     Raw(ON);
	     ch =  tolower(ReadCh());
	     Raw(OFF);
loop_top:     switch (ch) {
	       case '\n' : 
	       case '\r' : /* <return> pressed... */
			   lines_put_on_screen -= lines_needed;
			   okay_char = TRUE;
			   break;
	       case ' '  : /* <space> pressed... */
			   lines_put_on_screen = 0;
			   okay_char = TRUE;
			   break;
	       case '/'  : putchar('/');fflush(stdout);
			   Raw(ON);
			   optionally_enter(pattern,-1,-1,FALSE);
			   Raw(OFF);
			   CursorLeft(len+strlen(pattern)+1); CleartoEOLN();
			   printf("...searching for pattern \"%s\"...",
				  pattern);
			   fflush(stdout);
			   break;
	       case 'f'  : lines_to_ignore = ((iteration?iteration:1)*LINES)-5;	
			   CursorLeft(len); CleartoEOLN();
			   printf("...skipping %d lines...",lines_to_ignore+2); 
			   fflush(stdout);
			   lines_put_on_screen = 0;
			   return(0);
	       case 's'  : lines_to_ignore = (iteration?iteration-1:0);
			   CursorLeft(len); CleartoEOLN();
			   if (lines_to_ignore)
			    printf("...skipping %d lines...",lines_to_ignore+1);
			   else
			    printf("...skipping one line...\n");
			   fflush(stdout);
			   lines_put_on_screen = 0;
			   return(0);
	       case 'q'  :
	       case 'Q'  : return(TRUE);	/* get OUTTA here! */
	       default   : if (isdigit(ch)) {
		             Raw(ON);
			     do {
			       iteration = 10*iteration + (ch - '0');
			     } while (isdigit(ch = ReadCh()));
			     Raw(OFF);
	                     goto loop_top;	
			   }
			   putchar(BEEP);	
			   fflush(stdout);
			   break;
	     }
	  } while (! okay_char);

	  CursorLeft(len);		/* back up to the beginning of line */
	  CleartoEOLN();
	}

	Write_to_screen("%s", 1, line);

	lines_displayed     += 1;		   /* tossed on screen */
	lines_put_on_screen += lines_needed;	   /* read from file   */

	return (FALSE);
}
