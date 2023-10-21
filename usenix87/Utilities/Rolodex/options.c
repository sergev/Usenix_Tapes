#include <sys/file.h>
#include <stdio.h>
#include <ctype.h>
#include <sgtty.h>
#include <sys/time.h>
#include <signal.h>

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


print_short ()

/* print the names and phone numbers of everyone in the rolodex. */

{ 

  Ptr_Rolo_List rptr;
  Ptr_Rolo_Entry entry;
        
  rptr = Begin_Rlist;
  if (rptr == 0) {
     fprintf(stderr,"No entries to print out...\n");
     return;
  }
   
  fprintf (
     stdout,
     "\nNAME                      WORK PHONE                HOME PHONE"
   );
  fprintf (
     stdout,
     "\n----                      ----------                ----------\n\n\n"
   );

   while (rptr != 0) {
        entry = get_entry(rptr);
        fprintf (
            stdout,
            "%-25s %-25s %-25s\n",
            get_basic_rolo_field((int) R_NAME,entry),
            get_basic_rolo_field((int) R_WORK_PHONE,entry),
            get_basic_rolo_field((int) R_HOME_PHONE,entry)
         );   
         rptr = get_next_link(rptr);
   }       
   
 }  
   

person_match (person,entry) char *person; Ptr_Rolo_Entry entry; 
 
/* Match against a rolodex entry's Name and Company fields. */
/* Thus if I say 'rolo CCA' I will find people who work at CCA. */
/* This is good because sometimes you will forget a name but remember */
/* the company the person works for. */

{ 
  char *name, *company;        
  int len;
  len = strlen(person);
  name = get_basic_rolo_field((int) R_NAME,entry);
  company = get_basic_rolo_field((int) R_COMPANY,entry);
  if (strncsearch(name,strlen(name),person,len)) return(1);
  if (strncsearch(company,strlen(company),person,len)) return(1);
  return(0);
}
 

int find_all_person_matches (person) char *person;

{
  Ptr_Rolo_List rptr = Begin_Rlist;        
  int count = 0;
  while (rptr != 0) {
    unset_matched(rptr);
    if (person_match(person,get_entry(rptr))) {
       set_matched(rptr);
       count++;
    }
    rptr = get_next_link(rptr);
  }
  return(count);
}


look_for_person (person) char *person; 

/* search against Name and Company over the rolodex.  If a match is found */
/* display the entry and give the user a choice of what to do next. */

{
  Ptr_Rolo_List rptr;        
  int found = 0,result,nmatches;
  static displayed_menu = 0;
  char *response;
  
  rptr = Begin_Rlist;        
  while (rptr != 0) {        
    if (person_match(person,get_entry(rptr))) {
       clear_the_screen();
       display_entry(get_entry(rptr));
       if (!found) {
          nmatches = find_all_person_matches(person);
          if (nmatches > 1) {
             printf (
                "There are %d other entries which match '%s'\n\n",
                nmatches - 1, person
              );
          }
       }
       found = 1;
       try_again :
       if (!displayed_menu) cathelpfile(libdir("poptionmenu"),0,0);
       displayed_menu = 1;
       menu_match (
            &result,
            &response,
            "Select option (? for help): ",
            0,1,1,1,6,
            "Continue",P_CONTINUE,
            "",P_CONTINUE,
            "Next",P_NEXT_PERSON,
            "\\",P_ABORT,
            "Help",P_HELP,
            "?",P_HELP
         );
       switch (result) {
         case P_CONTINUE :
           break;
         case P_NEXT_PERSON :
           return;
           break;
         case P_ABORT :
           roloexit(0);
           break;
         case P_HELP :
           cathelpfile(libdir("poptionshelp"),"person search options",1);
           goto try_again;
         default :
           fprintf(stderr,"Impossible return from menu_match\n");
           exit(-1);
       }
    }
    rptr = get_next_link(rptr);
  }
  if (!found) {
     fprintf(stderr,"\nNo entry found for '%s'\n\n",person);
     sleep(2);
  }
  else {
     printf("No further matches for '%s'\n",person);
     sleep(2);
  }   
}
 
 
print_people ()

{
  int index;
  char *person;
  index = 1;
  while (T) {
     if (0 == (person = non_option_arg(index++))) break;
     look_for_person(person);
  }
}


interactive_rolo ()

/* Top level of the iteractive rolodex.  This code is just a big switch */
/* which passes responsibility off to various routines in 'operations.c' */
/* and 'update.c' */

{

  int result,rval,field_index;
  char *response,*field_name,*search_string;

  fprintf(stdout,"\n\nTMC ROLODEX, Version %s\n\n\n",VERSION);        
  
  while (1) {        
        
    cathelpfile(libdir("mainmenu"),0,0);
    rval = menu_match (
         &result,
         &response,
         "Select option (? for help): ",
         0,1,0,1,7,
         "+",M_ADD,
         "%",M_PERUSE,
         "\\",M_EXIT,
         "?",M_HELP,
         "*",M_SAVE,
         "$",M_SEARCH_BY_OTHER,
         "!",M_PRINT_TO_LASER_PRINTER
      );
      
    switch (rval) {
        
      case MENU_AMBIGUOUS :
      case MENU_ERROR :
        fprintf(stderr,"Impossible return 1 from main menu_match\n");
        exit(-1);
        break;
        
      case MENU_NO_MATCH :
        response = copystr(response);
        rolo_search_mode((int) R_NAME,Field_Names[(int) R_NAME],response);
        break;
        
      case MENU_MATCH :
        
        switch (result) {
          case M_ADD :
            rolo_add();
            break;
          case M_SEARCH_BY_OTHER :
            if ((-1 == select_field_to_search_by(&field_index,&field_name)) ||
                (0 == (search_string = select_search_string()))) {
               break;
            }
            rolo_search_mode(field_index,field_name,search_string);
            break;
          case M_PRINT_TO_LASER_PRINTER :
            fprintf(stderr,"Not implemented\n");
            sleep(1);
            break;
          case M_PERUSE :
            rolo_peruse_mode(Begin_Rlist);
            break;
          case M_EXIT :
            save_and_exit(0);
            break;
          case M_SAVE :
            if (changed) {
               save_to_disk();
               sleep(1);
            }
            else {
               printf("No changes to be saved...\n");
               sleep(2);
            }
            break;
          case M_HELP :
            cathelpfile(libdir("moptionhelp"),"top level menu",1);
            any_char_to_continue();
            break;
          default :
            fprintf(stderr,"Impossible result from menu_match...\n");
            save_and_exit(-1);
        }
        break;
            
      case MENU_EOF :
        user_eof();
        break;
        
      default :
        fprintf(stderr,"Impossible return 2 from menu_match\n");
        save_and_exit(-1);
        
    }
    
    clear_the_screen();
    
  }

}
