#include <stdio.h>
#include <ctype.h>

#include "sys5.h"

#ifdef TMC
#include <ctools.h>
#else
#include "ctools.h"
#endif
#include "args.h"
#include "menu.h"
#include "mem.h"

#include "rolofiles.h"
#include "rolodefs.h"
#include "datadef.h"
#include "choices.h"


char *select_search_string ()

/* returns 0 if user wants to quit, otherwise returns a user-provided string */

{
  int rval;        
  char *response;
  rval = rolo_menu_data_help_or_abort (        
              "Enter string to search for: ",
              "searchstringhelp",
              "string to search for",
              &response
          );
  switch (rval) {          
    case MENU_ABORT :
      return(0);
      break;
    case MENU_DATA :
      return(copystr(response));
      break;
  }
}  
  

select_field_to_search_by (ptr_index,ptr_name) int *ptr_index; char **ptr_name;

/* returns -1 if the user wishes to abort, otherwise returns 0. */
/* if the user wishes to search by a user-defined field, *ptr_index is OTHER */
/* and *ptr_name is the user-provided name of the field. */

{
  char *response;
  int nchoices = N_BASIC_FIELDS;
  int field_index,rval;
  
  redo :
  
  /* list out each basic field that the user can search by.   The user is */
  /* also given an option to search by a user-provided field.   At the */
  /* moment you cannot search by 'Date Updated' */
  
  display_field_names();  
  
  reask :
  
  rval = rolo_menu_number_help_or_abort (
       "Number of item to search by? ",
       1,nchoices,&field_index
    );
    
  switch (rval) {
        
    case MENU_ABORT :
      return(-1);
      break;
      
    case MENU_HELP :
      cathelpfile(libdir("fieldsearchhelp"),"entering search field",1);
      any_char_to_continue();
      goto redo;
      break;
      
    case MENU_DATA :
    
      if (field_index != nchoices) {
         *ptr_index = field_index - 1;
         *ptr_name = copystr(Field_Names[*ptr_index]);
         return(0);
      }
      
      /* the user wants to search by a user-specified field */
      
      else {
        
         reask2 :
        
         rval = rolo_menu_data_help_or_abort (
                    "Name of user-defined field? ",
                    "userfieldhelp",
                    "name of user field to search by",
                    &response
                 );
         switch (rval) {
           case MENU_ABORT :
             return(-1);
             break;
           case MENU_DATA :
             *ptr_index = OTHER;
             *ptr_name = copystr(response);           
             return(0);         
             break;
         }
      }
      break;
  }
  
}  
      

match_by_name_or_company (search_string,sslen) char *search_string; int sslen;

{
  char *name,*company;
  Ptr_Rolo_Entry entry;        
  Ptr_Rolo_List rlist;
  int count = 0;
  
  rlist = Begin_Rlist;
  while (rlist != 0) {  
    entry = get_entry(rlist);
    name = get_basic_rolo_field((int) R_NAME,entry);
    company = get_basic_rolo_field((int) R_COMPANY,entry);
    if (strncsearch(name,strlen(name),search_string,sslen) ||
        strncsearch(company,strlen(company),search_string,sslen)) {
       set_matched(rlist);
       count++;
    }
  }
  return(count);
  
}


match_link (rlink,field_index,field_name,fnlen,search_string,sslen)

  /* if a match is present, sets the 'matched' field in the link, and */
  /* returns 1, otherwise returns 0. */

  Ptr_Rolo_List rlink;
  int field_index;
  char *field_name;
  int fnlen;
  char *search_string;
  int sslen;
  
{
  Ptr_Rolo_Entry entry;        
  char *field;
  char name[100];
  int j;
        
  entry = get_entry(rlink);
  
  if (field_index == OTHER) {
     for (j = 0; j < get_n_others(entry); j++) {
         field = get_other_field(j,entry);
         while (*field != ':') *field++;
         *field = '\0';
         remove_excess_blanks(name,get_other_field(j,entry));
         *field++ = ':';
         if (0 != nocase_compare(name,strlen(name),field_name,fnlen)) {
            continue;
         }
         if (strncsearch(field,strlen(field),search_string,sslen)) {
            set_matched(rlink);
            return(1);
         }
     }
     return(0);
  }
  else {
     field = get_basic_rolo_field(field_index,entry);
     if (strncsearch(field,strlen(field),search_string,sslen)) {
        set_matched(rlink);
        return(1);
     }
     return(0);
  }

}


find_all_matches (field_index,field_name,search_string,ptr_first_match) 

  /* mark every entry in the rolodex which matches against the search_string */
  /* If the search_string is a substring of the data in the given field then */
  /* that is a match.  Return the number of matches.  If there are any */
  /* matches *ptr_first_match will contain the first matching link. */

  int field_index;
  char *field_name, *search_string;
  Ptr_Rolo_List *ptr_first_match;

{  
  char buffer[100];    
  int fnlen,sslen;
  int count = 0;
  Ptr_Rolo_List rlist = Begin_Rlist;
  
  remove_excess_blanks(buffer,field_name);
  fnlen = strlen(buffer);
  sslen = strlen(search_string);
  
  while (rlist != 0) {  
    unset_matched(rlist);
    if (match_link(rlist,field_index,buffer,fnlen,search_string,sslen)) {
       if (count++ == 0) *ptr_first_match = rlist;
    }
    rlist = get_next_link(rlist);
  }    
  
  return(count);
  
}


rolo_search_mode (field_index,field_name,search_string)

  int field_index;
  char *field_name;
  char *search_string;

{
  int rval,n,j,menuval,ival;
  char *response;
  Ptr_Rolo_List first_match,rmatch,rlist;

  /* mark every entry in the rolodex that satisfies the search criteria */
  /* and return the number of items so marked. */
  
  in_search_mode = 1;
  n = find_all_matches(field_index,field_name,search_string,&first_match);

  if (n == 0) {
     printf (
         "No match found for search string '%s' for field '%s'\n",
         search_string,
         field_name
      );
      sleep(2);
      goto rtn;
  }

  /* if the match is unique, just display the entry. */
  
  else if (n == 1) {
     display_entry(get_entry(first_match));
     switch (entry_action(first_match)) {
       case E_CONTINUE :
         printf("No further matches...\n");
         sleep(2);
         break;
       default :
         break;
     }
     goto rtn;
  }

  /* if there are too many matches to itemize them on a single small */
  /* screen, tell the user that there are lots of matches and suggest */
  /* he specify a better search string, but give him the option of */
  /* iterating through every match. */
  
  else if (n > MAXMATCHES) {
     clear_the_screen();
     printf("There are %d entries that match '%s' !\n",n,search_string);
     printf("Type 'v' to view them one by one,\n");
     printf("or '\\' to abort and enter a more specific search string: ");
     rval = rolo_menu_data_help_or_abort (
                 "","manymatchhelp","many matching entries",&response
              );
     if (rval == MENU_ABORT) goto rtn;              
     display_list_of_entries(Begin_Rlist);
     goto rtn;
  }

  /* there are a small number of matching entries.  List the name of each */
  /* matching entry and let the user select which one he wants to view, */
  /* or whether he wants to iterate through each matching entry. */
  
  else {
     relist :
     summarize_entry_list(Begin_Rlist,search_string);
     cathelpfile(libdir("pickentrymenu"),0,0);  
     rval = menu_match (
          &menuval,&response,
          ": ",
          0,1,0,1,4,
          "\\",S_ABORT,
          "?",S_HELP,
          "Help",S_HELP,
          "",S_SCAN_ONE_BY_ONE
       );
     switch (rval) {    
       case MENU_MATCH :
         switch (menuval) {
           case S_HELP :
             cathelpfile(libdir("pickentryhelp"),"selecting entry to view",1);
             any_char_to_continue();
             goto relist;
             break;
           case S_ABORT :
             goto rtn;
             break;
           case S_SCAN_ONE_BY_ONE :
             display_list_of_entries(Begin_Rlist);
             goto rtn;
             break;
         }
         break;
         
       /* make sure the user entered a valid integer, ival */
       /* if so, find the ivalth entry marked as matched in the rolodex */
       /* and display it. */
         
       case MENU_NO_MATCH : 
         ival = str_to_pos_int(response,1,n);
         if (ival < 0) {
            printf("Not a valid number... Please try again\n");
            sleep(2);
            goto relist;
         }
         rlist = Begin_Rlist;
         for (j = 0; j < ival; j++) {
             while (rlist != 0) {
               if (get_matched(rmatch = rlist)) break;
               rlist = get_next_link(rlist);
             }
             if (rlist != 0) rlist = get_next_link(rlist);                
         }
         display_entry(get_entry(rmatch));
         switch (entry_action(rmatch)) {
           case E_CONTINUE :
           case E_PREV :
             goto relist;
             break;
           default :
             goto rtn;
             break;
         }
         break;
     }
  }

  rtn :
  in_search_mode = 0;
  
}
