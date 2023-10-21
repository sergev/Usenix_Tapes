#include <stdio.h>
#include <ctype.h>
#include "basics.h"
#include "args.h"        
#include "sys5.h"

/***************************************************************************/
/***************************************************************************/

           /*****     COMMAND LINE ARGUMENT PARSER    *****/

/***************************************************************************/
/***************************************************************************/

/* Author: JP Massar */

/* parses command line arguments in argv under the following rules: */
/* any argument not beginning with '-' is treated as a unit. */
/* any argument beginning with a '-' must have the remaining characters */
/* be in the set {a-zA-Z}.  Each character is considered a separate option. */
/* (Thus -abc is equivalent to -a -b -c).  Non-dashed arguments are */
/* associated with the option that precedes them, e.g, 'cat -a foo -b bar' */
/* has foo associated with 'a' and bar associated with 'b'.  A non-dashed */
/* argument preceding any option is not associated with any option. */
/* users can specify whether duplicate options are errors. */

/* Non-dashed arguments are henceforth referred to as arguments, and */
/* dashed arguments are henceforth referred to as options. */
/* Arguments are ordered from left to right. */

/* The following routines are available to users: */

/* get_args()           called to parse argv.  Detects syntactic errors */
/* any_option_present() are any options present in the command line? */
/* option_present()     is an option present? */
/* option_arg()         returns an argument associated with an option */
/* non_option_arg()     returns an argument not associated with any option */
/* non_dash_arg()       returns an argument */
/* n_option_args()      returns number of args associated with an option */
/* n_non_option_args()  rtns number of args not associated with any option */
/* n_non_dash_args()    returns number of arguments. */
/* check_option_args()  checks bounds on number of args assoc with an option */
/* legal_options()      checks that options provided are a subset of a-zA-Z */
/* set_option()         turns on option */
/* error_message()      prints out an illegal syntax error message to stderr */


int option_to_index (achar) char achar;
{
  if (isupper(achar)) return(achar - 'A');
  if (islower(achar)) return(achar - 'a' + 26);
  return(NO_OPTION);
}

char index_to_option (index) int index;
{        
  if (index < 26) return('A' + index);
  return('a' + index - 26);
}  


/* the command line arguments are parsed into Cmd when get_args returns */
/* successfully */

static Ptr_Cmd_Line Cmd = NIL;

int get_args (argc, argv, dup_error, print_msg)
        
  /* Returns one of NO_ARGS, ARG_ERROR, or ARGS_PRESENT */

  int argc;
  char **argv;
  Bool print_msg, dup_error;

{
  int i,j,dash_index;
  Ptr_Cmd_Arg arg,last = NIL;
  char echar, *optr;

  Cmd = (Ptr_Cmd_Line) malloc(sizeof(Cmd_Line));
  Cmd -> non_dash_arg_list = NIL;
  for (j = 0; j < MAX_OPTIONS; j++) (Cmd -> dash_options)[j] = F;

  if (argc == 1) return(NO_ARGS);

  i = 0;
  dash_index = NO_OPTION;
  
  while (++i < argc) {
        
        /* parse arguments (i.e., anything not beginning with '-' */
        
        if (argv[i][0] != '-') {
                arg = (Ptr_Cmd_Arg) malloc(sizeof(Cmd_Arg));
                arg -> option = argv[i];
                arg -> option_index = dash_index;
                arg -> next = NIL;
                if (last == NIL) {
                        Cmd -> non_dash_arg_list = arg;
                        last = arg;
                }                
                else {
                        last -> next = arg;
                        last = arg;
                }
                continue;
        }

        /* parse options. '-' by itself is illegal syntax */
        
        if (strlen(argv[i]) < 2) {
                echar = '-';
                goto parse_error;
        }        
        optr = argv[i];
        optr++;
        while (*optr != '\0') {
                if (NO_OPTION == (dash_index = option_to_index(*optr))) {
                        echar = *optr;
                        goto parse_error;
                };
                if ((Cmd -> dash_options)[dash_index] && dup_error) {
                        echar = *optr;
                        goto duplicate_error;
                }
                (Cmd -> dash_options)[dash_index] = T;
                optr++;
        }

  }

  return(ARGS_PRESENT);

  parse_error :

  if (print_msg) fprintf(stderr,"illegal option: %c\n",echar);
  return(ARG_ERROR);

  duplicate_error:

  if (print_msg) fprintf(stderr,"duplicate option: %c\n",echar);
  return(ARG_ERROR);

}  


Bool option_present (achar) char achar;
{        
  return((Cmd -> dash_options)[option_to_index(achar)]);
}


Bool any_option_present ()
{
  int j;
  for (j = 0; j < MAX_OPTIONS; j++) {
      if ((Cmd -> dash_options)[j]) return(T);
  }      
  return(F);
}  

  
char * get_option_arg (i,n) int i; int n;

  /* get the nth option associated with the option whose index is 'i' */
        
{
  int count;
  Ptr_Cmd_Arg args;        
  args = Cmd -> non_dash_arg_list;  
  count = 0;
  while (args != NIL) {
        if (i == args -> option_index && ++count == n) {
                return(args -> option);
        }
        args = args -> next;
  }
  return(NIL);
}  


char * option_arg (achar,n) char achar; int n;
{
  return(get_option_arg(option_to_index(achar),n));
}


char * non_option_arg (n) int n;
{
  return(get_option_arg(NO_OPTION,n));
}  


char * non_dash_arg (n) int n;

{
  int count = 0;
  Ptr_Cmd_Arg arg;
  arg = Cmd -> non_dash_arg_list;
  while (arg != NIL) {
        if (++count == n) return(arg -> option);
        arg = arg -> next;
  }
  return(NIL);
}

print_args ()

  /* debugging routine which prints out the Cmd structure in readable form */

{
  int j,i,n;
  char *option,ochar;

  if (Cmd == NIL) {
        printf("\n\nNo arguments\n\n");
        return;
  }        
  
  printf("\n\narguments not associated with options: ");
  n = 1;
  while (T) {
        if (NIL == (option = non_option_arg(n++))) break;
        printf("%s ",option);
  }
  printf("\n");

  printf("\n\noptions and their arguments:\n\n");
  for (j = 0; j < MAX_OPTIONS; j++) {
      ochar = index_to_option(j);
      if (option_present(ochar)) {
         printf("%c : \t",ochar);
         i = 1;
         while (T) {
           if (NIL == (option = option_arg(ochar,i++))) break;
           printf("%s ",option);
         }
         printf("     \t(# is %d)",n_option_args(ochar));
         printf("\n");
      }
  }

  printf("\nnumber of non-dashed args is: %d\n",n_non_dash_args());
  printf("number of non-option args is  : %d\n",n_non_option_args());
  
}


#define ALL -1
#define NON_OPTION -2

int arg_counter (type) int type;

  /* general routine which counts arguments */
  /* if type isn't ALL or NON_OPTION then type is an index of an option */

{
  int index,count;
  Ptr_Cmd_Arg arg;
  arg = Cmd -> non_dash_arg_list;        
  count = 0;
  index = (type == NON_OPTION) ? NO_OPTION : type;
  while (arg != NIL) {
        if (type == ALL) {
                count++;
        }
        else if (arg -> option_index == index) count++;
        arg = arg -> next;
  }
  return(count);
}

int n_option_args (achar) char achar;
{        
  return(arg_counter(option_to_index(achar)));
}

int n_non_option_args ()
{
  return(arg_counter(NON_OPTION));
}

int n_non_dash_args ()
{
  return(arg_counter(ALL));
}


set_option (achar) char achar;
{
  (Cmd -> dash_options)[option_to_index(achar)] = T;
}


error_message (progname, argv, i, usage)
        char *progname; char ** argv; int i; char *usage;
{
  fprintf(stderr,"\nillegal argument to %s : %s\n",progname,argv[i]);
  if (usage) fprintf(stderr,"%s\n",usage);
}


Bool
check_option_args (achar,themin,themax) char achar; int themin,themax;
{
  int n;
  if (themin > themax) return(T);
  n = n_option_args(achar);
  return ((Bool) (n >= themin && n <= themax));
}


char legal_options (legalstring) char *legalstring;

  /* are all the options the user specified characters in legalstring? */
  /* returns ALL_LEGAL if so, otherwise the first option not in the string */

{
  int j;
  char option, *s;
  for (j = 0; j < MAX_OPTIONS; j++) {
      if ((Cmd -> dash_options)[j]) {
         option = index_to_option(j);
         s = legalstring;
         while (T) {
               if (*s == '\0') return(option);
               if (*s == option) break;
               s++;
         }
      }
  }
  return(ALL_LEGAL);
}
