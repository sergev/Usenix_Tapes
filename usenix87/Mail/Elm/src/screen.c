/**		screen.c		**/

/**  screen display routines for ELM program 

     (C) Copyright 1985, Dave Taylor
**/

#include "headers.h"

#define  minimum(a,b)	((a) < (b) ? (a) : (b))

static   int  last_current	 = -1;

char *strcpy(), *strncpy(), *nameof();

showscreen()
{
	char buffer[SLEN];

	ClearScreen();

	if (selected)
	  sprintf(buffer, 
		"Mailbox is '%s' with %d shown out of %d [Elm %s]",
	      nameof(infile), selected, message_count, VERSION);
	else
	  sprintf(buffer, "Mailbox is '%s' with %d message%s [Elm %s]",
	      nameof(infile), message_count, plural(message_count), VERSION);
	Centerline(1, buffer);

	last_header_page = -1;	 	/* force a redraw regardless */
	show_headers();

	if (mini_menu)
	  show_menu();

	show_last_error();
	
	if (hp_terminal) 
	  define_softkeys(MAIN);
}

update_title()
{
	/** display a new title line, probably due to new mail arriving **/

	char buffer[SLEN];

	if (selected)
	  sprintf(buffer, 
		"Mailbox is '%s' with %d shown out of %d [Elm %s]",
	      nameof(infile), selected, message_count, VERSION);
	else
	  sprintf(buffer, "Mailbox is '%s' with %d message%s [Elm %s]",
	      nameof(infile), message_count, plural(message_count), VERSION);

	ClearLine(1);

	Centerline(1, buffer);
}

show_menu()
{
	/** write main system menu... **/

	if (user_level == 0) {	/* a rank beginner.  Give less options  */
	  Centerline(LINES-7,
  "You can use any of the following commands by pressing the first character;");
          Centerline(LINES-6,
"D)elete mail,  M)ail a message,  R)eply to mail,  U)ndelete, or Q)uit");
	  Centerline(LINES-5,
  "To read a message, press <return>.  j = move arrow down, k = move arrow up");
	} else {
	Centerline(LINES-7,
  "|=pipe, !=shell, ?=help, <n>=set current to n, /=search pattern");
        Centerline(LINES-6,
"A)lias, C)hange mailbox, D)elete, E)dit, F)orward, G)roup reply, M)ail,"); 
	Centerline(LINES-5,
  "N)ext, O)ptions, P)rint, R)eply, S)ave, T)ag, Q)uit, U)ndelete, or eX)it");
	}
}

int
show_headers()
{
	/** Display page of headers (10) if present.  First check to 
	    ensure that header_page is in bounds, fixing silently if not.
	    If out of bounds, return zero, else return non-zero 
	    Modified to only show headers that are "visible" to ze human
	    person using ze program, eh?
	**/

	register int this_msg = 0, line = 4, last = 0, last_line, 
		     displayed = 0;
	char newfrom[SLEN], buffer[SLEN];
	
	if (fix_header_page()) {
	  dprint0(7, "show_headers returned FALSE 'cause of fix-header-page\n");
	  return(FALSE);
	}

	if (selected) {
	  if ((header_page*headers_per_page) > selected) {
	    dprint2(7, "show_headers returned FALSE since selected [%d] < %d\n",
		    selected, header_page*headers_per_page);
	    return(FALSE); 	/* too far! too far! */
	  }

	  dprint0(6,"** show_headers AND selected...\n");

	  if (header_page == 0) {
	    this_msg = visible_to_index(1);	
	    displayed = 0;
	  }
	  else {
	    this_msg = visible_to_index(header_page * headers_per_page + 1);
	    displayed = header_page * headers_per_page;
	  }

	  dprint2(7,"this_msg (index) = %d  [header_page = %d]\n", this_msg, 
		  header_page);

	  dprint1(7,"we've already displayed %d messages\n", displayed);

	  last = displayed+headers_per_page;

	  dprint1(7,"and the last msg on this page is %d\n", last);
	}
	else {
	  if (header_page == last_header_page) 	/* nothing to do! */
	    return(FALSE);

	  /** compute last header to display **/
  
	  this_msg = header_page * headers_per_page;
	  last = this_msg + (headers_per_page - 1);
	}

	if (last >= message_count) last = message_count-1;

	/** Okay, now let's show the header page! **/

	ClearLine(line);	/* Clear the top line... */

	MoveCursor(line, 0);	/* and move back to the top of the page... */

	while ((selected && displayed < last) || this_msg <= last) {
	  tail_of(header_table[this_msg].from, newfrom, TRUE); 

	  if (selected) {
	    if (this_msg == current-1) 
	      build_header_line(buffer, &header_table[this_msg], ++displayed,
			      TRUE, newfrom);
	    else
	      build_header_line(buffer, &header_table[this_msg], 
			      ++displayed, FALSE, newfrom);
	  } 
	  else {
	    if (this_msg == current-1) 
	      build_header_line(buffer, &header_table[this_msg], this_msg+1, 
		              TRUE, newfrom);
	    else
	      build_header_line(buffer, &header_table[this_msg], 
			      this_msg+1, FALSE, newfrom);
	  }

	  Write_to_screen("%s\r\n", 1, buffer);	/* avoid '%' probs */
	  CleartoEOLN();
	  line++;		/* for clearing up in a sec... */

	  if (selected) {
	    if ((this_msg = next_visible(this_msg+1)-1) < 0)
	      break;	/* GET OUTTA HERE! */

	    /* the preceeding looks gross because we're using an INDEX
	       variable to pretend to be a "current" counter, and the 
	       current counter is always 1 greater than the actual 
	       index.  Does that make sense??
	     */
	  }
	  else
	    this_msg++;					/* even dumber...  */
	}

	dprint0(1,"** out of redraw loop! **\n");

	if (mini_menu)
	  last_line = LINES-8;
	else
	  last_line = LINES-3;

	while (line < last_line) {
	  CleartoEOLN();
	  Writechar('\r');
	  Writechar('\n');
	  line++;
	}

	display_central_message();

	last_current = current;
	last_header_page = header_page;

	return(TRUE);
}

show_current()
{
	/** Show the new header, with all the usual checks **/

	register int first = 0, last = 0, last_line, new_line, i=0, j=0;
	char     newfrom[SLEN], old_buffer[SLEN], new_buffer[SLEN];

	(void) fix_header_page();	/* Who cares what it does? ;-) */

	/** compute last header to display **/

	first = header_page * headers_per_page;
	last  = first + (headers_per_page - 1);

	if (last > message_count) 
	  last = message_count;

	/** okay, now let's show the pointers... **/

	/** have we changed??? **/

	if (current == last_current) 
	  return;

	if (selected) {
	  dprint2(2,"\nshow_current\n* last_current = %d, current = %d\n", 
			last_current, current);
	  last_line = ((i=compute_visible(last_current-1)-1) %
			 headers_per_page)+4;
	  new_line  = ((j=compute_visible(current-1)-1) % headers_per_page)+4;
	  dprint1(2,"* compute_visible(last-1)=%d\n", 
		   compute_visible(last_current-1));
	  dprint1(2,"* compute_visible(current-1)=%d\n", 
		   compute_visible(current-1));
	  dprint2(2,"* ending up with last_line = %d and new_line = %d\n",
		   last_line, new_line);
	}
	else {
	  last_line = ((last_current-1) % headers_per_page)+4;
	  new_line  = ((current-1) % headers_per_page)+4;
	}
	
	dprint4(7,
	   "--> show-current: last_current=%d [%d] and current=%d [%d]\n",
	   last_current, i, current, j);

	dprint2(7,"    maps to lines %d and %d\n", last_line, new_line);

	if (has_highlighting && ! arrow_cursor) {
	  /** build the old and new header lines... **/
  
	  tail_of(header_table[current-1].from, newfrom, TRUE); 
	  build_header_line(new_buffer, &header_table[current-1], 
	         (selected? compute_visible(current-1) : current), 
		  TRUE, newfrom);

	  if (last_current > 0) {  /* say we went from no mail to new... */
	    tail_of(header_table[last_current-1].from, newfrom, TRUE); 
	    build_header_line(old_buffer, &header_table[last_current-1], 
	         (selected? compute_visible(last_current-1) : last_current), 
		  FALSE, newfrom);

	    ClearLine(last_line);
	    PutLine0(last_line, 0, old_buffer);
	  }
	  PutLine0(new_line, 0, new_buffer);
	}
	else {
	  if (on_page(last_current)) 
	    PutLine0(last_line,0,"  ");	/* remove old pointer... */
	  if (on_page(current))
	    PutLine0(new_line, 0,"->");
	}
	
	last_current = current;
}

build_header_line(buffer, entry, message_number, highlight, from)
char *buffer;
struct header_rec *entry;
int message_number, highlight;
char *from;
{
	/** Build in buffer the message header ... entry is the current
	    message entry, 'from' is a modified (displayable) from line, 
	    'highlight' is either TRUE or FALSE, and 'message_number'
	    is the number of the message.
	**/

	/** Note: using 'strncpy' allows us to output as much of the
	    subject line as possible given the dimensions of the screen.
	    The key is that 'strncpy' returns a 'char *' to the string
	    that it is handing to the dummy variable!  Neat, eh? **/
	
	char subj[LONG_SLEN],		/* to output subject */
	     buff[NLEN];		/* keep start_highlight value */

	if (strcmp(start_highlight,"->") != 0 && arrow_cursor) {
	  strcpy(buff, start_highlight);
	  strcpy(start_highlight, "->");
	}

	strncpy(subj, entry->subject, COLUMNS-45);

	subj[COLUMNS-45] = '\0';	/* insurance, eh? */

	/* now THIS is a frightening format statement!!!  */

	sprintf(buffer, "%s%s%c%c%c%-3d %3.3s %-2d %-18.18s (%d) %s%s%s", 
		highlight? ((has_highlighting && !arrow_cursor) ?
		           start_highlight : "->") : "  ",
		(highlight && has_highlighting && !arrow_cursor)? "  " : "",
		show_status(entry->status),
		(entry->status & DELETED? 'D' : ' '), 
		(entry->status & TAGGED?  '+' : ' '),
	        message_number,
	        entry->month, 
		atoi(entry->day), 
		from, 
		entry->lines, 
		(entry->lines / 1000   > 0? ""   :	/* spacing the  */
		  entry->lines / 100   > 0? " "  :	/* same for the */
		    entry->lines / 10  > 0? "  " :	/* lines in ()  */
		                            "   "),    /*   [wierd]    */
		subj,
		(highlight && has_highlighting && !arrow_cursor) ?
			  end_highlight : "");

	/** Actually, it's rather an impressive feat that we can
	    do so much in essentially one statement!  (Of course,
	    I'll bet the test suite for the printf routine isn't
	    THIS rigorous either!!!) (to be honest, though, just 
	    looking at this statement makes me chuckle...)
	**/

	if (arrow_cursor) 			/* restore! */
	  strcpy(start_highlight, buff);

}

int
fix_header_page()
{
	/** this routine will check and ensure that the current header
	    page being displayed contains messages!  It will silently
	    fix 'header-page' if wrong.  Returns TRUE if changed.  **/

	int last_page, old_header;

	old_header = header_page;

	last_page = (int) ((message_count-1) / headers_per_page);
 
	if (header_page > last_page) 
	  header_page = last_page;
	else if (header_page < 0) 
          header_page = 0;

	return(old_header != header_page);
}

int
on_page(message)
int message;
{
	/** Returns true iff the specified message is on the displayed page. **/

	dprint1(6,"** on_page(%d) returns...", message);

	if (selected) message = compute_visible(message-1);

	if (message >= header_page * headers_per_page)
	  if (message <= ((header_page+1) * headers_per_page)) {
	    dprint0(6,"TRUE\n");
	    return(TRUE);
	  }

	dprint0(6,"FALSE\n");
	return(FALSE);
}

show_status(status)
int status;
{
	/** This routine returns a single character indicative of
	    the status of this message.  The precedence is;

		F = form letter
		E = Expired message
	        P = Priority message
		A = Action associated with message
		N = New message
		_ = (space) default 
	**/

	     if (status & FORM_LETTER) return( 'F' );
	else if (status & EXPIRED)     return( 'E' );
	else if (status & PRIORITY)    return( 'P' );
	else if (status & ACTION)      return( 'A' );
	else if (status & NEW)         return( 'N' );
	else 			       return( ' ' );
}
