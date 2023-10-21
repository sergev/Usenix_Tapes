/**		from.c		**/

/** print out whom each message is from in the pending mailbox
    or specified one, including a subject line if available.. 

    Added PREFER_UUCP knowledge 6/86
    Added "-n" flag, 9/86

    (C) Copyright 1986 Dave Taylor 
**/

#include <stdio.h>
#include "defs.h"

static char ident[] = { WHAT_STRING };

#define LINEFEED	(char) 10

#define metachar(c)	(c == '=' || c == '+' || c == '%')

FILE *mailfile;

char *expand_define();
int   number = 0;	/* should we number the messages?? */

main(argc, argv)
int argc;
char *argv[];
{
	char infile[LONG_SLEN], username[SLEN];

	if (argc > 2) {
	  printf("Usage: %s [-n] {filename | username}\n", argv[0]);
	  exit(1);
	}

	if (argc > 1)
	  if (strcmp(argv[1], "-n") == 0) {
	    number++;
	    *argv++;
	    argc--;
	  }

	if (argc == 2) 
	  strcpy(infile, argv[1]);
	else {
	  strcpy(username, getlogin());
	  if (strlen(username) == 0)
	    cuserid(username);
	  sprintf(infile,"%s%s",mailhome, username);
	}

	if (metachar(infile[0])) {
	  if (expand(infile) == 0) {
	     fprintf(stderr, "%s: couldn't expand filename %s!\n", 
		     argv[0], infile);
	     exit(1);
	  }
	}
	else
	  printf("filename '%s' doesn't have a metacharacter!\n", infile);

	if ((mailfile = fopen(infile,"r")) == NULL) {
	  if (argc == 1)
	    printf("No mail!\n");
	  else {
	    if (argv[1][0] == '/') 
	      printf("Couldn't open folder/mailfile %s!\n", infile);
	    else {
	      sprintf(infile,"%s%s", mailhome, argv[1]);
	      if ((mailfile = fopen(infile,"r")) == NULL)
	        printf("Couldn't open folder/mailfile %s!\n", infile);
	      else if (read_headers()==0)
	        printf("No messages in mailbox!\n");
	     }
	  }
	}
	else
	  if (read_headers()==0)
	    printf("No messages in mailbox!\n");
}

int
read_headers()
{
	/** read the headers, output as found **/

	char buffer[LONG_SLEN], from_whom[SLEN], subject[SLEN];
	register int subj = 0, in_header = 1, count = 0;

	while (fgets(buffer, LONG_SLEN, mailfile) != NULL) {
	  if (first_word(buffer,"From ")) {
	    if (real_from(buffer, from_whom)) {
	      subj = 0;
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
	    else if (buffer[0] == LINEFEED) {
	      in_header = 0;	/* in body of message! */
	      show_header(count+1, from_whom, subject);
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

show_header(count, from, subject)
int  count;
char *from, *subject;
{
	/** output header in clean format, including abbreviation
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
	  if (number)
	    printf("%3d: %-20s  %s\n", count, buffer, subject);
	  else
	    printf("%-20s  %s\n", buffer, subject);
	}
	else
	  if (number)
	    printf("%3d: %-20s  %s\n", count, from, subject);
	  else
	    printf("%-20s  %s\n", from, subject);
}	

parse_arpa_from(buffer, newfrom)
char *buffer, *newfrom;
{
	/** try to parse the 'From:' line given... It can be in one of
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

int
expand(infile)
char *infile;
{
	/** Expand the filename since the first character is a meta-
	    character that should expand to the "maildir" variable
	    in the users ".elmrc" file...

	    Note: this is a brute force way of getting the entry out 
	    of the .elmrc file, and isn't recommended for the faint 
	    of heart!
	**/

	FILE *rcfile;
	char  buffer[SLEN], *expanded_dir, *home, *getenv(), *bufptr;
	int   foundit = 0;

	bufptr = (char *) buffer;		/* same address */
	
	if ((home = getenv("HOME")) == NULL) {
	  printf(
	     "Can't expand environment variable $HOME to find .elmrc file!\n");
	  exit(1);
	}

	sprintf(buffer, "%s/%s", home, elmrcfile);

	if ((rcfile = fopen(buffer, "r")) == NULL) {
	  printf("Can't open your \".elmrc\" file (%s) for reading!\n",
		 buffer);
	  exit(1);
	}

	while (fgets(buffer, SLEN, rcfile) != NULL && ! foundit) {
	  if (strncmp(buffer, "maildir", 7) == 0 ||
	      strncmp(buffer, "folders", 7) == 0) {
	    while (*bufptr != '=' && *bufptr) 
	      bufptr++;
	    bufptr++;			/* skip the equals sign */
	    while (whitespace(*bufptr) && *bufptr)
	      bufptr++; 
	    home = bufptr;		/* remember this address */

	    while (! whitespace(*bufptr) && *bufptr != '\n')
	      bufptr++;

	    *bufptr = '\0';		/* remove trailing space */
	    foundit++;
	  }
	}

	fclose(rcfile);			/* be nice... */

	if (! foundit) {
	  printf("Couldn't find \"maildir\" in your .elmrc file!\n");
	  exit(1);
	}

	/** Home now points to the string containing your maildir, with
	    no leading or trailing white space...
	**/

	expanded_dir = expand_define(home);

	sprintf(buffer, "%s%s%s", expanded_dir, 
		(expanded_dir[strlen(expanded_dir)-1] == '/' ||
		infile[0] == '/') ? "" : "/", (char *) infile+1);

	strcpy(infile, buffer);
}

char *expand_define(maildir)
char *maildir;
{
	/** This routine expands any occurances of "~" or "$var" in
	    the users definition of their maildir directory out of
	    their .elmrc file.

	    Again, another routine not for the weak of heart or staunch
	    of will!
	**/

	static char buffer[SLEN];	/* static buffer AIEE!! */
	char   name[SLEN],		/* dynamic buffer!! (?) */
	       *nameptr,	       /*  pointer to name??     */
	       *value;		      /* char pointer for munging */

	if (*maildir == '~') 
	  sprintf(buffer, "%s%s", getenv("HOME"), ++maildir);
	else if (*maildir == '$') { 	/* shell variable */

	  /** break it into a single word - the variable name **/

	  strcpy(name, (char *) maildir + 1);	/* hurl the '$' */
	  nameptr = (char *) name;
	  while (*nameptr != '/' && *nameptr) nameptr++;
	  *nameptr = '\0';	/* null terminate */
	  
	  /** got word "name" for expansion **/

	  if ((value = getenv(name)) == NULL) {
	    printf("Couldn't expand shell variable $%s in .elmrc!\n", name);
	    exit(1);
	  }
	  sprintf(buffer, "%s%s", value, maildir + strlen(name) + 1);
	}
	else strcpy(buffer, maildir);

	return( ( char *) buffer);
}
