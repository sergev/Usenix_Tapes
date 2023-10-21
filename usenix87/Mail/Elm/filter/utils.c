/**			utils.c				**/

/** Utility routines for the filter program...

   (C) Copyright 1986, Dave Taylor
**/

#include <stdio.h>
#include <pwd.h>
#include <ctype.h>
#include <fcntl.h>

#include "defs.h"
#include "filter.h"

leave(reason)
char *reason;
{
	fprintf(stderr,"%sfilter (%s): LEAVE %s\n", BEEP, username, reason);
	exit(1);
}

log(what)
int what;
{
	/** make an entry in the log files for the specified entry **/

	FILE *fd;
	char filename[SLEN];

	if (! show_only) {
	  sprintf(filename, "%s/%s", home, filtersum);	/* log action once! */
	  if ((fd = fopen(filename, "a")) == NULL) {
	    fprintf(stderr,"%sfilter (%s): Couldn't open log file %s\n", 
		    BEEP, filename);
	    fd = stdout;
	  }
	  fprintf(fd, "%d\n", rule_choosen);
	  fclose(fd);
	}

	sprintf(filename, "%s/%s", home, filterlog);

	if (show_only)
	  fd = stdout;
	else if ((fd = fopen(filename, "a")) == NULL) {
	  fprintf(stderr,"%sfilter (%s): Couldn't open log file %s\n", 
		  BEEP, filename);
	  fd = stdout;
	}
	
	setvbuf(fd, NULL, _IOFBF, BUFSIZ);

	if (strlen(from) + strlen(subject) > 60)
	  fprintf(fd, "\nMail from %s\n\tabout %s\n", from, subject);
	else
	  fprintf(fd, "\nMail from %s about %s\n", from, subject);

	if (rule_choosen != -1)
	  if (rules[rule_choosen].condition->matchwhat == TO)
	    fprintf(fd, "\t(addressed to %s)\n", to);

	switch (what) {
	  case DELETE : fprintf(fd, "\tDELETED");			break;
	  case SAVE   : fprintf(fd, "\tSAVED in file \"%s\"", 
				rules[rule_choosen].argument2);		break;
	  case SAVECC : fprintf(fd,"\tSAVED in file \"%s\" AND PUT in mailbox", 
				rules[rule_choosen].argument2);  	break;
	  case FORWARD: fprintf(fd, "\tFORWARDED to \"%s\"", 
				rules[rule_choosen].argument2);		break;
	  case EXEC   : fprintf(fd, "\tEXECUTED \"%s\"",
				rules[rule_choosen].argument2);		break;
	  case LEAVE  : fprintf(fd, "\tPUT in mailbox");		break;
	}

	if (rule_choosen != -1)
	  fprintf(fd, " by rule #%d\n", rule_choosen+1);
	else
	  fprintf(fd, ": the default action\n");

	fflush(fd);
	fclose(fd);
}

int
contains(string, pattern)
char *string, *pattern;
{
	/** Returns TRUE iff pattern occurs IN IT'S ENTIRETY in buffer. **/ 

	register int i = 0, j = 0;

	while (string[i] != '\0') {
	  while (tolower(string[i++]) == tolower(pattern[j++])) 
	    if (pattern[j] == '\0') 
	      return(TRUE);
	  i = i - j + 1;
	  j = 0;
	}
	return(FALSE);
}

char *itoa(i, two_digit)
int i, two_digit;
{	
	/** return 'i' as a null-terminated string.  If two-digit use that
	    size field explicitly!  **/

	static char value[10];
	
	if (two_digit)
	  sprintf(value, "%02d", i);
	else
	  sprintf(value, "%d", i);

	return( (char *) value);
}

lowercase(string)
char *string;
{
	/** translate string into all lower case **/

	register int i;

	for (i=0; i < strlen(string); i++)
	  if (isupper(string[i]))
	    string[i] = tolower(string[i]);
}
