/**			elm.c			**/

/* Main program of the ELM mail system! 

   This file and all associated files and documentation:
	(C) Copyright 1986 Dave Taylor
*/

#include "elm.h"

#ifdef BSD
# undef toupper
# undef tolower
#endif

long bytes();
char *format_long();

main(argc, argv)
int argc;
char *argv[];
{
	char ch, address[SLEN], to_whom[LONG_SLEN];
	int  redraw, 		/** do we need to rewrite the entire screen? **/
	     nuhead, 		/** or perhaps just the headers section...   **/
	     nucurr, 		/** or just the current message pointer...   **/
	     nufoot; 		/** clear lines 16 thru bottom and new menu  **/
	int  i;      		/** Random counting variable (etc)           **/
	int  pageon, 		/** for when we receive new mail...          **/
	     last_in_mailbox;	/** for when we receive new mail too...      **/

	parse_arguments(argc, argv, to_whom);

	if (mail_only) {

	   initialize(FALSE);

	   Raw(ON);
	   dprint1(3,"Mail-only: mailing to\n-> \"%s\"\n", 
		   format_long(to_whom, 3));
	   (void) send(to_whom, "", TRUE, NO); 
	   leave(0);
	}

	initialize(TRUE);

	ScreenSize(&LINES, &COLUMNS);

	showscreen();

	mailfile_size = bytes(infile);

	Raw(ON);

	while (1) {
	  redraw = 0;
	  nuhead = 0;
	  nufoot = 0;
	  nucurr = 0;
	  if ((i = bytes(infile)) != mailfile_size) {
	    dprint1(2,"Just received %d bytes more mail (elm)\n", 
		    i - mailfile_size);
	    error("New mail has arrived!   Hang on...");
	    last_in_mailbox = message_count;
	    pageon = header_page;
	    newmbox(2, FALSE, TRUE);	/* last won't be touched! */
	    clear_error();
	    header_page = pageon;

	    if (on_page(current))   /* do we REALLY have to rewrite? */
	      showscreen();
	    else {
	      update_title();
	      ClearLine(LINES-1);	     /* remove reading message... */
	      error2("%d new message%s received", 
		     message_count - last_in_mailbox,
		     plural(message_count - last_in_mailbox));
	    }
	    mailfile_size = i;
	    if (cursor_control)
	      transmit_functions(ON);	/* insurance */
	  }

	  prompt("Command: ");

	  CleartoEOLN();
	  ch = tolower(GetPrompt()); 
	  CleartoEOS();
	  dprint1(4, "\nCommand: %c\n\n", ch);

	  set_error("");	/* clear error buffer */

	  MoveCursor(LINES-3,strlen("Command: "));

	  switch (ch) {

	    case '?' 	:  if (help())
	  		     redraw++;
			   else
			     nufoot++;
			   break;

	    case '$'    : resync(); 					break;

	    case ' '    : 
	    case '+'	:  header_page++; nuhead++;	
			   if (move_when_paged && header_page <=
			      (message_count / headers_per_page)) {
			     current = header_page*headers_per_page + 1;
			     if (selected)
			       current = visible_to_index(current)+1;
	                   }
			   break;

	    case '-'    :  header_page--; nuhead++;	
			   if (move_when_paged && header_page >= 0) {
			     current = header_page*headers_per_page + 1;
			     if (selected)
			       current = visible_to_index(current)+1;
			   }
			   break;

	    case '='    :  if (selected)
			     current = visible_to_index(1)+1;
	                   else
			     current = 1;
	                   if (get_page(current))
			     nuhead++;	
			   else
			     nucurr++; 			break;

	    case '*'    :  if (selected) 
			     current = (visible_to_index(selected)+1);
			   else
			     current = message_count;	
	                   if (get_page(current))
			     nuhead++;	
			   else
			     nucurr++; 			break;

	    case '|'    :  Writechar('|'); 
	    		   softkeys_off();
                           redraw = do_pipe();		
	                   softkeys_on(); 		break;

	    case '!'    :  Writechar('!'); 
	    		   softkeys_off();
                           redraw = subshell();		
	                   softkeys_on(); 		break;

	    case '%'    :  get_return(address);
			   clear_error();
			   PutLine1(LINES,(COLUMNS-strlen(address))/2,
			            "%.78s", address);	
		           break;

	    case '/'    :  if (pattern_match()) {
	                     if (get_page(current))
			       nuhead++;
	                     else
	                       nucurr++;
	                   }
			   else 
			      error("pattern not found!");
			   break;

	    case '<'    :  /* scan current message for calendar information */
#ifdef ENABLE_CALENDAR
			   PutLine0(LINES-3, strlen("Command: "), 	
				   "Scan message for calendar entries...");
			   scan_calendar();
#else
	 		   error("Sorry - calendar function disabled");
#endif
			   break;

	    case 'a'    :  alias();     
			   nufoot++; 	
			   define_softkeys(MAIN); 	break;
			
	    case 'b'    :  PutLine0(LINES-3, strlen("Command: "), 
		             "Bounce message");
			   fflush(stdout);
			   if (message_count < 1)
	  		     error("No mail to bounce!");
			   else 
			     nufoot = remail();
			   break;

	    case 'c'    :  PutLine0(LINES-3, strlen("Command: "), 
			      "Change mailbox");
			   define_softkeys(CHANGE);
			   if ((file_changed = leave_mbox(FALSE)) != -1) {
	                     redraw = newmbox(0, TRUE, TRUE);
	    		     dprint1(1, "** redraw returned as %d **\n",
				     redraw);
			     mailfile_size = bytes(infile);	
	      	           }
			   else {
			     file_changed = 0;
			     sort_mailbox(message_count, FALSE);
			   }
			   define_softkeys(MAIN);
			   break;

	    case '^'    :
	    case 'd'    :  if (message_count < 1)
			     error("No mail to delete!");
			   else {
 	                     delete_msg((ch == 'd'));			
			     if (resolve_mode) 	/* move after mail resolved */
			       if (current < message_count) {
	                         current++;  		
			         if (get_page(current))
			           nuhead++;
			         else
			           nucurr++;
			       }
	                   }
			   break;

	    case ctrl('D') : if (message_count < 1)
			       error("No mail to delete!");
			     else 
			       meta_match(DELETED);
			     break;

	    case 'e'    :  PutLine0(LINES-3,strlen("Command: "),"Edit mailbox");
			   if (current > 0) {
			     edit_mailbox();
	    		     if (cursor_control)
	                       transmit_functions(ON);	/* insurance */
	   		   }
			   else
			     error("Mailbox is empty!");
			   break;
		
	    case 'f'    :  PutLine0(LINES-3, strlen("Command: "), "Forward");
			   define_softkeys(YESNO);
			   if (current > 0)  
	                     redraw = forward();   
			   else 
	                     error("No mail to forward!");
			   define_softkeys(MAIN);
			   break;

	    case 'g'    :  PutLine0(LINES-3,strlen("Command: "), "Group reply");
		           fflush(stdout);
			   if (current > 0) {
			     if (header_table[current-1].status & FORM_LETTER)
			       error("Can't group reply to a Form!!");
			     else {
			       PutLine0(LINES-3,COLUMNS-40,
                                       "building addresses...");
			       define_softkeys(YESNO);
	                       redraw = reply_to_everyone();	
			       define_softkeys(MAIN);
	                     }
	                   }
			   else 
			     error("No mail to reply to!"); 
			   break;

	    case 'h'    :  if (filter)
	                     PutLine0(LINES-3, strlen("Command: "), 
				"Message with headers...");
	                   else
			     PutLine0(LINES-3, strlen("Command: "),"Read message");
			   fflush(stdout);
			   i = filter;
			   filter = FALSE;
			   redraw = show_msg(current);
			   filter = i;
			   break;

	    case 'j'    :  if (selected) {
			     if ((current = next_visible(current)) < 0)
	                       current = visible_to_index(selected)+1;
			   }
			   else 
	                     current++;  
			   if (get_page(current))
			     nuhead++;
			   else
			     nucurr++;			break;

	    case 'k'    :  if (selected) 
			     current = previous_visible(current);
	     		   else
	                     current--;  
			   if (get_page(current))
			     nuhead++;
			   else
			     nucurr++;			break;

	    case 'l'    :  PutLine0(LINES-3, strlen("Command: "),
			           "Limit displayed messages by...");
			   if (limit() != 0) {
	                     nuhead++;
	   		     update_title();	/* poof! */
			   }
			   else
	    		     nufoot++;
			   break;

	    case 'm'    :  PutLine0(LINES-3, strlen("Command: "), "Mail");
			   redraw = send("", "", TRUE, allow_forms); 
			   break;

	    case ctrl('J'):
	    case ctrl('M'):PutLine0(LINES-3, strlen("Command: "), "Read Message");	
			   fflush(stdout);
			   define_softkeys(READ);
			   redraw = show_msg(current);
			   break;

	    case 'n'    :  PutLine0(LINES-3, strlen("Command: "), "Next Message");
			   fflush(stdout);
			   define_softkeys(READ);
			   redraw = show_msg(current);
			   current += redraw;		
			   if (current > message_count)
			     current = message_count;
			   (void) get_page(current); /* rewrites ANYway */
			   break;

	    case 'o'    :  PutLine0(LINES-3, strlen("Command: "), "Options");
			   options();
			   redraw++;	/* always fix da screen... */
			   break;

	    case 'p'    :  PutLine0(LINES-3, strlen("Command: "), "Print mail");
			   fflush(stdout);
			   if (message_count < 1)
			     error("No mail to print!");
			   else
			     printmsg();			
			   break;

	    case 'q'    :  PutLine0(LINES-3, strlen("Command: "), "Quit");

			   if (mbox_specified == 0) lock(OUTGOING);

			   if (mailfile_size != bytes(infile)) {
			     error("New Mail!  Quit cancelled...");
	  		     if (mbox_specified == 0) unlock();
	                   }
		           else
			     quit();		

			   break;

	    case 'r'    :  PutLine0(LINES-3, strlen("Command: "), 
			      "Reply to message");
			   if (current > 0) 
	                     redraw = reply();	
			   else 
			     error("No mail to reply to!"); 
			   softkeys_on();
			   break;

	    case '>'    : /** backwards compatibility **/

	    case 's'    :  if  (message_count < 1)
			     error("No mail to save!");
			   else {
	                     PutLine0(LINES-3, strlen("Command: "),
				      "Save Message");
			     PutLine0(LINES-3,COLUMNS-40,
				"(Use '?' to list your folders)");
			     if (save(&redraw) && resolve_mode) {
			       if (current < message_count) {
			         current++;	/* move to next message */
			         if (get_page(current))
			           nuhead++;
			         else
			           nucurr++;		
			       }
			     }
			   }
			   ClearLine(LINES-2);		
			   break;

            case ctrl('T') :
	    case 't'       :  if (message_count < 1)
			        error("no mail to tag!");
			      else if (ch == 't')
			        tag_message(); 
			      else
			        meta_match(TAGGED);
	                      break;

	    case 'u'    :  if (message_count < 1)
			     error("no mail to mark as undeleted!");
			   else {
	                     undelete_msg();		        
			     if (resolve_mode) 	/* move after mail resolved */
			       if (current < message_count) {
	                         current++;  		
			         if (get_page(current))
			           nuhead++;
			         else
			           nucurr++;
			       }
			   }
			   break;

	    case ctrl('U') : if (message_count < 1)
			       error("No mail to undelete!");
			     else 
			       meta_match(UNDELETE);
			     break;

	    case ctrl('Q') :
	    case ctrl('?') : 
	    case 'x'    :  PutLine0(LINES-3, strlen("Command: "), "Exit");  
                           fflush(stdout);              leave();

	    case ctrl('L') : redraw++;	break;
	    
	    case '@'    : debug_screen();  redraw++;	break;
	
	    case '#'    : debug_message(); redraw++;	break;

	    case NO_OP_COMMAND : break;	/* noop for timeout loop */

	    case ESCAPE : if (cursor_control) {
			    ch = ReadCh(); 
	                    if (ch == up[1]) {
			      if (selected)
			        current = previous_visible(current);
			      else 
			        current--;
			      if (get_page(current))
			        nuhead++;
			      else
			        nucurr++;			
	                    }
			    else if (ch == down[1]) {
			      if (selected) {
			        if ((current = next_visible(current)) < 0)
	                          current = visible_to_index(selected)+1;
			      }
			      else 
			        current++;
			      if (get_page(current))
			        nuhead++;
			      else
			        nucurr++;			
			    }
			    else if (hp_terminal) { 

			     switch (ch) {
			      case 'U' :	/* <NEXT> */
	    			 header_page++; 
				 nuhead++;
			         if (move_when_paged && header_page 
			             <= (message_count / headers_per_page)) {
			           current = header_page*headers_per_page + 1;
			           if (selected)
			             current = visible_to_index(current)+1;
			         }
				 break;

			      case 'V' :       /* <PREV> */
	    			header_page--; 
				nuhead++;
			        if (move_when_paged && header_page >= 0) {
			           current = header_page*headers_per_page + 1;
			           if (selected)
			             current = visible_to_index(current)+1;
			         }
				break;

			      case 'h' : 	
			      case 'H' : 	/* <HOME UP> */
	    		        if (selected)
			          current = visible_to_index(1)+1;
	                        else
			          current = 1;
	                        if (get_page(current))
			          nuhead++;
	                        else
	                          nucurr++;
				break;

			      case 'F' : 	/* <HOME DOWN> */
	    		        if (selected)
			          current = visible_to_index(selected)+1;
	                        else
			          current = message_count;
	                        if (get_page(current))
			          nuhead++;
	                        else
	                          nucurr++;
			        break;

			      /** let's continue, what the heck... **/

			      case 'A' : 	/* <UP> */
			      case 'D' : 	/* <BACKTAB> */
			      case 'i' : 	/* <LEFT> */
			         if (selected)
			           current = previous_visible(current);
			         else 
			           current--;
			         if (get_page(current))
			           nuhead++;
			         else
			           nucurr++;			
				 break;

			      case 'B' : 	/* <UP> */
			      case 'I' : 	/* <BACKTAB> */
			      case 'C' : 	/* <LEFT> */
			         if (selected) {
			           if ((current = next_visible(current)) < 0)
	                             current = visible_to_index(selected)+1;
			         }
			         else 
			           current++;
			         if (get_page(current))
			           nuhead++;
			         else
			           nucurr++;			
			         break;

			      default: PutLine2(LINES-3, strlen("Command: "), 
				          "%c%c", ESCAPE, ch);
			     }
			    }
			    else /* false hit - output */
			      PutLine2(LINES-3, strlen("Command: "), 
				          "%c%c", ESCAPE, ch);
			    break;
			  }

			  /* else fall into the default error message! */

	    default	: if (ch > '0' && ch <= '9') {
			    PutLine0(LINES-3, strlen("Command: "), 
				    "New Current Message");
			    current = read_number(ch);
			    if (selected) {
			      if ((current = visible_to_index(current)+1) >
				  message_count)
			        goto too_big;
	                    }
	                    if (get_page(current))
			      nuhead++;
	                    else
	                      nucurr++;
			  }
			  else
	 		    error("Unknown command: Use '?' for commands");
	  }

	  dprint5(4,"redraw=%s, current=%d, nuhead=%s, nufoot=%s, nucurr=%s\n",
		  onoff(redraw), current, onoff(nuhead), onoff(nufoot),
		  onoff(nucurr));

	  if (redraw)
	    showscreen();

	  if (current < 1) {
	    if (message_count > 0) {
	      error("already at message #1!");
	      if (selected)
	        current = compute_visible(0);	/* get to #0 */
	      else
	        current = 1;
	    }
	    else if (current < 0) {
	      error("No messages to read!");
	      current = 0;
	    }
	  }
	  else if (current > message_count) {
	    if (message_count > 0) {
too_big:
	      if (selected) {
	        error2("only %d message%s selected!", selected, 
			plural(selected));
	        current = compute_visible(selected);
	      }
	      else {
	        error2("only %d message%s!", message_count, 
		       plural(message_count));
	        current = message_count;
	      }
	    }
	    else {
	      error("No messages to read!");
	      current = 0;
	    }
	  }
	  else if (selected && (i=visible_to_index(selected)) > message_count) {
	    error2("only %d message%s selected!", selected, plural(selected));
	    current = i+1;	/* FIXED!  Phew! */
	  }
	    
	  if (nuhead) 
	    show_headers();
	  else if (nucurr)
	    show_current();
	  else if (nufoot) {
	    if (mini_menu) {
	      MoveCursor(LINES-7, 0);  
              CleartoEOS();
	      show_menu();
	    }
	    else {
	      MoveCursor(LINES-4, 0);
	      CleartoEOS();
	    }
	  }

	} /* the BIG while loop! */
}

debug_screen()
{
	/**** spit out all the current variable settings and the table
	      entries for the current 'n' items displayed. ****/

	register int i, j;
	char     buffer[SLEN];

	ClearScreen();
	Raw(OFF);

	PutLine2(0,0,"Current message number = %d\t\t%d message(s) total\n",
	        current, message_count);
	PutLine2(2,0,"Header_page = %d           \t\t%d possible page(s)\n",
		header_page, (int) (message_count / headers_per_page) + 1);

	PutLine1(4,0,"\nCurrent mailfile is %s.\n\n", infile);

	i = header_page*headers_per_page;	/* starting header */

	if ((j = i + (headers_per_page-1)) >= message_count) 
	  j = message_count-1;

	Write_to_screen(
"Num      From                 	Subject                         Lines  Offset\n\n",0);

	while (i <= j) {
	   sprintf(buffer, 
	   "%3d  %-16.16s  %-40.40s  %4d  %d\n",
		    i+1,
	            header_table[i].from, 
	            header_table[i].subject,
		    header_table[i].lines,
		    header_table[i].offset);
	    Write_to_screen(buffer, 0);
	  i++;
	}
	
	Raw(ON);

	PutLine0(LINES,0,"Press any key to return: ");
	(void) ReadCh();
}


debug_message()
{
	/**** Spit out the current message record.  Include EVERYTHING
	      in the record structure. **/
	
	char buffer[SLEN];

	ClearScreen();
	Raw(OFF);

	Write_to_screen("\t\t\t----- Message %d -----\n\n\n\n", 1,
		current);

	Write_to_screen("Lines : %-5d\t\t\t\tStatus: N  E  A  P  D  T  V\n", 1,
		header_table[current-1].lines);
	Write_to_screen("            \t\t\t\t        e  x  c  r  e  a  i\n", 0);
	Write_to_screen("            \t\t\t\t        w  p  t  i  l  g  s\n", 0);

	sprintf(buffer, 
		"\nOffset: %ld\t\t\t\t          %d  %d  %d  %d  %d  %d  %d\n",
		header_table[current-1].offset,
		(header_table[current-1].status & NEW) != 0,
		(header_table[current-1].status & EXPIRED) != 0,
		(header_table[current-1].status & ACTION) != 0,
		(header_table[current-1].status & PRIORITY) != 0,
		(header_table[current-1].status & DELETED) != 0,
	        (header_table[current-1].status & TAGGED) != 0,
		(header_table[current-1].status & VISIBLE) != 0);
	Write_to_screen(buffer, 0);

	sprintf(buffer, "\nReceived on: %d/%d/%d at %d:%02d\n\n",
	        header_table[current-1].received.month+1,
	        header_table[current-1].received.day,
	        header_table[current-1].received.year,
	        header_table[current-1].received.hour,
	        header_table[current-1].received.minute);
	Write_to_screen(buffer, 0);

	sprintf(buffer, "Message sent on: %s, %s %s, %s at %s\n\n",
	        header_table[current-1].dayname,
	        header_table[current-1].month,
	        header_table[current-1].day,
	        header_table[current-1].year,
	        header_table[current-1].time);
	Write_to_screen(buffer, 0);
	
	Write_to_screen("\nFrom: %s\n\nSubject: %s", 2,
		header_table[current-1].from,
	        header_table[current-1].subject);

	Write_to_screen("\nTo: %s\n\nIndex = %d\n", 2,
		header_table[current-1].to,
		header_table[current-1].index_number);
	
	Raw(ON);

	PutLine0(LINES,0,"Press any key to return: ");
	(void) ReadCh();
}
