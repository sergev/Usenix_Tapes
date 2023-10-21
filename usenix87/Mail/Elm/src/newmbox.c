/**			newmbox.c			**/

/**  read new mailbox file, (C) Copyright 1986 by Dave Taylor  **/

#include <ctype.h>

#ifdef BSD
#undef tolower		/* we have our own "tolower" routine instead! */
#endif

#include "headers.h"

#include <sys/types.h>		
#include <sys/stat.h>
#include <errno.h>

#ifdef BSD			/* Berkeley has library elsewhere... */
#  ifndef BSD4.1
#    include <sys/time.h>
#  else
#    include <time.h>
#  endif
#else
#  include <time.h>
#endif

extern int errno;

char *error_name(), *error_description(), *strcpy(), *strncpy();
unsigned long sleep();
void rewind();
void  exit();

struct header_rec *realloc();

int
newmbox(stat, resync, main_screen)
int stat, resync, main_screen;
{
	/** Read a new mailbox file or resync on current file.

	    Values of stat and what they mean;

		stat = 0	- changing mailboxes from within program
		stat = 1	- read default mailbox or infile for the 
			          first time
	        stat = 2	- read existing mailbox, new mail arrived

	    resync is TRUE iff we know the current mailbox has changed.  If
	    it's set to true this means that we MUST READ SOMETHING, even 
	    if it's the current mailbox again!!

	    main_screen simply tells where the counting line should be.

	**/

	int  switching_to_default = 0, switching_from_default = 0;
	int  iterations = 0, redraw = 0; /* for dealing with the '?' answer */
	char buff[SLEN];

	if (mbox_specified == 0 && stat == 0)
	  switching_from_default++;

	if (stat > 0) {
	  if (stat == 1 && strlen(infile) == 0) {

	    /* Subtlety - check to see if there's another instantiation
	       of Elm (e.g. if the /tmp file is in use).  If so, DIE! */

	    sprintf(infile, "%s%s", temp_mbox, username);
	    if (access(infile, ACCESS_EXISTS) != -1) {
	      error(
	    "Hey!  An instantiation of Elm is already reading this mail!\n\r");
	      fprintf(stderr,
"\n\r     [if this is in error then you'll need to remove '/tmp/mbox.%s']\n\r", 
		  username);
              exit(1);
            }
            sprintf(infile, "%s%s", mailhome, username);
	  }
	  if (strlen(infile) == 0)	/* no filename yet?? */
	    sprintf(infile,"%s%s",mailhome, username);
	  if ((errno = can_access(infile, READ_ACCESS))) {
	    if (strncmp(infile, mailhome, strlen(mailhome)) == 0) {
	      /* oh wow.  incoming mailbox with no messages... */
	      return(1);
	    }
	    error2("Can't open mailbox '%s' for reading [%s]", infile,
		    error_name(errno));
	    exit(1);
	  }
	}
	else { 		 	/* get name of new mailbox! */
	  MoveCursor(LINES-3, 30);
	  CleartoEOS();
	  PutLine0(LINES-3, 40, "(Use '?' to list your folders)");
	  show_last_error();
ask_again:
	  buff[0] = '\0';
	  if (iterations++ == 0) {
	    PutLine0(LINES-2,0,"Name of new mailbox: ");
	    (void) optionally_enter(buff, LINES-2, 21, FALSE);
	    ClearLine(LINES-2);
	  }
	  else {
	    printf("\n\rName of new mailbox: ");
	    (void) optionally_enter(buff, -1, -1, FALSE);
	  }
	  if (strlen(buff) == 0) {
	    if (resync && file_changed)
	      strcpy(buff, infile);
	    else
	      return(redraw);
	  }
	  if (strcmp(buff, "?") == 0) {
	    redraw = 1;		/* we'll need to clean the screen */
	    dprint0(1,"***  setting redraw to 1 ***\n");
	    list_folders();
	    goto ask_again;
	  }
	  if (strcmp(buff, "!") == 0 ||
		   strcmp(buff, "%") == 0) 	/* go to mailbox */
	    sprintf(buff,"%s%s", mailhome, username);
	  else if (! expand_filename(buff)) {
	    error1("can't expand file %s", buff);
	    if (resync && file_changed)
	      strcpy(buff, infile);
	    else
	      return(FALSE);	
	  }

	  if (strcmp(buff, infile) == 0 && ! resync) { 
	    dprint0(3,"User requested change to current mailbox! (newmbox)\n");
	    error("already reading that mailbox!");
	    return(FALSE);
	  }

	  if ((errno = can_access(buff, READ_ACCESS))) {
	    dprint2(2,"Error: attempt to open %s as mailbox denied (%s)!\n",
		     buff, "newmbox");
	    error1("Permission to open file %s denied", buff);
	    if (resync && file_changed)
	      strcpy(buff, infile);
	    else
	      return(FALSE);	
	  }

	  if (first_word(buff, mailhome)) {	/* a mail file! */
	    mbox_specified = 0; 	  /* fake program to think that */
	    stat = 1;		    	  /*     we're the default file */
	    switching_to_default++;	  /*        remember this act!  */
	  }

	  if (resync && file_changed && strcmp(buff, infile) == 0)
	    PutLine0(LINES-3,COLUMNS-40,"Resynchronizing file");
	  else
	    PutLine1(LINES-3,COLUMNS-40,"Mailbox: %s", buff);
	  CleartoEOLN();
	  strcpy(infile,buff);
	  if (! switching_to_default) mbox_specified = 1;
	
	}

	if (switching_from_default) {	/* we need to remove the tmp file */
	    sprintf(buff, "%s%s", temp_mbox, username);
	    if (unlink(buff) != 0) {
	      error1(
	    "Sorry, but I can't seem to unlink your temp mail file [%s]\n\r",
		error_name(errno));
              silently_exit();
	    }
	}

	clear_error();
	clear_central_message();

	header_page = 0;

	if (mailfile != NULL)
	  (void) fclose(mailfile);  /* close it first, to avoid too many open */

	if ((mailfile = fopen(infile,"r")) == NULL) 
	  message_count = 0;
	else if (stat < 2) {          /* new mail file! */
	  current = 1;
	  message_count = read_headers(FALSE, main_screen);
	  if (! message_count) current = 0;
	}
	else 	/* resync with current mail file */
	  message_count = read_headers(TRUE, main_screen);

	if (stat < 2)
	  selected = 0;	/* we don't preselect new mailboxes, boss! */

	return(TRUE);
}

int
read_headers(rereading, main_screen)
int rereading, main_screen;
{
	/** Reads the headers into the header_table structure and leaves
	    the file rewound for further I/O requests.   If the file being
	    read is the default mailbox (ie incoming) then it is copied to
	    a temp file and closed, to allow more mail to arrive during 
	    the elm session.  If 'rereading' is set, the program will copy
	    the status flags from the previous data structure to the new 
	    one if possible.  This is (obviously) for re-reading a mailfile!
	**/

	FILE *temp;
	struct header_rec *temp_struct;
	char buffer[LONG_STRING], temp_filename[SLEN];
	long bytes = 0L, line_bytes = 0L;
	register int line = 0, count = 0, subj = 0, copyit = 0, in_header = 1;
	int count_x, count_y = 17, new_messages = 0, err;
	int in_to_list = FALSE, forwarding_mail = FALSE;

	static int first_read = 0;

	if (! first_read++) {
	  ClearLine(LINES-1);
	  ClearLine(LINES);
	  if (rereading)
	    PutLine2(LINES, 0, "Reading in %s, message: %d", infile, 
		     message_count);
	  else
	    PutLine1(LINES, 0, "Reading in %s, message: 0", infile);
	  count_x = LINES;
          count_y = 22 + strlen(infile);
	}
	else {
	  count_x = LINES-2;
	  if (main_screen)
	    PutLine0(LINES-2, 0, "Reading message: 0");
	  else {
	    PutLine0(LINES, 0, "\n");
	    PutLine0(LINES, 0, "Reading message: 0");
	    count_x = LINES;
	  }
	}

	if (mbox_specified == 0) {
	  lock(INCOMING);	/* ensure no mail arrives while we do this! */
	  sprintf(temp_filename,"%s%s",temp_mbox, username);
	  if (! rereading) {
	    if (access(temp_filename, ACCESS_EXISTS) != -1) {
	      /* Hey!  What the hell is this?  The temp file already exists? */
	      /* Looks like a potential clash of processes on the same file! */
	      unlock();				     /* so remove lock file! */
	      error("What's this?  The temp mailbox already exists??");
	      sleep(2);
	      error("Ahhhh.....I give up");
	      silently_exit();	/* leave without tampering with it! */
	    }
	    if ((temp = fopen(temp_filename,"w")) == NULL) {
	     err = errno;
	     unlock();	/* remove lock file! */
	     Raw(OFF);
	     Write_to_screen(
		     "\nCouldn't open file %s for use as temp mailbox;\n", 1,
	             temp_filename);
	     Write_to_screen("** %s - %s **\n", 2,
		     error_name(err), error_description(err));
	     dprint3(1,
                "Error: Couldn't open file %s as temp mbox.  errno %s (%s)\n",
	         temp_filename, error_name(err), "read_headers");
	     leave();
	    }
	   get_mailtime();
	   copyit++;
	   chown(temp_filename, userid, groupid);
	   chmod(temp_filename, 0700);	/* shut off file for other people! */
	 }
	 else {
	   if ((temp = fopen(temp_filename,"a")) == NULL) {
	     err = errno;
	     unlock();	/* remove lock file! */
	     Raw(OFF);
	     Write_to_screen(
		     "\nCouldn't reopen file %s for use as temp mailbox;\n", 1,
	             temp_filename);
	     Write_to_screen("** %s - %s **\n", 2,
		     error_name(err), error_description(err));
	     dprint3(1,
                "Error: Couldn't reopen file %s as temp mbox.  errno %s (%s)\n",
	         temp_filename, error_name(err), "read_headers");
	     emergency_exit();
	    }
	   copyit++;
	  }
	}

	if (rereading) {
	   if (fseek(mailfile, mailfile_size, 0)) {
	     err = errno;
	     Write_to_screen(
		"\nCouldn't seek to %ld (end of mailbox) in %s!\n", 2,
	     	mailfile_size, infile);	
	     Write_to_screen("** %s - %s **\n", 2,
		     error_name(err), error_description(err));
	     dprint4(1,
     "Error: Couldn't seek to end of mailbox %s: (offset %ld) Errno %s (%s)\n",
	        infile, mailfile_size, error_name(err), "read_headers");
	     emergency_exit();
	   }
	   count = message_count;		/* next available  */
	   bytes = mailfile_size;		/* start correctly */
	   if (message_count > 0)
	     line = header_table[message_count - 1].lines;
	   else
	     line = 0;
	}

	while (fgets(buffer, LONG_STRING, mailfile) != NULL) {

	  if (bytes == 0L) { 	/* first line of file... */	
	    if (! mbox_specified) {
	      if (first_word(buffer, "Forward to ")) {
	        set_central_message("Mail being forwarded to %s", 
                   (char *) (buffer + 11));
	        forwarding_mail = TRUE;
	      }
	    }
	    if (! first_word(buffer, "From ") && !forwarding_mail) {
	        PutLine0(LINES, 0, 
		  "\n\rMail file is corrupt!!  I can't read it!!\n\r\n\r");
		fflush(stderr);
		dprint0(1, "\n\n**** First mail header is corrupt!! ****\n\n");
		dprint1(1, "Line is;\n\t%s\n\n", buffer);
	        mail_only++;	/* to avoid leave() cursor motion */
	        leave();
	    }
	  }

	  if (copyit) fputs(buffer, temp);
	  line_bytes = (long) strlen(buffer); 
	  line++;
	  if (first_word(buffer,"From ")) {
	
	    /** try to allocate new headers, if needed... **/

	    if (count >= max_headers) {
	      max_headers += KLICK;
	      dprint2(3,
		  "\n\nAbout to allocate headers, count = %d, max_headers=%d\n",
		  count, max_headers);
	      if ((temp_struct = realloc(header_table, max_headers * 
		   sizeof(struct header_rec))) == NULL) {
	        error1(
      "\n\r\n\rCouldn't allocate enough memory!  Failed on message #%d\n\r\n\r",
			count);
	        leave();
	       }
	       header_table = temp_struct;
	       dprint1(7,"\tallocated %d more headers!\n\n", KLICK);
	     }
	      
	    if (real_from(buffer, &header_table[count])) {
	      header_table[count].offset = (long) bytes;
	      header_table[count].index_number = count+1;
	      if (! rereading || count > message_count) 
	        header_table[count].status = VISIBLE;     /* default status! */
	      strcpy(header_table[count].subject, "");	/* clear subj    */
	      header_table[count-1].lines = line;
	      if (new_msg(header_table[count])) {
	        header_table[count].status |= NEW;	/* new message!  */

	        if (! new_messages++ && point_to_new && ! rereading &&
	            sortby == RECEIVED_DATE) {
		  current = count+1;
	          get_page(current);	/* make sure we're ON that page! */
	        }

		/* Quick comment on that last conditional test...

		   We want to move the current pointer to the first new
		   message IF this is the first of the new messages, the
		   user requested this feature, we're not rereading the 
		   mailbox (imagine how THAT could screw the user up!),
		   and we're not in some funky sorting mode (received-date is
		   the default).  As always, I'm open to suggestions on
		   other ways to have this work intelligently.
		*/
	
	      }
	      count++;
	      subj = 0;
	      line = 0;
	      in_header = 1;
	      PutLine1(count_x, count_y, "%d", count);
	    }
	  }
	  else if (in_header) {
	    if (first_word(buffer,">From")) 
	      forwarded(buffer, &header_table[count-1]); /* return address */
	    else if (first_word(buffer,"Subject:") ||
		     first_word(buffer,"Subj:") ||
		     first_word(buffer,"Re:")) {
	      if (! subj++) {
	        remove_first_word(buffer);
	        strncpy(header_table[count-1].subject, buffer, STRING);
	      }
	    }
	    else if (first_word(buffer,"From:"))
	      parse_arpa_from(buffer, header_table[count-1].from);
	    
	    /** when it was sent... **/

	    else if (first_word(buffer, "Date:")) 
	      parse_arpa_date(buffer, &header_table[count-1]);

	    /** some status things about the message... **/

	    else if (first_word(buffer, "Priority:"))
	      header_table[count-1].status |= PRIORITY;
	    else if (first_word(buffer, "Content-Type: mailform"))
	      header_table[count-1].status |= FORM_LETTER;
	    else if (first_word(buffer, "Action:"))
	      header_table[count-1].status |= ACTION;

	    /** next let's see if it's to us or not... **/

	    else if (first_word(buffer, "To:")) {
	      in_to_list = TRUE;
	      header_table[count-1].to[0] = '\0';	/* nothing yet */
	      figure_out_addressee((char *) buffer +3, 
				   header_table[count-1].to);
	    }

	    else if (buffer[0] == LINE_FEED || buffer[0] == '\0') {
	      if (in_header) {
	        in_header = 0;	/* in body of message! */
	        fix_date(&header_table[count-1]);
	      }
	    }
	    else if (in_to_list == TRUE) {
	      if (whitespace(buffer[0]))
	        figure_out_addressee(buffer, header_table[count-1].to);
	      else in_to_list = FALSE;
	    }
	  }
	  bytes += (long) line_bytes;
	}

	header_table[count > 0? count-1:count].lines = line + 1;
	
	if (mbox_specified == 0) {
	  unlock();	/* remove lock file! */
	  fclose(mailfile);
	  fclose(temp);
	  if ((mailfile = fopen(temp_filename,"r")) == NULL) {
	    err = errno;
	    MoveCursor(LINES,0);
	    Raw(OFF);
	    Write_to_screen("\nAugh! Couldn't reopen %s as temp mail file;\n",
	           1, temp_filename);
	    Write_to_screen("** %s - %s **\n", 2, error_name(err),
		   error_description(err));
	    dprint3(1,
          "Error: Reopening %s as temp mail file failed!  errno %s (%s)\n",
	           temp_filename, error_name(errno), "read_headers");
	    leave();
	  }
	}
	else 
          rewind(mailfile);

	sort_mailbox(count, 1);	 		/* let's sort this baby now! */

	return(count);
}
