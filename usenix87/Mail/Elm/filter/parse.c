/**			filter_parse.c			**/

/** This is the parser for the filter program.  It accepts a wide variety of
    constructs, building the ruleset table as it goes along.  Check the 
    data structure in filter.h for more information on how the rules are
    stored.  The parser is a cunning state-table based program.

   (C) Copyright 1986, Dave Taylor
**/

#include <stdio.h>
#include <ctype.h>

#include "defs.h"
#include "filter.h"

#define NONE			0
#define AND			10

#define NEXT_CONDITION		0
#define GETTING_OP		1
#define READING_ARGUMENT	2
#define READING_ACTION		3
#define ACTION_ARGUMENT		4

char *strtok(), *whatname(), *actionname();

int
get_filter_rules()
{
	/** Given the users home directory, open and parse their rules table,
	    building the data structure as we go along.
	    returns -1 if we hit an error of any sort...
	**/

	FILE *fd;				/* the file descriptor     */
	char  buffer[SLEN], 			/* fd reading buffer       */
	      *str, 				/* ptr to read string      */
	      *word,				/* ptr to 'token'          */
	      filename[SLEN], 			/* the name of the ruleset */
	      action_argument[SLEN], 		/* action arg, per rule    */
	      cond_argument[SLEN];		/* cond arg, per condition */
	int   not_condition = FALSE, 		/* are we in a "not" ??    */
	      type=NONE, 			/* what TYPE of condition? */
	      lasttype, 			/* and the previous TYPE?  */
	      state = NEXT_CONDITION,		/* the current state       */
	      in_single, in_double, 		/* for handling spaces.    */
	      i, 				/* misc integer for loops  */
	      relop = NONE,			/* relational operator     */
	      action, 				/* the current action type */
	      line = 0;				/* line number we're on    */

	struct condition_rec	*cond, *newcond;

	sprintf(filename,"%s/%s", home, filterfile);

	if ((fd = fopen(filename,"r")) == NULL) {
	  fprintf(stderr,"filter (%s): Couldn't read user filter rules file!\n",
		  username);
	  return(-1);
	}

	cond_argument[0] = action_argument[0] = '\0';

	/* Now, for each line... **/

	if ((cond = (struct condition_rec *) 
		     malloc(sizeof(struct condition_rec))) == NULL) {
	  fprintf(stderr,"couldn't malloc first condition rec!\n");
	  return(-1);
	}
	
	rules[total_rules].condition = cond;	/* hooked in! */

	while (fgets(buffer, SLEN, fd) != NULL) {
	  line++;

	  if (buffer[0] == '#' || strlen(buffer) < 2)
	    continue;		/* nothing to look at! */

	  in_single = in_double = 0;

	  for (i=0; i < strlen(buffer); i++) {
	    if (buffer[i] == '"') 
	      in_double = ! in_double;
	    else if (buffer[i] == '\'')
	      in_single = ! in_single;
	    if ((in_double || in_single) && buffer[i] == ' ')
	      buffer[i] = '_';
	  }

	  lasttype = type;
	  type = NONE;
	  str = (char *) buffer;

	  /** Three pieces to this loop - get the `field', the 'relop' (if
	      there) then, if needed, get the argument to check against (not 
	      needed for errors or the AND, of course)
	  **/

	  while ((word = strtok(str, " ()[]:\t\n")) != NULL) {

	    str = (char *) NULL;		/* we can start stomping! */
	  
	    lowercase(word);

	    if (strcmp(word, "if") == 0) {	/* only ONE 'if' allowed */
	      if ((word = strtok(str, " ()[]:\t\n")) == NULL)	/* NEXT! */
	        continue;
	      lowercase(word);
	    }
	
	    if (state == NEXT_CONDITION) {
	      lasttype = type;
	      type = NONE;

	      if (the_same(word, "not") || the_same(word, "!")) {
	        not_condition = TRUE;
	        if ((word = strtok(str, " ()[]'\"\t\n")) == NULL)
	          continue;
	      }

	           if (the_same(word, "from")) 	    type = FROM;
	      else if (the_same(word, "to")) 	    type = TO;
	      else if (the_same(word, "subject"))   type = SUBJECT;
	      else if (the_same(word, "lines"))     type = LINES;
	      else if (the_same(word, "contains"))  type = CONTAINS;
	      else if (the_same(word, "and") || 
	               the_same(word, "&&")) 	    type = AND;

	      else if (the_same(word,"?") || the_same(word, "then")) {

		/** shove THIS puppy into the structure and let's continue! **/

	        if (lasttype == AND) {
	          fprintf(stderr,
       "%sfilter (%s): Error reading line %d of rules - badly placed \"and\"\n",
		  BEEP, username, line);
		  return(-1);
	        }

		cond->matchwhat = lasttype;
	        if (relop == NONE) relop = EQ;	/* otherwise can't do -relop */
	        cond->relation  = (not_condition? - (relop) : relop);

		for (i=0;i<strlen(cond_argument);i++)
	          if (cond_argument[i] == '_') cond_argument[i] = ' ';

		strcpy(cond->argument1, cond_argument);
	        if ((newcond = (struct condition_rec *)
		     malloc(sizeof(struct condition_rec))) == NULL) {
	          fprintf(stderr,"Couldn't malloc new cond rec!!\n");
		  return(-1);
	        }
	        cond->next = NULL;

	        relop = EQ;	/* default relational condition */

	        state = READING_ACTION;
	        if ((word = strtok(str, " ()[]'\"\t\n")) == NULL)
	          continue;
	        goto get_outta_loop;
	      }

	      if (type == NONE) {
	        fprintf(stderr,
      "%sfilter (%s): Error reading line %d of rules - field \"%s\" unknown!\n",
		     BEEP, username, line, word);
		return(-1);
	      }

	      if (type == AND) {

		/** shove THIS puppy into the structure and let's continue! **/

		cond->matchwhat = lasttype;
	        cond->relation  = (not_condition? - (relop) : relop);
		strcpy(cond->argument1, cond_argument);
	        if ((newcond = (struct condition_rec *)
	             malloc(sizeof(struct condition_rec))) == NULL) {
	          fprintf(stderr,"Couldn't malloc new cond rec!!\n");
		  return(-1);
	        }
	        cond->next = newcond;
		cond = newcond;
		cond->next = NULL;

	        not_condition = FALSE;
	        state = NEXT_CONDITION;
	      }
	      else {
	        state = GETTING_OP;
	      }
	    }

get_outta_loop: 	/* jump out when we change state, if needed */

	    if (state == GETTING_OP) {

	       if ((word = strtok(str, " ()[]'\"\t\n")) == NULL)
	         continue;

	       lowercase(word);

	       relop = NONE;

	       if (the_same(word, "=") || the_same(word, "in") || 
                   the_same(word, "contains")) {
                 state = READING_ARGUMENT;
	         relop = EQ;
	       }
	       else {
	         if (the_same(word, "<=")) 	relop = LE;
	         else if (the_same(word, ">="))	relop = GE;
	         else if (the_same(word, ">"))	relop = GT;
	         else if (the_same(word, "<>")||
		          the_same(word, "!="))	relop = NE;
	         else if (the_same(word, "<"))	relop = LT;

		 /* maybe there isn't a relop at all!! */

		 state=READING_ARGUMENT;

	       }
	    }
		 
	    if (state == READING_ARGUMENT) {
	      if (relop != NONE) {
	        if ((word = strtok(str, " ()[]'\"\t\n")) == NULL)
	          continue;
	      }
	      for (i=0;i<strlen(word);i++)
	        if (word[i] == '_') word[i] = ' ';

	      strcpy(cond_argument, word);
	      state = NEXT_CONDITION;
	    }

	    if (state == READING_ACTION) {
	      action = NONE;

	      not_condition = FALSE;

	      if (the_same(word, "delete"))       action = DELETE;
	      else if (the_same(word, "savec"))   action = SAVECC;
	      else if (the_same(word, "save"))    action = SAVE;
	      else if (the_same(word, "forward")) action = FORWARD;
	      else if (the_same(word, "exec"))    action = EXEC;
	      else if (the_same(word, "leave"))   action = LEAVE;
	      else {
	        fprintf(stderr,
	"%sfilter (%s): Error on line %d of rules - action \"%s\" unknown\n",
			BEEP, username, line, word);
	      }

	      if (action == DELETE || action == LEAVE) {
	        /** add this to the rules section and alloc next... **/

	        rules[total_rules].action = action;
		rules[total_rules].argument2[0] = '\0';	/* nothing! */
	        total_rules++;
	         
	        if ((cond = (struct condition_rec *)
		     malloc(sizeof(struct condition_rec))) == NULL) {
	          fprintf(stderr,"couldn't malloc first condition rec!\n");
	          return(-1);
	        }
	
	        rules[total_rules].condition = cond;	/* hooked in! */
	        state = NEXT_CONDITION;	
	      }
	      else {
	        state = ACTION_ARGUMENT;
	      }

	      if ((word = strtok(str, " ()[]'\"\t\n")) == NULL)
	        continue;

	    }
	
	    if (state == ACTION_ARGUMENT) {
	      strcpy(action_argument, word);

	      /** add this to the rules section and alloc next... **/

	      rules[total_rules].action = action;
	      expand_macros(action_argument, rules[total_rules].argument2,line);
	      total_rules++;
	         
	      if ((cond = (struct condition_rec *)
		     malloc(sizeof(struct condition_rec))) == NULL) {
	        fprintf(stderr,"couldn't malloc first condition rec!\n");
	        return(-1);
	      }
	
	      rules[total_rules].condition = cond;	/* hooked in! */

	      state = NEXT_CONDITION;
	      if ((word = strtok(str, " ()[]'\"\t\n")) == NULL)
	        continue;
	    }
	  }
	}

	return(0);
}
