/**			hdrconfg.c			**/

/**   This file contains the routines necessary to be able to modify
      the mail headers of messages on the way off the machine.  The
      headers currently supported for modification are:

	Subject:
	To:
	Cc:
	Bcc:
	Reply-To:

	Expiration-Date:
	Priority:
        In-Reply-To:
	Action:

	<user defined>
	
      (C) Copyright 1985, Dave Taylor
**/

#include <stdio.h>
#include "headers.h"

#include <ctype.h>

#ifdef BSD
#undef toupper
#endif

/* these are all defined in the mailmsg file! */

extern char subject[SLEN], action[SLEN], in_reply_to[SLEN], expires[SLEN], 
            priority[SLEN], reply_to[SLEN], to[VERY_LONG_STRING], 
	    cc[VERY_LONG_STRING], expanded_to[VERY_LONG_STRING], 
	    expanded_cc[VERY_LONG_STRING], user_defined_header[SLEN];

#ifdef ALLOW_BCC
extern char bcc[VERY_LONG_STRING], expanded_bcc[VERY_LONG_STRING];
#endif

char *strip_commas(), *strcpy();

edit_headers()
{
	/** Edit headers.  **/

	int unexpanded_to = TRUE, unexpanded_cc = TRUE;
#ifdef ALLOW_BCC
	int unexpanded_bcc = TRUE;
#endif
	char c;
	
	if (mail_only) goto outta_here;		/* how did we get HERE??? */

	display_headers();

	while (TRUE) {	/* forever */
	  PutLine0(LINES-1,0,"Choice: ");
	  CleartoEOLN();
	  c = toupper(getchar());
	  clear_error();
	  switch (c) {
	    case RETURN:
	    case LINE_FEED:
	    case 'Q' : goto outta_here;
	    case ctrl('L') : display_headers();
		       break;
	    case 'T' : if (optionally_enter(to, 2, 5, TRUE) == -1)
	                 goto outta_here;
	    	       build_address(strip_commas(to), expanded_to);
		       unexpanded_to = FALSE; 
		       break;
	    case 'S' : if (optionally_enter(subject, 7, 9, FALSE) == -1)
			 goto outta_here;
		       break;
#ifdef ALLOW_BCC
	    case 'B' : if (optionally_enter(bcc, 4, 5, TRUE) == -1)
			 goto outta_here;
	  	       build_address(strip_commas(bcc), expanded_bcc);
		       unexpanded_bcc = FALSE;
		       break;
#endif
	    case 'C' : if (optionally_enter(cc, 3, 5, TRUE) == -1)
			 goto outta_here;
	  	       build_address(strip_commas(cc), expanded_cc);
		       unexpanded_cc = FALSE;
		       break;
	    case 'R' : if (optionally_enter(reply_to, 5, 10, FALSE) == -1)
			 goto outta_here;
		       break;
	    case 'A' : if (optionally_enter(action, 9, 9, FALSE) == -1)
			 goto outta_here;
		       break;
	    case 'E' : enter_date(10, 17, expires);
		       break;
	    case 'P' : if (optionally_enter(priority, 11,10, FALSE) == -1)
			 goto outta_here;
		       break;
	    case 'U' : if (optionally_enter(user_defined_header,14,0,FALSE)==-1)
			 goto outta_here;
		       else
		         check_user_header(user_defined_header);
		       break;
	    case 'I' : if (strlen(in_reply_to) > 0) {
	                 if (optionally_enter(in_reply_to, 12,13, FALSE) == -1)
			   goto outta_here;
			 break;		
		       }
		       /** else fall through as an error **/
	    default  : error("Unknown header being specified!");
	  }
	} 

outta_here:	/* this section re-expands aliases before we leave... */

	if (unexpanded_to)
	  build_address(strip_commas(to), expanded_to);
	if (unexpanded_cc)
	  build_address(strip_commas(cc), expanded_cc);
#ifdef ALLOW_BCC
	if (unexpanded_bcc)
	  build_address(strip_commas(bcc), expanded_bcc);
#endif
}

display_headers()
{
	ClearScreen();

	Centerline(0,"Message Header Edit Screen");

	PutLine1(2,0,"To : %s", to);
	PutLine1(3,0,"Cc : %s", cc); CleartoEOLN();
#ifdef ALLOW_BCC
	PutLine1(4,0,"Bcc: %s", bcc); CleartoEOLN();
#endif
	PutLine1(5,0,"Reply-To: %s", reply_to); CleartoEOS();

	PutLine1(7,0,"Subject: %s", subject);
	PutLine1(9,0,"Action : %s", action);
	PutLine1(10,0,"Expiration-Date: %s", expires);
	PutLine1(11,0,"Priority: %s", priority);
	if (strlen(in_reply_to) > 0)
	  PutLine1(12,0,"In-Reply-To: %s", in_reply_to);

	if (strlen(user_defined_header) > 0)
	  PutLine1(14,0, "%s", user_defined_header);

	Centerline(LINES-5, 
"Choose first letter of existing header, U)ser defined header, or <return>");
}

enter_date(x, y, buffer)
int x, y;
char *buffer;
{
	/** Enter the number of days this message is valid for, then
	    display at (x,y) the actual date of expiration.  This 
	    routine relies heavily on the routine 'days_ahead()' in
	    the file date.c
	**/

	int days;

	PutLine0(LINES-1,0, "How many days in the future should it expire? ");
	CleartoEOLN();
	Raw(OFF);
	gets(buffer);
	Raw(ON);
	sscanf(buffer, "%d", &days);
	if (days < 1)
	  error("That doesn't make sense!");
	else if (days > 14)
	  error("Expiration date must be within two weeks of today");
	else {
	  days_ahead(days, buffer);
	  PutLine0(x, y, buffer);
	}
}

check_user_header(header)
char *header;
{
	/** check the header format...if bad print error and erase! **/

	register int i = -1;

	if (strlen(header) == 0)
	   return;

	if (whitespace(header[0])) {
	  error ("you can't have leading white space in a header!");
	  header[0] = '\0';
	  ClearLine(14);
	  return;
	}

	if (header[0] == ':') {
	  error ("you can't have a colon as the first character!");
	  header[0] = '\0';
	  ClearLine(14);
	  return;
	}

	while (header[++i] != ':') {
	  if (header[i] == '\0') {
	    error("you need a colon ending the field!");
	    header[0] = '\0';
	    ClearLine(14);
	    return;
	  }
	  else if (whitespace(header[i])) {
	    error("You can't have white space imbedded in the header name!");
	    header[0] = '\0';
	    ClearLine(14);
	    return;
	  }
	}
	
	return;
}
