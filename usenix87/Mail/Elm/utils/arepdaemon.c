/**			arepdaemon.c			**/

/** (C) Copyright 1986 Dave Taylor			**/

/** Keep track of mail as it arrives, and respond by sending a 'recording'
    file to the sender as new mail is received.

    Note: the user program that interacts with this program is the
    'autoreply' program and that should be consulted for further
    usage information.

    This program is part of the 'autoreply' system, and is designed
    to run every hour and check all mailboxes listed in the file 
    "/etc/autoreply.data", where the data is in the form:

	username	replyfile	current-mailfile-size

    To avoid a flood of autoreplies, this program will NOT reply to mail 
    that contains header "X-Mailer: fastmail".  Further, each time the 
    program responds to mail, the 'mailfile size' entry is updated in
    the file /etc/autoreply.data to allow the system to be brought 
    down and rebooted without any loss of data or duplicate messages.

    This daemon also uses a lock semaphore file, /usr/spool/uucp/LCK..arep,
    to ensure that more than one copy of itself is never running.  For this
    reason, it is recommended that this daemon be started up each morning
    from cron, since it will either start since it's needed or simply see
    that the file is there and disappear.

    Since this particular program is the main daemon answering any
    number of different users, it must be run with uid root.

    (C) 1985, Dave Taylor, HP Colorado Networks Operation
**/

#include <stdio.h>

#ifdef BSD
# include <sys/time.h>
#else
# include <time.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

#include "defs.h"

static char ident[] = { WHAT_STRING };

#define arep_lock_file	"/usr/spool/uucp/LCK..arep"

#define autoreply_file	"/etc/autoreply.data"
#define fastmail	"/usr/local/bin/fastmail"

#define logfile		"/etc/autoreply.log"	/* first choice   */
#define logfile2	"/tmp/autoreply.log"	/* second choice  */

#define BEGINNING	0		/* see fseek(3S) for info */
#define SLEEP_TIME	3600		/* run once an hour       */
#define MAX_PEOPLE	20		/* max number in program  */

#define EXISTS		00		/* lock file exists??     */
#define MODE		0777		/* lockfile creation mode */

#define remove_return(s)	if (strlen(s) > 0) { \
			          if (s[strlen(s)-1] == '\n') \
				    s[strlen(s)-1] = '\0'; \
		                }

struct replyrec {
	char 	username[NLEN];		/* login name of user */
	char	mailfile[SLEN];		/* name of mail file  */
	char    replyfile[SLEN];	/* name of reply file */
	long    mailsize;		/* mail file size     */
	int     in_list;		/* for new replies    */
      } reply_table[MAX_PEOPLE];

FILE  *logfd;				/* logfile (log action)   */
long  autoreply_size = 0L;		/* size of autoreply file */
int   active = 0;			/* # of people 'enrolled' */

FILE  *open_logfile();			/* forward declaration    */

long  bytes();				/*       ditto 		  */

main()
{
	long size;
	int  person, data_changed;

	if (! lock())
	  exit(0);	/* already running! */

	while (1) {

	  logfd = open_logfile();	/* open the log */

	  /* 1. check to see if autoreply table has changed.. */

	  if ((size = bytes(autoreply_file)) != autoreply_size) {
	    read_autoreply_file(); 
	    autoreply_size = size;
	  }

	  /* 2. now for each active person... */
	
	  data_changed = 0;

	  for (person = 0; person < active; person++) {
	    if ((size = bytes(reply_table[person].mailfile)) != 
		reply_table[person].mailsize) {
	      if (size > reply_table[person].mailsize)
	        read_newmail(person);
	      /* else mail removed - resync */
	      reply_table[person].mailsize = size;
	      data_changed++;
	    }
	  }

	  /* 3. if data changed, update autoreply file */

	  if (data_changed)
	    update_autoreply_file();

	  close_logfile();		/* close the logfile again */

	  /* 4. Go to sleep...  */

	  sleep(SLEEP_TIME);
	}
}

int
read_autoreply_file()
{
	/** We're here because the autoreply file has changed size!!  It
	    could either be because someone has been added or because
	    someone has been removed...since the list will always be in
	    order (nice, eh?) we should have a pretty easy time of it...
	**/

	FILE *file;
	char username[SLEN], 	replyfile[SLEN];
	int  person;
 	long size;
	
	log("Autoreply data file has changed!  Reading...");

	if ((file = fopen(autoreply_file,"r")) == NULL) {
	  log("No-one is using autoreply...");
	  return(0);
	}
	
	for (person = 0; person < active; person++)
	  reply_table[person].in_list = 0;
	
	while (fscanf(file, "%s %s %dl", username, replyfile, &size) != EOF) {
	  /* check to see if this person is already in the list */
	  if ((person = in_list(username)) != -1) {
	    reply_table[person].in_list = 1;
	    reply_table[person].mailsize = size;	 /* sync */
	  }
	  else { 	/* if not, add them */
	    if (active == MAX_PEOPLE) {
	      unlock();
	      exit(log("Couldn't add %s - already at max people!", 
		         username));
	    }
	    log("adding %s to the active list", username);
	    strcpy(reply_table[active].username, username);
	    sprintf(reply_table[active].mailfile, "/usr/mail/%s", username);
	    strcpy(reply_table[active].replyfile, replyfile);
	    reply_table[active].mailsize = size;
	    reply_table[active].in_list = 1;	/* obviously! */
	    active++;
	  }
	}

	/** now check to see if anyone has been removed... **/

	for (person = 0; person < active; person++)
	  if (reply_table[person].in_list == 0) {
	     log("removing %s from the active list", 
		  reply_table[person].username);
	    strcpy(reply_table[person].username, 
		   reply_table[active-1].username);
	    strcpy(reply_table[person].mailfile, 
		   reply_table[active-1].mailfile);
	    strcpy(reply_table[person].replyfile, 
		   reply_table[active-1].replyfile);
	    reply_table[person].mailsize = reply_table[active-1].mailsize;
	    active--;
	  }
}

update_autoreply_file()
{
	/** update the entries in the autoreply file... **/

	FILE *file;
	register int person;

	if ((file = fopen(autoreply_file,"w")) == NULL) {
          log("Couldn't update autoreply file!");
	  return;
	}

	for (person = 0; person < active; person++)
	  fprintf(file, "%s %s %ld\n",
		  reply_table[person].username,
		  reply_table[person].replyfile,
		  reply_table[person].mailsize);

	fclose(file);

	printf("updated autoreply file\n");
	autoreply_size = bytes(autoreply_file);
}

int
in_list(name)
char *name;
{
	/** search the current active reply list for the specified username.
	    return the index if found, or '-1' if not. **/

	register int index;

	for (index = 0; index < active; index++)
	  if (strcmp(name, reply_table[index].username) == 0)
	    return(index);
	
	return(-1);
}

read_newmail(person)
int person;
{
	/** Read the new mail for the specified person. **/

	
	FILE *mailfile;
	char from_whom[LONG_SLEN], subject[SLEN];
	int  sendit;

	log("New mail for %s", reply_table[person].username);

        if ((mailfile = fopen(reply_table[person].mailfile,"r")) == NULL)
           return(log("can't open mailfile for user %s", 
		    reply_table[person].username));

        if (fseek(mailfile, reply_table[person].mailsize, BEGINNING) == -1)
           return(log("couldn't seek to %ld in mail file!", 
	               reply_table[person].mailsize));

	while (get_return(mailfile, person, from_whom, subject, &sendit) != -1)
	  if (sendit)
	    reply_to_mail(person, from_whom, subject);

	return;
}

int
get_return(file, person, from, subject, sendit)
FILE *file;
int  person, *sendit;
char *from, *subject;
{
	/** Reads the new message and return the from and subject lines.
	    sendit is set to true iff it isn't a machine generated msg
	**/
	
    char name1[SLEN], name2[SLEN], lastname[SLEN];
    char buffer[LONG_SLEN], hold_return[NLEN];
    int done = 0, in_header = 0;

    from[0] = '\0';
    *sendit = 1;

    while (! done) {

      if (fgets(buffer, LONG_SLEN, file) == NULL)
	return(-1);

      if (first_word(buffer, "From ")) {
	in_header++;
	sscanf(buffer, "%*s %s", hold_return);
      }
      else if (in_header) {
        if (first_word(buffer, ">From")) {
	  sscanf(buffer,"%*s %s %*s %*s %*s %*s %*s %*s %*s %s", name1, name2);
	  add_site(from, name2, lastname);
        }
        else if (first_word(buffer,"Subject:")) {
	  remove_return(buffer);
	  strcpy(subject, (char *) (buffer + 8));
        }
        else if (first_word(buffer,"X-Mailer: fastmail"))
	  *sendit = 0;
        else if (strlen(buffer) == 1)
	  done = 1;
      }
    }

    if (from[0] == '\0')
      strcpy(from, hold_return); /* default address! */
    else
      add_site(from, name1, lastname);	/* get the user name too! */

    return(0);
}

add_site(buffer, site, lastsite)
char *buffer, *site, *lastsite;
{
	/** add site to buffer, unless site is 'uucp', or the same as 
	    lastsite.   If not, set lastsite to site.
	**/

	char local_buffer[LONG_SLEN], *strip_parens();

	if (strcmp(site, "uucp") != 0)
	  if (strcmp(site, lastsite) != 0) {
	      if (buffer[0] == '\0')
	        strcpy(buffer, strip_parens(site));         /* first in list! */
	      else {
	        sprintf(local_buffer,"%s!%s", buffer, strip_parens(site));
	        strcpy(buffer, local_buffer);
	      }
	      strcpy(lastsite, strip_parens(site)); /* don't want THIS twice! */
	   }
}

remove_first_word(string)
char *string;
{	/** removes first word of string, ie up to first non-white space
	    following a white space! **/

	register int loc;

	for (loc = 0; string[loc] != ' ' && string[loc] != '\0'; loc++) 
	    ;

	while (string[loc] == ' ' || string[loc] == '\t')
	  loc++;
	
	move_left(string, loc);
}

move_left(string, chars)
char string[];
int  chars;
{
	/** moves string chars characters to the left DESTRUCTIVELY **/

	register int i;

	chars--; /* index starting at zero! */

	for (i=chars; string[i] != '\0' && string[i] != '\n'; i++)
	  string[i-chars] = string[i];

	string[i-chars] = '\0';
}

reply_to_mail(person, from, subject)
int   person;
char *from, *subject;
{
	/** Respond to the message from the specified person with the
	    specified subject... **/
	
	char buffer[SLEN];

	if (strlen(subject) == 0)
	  strcpy(subject, "Auto-reply Mail");
	else if (! first_word(subject,"Auto-reply")) {
	  sprintf(buffer, "Auto-reply to:%s", subject);
	  strcpy(subject, buffer);
	}

	log("auto-replying to '%s'", from);

	mail(from, subject, reply_table[person].replyfile, person);
}	

reverse(string)
char *string;
{
	/** reverse string... pretty trivial routine, actually! **/

	char buffer[SLEN];
	register int i, j = 0;

	for (i = strlen(string)-1; i >= 0; i--)
	  buffer[j++] = string[i];

	buffer[j] = '\0';

	strcpy(string, buffer);
}

long
bytes(name)
char *name;
{
	/** return the number of bytes in the specified file.  This
	    is to check to see if new mail has arrived....  **/

	int ok = 1;
	extern int errno;	/* system error number! */
	struct stat buffer;

	if (stat(name, &buffer) != 0)
	  if (errno != 2) {
	   unlock();
	   exit(fprintf(stderr,"Error %d attempting fstat on %s", errno, name));
	  }
	  else
	    ok = 0;
	
	return(ok ? buffer.st_size : 0);
}

mail(to, subject, filename, person)
char *to, *subject, *filename;
int   person;
{
	/** Mail 'file' to the user from person... **/
	
	char buffer[VERY_LONG_STRING];

	sprintf(buffer, "%s -f '%s [autoreply]' -s '%s' %s %s",
		fastmail, reply_table[person].username,
	        subject, filename, to);
	
	system(buffer);
}

log(message, arg)
char *message;
char *arg;
{
	/** Put log entry into log file.  Use the format:
	      date-time: <message>
	**/

	struct tm *localtime(), *thetime;
	long      time(), clock;
	char      buffer[SLEN];

	/** first off, get the time and date **/

	clock = time((long *) 0);       /* seconds since ???   */
	thetime = localtime(&clock);	/* and NOW the time... */

	/** then put the message out! **/

	sprintf(buffer, message, arg);

	fprintf(logfd,"%d/%d-%d:%02d: %s\n", 
		thetime->tm_mon+1, thetime->tm_mday,
	        thetime->tm_hour,  thetime->tm_min,
	        buffer);
}

FILE *open_logfile()
{
	/** open the logfile.  returns a valid file descriptor **/

	FILE *fd;

	if ((fd = fopen(logfile, "a")) == 0)
	  if ((fd = fopen(logfile2, "a")) == 0) {
	    unlock();
	    exit(1);	/* give up! */
	  }

	return( (FILE *) fd);
}

close_logfile()
{
	/** Close the logfile until needed again. **/

	fclose(logfd);
}

char *strip_parens(string)
char *string;
{
	/** Return string with all parenthesized information removed.
	    This is a non-destructive algorithm... **/

	static char  buffer[LONG_SLEN];
	register int i, depth = 0, buffer_index = 0;

	for (i=0; i < strlen(string); i++) {
	  if (string[i] == '(')
	    depth++;
	  else if (string[i] == ')') 
	    depth--;
	  else if (depth == 0)
	    buffer[buffer_index++] = string[i];
	}
	
	buffer[buffer_index] = '\0';

	return( (char *) buffer);
}

/*** LOCK and UNLOCK - ensure only one copy of this daemon running at any
     given time by using a file existance semaphore (wonderful stuff!) ***/

lock()
{
	/** Try to create the lock file.  If it's there, or we can't
	    create it for some stupid reason, return zero, otherwise,
	    a non-zero return code indicates success in locking this
	    process in. **/

	if (access(arep_lock_file, EXISTS) == 0)
	  return(0); 	/* file already exists!! */

	if (creat(arep_lock_file, MODE) == -1)
	  return(0);	/* can't create file!!   */

	return(1);
}

unlock()
{
	/** remove lock file if it's there! **/

	(void) unlink(arep_lock_file);
}
