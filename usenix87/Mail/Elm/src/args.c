/**			args.c			**/

/** starting argument parsing routines for ELM system...

    (C) Copyright 1986 Dave Taylor
**/

#include "headers.h"

#define DONE		0
#define ERROR		-1

extern char *optional_arg;		/* optional argument as we go */
extern int   opt_index;			/* argnum + 1 when we leave   */

void exit();	/* just keeping lint happy.... */

parse_arguments(argc, argv, to_whom)
int argc;
char *argv[], *to_whom;
{
	/** Set flags according to what was given to program.  If we are 
	    fed a name or series of names, put them into the 'to_whom' buffer
	    and set "mail_only" to TRUE **/

	register int c = 0, check_size = 0;
	char *strcpy();

	infile[0] = '\0';
	to_whom[0] = '\0';
	batch_subject[0] = '\0';

	while ((c = get_options(argc, argv, "?acd:f:hkKms:wz")) > 0) {
	   switch (c) {
	     case 'a' : arrow_cursor++;		break;
	     case 'c' : check_only++;		break;
	     case 'd' : debug = atoi(optional_arg);	break;
	     case 'f' : strcpy(infile, optional_arg); 
	                mbox_specified = 2;  break;
	     case '?' :
	     case 'h' : args_help();
	     case 'k' : hp_terminal++;	break;
	     case 'K' : hp_terminal++; hp_softkeys++;	break;
	     case 'm' : mini_menu = 0;	break;
	     case 's' : strcpy(batch_subject, optional_arg);	break;
	     case 'w' : warnings = 0;	break;
	     case 'z' : check_size++;   break;
	    }
	 }

	 if (c == ERROR) {
	   printf(
          "Usage: %s [achkKmwz] [-d level] [-f file] [-s subject] <names>\n\n",
	     argv[0]);
	   args_help();
	}

	if (opt_index < argc) {
	  while (opt_index < argc) {
	    sprintf(to_whom, "%s%s%s", to_whom, 
	            to_whom[0] != '\0'? " " : "", argv[opt_index]);
	    mail_only++;
	    opt_index++;
	  }
	  check_size = 0;	/* NEVER do this if we're mailing!! */
	}

	 if (strlen(batch_subject) > 0 && ! mail_only) 
	   exit(printf(
     "\n\rDon't understand specifying a subject and no-one to send to!\n\r"));

	if (!isatty(fileno(stdin)) && strlen(batch_subject) == 0 && !check_only)
	  strcpy(batch_subject, DEFAULT_BATCH_SUBJECT);

	if (check_size)
	  check_mailfile_size();
}

args_help()
{
	/**  print out possible starting arguments... **/

	printf("\nPossible Starting Arguments for ELM program:\n\n");
	printf("\targ\t\t\tMeaning\n");
	printf("\t -a \t\tArrow - use the arrow pointer regardless\n");
	printf("\t -c \t\tCheckalias - check the given aliases only\n");
	printf("\t -dn\t\tDebug - set debug level to 'n'\n");
	printf("\t -fx\t\tFile - read file 'x' rather than mailbox\n");
	printf("\t -h \t\tHelp - give this list of options\n");
	printf("\t -k \t\tKeypad - enable HP 2622 terminal keyboard\n");
	printf("\t -K \t\tKeypad&softkeys - enable use of softkeys + \"-k\"\n");
	printf("\t -m \t\tMenu - Turn off menu, using more of the screen\n");
	printf("\t -sx\t\tSubject 'x' - for batchmailing\n");
	printf("\t -w \t\tSupress warning messages...\n");
	printf("\t -z \t\tZero - don't enter Elm if no mail is pending\n");
	printf("\n");
	printf("\n");
	exit(1);
}
