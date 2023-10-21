/**			limit.c			**/

/** This stuff is inspired by MH and dmail and is used to 'select'
    a subset of the existing mail in the folder based on one of a
    number of criteria.  The basic tricks are pretty easy - we have
    as status of VISIBLE associated with each header stored in the
    (er) mind of the computer (!) and simply modify the commands to
    check that flag...the global variable `selected' is set to the
    number of messages currently selected, or ZERO if no select.

       (C) Copyright 1986, Dave Taylor
**/

#include "headers.h"

#define TO		1
#define FROM		2

char *shift_lower();

int
limit()
{
	/** returns non-zero if we did enough to redraw the screen **/
	
	char criteria[STRING], first[STRING], rest[STRING];
	int  last_current;

	if (selected) {
	  PutLine1(LINES-2, 0, 
		"Already have selection criteria - add more? (y/n) n%c",
		BACKSPACE);
	  ReadCh(criteria[0]);
	  if (tolower(criteria[0]) == 'y') 
	    PutLine0(LINES-3, COLUMNS-30, "Adding criteria...");
	  else {
	    selected = 0;
	    PutLine0(LINES-3, COLUMNS-30, "Use '?' for help");
	  }
	  
	}

	PutLine1(LINES-2, 0, "Enter criteria: ");
	CleartoEOLN();

	criteria[0] = '\0';
	optionally_enter(criteria, LINES-2, 16, FALSE);
	
	if (strlen(criteria) == 0) return(0);

	split_word(criteria, first, rest);
	
	if (equal(first, "all")) {
	   selected = 0;
	   return(TRUE);
	}

	last_current = current;
	current = -1;

	if (equal(first, "subj") || equal(first, "subject"))
	  selected = limit_selection(SUBJECT, rest, selected);
	else if (equal(first, "to"))
	  selected = limit_selection(TO, rest, selected);
	else if (equal(first, "from"))
	  selected = limit_selection(FROM, rest, selected);
	else {
	  selected = 0;
	  error1("Don't understand \"%s\" as a selection criteria!", first);
	  sleep(2);
	}

	if (! selected)
	  current = last_current;
	else
	  current = visible_to_index(1)+1;	/* map it and shift up 1 */

	if (! selected)
	  set_error("no items selected");
	else {
	  sprintf(first, "%d items selected", selected);
	  set_error(first);
	}
	
	return(selected);
}

int
limit_selection(based_on, pattern, additional_criteria)
int based_on, additional_criteria;
char *pattern;
{
	/** Given the type of criteria, and the pattern, mark all
	    non-matching headers as ! VISIBLE.  If additional_criteria,
	    don't mark as visible something that isn't currently!
	**/

	register int index, count = 0;

	dprint3(2,"\n\n\n**limit on %d - '%s' - (%s) **\n\n",
		   based_on, pattern, additional_criteria?"add'tl":"base");

	if (based_on == SUBJECT) {
	  for (index = 0; index < message_count; index++)
	    if (! in_string(shift_lower(header_table[index].subject), pattern))
	      header_table[index].status &= ~VISIBLE;
	    else if (additional_criteria && 	
		     header_table[index].status | VISIBLE)
	      header_table[index].status &= ~VISIBLE;	/* shut down! */
	    else { /* mark it as readable */
	      header_table[index].status |= VISIBLE;
	      count++;
	      dprint3(5,"  Message %d (%s from %s) marked as visible\n",
			index, header_table[index].subject,
			header_table[index].from);
	    }
	}
	else if (based_on == FROM) {
	  for (index = 0; index < message_count; index++)
	    if (! in_string(shift_lower(header_table[index].from), pattern))
	      header_table[index].status &= ~VISIBLE;
	    else if (additional_criteria && 	
		     header_table[index].status | VISIBLE)
	      header_table[index].status &= ~VISIBLE;	/* shut down! */
	    else { /* mark it as readable */
	      header_table[index].status |= VISIBLE;
	      count++;
	      dprint3(5,"  Message %d (%s from %s) marked as visible\n",
			index, header_table[index].subject,
			header_table[index].from);
	    }
	}
	else if (based_on == TO) {
	  for (index = 0; index < message_count; index++)
	    if (! in_string(shift_lower(header_table[index].to), pattern))
	      header_table[index].status &= ~VISIBLE;
	    else if (additional_criteria && 	
		     header_table[index].status | VISIBLE)
	      header_table[index].status &= ~VISIBLE;	/* shut down! */
	    else { /* mark it as readable */
	      header_table[index].status |= VISIBLE;
	      count++;
	      dprint3(5,"  Message %d (%s from %s) marked as visible\n",
			index, header_table[index].subject,
			header_table[index].from);
	    }
	}

	dprint1(4,"\n** returning %d selected **\n\n\n", count);

	return(count);
}

int
next_visible(index)
int index;
{
	/** Given 'index', this routine will return the actual index into the
	    array of the NEXT visible message, or '-1' if none are visible 
	**/
	int remember_for_debug;

	remember_for_debug = index;

	index--;	/* shift from 'current' to actual index  */
	index++;	/* make sure we don't bump into ourself! */

	while (index < message_count) {
	  if (header_table[index].status & VISIBLE) {
	    dprint2(9,"[Next visible: given %d returning %d]\n", 
		       remember_for_debug, index+1);
	    return(index+1);
	  }
	  index++;
	}

	return(-1);
}

int
previous_visible(index)
int index;
{
	/** Just like 'next-visible', but backwards FIRST... */
	
	int remember_for_debug;

	remember_for_debug = index;

	index -= 2;	/* shift from 'current' to actual index, and skip us! */

	while (index > -1) {
	  if (header_table[index].status & VISIBLE) {
	    dprint2(9,"[previous visible: given %d returning %d]",
		    remember_for_debug, index+1);
	    return(index+1);
	  }
	  index--;
	}

	return(-1);
}

int
compute_visible(message)
int message;
{
	/** return the 'virtual' index of the specified message in the
	    set of messages - that is, if we have the 25th message as
	    the current one, but it's #2 based on our limit criteria,
	    this routine, given 25, will return 2.
	**/

	register int index, count = 0;

	if (! selected) return(message);

	if (message < 0) message = 0;	/* normalize */

	for (index = 0; index <= message; index++)
	   if (header_table[index].status & VISIBLE) 
	     count++;

	dprint2(4, "[compute-visible: displayed message %d is actually %d]\n",
		count, message);

	return(count);
}

int
visible_to_index(message)
int message;
{
	/** Given a 'virtual' index, return a real one.  This is the
	    flip-side of the routine above, and returns (message_count+1)
	    if it cannot map the virtual index requested (too big) 
	**/

	register int index = 0, count = 0;

	for (index = 0; index < message_count; index++) {
	   if (header_table[index].status & VISIBLE) 
	     count++;
	   if (count == message) {
	     dprint2(4,"visible-to-index: (up) index %d is displayed as %d\n",
		     message, index);
	     return(index);
	   }
	}

	dprint1(4, "index %d is NOT displayed!\n", message);

	return(message_count+1);
}
