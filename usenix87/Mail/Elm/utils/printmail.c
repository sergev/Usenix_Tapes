/**		printmail.c		**/

/** print mail, adding a formfeed between each message  **/
/** Modified to use stdin if being fed from a pipe.	**/

/** (C) Copyright 1985, Dave Taylor **/

#include <stdio.h>
#include "defs.h"

static char ident[] = { WHAT_STRING };

#define  dashes		  \
"\n\n------------------------------------------------------------------------------\n\n"
#define  FF	  	  "\014"	/* form feed! */

FILE *mailfile;
char  separator[80];

main(argc, argv)
int argc;
char *argv[];
{
	char infile[80], username[40], c;

	strcpy(separator, FF);

	argv++;				/* get past argv[0] */

	if (argc > 1) 
	  if (strcmp(*argv, "-d") == 0) {
	    strcpy(separator, dashes);
	    --argc;
	    argv++;
	  }
	
	if (argc > 1 && *argv[0] == '-') 
	    exit(fprintf(stderr, "Usage: printmail {-d} {filename (s)}\n"));
	
	if (argc == 1) {
	  strcpy(username, getlogin());
	  if (strlen(username) == 0)
	    cuserid(username);
	  if (isatty(fileno(stdin))) {	/* normal invokation... */
	    sprintf(infile,"%s/%s",mailhome, username);
	    if ((mailfile = fopen(infile,"r")) == NULL) {
	      fprintf(stderr, "No mail!\n");
	      exit(0);
	    }
	  }
	  else 
	    mailfile = stdin;	/* read from stdin! */

	  if (read_headers() == 0)
	    fprintf(stderr, "No messages in mailbox!\n");
	}

	if (argc > 1)		/* more than one file - delimit each */
	  if (strcmp(separator, FF) != 0) 
	    printf("\t\t\t%s\n%s", *argv, separator);
	  else
	    printf("\t\t\t%s\n\n", *argv);	/* Don't put a formfeed! */

	while (--argc) {
	  if ((mailfile = fopen(*argv,"r")) == NULL) {
	    fprintf(stderr, "Could not open file '%s'!", *argv);
	    break;
	  }
	  else
	    if (read_headers() == 0)
	      fprintf(stderr, "No messages in mailbox '%s'!\n", *argv);
	  argv++;
	  if (argc-1) {
	    if (strcmp(separator, FF) != 0) 
	      printf("%s\t\t\t%s%s", separator, *argv, separator);
	    else
	      printf("%s\t\t\t%s\n\n", separator, *argv);
	  }
	}
}

int
read_headers()
{
	char buffer[100];
	register int count = 0;

	while (fgets(buffer, 100, mailfile) != NULL) 
	  if (first_word(buffer,"From ")) {
	    if (real_from(buffer)) {
	      printf("%s%s", count ? separator : "", buffer);
	      count++;
	    }
	  }
	  else
	    printf("%s", buffer);

	return(count);
}

int
real_from(buffer)
char *buffer;
{
	/***** returns true iff 's' has the seven 'from' fields *****/

	char junk[80];

	junk[0] = '\0';
	sscanf(buffer, "%*s %*s %*s %*s %*s %*s %s", junk);
	return(junk[0] != '\0');
}
