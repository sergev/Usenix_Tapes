/**			readmsg.c			**/

/** This routine adds the functionality of the "~r" command to the Elm mail 
    system while still allowing the user to use the editor of their choice.

    The program, without any arguments, tries to read a file in the users home 
    directory called ".readmsg" (actually defined in the sysdefs.h system 
    defines file) and if it finds it reads the current message.  If it doesn't 
    find it, it will return a usage error.

    The program can also be called with an explicit message number, list of 
    message numbers, or a string to match in the message (including the header).
    NOTE that when you use the string matching option it will match the first 
    message containing that EXACT (case sensitive) string and then exit.

    Changed 5/86 to SORT the input list of message numbers to ensure that
    they're in first-to-last order...
	
    Added the "weed" option as the default.  This is inspired by the mail
    system used at NASA RIACS.  If THEY can do it, so can we!!

    Also added '*' as a flag - indicating ALL messages in the mailbox.

    (C) Copyright 1985, Dave Taylor

**/

#include <stdio.h>
#include <ctype.h>

#include "defs.h"

/** three defines for what level of headers to display **/

#define ALL		1
#define WEED		2
#define NONE		3

#define metachar(c)	(c == '=' || c == '+' || c == '%')

static char ident[] = { WHAT_STRING };

#define  MAX_LIST	25		/* largest single list of arguments */

#define  LAST_MESSAGE	9999		/* last message in list ('$' char)  */
#define  LAST_CHAR	'$'		/* char to delimit last message..   */
#define  STAR		'*'		/* char to delimit all messages...  */

int read_message[MAX_LIST]; 		/* list of messages to read	    */
int messages = 0;			/* index into list of messages      */

int numcmp();				/* strcmp, but for numbers          */
char *words(),				/* function defined below...        */
     *expand_define();			/*     ditto...                     */

#define DONE	0			/* for use with the getopt	    */
#define ERROR   -1			/*   library call...		    */

extern char *optional_arg;		/* for parsing the ... 		    */
extern int   opt_index;			/*  .. starting arguments           */

char *getenv();				/* keep lint happy */

main(argc, argv)
int argc;
char *argv[];
{
	FILE *file;			        /* generic file descriptor! */
	char filename[SLEN], 			/* filename buffer          */
	     infile[SLEN],			/* input filename	    */
	     buffer[SLEN], 			/* file reading buffer      */
	     string[SLEN];			/* string match buffer      */

	int current_in_queue = 0, 		/* these are used for...     */
	    current = 0,			/* ...going through msgs     */
	    list_all_messages = 0,		/* just list 'em all??       */
	    num, 				/* for argument parsing      */
	    page_breaks = 0,			/* use "^L" breaks??         */
            total,				/* number of msgs current    */
	    include_headers = WEED, 		/* flag: include msg header? */
	    last_message = 0, 			/* flag: read last message?  */
	    not_in_header = 0,			/* flag: in msg header?      */
	    string_match = 0;			/* flag: using string match? */

	/**** start of the actual program ****/

	while ((num = get_options(argc, argv, "nhf:p")) > 0) {
	  switch (num) {
	    case 'n' : include_headers = NONE;		break;
	    case 'h' : include_headers = ALL;		break;
	    case 'f' : strcpy(infile, optional_arg);	
		       if (metachar(infile[0]))
	                 if (expand(infile) == 0)
	                   printf("%s: couldn't expand filename %s!\n", 
			          argv[0], infile);
		       break;
	    case 'p' : page_breaks++;			break;
	  }
	}
	
	if (num == ERROR) {
	  printf("Usage: %s [-n|-h] [-f filename] [-p] <message list>\n",
		  argv[0]);
	  exit(1);
	}

	/** whip past the starting arguments so that we're pointing
	    to the right stuff... **/

	*argv++;	/* past the program name... */

	while (opt_index-- > 1) {
	  *argv++;
	  argc--;
	}

	/** now let's figure out the parameters to the program... **/

	if (argc == 1) {	/* no arguments... called from 'Elm'? */
	  sprintf(filename, "%s/%s", getenv("HOME"), readmsg_file);
	  if ((file = fopen(filename, "r")) != NULL) {
	    fscanf(file, "%d", &(read_message[messages++]));
	    fclose(file);
	  }
	  else {	/* no arguments AND no .readmsg file!! */
	    fprintf(stderr,
	        "Usage: readmsg [-n|-h] [-f filename] [-p] <message list>\n");
	    exit(1);
	  }
	}
	else if (! isdigit(*argv[0]) && *argv[0] != LAST_CHAR && 
	         *argv[0] != STAR) {  
	  string_match++;
	 
	  while (*argv)
	    sprintf(string, "%s%s%s", string, string[0] == '\0'? "" : " ",
		    *argv++);
	}
	else if (*argv[0] == STAR) 		/* all messages....   */
	  list_all_messages++;
	else { 					  /* list of nums   */

	  while (--argc > 0) {
	    num = -1;

	    sscanf(*argv,"%d", &num);

	    if (num < 0) {
	      if (*argv[0] == LAST_CHAR) {
	        last_message++;
		num = LAST_MESSAGE;
	      }
	      else {
	        fprintf(stderr,"I don't understand what '%s' means...\n",
			*argv); 
	       	exit(1); 
	      }
	    }
	    else if (num == 0) {	/* another way to say "last" */
	      last_message++;
	      num = LAST_MESSAGE;
	    }

	    *argv++; 

	    read_message[messages++] = num;
	  }

	  /** and now sort 'em to ensure they're in a reasonable order... **/

	  qsort(read_message, messages, sizeof(int), numcmp);
	}

	/** Now let's get to the mail file... **/

	if (strlen(infile) == 0) 
	  sprintf(infile, "%s/%s", mailhome, getenv("LOGNAME"));

	if ((file = fopen(infile, "r")) == NULL) {
	  printf("But you have no mail! [ file = %d ]\n", infile);
	  exit(0);
	}

	/** Now it's open, let's display some 'ole messages!! **/

	if (string_match || last_message) {   /* pass through it once */

	  if (last_message) {
	    total = count_messages(file);	/* instantiate count */
	    for (num=0; num < messages; num++)
	      if (read_message[num] == LAST_MESSAGE)
		read_message[num] = total;
	  }
	  else if (string_match)
	    match_string(file, string);		/* stick msg# in list */

	  if (total == 0 && ! string_match) {
	    printf("There aren't any messages to read!\n");
	    exit(0);
	  }
	}

 	/** now let's have some fun! **/
	
	while (fgets(buffer, SLEN, file) != NULL) {
	  if (strncmp(buffer, "From ", 5) == 0) {
	    if (! list_all_messages) {
	      if (current == read_message[current_in_queue])
	        current_in_queue++;
	      if (current_in_queue >= messages) 
	        exit(0);
	    }
	    current++;
	    not_in_header = 0;	/* we're in the header! */
	  }
	  if (current == read_message[current_in_queue] || list_all_messages)
	    if (include_headers==ALL || not_in_header)
	      printf("%s", buffer);
	    else if (strlen(buffer) < 2) {
	      not_in_header++;
	      if (include_headers==WEED) 
		list_saved_headers(page_breaks);
	    }
	    else if (include_headers==WEED)
	      possibly_save(buffer); 	/* check to see if we want this */ 
	}
	
	exit(0);
}

int
count_messages(file)
FILE *file;
{
	/** Returns the number of messages in the file **/

	char buffer[SLEN];
	int  count = 0;

	while (fgets(buffer, SLEN, file) != NULL)
	  if (strncmp(buffer, "From ", 5) == 0)
	    count++;

	rewind( file );
	return( count );
}

match_string(mailfile, string)
FILE *mailfile;
char *string;
{
	/** Increment "messages" and put the number of the message
	    in the message_count[] buffer until we match the specified 
	    string... **/

	char buffer[SLEN];
	int  message_count;

	while (fgets(buffer, SLEN, mailfile) != NULL) {
	  if (strncmp(buffer, "From ", 5) == 0)
	    message_count++;

	  if (in_string(buffer, string)) {
	    read_message[messages++] = message_count;
	    rewind(mailfile);	
	    return;
	  }
	}

	fprintf(stderr,"Couldn't find message containing '%s'\n", string);
	exit(1);
}

int 
numcmp(a, b)
int *a, *b;
{
	/** compare 'a' to 'b' returning less than, equal, or greater
	    than, accordingly.
	 **/

	return(*a - *b);
}

static char from[SLEN], subject[SLEN], date[SLEN], to[SLEN];

possibly_save(buffer)
char *buffer;
{
	/** Check to see what "buffer" is...save it if it looks 
	    interesting... We'll always try to get SOMETHING
	    by tearing apart the "From " line...  **/

	if (strncmp(buffer, "Date:", 5) == 0)
	  strcpy(date, buffer);
	else if (strncmp(buffer, "Subject:", 8) == 0)
	  strcpy(subject,buffer);
	else if (strncmp(buffer,"From:", 5) == 0)
	  strcpy(from, buffer);
	else if (strncmp(buffer,"To: ", 3) == 0)
	  strncpy(to, buffer, SLEN);
	else if (strncmp(buffer,"From ", 5) == 0) {
	  sprintf(from, "From: %s\n", words(2,1, buffer));	
	  sprintf(date,"Date: %s",    words(3,7, buffer));
	  to[0] = '\0';
	}
}

list_saved_headers(page_break)
int page_break;
{
	/** This routine will display the information saved from the
	    message being listed...If it displays anything it'll end
	    with a blank line... **/

	register int displayed_line = FALSE;
	static int messages_listed = 0;

	if (page_break && messages_listed++) putchar(FORMFEED);	

	if (strlen(from)    > 0) { printf("%s", from);    displayed_line++;}
	if (strlen(subject) > 0) { printf("%s", subject); displayed_line++;}
	if (strlen(to)      > 0) { printf("%s", to);      displayed_line++;}
	if (strlen(date)    > 0) { printf("%s", date);    displayed_line++;}
	
	if (displayed_line)
	   putchar('\n');
}

char *words(word, num_words, buffer)
int word, num_words;
char *buffer;
{
	/** Return a buffer starting at 'word' and containing 'num_words'
	    words from buffer.  Assume white space will delimit each word.
	**/

	static char internal_buffer[SLEN];
	char   *wordptr, *bufptr, mybuffer[SLEN], *strtok();
	int    wordnumber = 0, copying_words = 0;

	internal_buffer[0] = '\0';	/* initialize */

	strcpy(mybuffer, buffer);
	bufptr = (char *) mybuffer;	/* and setup */

	while ((wordptr = strtok(bufptr, " \t")) != NULL) {
	  if (++wordnumber == word) {
	    strcpy(internal_buffer, wordptr);
	    copying_words++;
	    num_words--;
	  }
	  else if (copying_words) {
	    strcat(internal_buffer, " ");
	    strcat(internal_buffer, wordptr);
	    num_words--;
	  }

	  if (num_words < 1) 
	    return((char *) internal_buffer);

	  bufptr = NULL;
	}

	return( (char *) internal_buffer);
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
	else
	   strcpy(buffer, maildir);

	return( ( char *) buffer);
}

