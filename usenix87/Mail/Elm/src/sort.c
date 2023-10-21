/**			sort.c				**/

/** Sort mailbox header table by the field specified in the global
    variable "sortby"...if we're sorting by something other than
    the default SENT_DATE, also put some sort of indicator on the
    screen.

    (C) Copyright 1986, Dave Taylor
**/

#include "headers.h"

char *sort_name();
void   qsort();

sort_mailbox(entries, visible)
int entries, visible;
{
	/** Sort the header_table definitions... If 'visible', then
	    put the status lines etc **/
	
	int last_index = -1;
	int compare_headers();	/* for sorting */

	dprint1(2,"\n** sorting mailbox by %s **\n\n", sort_name(FULL));

	if (entries > 0)
	  last_index = header_table[current-1].index_number;

	if (entries > 30 && visible)  
	  error1("sorting messages by %s", sort_name(FULL));
	
	qsort(header_table, (unsigned) entries, sizeof (struct header_rec), 
	      compare_headers);

	if (last_index > -1)
	  find_old_current(last_index);

	clear_error();
}

int
compare_headers(first, second)
struct header_rec *first, *second;
{
	/** compare two headers according to the sortby value.

	    Sent Date uses a routine to compare two dates,
	    Received date is keyed on the file offsets (think about it)
	    Sender uses the truncated from line, same as "build headers",
	    and size and subject are trivially obvious!!
	 **/

	char from1[SLEN], from2[SLEN];	/* sorting buffers... */
	int  sign = 1;
	
	if (sortby < 0)
	  sign = -1;

	switch (abs(sortby)) {

	  case SENT_DATE : return( sign*compare_dates(first, second));

	  case RECEIVED_DATE: return( sign*
			              compare_parsed_dates(first->received, 
				           second->received));

	  case SENDER    : tail_of(first->from, from1, TRUE);
			   tail_of(second->from, from2, TRUE);
	  		   return( sign*strcmp(from1, from2));

	  case SIZE      : return( sign*(first->lines - second->lines));

	  case SUBJECT   : /* need some extra work 'cause of STATIC buffers */
	                   strcpy(from1, shift_lower(first->subject));	
			   return( 
			     sign*strcmp(from1, shift_lower(second->subject)));

	  case STATUS    : return( sign*(first->status - second->status));
	}

	return(0);	/* never get this! */
}

char *sort_name(type)
int type;
{
	/** return the name of the current sort option...
	    type can be "FULL", "SHORT" or "PAD"
	**/
	int pad, abr;
	
	pad = (type == PAD);
	abr = (type == SHORT);

	if (sortby < 0) {
	  switch (- sortby) {
	    case SENT_DATE    : return( 
		              pad?     "Reverse Date Mail Sent  " : 
			      abr?     "Reverse-Sent" :
				       "Reverse Date Mail Sent");
	    case RECEIVED_DATE: return(
			      abr?     "Reverse-Received":
			               "Reverse Date Mail Rec'vd");
	    case SENDER       : return(
			      pad?     "Reverse Message Sender  " : 
			      abr?     "Reverse-From":
				       "Reverse Message Sender");
	    case SIZE         : return(
			      abr?     "Reverse-Lines" : 
				       "Reverse Lines in Message");
	    case SUBJECT      : return(
			      pad?     "Reverse Message Subject " : 
			      abr?     "Reverse-Subject" : 
				       "Reverse Message Subject");
	    case STATUS	      : return(
			      pad?     "Reverse Message Status  " :
			      abr?     "Reverse-Status":
			               "Reverse Message Status");
	  }
	}
	else {
	  switch (sortby) {
	    case SENT_DATE    : return( 
		                pad?   "Date Mail Sent          " : 
		                abr?   "Sent" : 
				       "Date Mail Sent");
	    case RECEIVED_DATE: return(
	                        pad?   "Date Mail Rec'vd        " :
	                        abr?   "Received" :
	                               "Date Mail Rec'vd");
	    case SENDER       : return(
			        pad?   "Message Sender          " : 
			        abr?   "From" : 
				       "Message Sender");
	    case SIZE         : return(
	    			pad?   "Lines in Message        " :
	    			abr?   "Lines" :
	    			       "Lines in Message");
	    case SUBJECT      : return(
			        pad?   "Message Subject         " : 
			        abr?   "Subject" : 
				       "Message Subject");
	    case STATUS	      : return(
			        pad?   "Message Status          " :
			        abr?   "Status" :
			               "Message Status");
	  }
	}

	return("*UNKNOWN-SORT-PARAMETER*");
}

find_old_current(index)
int index;
{
	/** Set current to the message that has "index" as it's 
	    index number.  This is to track the current message
	    when we resync... **/

	register int i;

	dprint1(2,"find-old-current(%d)\n", index);

	for (i = 0; i < message_count; i++)
	  if (header_table[i].index_number == index) {
	    current = i+1;
	    dprint1(2,"\tset current to %d!\n", current);
	    return;
	  }

	dprint1(2,"\tcouldn't find current index.  Current left as %d\n",
		current);
	return;		/* can't be found.  Leave it alone, then */
}
