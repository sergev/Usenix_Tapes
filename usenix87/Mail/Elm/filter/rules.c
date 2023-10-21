/**			rules.c			**/

/** This file contains all the rule routines, including those that apply the
    specified rules and the routine to print the rules out.

    (C) Copyright 1986, Dave Taylor
**/

#include <stdio.h>
#include <pwd.h>
#include <ctype.h>
#ifdef BSD
# include <sys/time.h>
#else
# include <time.h>
#endif
#include <fcntl.h>

#include "defs.h"
#include "filter.h"

int
action_from_ruleset()
{
	/** Given the set of rules we've read in and the current to, from, 
	    and subject, try to match one.  Return the ACTION of the match
            or LEAVE if none found that apply.
	**/

	register int index = 0, not, relation, try_next_rule, x;
	struct condition_rec *cond;

	while (index < total_rules) {
	  cond = rules[index].condition;
	  try_next_rule = 0;

	  while (cond != NULL && ! try_next_rule) {
	    
	    not = (cond->relation < 0);
	    relation = abs(cond->relation);
	
	    switch (cond->matchwhat) {

	      case TO     : x = contains(to, cond->argument1); 		break;
	      case FROM   : x = contains(from, cond->argument1); 	break;
	      case SUBJECT: x = contains(subject, cond->argument1);	break;
	      case LINES  : x = compare(lines, relation, cond->argument1);break;
		       
	      case CONTAINS: fprintf(stderr,
       "%sfilter (%s): Error: rules based on 'contains' are not implemented!\n",
			     BEEP, username);
			     exit(0);
	    }

	    if ((not && x) || ((! not) && (! x))) /* this test failed (LISP?) */
	      try_next_rule++;
	    else
	      cond = cond->next;		  /* next condition, if any?  */
	  }

	  if (! try_next_rule) {
	    rule_choosen = index;
 	    return(rules[rule_choosen].action);
	  }
	  index++;
	}

	rule_choosen = -1;
	return(LEAVE);
}

#define get_the_time()	if (!gotten_time) { 		  \
			   thetime = time( (long *) 0);   \
			   timerec = localtime(&thetime); \
			   gotten_time++; 		  \
			}

expand_macros(word, buffer, line)
char *word, *buffer;
int  line;
{
	/** expand the allowable macros in the word;
		%d	= day of the month  
		%D	= day of the week  
	        %h	= hour (0-23)	 
		%m	= month of the year
		%r	= return address of sender
	   	%s	= subject of message
	   	%S	= "Re: subject of message"  (only add Re: if not there)
		%t	= hour:minute 	
		%y	= year		  
	    or simply copies word into buffer.
	**/

	struct tm *localtime(), *timerec;
	long     time(), thetime;
	register int i, j=0, gotten_time = 0, reading_a_percent_sign = 0;

	for (i = 0; i < strlen(word); i++) {
	  if (reading_a_percent_sign) {
	    reading_a_percent_sign = 0;
	    switch (word[i]) {

	      case 'r' : buffer[j] = '\0';
			 strcat(buffer, from);
	                 j = strlen(buffer);
			 break;

	      case 's' : buffer[j] = '\0';
			 strcat(buffer, subject);
	                 j = strlen(buffer);
			 break;

	      case 'S' : buffer[j] = '\0';
			 if (! the_same(subject, "Re:")) 
			   strcat(buffer, subject);
			 strcat(buffer, subject);
	                 j = strlen(buffer);
			 break;

	      case 'd' : get_the_time(); buffer[j] = '\0';
			 strcat(buffer, itoa(timerec->tm_mday,FALSE));
	                 j = strlen(buffer);
			 break;

	      case 'D' : get_the_time(); buffer[j] = '\0';
			 strcat(buffer, itoa(timerec->tm_wday,FALSE));
	                 j = strlen(buffer);
			 break;

	      case 'm' : get_the_time(); buffer[j] = '\0';
			 strcat(buffer, itoa(timerec->tm_mon,FALSE));
	                 j = strlen(buffer);
			 break;

	      case 'y' : get_the_time(); buffer[j] = '\0';
			 strcat(buffer, itoa(timerec->tm_year,FALSE));
	                 j = strlen(buffer);
			 break;

	      case 'h' : get_the_time(); buffer[j] = '\0';
			 strcat(buffer, itoa(timerec->tm_hour,FALSE));
	                 j = strlen(buffer);
			 break;

	      case 't' : get_the_time(); buffer[j] = '\0';
			 strcat(buffer, itoa(timerec->tm_hour,FALSE));
			 strcat(buffer, ":");
			 strcat(buffer, itoa(timerec->tm_min,TRUE));
	                 j = strlen(buffer);
			 break;

	      default  : fprintf(stderr,
   "%sfilter (%s): Error on line %d translating %%%c macro in word \"%s\"!\n",
			         BEEP, username, line, word[i], word);
			 exit(1);
	    }
	  }
	  else if (word[i] == '%') 
	    reading_a_percent_sign++;
	  else 
	    buffer[j++] = word[i];
	}
	buffer[j] = '\0';
}

print_rules()
{
	/** print the rules out.  A double check, of course! **/

	register int i = -1;
	char     *whatname(), *actionname();
	struct   condition_rec *cond;

	while (++i < total_rules) {
	  printf("\nRule %d:  if (", i+1);

	  cond = rules[i].condition;

	  while (cond != NULL) {
	    if (cond->relation < 0)
	      printf("not %s %s %s%s%s", 
		      whatname(cond->matchwhat),
		      relationname(- (cond->relation)),
		      quoteit(cond->matchwhat),
		      cond->argument1,
		      quoteit(cond->matchwhat));
	    else
	      printf("%s %s %s%s%s",
		      whatname(cond->matchwhat),
		      relationname(cond->relation),
		      quoteit(cond->matchwhat),
		      cond->argument1,
		      quoteit(cond->matchwhat));

	    cond = cond->next;

	    if (cond != NULL) printf(" and ");
	  }
	    
	  printf(") then\n\t  %s %s\n", 
		 actionname(rules[i].action), 
		 rules[i].argument2);
	}
	printf("\n");
}

char *whatname(n)
int n;
{
	static char buffer[10];

	switch(n) {
	  case FROM   : return("from");
	  case TO     : return("to");
	  case SUBJECT: return("subject");
	  case LINES  : return ("lines");
	  case CONTAINS: return("contains");
	  default     : sprintf(buffer, "?%d?", n); return((char *)buffer);
	}
}

char *actionname(n)
int n;
{
	switch(n) {
	  case DELETE  : return("Delete");
	  case SAVE    : return("Save");
	  case SAVECC  : return("Copy and Save");
	  case FORWARD : return("Forward");
	  case LEAVE   : return("Leave"); 
	  case EXEC    : return("Execute");
	  default      : return("?action?");
	}
}

int
compare(line, relop, arg)
int line, relop;
char *arg;
{
	/** Given the actual number of lines in the message, the relop
	    relation, and the number of lines in the rule, as a string (!),
   	    return TRUE or FALSE according to which is correct.
	**/

	int rule_lines;

	rule_lines = atoi(arg);

	switch (relop) {
	  case LE: return(line <= rule_lines);
	  case LT: return(line <  rule_lines);
	  case GE: return(line >= rule_lines);
	  case GT: return(line >  rule_lines);
	  case NE: return(line != rule_lines);
	  case EQ: return(line == rule_lines);
	}
	return(-1);
}
