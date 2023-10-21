/**			wnewmail.c			**/

/** Same as newmail.c but for a windowing system...
    
    (C) Copyright 1986, Dave Taylor
**/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "defs.h"

#ifdef AUTO_BACKGROUND
#include <signal.h>	/* background jobs ignore some signals... */
#endif

static char ident[] = { WHAT_STRING };

#define LINEFEED	(char) 10
#define BEGINNING	0			/* seek fseek(3S) */
#define SLEEP_TIME	10		

#define NO_SUBJECT	"(No Subject Specified)"

FILE *mailfile;

long  bytes();
char  *getusername();

main(argc, argv)
int argc;
char *argv[];
{
	char filename[LONG_SLEN];
	long size, newsize;

	if (argc > 2) 
	  fprintf(stderr, "Usage: %s [filename] &\n", argv[0]);
	else if (argc == 2) {
	  strcpy(filename, argv[1]);
	  if (access(filename, ACCESS_EXISTS) == -1) {
	    fprintf(stderr,
	       "%s: Can't open file %s to keep track of incoming mail.\n",
		    argv[0], filename);
	    exit(1);
	  }
	}
	else
	  sprintf(filename,"%s%s",mailhome, getusername());

#ifdef AUTO_BACKGROUND
	if (fork())	    /* automatically puts this task in background! */
	  exit(0);

	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGHUP,  SIG_DFL);	/* so we exit when logged out */
#endif

	size = bytes(filename);

	mailfile = (FILE *) NULL;

	printf("Incoming Mail;\n");

	while (1) {
	
#ifndef AUTO_BACKGROUND		/* won't work if we're nested this deep! */
	  if (getppid() == 1) 	/* we've lost our shell! */
	    exit();
#endif
	if (! isatty(1))     /* we're not talking to the screen any more */
	   exit();
	
	/** Note the lack of error checking on the fopen() (Philip Peake
	    did!) - this is okay since if it fails we don't have any 
	    mail and we can sleep(60) and try again later... 
	**/

	  if (mailfile == (FILE *) NULL) 
	    mailfile = fopen(filename,"r");

	  if ((newsize = bytes(filename)) > size) {	/* new mail */
	    fseek(mailfile, size, BEGINNING); /* skip all current mail */
	    size = newsize;
	    printf("%c", 007);	/* beep for new mail! */
	    read_headers();
	  }
	  else if (newsize != size) {
	    size = newsize; 		/* mail's been removed... */
	    (void) fclose(mailfile);	/* close it and ...       */
	    mailfile = (FILE *) NULL;	/* let's reopen the file  */
	  }

	  sleep(SLEEP_TIME);
	}
}

int
read_headers()
{
	/** read the headers, output as found **/

	char buffer[LONG_SLEN], from_whom[SLEN], subject[SLEN];
	register int subj = 0, in_header = 1, count = 0, priority = 0;

	while (fgets(buffer, LONG_SLEN, mailfile) != NULL) {
	  if (first_word(buffer,"From ")) {
	    if (real_from(buffer, from_whom)) {
	      subj = 0;
	      priority = 0;
	      in_header = 1;
	    }
	  }
	  else if (in_header) {
	    if (first_word(buffer,">From")) 
	      forwarded(buffer, from_whom); /* return address */
	    else if (first_word(buffer,"Subject:") ||
		     first_word(buffer,"Re:")) {
	      if (! subj++) {
	        remove_first_word(buffer);
		strcpy(subject, buffer);
	      }
	    }
	    else if (first_word(buffer,"From:")) 
	      parse_arpa_from(buffer, from_whom);
	    else if (first_word(buffer, "Priority:"))
	      priority++;
	    else if (buffer[0] == LINEFEED) {
	      in_header = 0;	/* in body of message! */
	      show_header(priority, from_whom, subject);
	      from_whom[0] = 0;
	      subject[0] = 0;
	      count++;
	    }
	  }
	}
	return(count);
}

int
real_from(buffer, who)
char *buffer, *who;
{
	/***** returns true iff 's' has the seven 'from' fields,
	       initializing the who to the sender *****/

	char junk[80];

	junk[0] = '\0';
	sscanf(buffer, "%*s %s %*s %*s %*s %*s %s",
	            who, junk);
	return(junk[0] != '\0');
}

forwarded(buffer, who)
char *buffer, *who;
{
	/** change 'from' and date fields to reflect the ORIGINATOR of 
	    the message by iteratively parsing the >From fields... **/

	char machine[80], buff[80];

	machine[0] = '\0';
	sscanf(buffer, "%*s %s %*s %*s %*s %*s %*s %*s %*s %s",
	            who, machine);

	if (machine[0] == '\0') /* try for srm address */
	  sscanf(buffer, "%*s %s %*s %*s %*s %*s %*s %*s %s",
	            who, machine);

	if (machine[0] == '\0')
	  sprintf(buff,"anonymous");
	else
	  sprintf(buff,"%s!%s", machine, who);

	strncpy(who, buff, 80);
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

show_header(priority, from, subject)
int   priority;
char *from, *subject;
{
	/** Output header in clean format, including abbreviation
	    of return address if more than one machine name is
	    contained within it! **/

	char buffer[SLEN];
	int  loc, i=0, exc=0;

#ifdef PREFER_UUCP
	
	if (chloc(from,'!') != -1 && in_string(from, BOGUS_INTERNET))
	  from[strlen(from) - strlen(BOGUS_INTERNET)] = '\0';

#endif

	loc = strlen(from);

	while (exc < 2 && loc > 0)
	  if (from[--loc] == '!')
	    exc++;

	if (exc == 2) { /* lots of machine names!  Get last one */
	  loc++;
	  while (loc < strlen(from) && loc < SLEN)
	    buffer[i++] = from[loc++];
	  buffer[i] = '\0';
	  strcpy(from, buffer);
	}

	if (strlen(subject) < 2)
	  strcpy(subject, NO_SUBJECT);
	
	  if (strlen(from) > 0)	/* last final check... */
	    printf("%sMail from %s -- %s\n", 
		   priority? "PRIORITY ": "", from, subject);
}	

parse_arpa_from(buffer, newfrom)
char *buffer, *newfrom;
{
	/** Try to parse the 'From:' line given... It can be in one of
	    two formats:
		From: Dave Taylor <hpcnou!dat>
	    or  From: hpcnou!dat (Dave Taylor)
	    Change 'newfrom' ONLY if sucessfully parsed this entry and
	    the resulting name is non-null! 
	**/

	char temp_buffer[SLEN], *temp;
	register int i, j = 0;

	temp = (char *) temp_buffer;
	temp[0] = '\0';

	no_ret(buffer);		/* blow away '\n' char! */

	if (lastch(buffer) == '>') {
	  for (i=strlen("From: "); buffer[i] != '\0' && buffer[i] != '<' &&
	       buffer[i] != '('; i++)
	    temp[j++] = buffer[i];
	  temp[j] = '\0';
	}
	else if (lastch(buffer) == ')') {
	  for (i=strlen(buffer)-2; buffer[i] != '\0' && buffer[i] != '(' &&
	       buffer[i] != '<'; i--)
	    temp[j++] = buffer[i];
	  temp[j] = '\0';
	  reverse(temp);
	}
	  
	if (strlen(temp) > 0) {		/* mess with buffer... */

	  /* remove leading spaces... */

	  while (whitespace(temp[0]))
	    temp = (char *) (temp + 1);		/* increment address! */

	  /* remove trailing spaces... */

	  i = strlen(temp) - 1;

	  while (whitespace(temp[i]))
	   temp[i--] = '\0';

	  /* if anything is left, let's change 'from' value! */

	  if (strlen(temp) > 0)
	    strcpy(newfrom, temp);
	}
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
	  if (errno != 2)
	   exit(fprintf(stderr,"Error %d attempting fstat on %s", errno, name));
	  else
	    ok = 0;
	
	return(ok ? (long) buffer.st_size : 0L);
}

char  *getusername()
{
	/** Getting the username on some systems is a real pain, so...
	   This routine is guaranteed to return a usable username **/

	char *return_value, *cuserid(), *getlogin();

	if ((return_value = cuserid(NULL)) == NULL)
	  if ((return_value = getlogin()) == NULL)
	    exit(printf("Newmail: I can't get username!\n"));

	return( (char *) return_value);
}
