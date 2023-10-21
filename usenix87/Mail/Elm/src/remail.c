/**			remail.c			**/

/** For those cases when you want to have a message continue along
    to another person in such a way as they end up receiving it with
    the return address the person YOU received the mail from (does
    this comment make any sense yet??)...

    (C) Copyright 1986  Dave Taylor
**/

#include "headers.h"
#include <errno.h>

extern int errno;

char *error_name(), *error_description();

int
remail()
{
	/** remail a message... returns TRUE if new foot needed ... **/
	
	FILE *mailfd;
	char entered[VERY_LONG_STRING], expanded[VERY_LONG_STRING];
	char filename[SLEN], buffer[VERY_LONG_STRING];

	entered[0] = '\0';

	get_to(entered, expanded);
	if (strlen(entered) == 0)
	  return(0);

	display_to(expanded);

	/** now the munge... **/

	sprintf(filename, "%s%d", temp_file, getpid());

	if ((mailfd = fopen(filename, "w")) == NULL) {
	  dprint1(1,"couldn't open temp file %s! (remail)\n", filename);
	  dprint2(1,"** %s - %s **\n", error_name(errno),
		  error_description(errno));
	  sprintf(buffer, "Sorry - couldn't open file %s for writing (%s)",
		  error_name(errno));
	  set_error(buffer);
	  return(1);
	}

	/** now let's copy the message into the newly opened
	    buffer... **/

	copy_message("", mailfd, FALSE, TRUE);  

	fclose(mailfd);

	/** Got the messsage, now let's ensure the person really wants to 
	    remail it... **/

	ClearLine(LINES-1);
	ClearLine(LINES);
	PutLine1(LINES-1,0,
	    "Are you sure you want to remail this message (y/n) ? y%c",
	    BACKSPACE);
	fflush(stdin);
	fflush(stdout);
	if (tolower(ReadCh()) == 'n') { /* another day, another No... */
	  Write_to_screen("No", 0);
	  set_error("Bounce of message cancelled");
          return(1);
	}
	Write_to_screen("Yes!", 0);

	sprintf(buffer, "%s %s < %s", mailer, expanded, filename);

	PutLine0(LINES,0,"resending mail...");

	if ((errno = system_call(buffer, SH)) != 0) {
	  sprintf(buffer, "Remail failed with error %d!", errno);
	  set_error(buffer);
	}
	else
	  set_error("mail resent");

	return(1);
}
