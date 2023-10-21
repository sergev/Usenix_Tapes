/**			calendar.c		**/

/** This routine implements a rather snazzy idea suggested by Warren
    Carithers of the Rochester Institute of Technology that allows
    mail to contain entries formatted in a manner that will allow direct
    copying into a users calendar program.

    Never able to leave good-enough alone, I've extended this idea to a
    further one - the knowledge of a different type of calendar program
    too.  Specifically, the current message can contain either of;

	-> Mon 04/21 1:00p meet with chairman candidate

    or 

	- >April 21
	-   
	-     1:00 pm: Meet with Chairman Candidate
	-

    The first type will have the leading '->' removed and all subsequent
    white space, creating a simple one-line entry in the users calendar
    file.   The second type will remove the '-' and the leading white
    spaces and leave everything else intact (that is, the file in the
    second example would be appended with ">April 21" followed by a
    blank line, the 1:00 pm meeting info, and another blank line.
	
    The file to include this in is either the default as defined in the
    sysdefs.h file (see CALENDAR_FILE) or a filename contained in the
    users ".elmrc" file, "calendar= <filename>".

    (C) Copyright 1986 Dave Taylor
**/

#include "headers.h"

#ifdef ENABLE_CALENDAR		/* if not defined, this will be an empty file */

#include <errno.h>

extern int errno;

char *error_name(), *error_description(), *strcpy();

scan_calendar()
{
	FILE *calendar;
	int  count;

	/* First step is to open the celendar file for appending... **/

	if (can_open(calendar_file, "a") != 0) {
	  dprint1(2, "Error: wrong permissions to append to calendar %s\n",
		  calendar_file);
	  dprint2(2, "** %s - %s **\n",
		  error_name(errno), error_description(errno));
	  error1("Not able to append to file %s!", calendar_file);
	  return; 
	}

	if ((calendar = fopen(calendar_file,"a")) == NULL) {
	  dprint1(2, "Error: couldn't append to calendar file %s (save)\n", 
		   calendar_file);
	  dprint2(2, "** %s - %s **\n",
		  error_name(errno), error_description(errno));
	  error1("Couldn't append to file %s!", calendar_file);
	  return; 
	}
	
	count = extract_info(calendar);

	fclose(calendar);

	chown(calendar_file, userid, groupid);	/* ensure owned by user */

	if (count > 0)
	  error2("%d entr%s saved in calendar file", 
		 count, count > 1 ? "ies" : "y");
	else 
	  error("No calendar entries found in that message");

	return;
}

int
extract_info(save_to_fd)
FILE *save_to_fd;
{
	/** Save the relevant parts of the current message to the given
	    calendar file.  The only parameter is an opened file
	    descriptor, positioned at the end of the existing file **/
	    
	register int entries = 0, ok = 1, lines, index, in_entry = FALSE;
	char buffer[SLEN];

    	/** get to the first line of the message desired **/

    	if (fseek(mailfile, header_table[current-1].offset, 0) == -1) {
       	  dprint2(1,"ERROR: Attempt to seek %d bytes into file failed (%s)",
		header_table[current-1].offset, "extract_info");
       	  error1("ELM [seek] failed trying to read %d bytes into file",
	     	header_table[current-1].offset);
       	  return(0);
    	}

        /* how many lines in message? */

        lines = header_table[current-1].lines;

        /* now while not EOF & still in message... scan it! */

        while (ok && lines--) {
          ok = (int) (fgets(buffer, LONG_SLEN, mailfile) != NULL);
	  
	  /* now let's see if it matches the basic pattern... */

	  if ((index = calendar_line(buffer)) > -1) {

	    if (buffer[index] == '>') {	/* single line entry */
	      if (remove_through_ch(buffer, '>')) {
	        fprintf(save_to_fd,"%s", buffer);
	        entries++;
	      }
	    }
	    else {				/* multi-line entry  */
	        fprintf(save_to_fd, "%s", (char *) (buffer + index + 1));
	        in_entry = TRUE;	
	      }
           }
	   else if (in_entry) {
	     in_entry = FALSE;
	     entries++;
	   }
	}

	dprint2(4,"Got %d calender entr%s.\n", entries, entries > 1? "ies":"y");

	return(entries);
}

int
calendar_line(string)
char *string;
{
	/** Iff the input line is of the form;

	      {white space} <one or more '-'> 
	
	    this routine will return the index of the NEXT character
	    after the dashed sequence...If this pattern doesn't occur, 
	    or if any other problems are encountered, it'll return "-1"
	**/

	register int loc = 0;

	if (chloc(string,'-') == -1) 	  /* no dash??? */
	  return(-1);			/* that was easy! */

	/** skip leading white space... **/

	while (whitespace(string[loc])) loc++;	   /* MUST have '-' too! */

	if (string[loc] != '-')	return(-1);	   /* nice try, sleazo! */

	while (string[loc] == '-') loc++;

	if (loc >= strlen(string)) return(-1);	/* Empty line... */

	/* otherwise.... */  

	return(loc);
}
    

int
remove_through_ch(string, ch)
char *string;
char  ch;
{
	/** removes all characters from zero to ch in the string, and 
	    any 'white-space' following the 'n'th char... if it hits a
    	    NULL string, it returns FALSE, otherwise it'll return TRUE!
	**/

	char buffer[SLEN];
	register int index = 0, i = 0;

	while (string[index] != ch && string[index] != '\0')
	  index++;
	
	if (index >= strlen(string)) 
	  return(FALSE);	/* crash! burn! */

	index++;	/* get past the 'ch' character... */

	while (whitespace(string[index])) index++;

	while (index < strlen(string))
	  buffer[i++] = string[index++];

	buffer[i] = '\0';

	strcpy(string, buffer);
	
	return(TRUE);
}

#endif
