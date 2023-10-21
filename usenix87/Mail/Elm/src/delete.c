/**		delete.c		**/

/**  Delete or undelete files: just set flag in header record! 
     Also tags specified message(s)...

     (C) Copyright 1985  Dave Taylor
**/

#include "headers.h"

delete_msg(real_del)
int real_del;
{
	/** Delete current message.  If real-del is false, then we're
	    actually requested to toggle the state of the current
	    message... **/

	if (real_del)
	  header_table[current-1].status |= DELETED;
	else if (ison(header_table[current-1].status, DELETED))
	  clearit(header_table[current-1].status, DELETED);
	else
	  setit(header_table[current-1].status, DELETED);

	show_msg_status(current-1);
}

undelete_msg()
{
	/** clear the deleted message flag **/

	clearit(header_table[current-1].status, DELETED);

	show_msg_status(current-1);
}

show_msg_status(msg)
int msg;
{
	/** show the status of the current message only.  **/

	if (on_page(msg)) {
	  MoveCursor((msg % headers_per_page) + 4, 3);
	  if (msg == current && !arrow_cursor) {
	    StartBold();
	    Writechar( ison(header_table[msg].status, DELETED)? 'D' : ' ');
	    EndBold();
	  }
	  else
	    Writechar( ison(header_table[msg].status, DELETED)? 'D' : ' ');
	}
}

tag_message()
{
	/** Tag current message.  If already tagged, untag it. **/

	if (ison(header_table[current-1].status, TAGGED))
	  clearit(header_table[current-1].status, TAGGED);
	else
	  setit(header_table[current-1].status, TAGGED);

	show_msg_tag(current-1);
}

show_msg_tag(msg)
int msg;
{
	/** show the tag status of the current message only.  **/

	if (on_page(msg)) {
	  MoveCursor((msg % headers_per_page) + 4, 4);
	  if (msg == current && !arrow_cursor) {
	    StartBold();
	    Writechar( ison(header_table[msg].status, TAGGED)? '+' : ' ');
	    EndBold();
	  }
	  else
	    Writechar( ison(header_table[msg].status, TAGGED)? '+' : ' ');
	}	
}

show_new_status(msg)
int msg;
{
	/** If the specified message is on this screen, show
	    the new status (could be marked for deletion now,
	    and could have tag removed...)
	**/

	if (on_page(msg)) 
	  if (msg == current && !arrow_cursor) {
	    StartBold();
	    PutLine2((msg % headers_per_page) + 4, 3, "%c%c",
		   ison(header_table[msg].status, DELETED)? 'D' : ' ',
		   ison(header_table[msg].status, TAGGED )? '+' : ' ');
	    EndBold();
	  }
	  else
	    PutLine2((msg % headers_per_page) + 4, 3, "%c%c",
		   ison(header_table[msg].status, DELETED)? 'D' : ' ',
		   ison(header_table[msg].status, TAGGED )? '+' : ' ');
}
