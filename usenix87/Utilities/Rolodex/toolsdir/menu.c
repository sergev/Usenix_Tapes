#include <stdio.h>
#include <varargs.h>
#include "ctools.h"
#include "menu.h"
#include "sys5.h"


static char line[MAX_MENU_RESPONSE_LENGTH];


menu_match (

     ptr_rval,ptr_ur,prompt,casetest,isub,r_no_match,r_ambiguous,n_options,
     va_alist
     
   )

  int *ptr_rval;
  char **ptr_ur;
  char *prompt;
  int casetest;
  int isub;
  int r_no_match;
  int r_ambiguous;
  int n_options;
  va_dcl
    
{

  char sline[MAX_MENU_RESPONSE_LENGTH];
  char *option, *options[MAX_MENU_OPTIONS];
  int rvals[MAX_MENU_OPTIONS];
  int j,found,*addr,len,optionlen,rval,compare,blankindex;
  va_list pvar;
        
  if (n_options > MAX_MENU_OPTIONS) return(MENU_ERROR);
  for (j = 0; j < MAX_MENU_RESPONSE_LENGTH; j++) line[j] = ' ';
  
  /* grab all the menu options and return values.  */
  
  blankindex = -1;
  va_start(pvar);
  for (j = 0; j < n_options; j++) {
      options[j] = va_arg(pvar,char *);
      if (0 == strlen(options[j])) {
         if (blankindex == -1) blankindex = j;
      }
      rvals[j] = va_arg(pvar,int);
  }
  va_end(pvar);
  
  try_again :
  
  /* get the user's response */
  
  printf("%s",prompt);
  switch (rval = getline(stdin,line,MAX_MENU_RESPONSE_LENGTH)) {
    case AT_EOF :
      return(MENU_EOF);
      break;
    case TOO_MANY_CHARS :
     fprintf(stderr,"Response is too long to handle.  Please try again\n");
     goto try_again;
     break;
   default :
     *ptr_ur = line;
     remove_excess_blanks(sline,line);
     len = strlen(sline);
     break;
 }
    
  found = 0;
  rval = MENU_NO_MATCH;
  
  if (all_whitespace(sline)) {
     if (blankindex == -1) goto try_again;
     rval = MENU_MATCH;
     *ptr_rval = rvals[blankindex];
     goto rtn;
  }     
  
  for (j = 0; j < n_options; j ++) {

      /* if what he typed in is longer than any option it can't match */
        
      optionlen = strlen(options[j]);
      if (len > optionlen) continue;  
      
      /* if we aren't matching initial substrings, the response must */
      /* match exactly. */
      
      if (!isub && len != optionlen) continue;
      
      /* use different comparision functions depending on whether case */
      /* is important or not. */
      
      compare = casetest ? 
                strncmp(sline,options[j],len) : 
                nocase_compare(sline,len,options[j],len);
                
      /* if we must match exactly, if we find a match exit immediately. */
      /* if we can match on an initial substring, if we've already found */
      /* a match then we have an ambiguity, otherwise note that we've */
      /* matched and continue looking in case of ambiguities */
                
      if (0 == compare) {
         if (!isub) {
            found = 1;
            *ptr_rval = rvals[j];
            rval = MENU_MATCH;
            break;
         }
         if (found && isub) {
            rval = MENU_AMBIGUOUS;
            break;
         }
         else {
            found = 1;
            *ptr_rval = rvals[j];
            rval = MENU_MATCH;
         }
      }
      else {
         continue;
      }
  }

  rtn :
  
  switch (rval) {
    case MENU_MATCH :
      break;
    case MENU_NO_MATCH :
      if (r_no_match) {
         printf("Your response does not match any option.  Try again\n");
         goto try_again;
      }
      break;
    case MENU_AMBIGUOUS :
      if (r_ambiguous) {
         printf("Your response is ambiguous.  Try again\n");
         goto try_again;
      }
      break;
    default :
      fprintf(stderr,"Impossible case value in menu_match\n");
      exit(-1);
  }
      
  return(rval);
  
}


menu_yes_no (prompt,allow_help) char *prompt; int allow_help;

{
  int menuval,rval;
  char *response;
  
  redo :
  
  if (!allow_help) {
     rval = menu_match (
          &menuval,&response,
          prompt,
          0,1,0,0,2,
          "Yes",MENU_YES,
          "No",MENU_NO
       );
  }
  else {
     rval = menu_match (
          &menuval,&response,
          prompt,
          0,1,0,0,4,
          "Yes",MENU_YES,
          "No",MENU_NO,
          "?",MENU_HELP,
          "Help",MENU_HELP
       );
  }
  switch (rval) {
    case MENU_MATCH :
      return(menuval);
      break;
    case MENU_NO_MATCH :
      fprintf(stderr,"Please type 'Yes' or 'No'...\n");
      goto redo;
      break;
    case MENU_EOF :
      return(MENU_EOF);
      break;
    default :
      fprintf(stderr,"Fatal error:  Impossible return in menu_yes_no\n");
      exit(-1);
  }
}


extern int menu_data_help_or_abort (prompt,abortstring,ptr_response)

  char *prompt, *abortstring, **ptr_response;

{
  int rval,menuval;
  rval = menu_match (
       &menuval,ptr_response,
       prompt,
       0,1,0,1,3,
       abortstring,MENU_ABORT,
       "?",MENU_HELP,
       "Help",MENU_HELP
    );
  switch (rval) {
    case MENU_NO_MATCH :
      return(MENU_DATA);
      break;
    case MENU_MATCH :
      return(menuval);
      break;
    case MENU_EOF :
      return(MENU_EOF);
      break;
    default :
      fprintf(stderr,"Impossible return from menu_data_help_or_abort\n");
      exit(-1);
  }
}


menu_number_help_or_abort (prompt,abortstring,low,high,ptr_ival)

  char *prompt, *abortstring;
  int low,high,*ptr_ival;

{
  char *response,errprompt[80],numstring[MAX_MENU_RESPONSE_LENGTH];
  int rval,check;
  
  if (!(check = (low <= high)))
     nbuffconcat(errprompt,1,"Please enter a non-negative number...\n");
  else
     sprintf(errprompt,"Please enter a number between %d and %d\n",low,high);
  
  reask :     
     
  rval = menu_data_help_or_abort(prompt,abortstring,&response);
  switch (rval) {
    case MENU_EOF :
    case MENU_ABORT :
    case MENU_HELP :
      return(rval);
      break;
    case MENU_DATA :
      remove_excess_blanks(numstring,response);
      switch (*ptr_ival = str_to_pos_int(numstring,0,MAXINT)) {
        case -1:
        case -2:
          fprintf(stderr,"%s",errprompt);
          goto reask;
          break;
        default:
          return(MENU_DATA);
          break;
      }
  }

}


menu_yes_no_abort_or_help (prompt,abortstring,helpallowed,return_for_yes)

/*
    Returns one of MENU_YES, MENU_NO, MENU_ABORT or MENU_HELP.
    If !helpallowed, MENU_HELP will not be returned.  
    If return_for_yes is 0, hitting return will re-prompt.
    If it is 1, hitting return is like typing yes.
    If it is any other value, hitting return is like typing no.
*/

  char *prompt;
  char *abortstring;
  int helpallowed; 
  int return_for_yes;

{
  int menuval,rval;
  char *response;
  
  redo :
  
  if (!helpallowed) {
     rval = menu_match (
          &menuval,&response,
          prompt,
          0,1,0,0,4,
          "Yes",MENU_YES,
          "No",MENU_NO,
          abortstring,MENU_ABORT,
          "",MENU_RETURN
       );
  }
  else {
     rval = menu_match (
          &menuval,&response,
          prompt,
          0,1,0,0,6,
          "Yes",MENU_YES,
          "No",MENU_NO,
          "?",MENU_HELP,
          "Help",MENU_HELP,
          abortstring,MENU_ABORT,
          "",MENU_RETURN
       );
  }
  switch (rval) {
    case MENU_MATCH :
      if (menuval != MENU_RETURN) return(menuval);
      switch (return_for_yes) {
        case NO_DEFAULT :
          goto redo;
          break;
        case DEFAULT_YES :
          return(MENU_YES);
          break;
        default :
          return(MENU_NO);
          break;
      }
      break;
    case MENU_NO_MATCH :
      printf("Please type 'Yes' or 'No'...\n");
      goto redo;
      break;
    case MENU_EOF :
      return(MENU_EOF);
      break;
    default :
      fprintf(stderr,"Fatal error:  Impossible return in menu_yes_no\n");
      exit(-1);
  }
}

