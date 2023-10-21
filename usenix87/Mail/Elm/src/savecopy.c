/** 			savecopy.c			**/

/** Save a copy of the specified message in the users savemail mailbox.

         (C) Copyright 1986, Dave Taylor 			       
**/

#include "headers.h"
#ifdef BSD
# ifdef BSD4.1
#  include <time.h>
# else
#  include <sys/time.h>
# endif
#else
#  include <time.h>
#endif

#include <errno.h>

char *format_long(), *get_arpa_date();
char *error_name(), *error_description();
char *ctime();

extern char in_reply_to[SLEN];	/* In-Reply-To: string */
extern int gotten_key;		/* for encryption      */
extern int errno;

char *strcat(), *strcpy();
unsigned long sleep();
long  time();

save_copy(subject, to, cc, filename, original_to)
char *subject, *to, *cc, *filename, *original_to;
{
	/** This routine appends a copy of the outgoing message to the
	    file specified by the SAVEFILE environment variable.  **/

	FILE *save,		/* file id for file to save to */
	     *message;		/* the actual message body     */
	long  thetime,		/* variable holder for time    */
	      time();
	char  buffer[SLEN],	/* read buffer 		       */
	      savename[SLEN],	/* name of file saving into    */
	      newbuffer[SLEN];  /* first name in 'to' line     */
	register int i;		/* for chopping 'to' line up   */
	int   crypted=0;	/* are we encrypting?          */

	savename[0] = '\0';

	if (save_by_name) {
	  get_return_name(to, buffer, FALSE);
	  if (strlen(buffer) == 0) {
	    dprint1(3,"Warning: get_return_name couldn't break down %s\n", to);
	    savename[0] = '\0';
	  }
	  else {
	    sprintf(savename, "%s%s%s", folders, 
	            lastch(folders) == '/'? "" : "/", buffer);

	    if (can_access(savename, READ_ACCESS) != 0)
	      savename[0] = '\0';
	  }
	}

	if (strlen(savename) == 0) {
	  if (strlen(savefile) == 0)
	    return(error("variable 'SAVEFILE' not defined!"));
	  strcpy(savename, savefile);
	}

	if ((errno = can_access(savename, WRITE_ACCESS))) {
	  dprint0(2,"Error: attempt to autosave to a file that can't...\n");
	  dprint1(2,"\tbe appended to: %s (save_copy)\n", savename);
	  dprint2(2,"** %s - %s **\n", error_name(errno),
		  error_description(errno));
	  error1("permission to append to %s denied!", savename);
	  sleep(2);
	  return(FALSE);
	}

	if ((save = fopen(savename, "a")) == NULL) {
	  dprint2(1,"Error: Couldn't append message to file %s (%s)\n",
		  savename, "save_copy");
	  dprint2(1,"** %s - %s **\n", error_name(errno),
		  error_description(errno));
	  error1("couldn't append to %s", savename);
	  sleep(2);
	  return(FALSE);
	}

	if ((message = fopen(filename, "r")) == NULL) {
	  fclose(save);
	  dprint1(1,"Error: Couldn't read file %s (save_copy)\n", filename);
	  dprint2(1,"** %s - %s **\n", error_name(errno),
		  error_description(errno));
	  error1("couldn't read file %s!", filename);
	  sleep(2);
	  return(FALSE);
	}

	for (i=0; original_to[i] != '\0' && ! whitespace(original_to[i]); i++)
	  newbuffer[i] = original_to[i];

	newbuffer[i] = '\0';

	tail_of(newbuffer, buffer, FALSE);

	thetime = time((long *) 0);      /* dumb dumb dumb routine */

	fprintf(save,"\nFrom To:%s %s", buffer, ctime(&thetime));

	fprintf(save, "Date: %s\n", get_arpa_date());
			
	fprintf(save,"To: %s\nSubject: %s\n", 
		format_long(to,strlen("To: ")), subject);

	if (strlen(cc) > 0)
	  fprintf(save,"Cc: %s\n", 
		  format_long(cc, strlen("Cc:")));

	if (strlen(in_reply_to) > 0)
	  fprintf(save, "In-Reply-To: %s\n", in_reply_to);

	(void) putc('\n', save);	/* put another return, please! */

	/** now copy over the message... **/

	while (fgets(buffer, SLEN, message) != NULL) {
	  if (buffer[0] == '[') {
	    if (strncmp(buffer, START_ENCODE, strlen(START_ENCODE))==0)
	      crypted = 1;
	    else if (strncmp(buffer, END_ENCODE, strlen(END_ENCODE))==0)
	      crypted = 0;
	    else if (strncmp(buffer, DONT_SAVE, strlen(DONT_SAVE)) == 0) {
	      fclose(message);
	      fclose(save);
	      chown(savename, userid, groupid);	
	      return(TRUE);
	    }
	  }
	  else if (crypted) {
	    if (! gotten_key++)
	      getkey(ON);
	    encode(buffer);
	  }
	  fputs(buffer, save);
	}

	fclose(message);
	fclose(save);

	/* make sure save file isn't owned by root! */
	chown(savename, userid, groupid);	

	return(TRUE);
}
