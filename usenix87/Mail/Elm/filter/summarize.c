/**			summarize.c			**/

/** This routine is called from the filter program (or can be called
    directly with the correct arguments) and summarizes the users filterlog
    file.  To be honest, there are two sorts of summaries that are
    available - either the '.filterlog' file can be output (filter -S) 
    or a summary by rule and times acted upon can be output (filter -s).
    Either way, this program will delete the two associated files each
    time ($HOME/.filterlog and $HOME/.filtersum) *if* the -c option is
    used to the program (e.g. clear_logs is set to TRUE).

    (C) Copyright 1986, Dave Taylor
**/

#include <stdio.h>

#include "defs.h"

#include "filter.h"

show_summary()
{
	/* Summarize usage of the program... */

	FILE   *fd;				/* for output to temp file! */
	char filename[SLEN],			/* name of the temp file    */
	     buffer[LONG_SLEN];			/* input buffer space       */
	int  erroneous_rules = 0,
	     default_rules   = 0,
	     rule,
	     applied[MAXRULES];

	if (long_summary) 
	  long_sum();
	else {

	  sprintf(filename, "%s/%s", home, filtersum);

	  if ((fd = fopen(filename, "r")) == NULL) {
	    fprintf(stderr,"filter (%s): Can't open filterlog file %s!\n",
		    username, filename);
	    exit(1);
	  }

	  for (rule=0;rule < MAXRULES; rule++)
	    applied[rule] = 0;			/* initialize it all! */

	  /** Next we need to read it all in, incrementing by which rule
	      was used.  The format is simple - each line represents a 
	      single application of a rule, or '-1' if the default action
	      was taken.  Simple stuff, eh?  But oftentimes the best.  
	  **/

	  while (fgets(buffer, LONG_SLEN, fd) != NULL) {
	    if ((rule = atoi(buffer)) > total_rules || rule < -1) {
	      fprintf(stderr,
      "%sfilter (%s): Warning - rule #%d is invalid data for short summary!!\n",
		     BEEP, username, rule);
	      erroneous_rules++;
	    }
	    else if (rule == -1)
	      default_rules++;
	    else
	      applied[rule]++;
	  }
	
	  fclose(fd);

	  /** now let's summarize the data... **/

	  printf("\nSummary of filter activity;\n\n");

	  if (erroneous_rules)
	    printf("** Warning: %d erroneous rule%s logged and ignored! **\n\n",
		   erroneous_rules, erroneous_rules > 1? "s were" : " was");
	
	  if (default_rules)
	    printf(
   "The default rule of putting mail into your mailbox was used %d time%s\n\n",
		   default_rules, plural(default_rules));

	  /** and now for each rule we used... **/

	  for (rule = 0; rule < total_rules; rule++) {
	    if (applied[rule]) {
	       printf("Rule #%d: ", rule+1);
	       switch (rules[rule].action) {
		  case LEAVE:	printf("(leave mail in mailbox)");	break;
		  case DELETE:  printf("(delete message)");		break;
		  case SAVE  :  printf("(save in \"%s\")",
					rules[rule].argument2);		break;
		  case SAVECC:  printf("(left in mailbox and saved in \"%s\")",
					rules[rule].argument2);		break;
		  case FORWARD: printf("(forwarded to \"%s\")",
					rules[rule].argument2);		break;
		  case EXEC  :  printf("(given to command \"%s\")",
					rules[rule].argument2);		break;
	       }
	       printf(" was applied %d time%s.\n\n", applied[rule],
		      plural(applied[rule]));
	     }
	   }

	  /* next, after a ^L, include the actual log file... */

	  sprintf(filename, "%s/%s", home, filterlog);

	  if ((fd = fopen(filename, "r")) == NULL) {
	    fprintf(stderr,"filter (%s): Can't open filterlog file %s!\n",
		    username, filename);
	  }
	  else {
	    printf("\n\n\n%c\n\nExplicit log of each action;\n\n", (char) 12);
	    while (fgets(buffer, LONG_SLEN, fd) != NULL)
	      printf("%s", buffer);
	    printf("\n-----\n");
	    fclose(fd);
	  }

	  /* now remove the log files, please! */

	  if (clear_logs) {
	    unlink(filename);
	    sprintf(filename, "%s/%s", home, filtersum);
	    unlink(filename);
	  }

	  return;
	}
}

long_sum()
{
	/** summarize by listing the .fitlerlog file.  The simplest of the
	    two possible options this one is indeed rather trivial!  **/

	FILE   *fd;				/* for output to temp file! */
	char filename[SLEN],			/* name of the temp file    */
	     buffer[LONG_SLEN];			/* input buffer space       */

	sprintf(filename,"%s/%s", home, filterlog);

	if ((fd = fopen(filename, "r")) == NULL) {
	  fprintf(stderr,"filter (%s): Can't open filterlog file %s!\n",
		  username, filename);
	  exit(1);
	}

	while (fgets(buffer, SLEN, fd) != NULL)
	  printf("%s", buffer);	/* already has '\n' doesn't it? */
	fclose(fd);

	/** now let's waste the TWO log files and get outta here! **/

	if (clear_logs) {
	  unlink(filename);
	  sprintf(filename, "%s/%s", home, filtersum);
	  unlink(filename);
	}

	return;
}
