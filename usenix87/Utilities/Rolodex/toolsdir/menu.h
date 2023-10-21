/* You must include "<ctools.h>" before including this file. */

#define MENU_EOF -1
#define MENU_MATCH 0
#define MENU_AMBIGUOUS 1
#define MENU_NO_MATCH 2        
#define MENU_ERROR 3
#define MENU_RETURN 4

#define MAX_MENU_OPTIONS 20
#define MAX_MENU_RESPONSE_LENGTH 255


extern int menu_match();

/* menu_match (

        int *ptr_rval,
        char **ptr_user_response,
        char *prompt,
        int case_significant,
        int initial_substring_sufficient,
        int repeat_if_no_match,
        int repeat_if_ambiguous,
        int n_options,
        char *option1, int rval1,
        char *option2, int rval2,
        .
        .
        .

    );

    Returns one of MENU_MATCH, MENU_AMBIGUOUS, MENU_NO_MATCH, and MENU_ERROR.
    If MENU_MATCH is returned, *ptr_rval contains the value that the caller
    asked to be returned.  *ptr_user_response is what exactly what the user
    typed in, minus the linefeed at the end.
    
    prompt is a character string that is printed out.  A line of input is
    then read from the terminal, stripped of excess blanks, and compared
    with the menu items.  Up to MAX_MENU_OPTIONS can be specified.  Each
    option is a character string and has an associated integer return value.
    
    If case_significant is not zero, the user's response will be checked
    against the options with upper/lower case distinction. 
    If initial_substring_sufficient is not zero, then a match will occur
    if the user's response is the same as an initial substring of an option,
    otherwise the response must match the option exactly.
    
    If more than one menu option matches the response, MENU_AMBIGUOUS is
    returned.  If n_options exceeds MAX_MENU_OPTIONS or the read fails
    MENU_ERROR is returned.  If no option matches the response, MENU_NO_MATCH
    is returned.  However, if repeat_if_no_match and/or repeat_if_ambiguous
    are non_zero, the user is prompted again until he answers correctly if
    he answers wrongly/ambiguously, respectively.
    
    If "" is not specified as an option, and the user enters a blank line,
    the routine simply prompts the user again.  It is impossible to match
    on a string of blanks (e.g., "  "), because any blank line the user
    types is considered to be "".
    
    If the response the user types is too long, an error message is printed
    out and the user is asked to try again.
    
*/


#define MENU_NO 0
#define MENU_YES 1
#define MENU_HELP 2
#define MENU_ABORT 3
#define MENU_DATA 4

extern int menu_yes_no();

/* menu_yes_no (prompt,allow_help) char *prompt; int allow_help;

        returns one of MENU_NO, MENU_YES or, if help is allowed,
        MENU_HELP.  Help can be obtained via "?" or any initial
        substring of "Help", case independently.  Returns MENU_EOF
        if end of file encountered when reading. 

        Yes or no can be obtained via any case independent initial
        substring of "Yes" or "No" respectively.

*/


#define DEFAULT_YES 0
#define DEFAULT_NO 1
#define NO_DEFAULT 2

extern int menu_yes_no_abort_or_help ();

/*  menu_yes_no_abort_or_help (prompt,abortstring,helpallowed,return_for_yes)

    char *prompt, *abortstring;
    int helpallowed; 
    int return_for_yes;

    Returns one of MENU_YES, MENU_NO, MENU_ABORT, MENU_HELP or MENU_EOF.
    If !helpallowed, MENU_HELP will not be returned.  
    If return_for_yes is DEFAULT_NO, hitting return will re-prompt.
    If it is DEFAULT_YES, hitting return is like typing yes.
    If it is any other value, hitting return is like typing no.

*/    

extern int menu_data_help_or_abort ();

/*

extern int menu_data_help_or_abort (prompt,abortstring,ptr_response)

  char *prompt, *abortstring, **ptr_response;

  Returns either MENU_ABORT, MENU_HELP or MENU_DATA.  If MENU_DATA is
  returned, *response contains the user's response.  MENU_HELP is
  returned in the user types in "?" or any initial substring of "Help"
  (case independently).
  
*/  
  

extern int menu_number_help_or_abort ();

/*
menu_number_help_or_abort (prompt,abortstring,low,high,ptr_ival)

  char *prompt; *abortstring;
  int low,high,*ptr_ival;

  Returns either MENU_DATA, MENU_ABORT, MENU_HELP or MENU_EOF.
  If low > high any number will be considered valid, otherwise the
  range is checked, inclusivewise.  If MENU_DATA is returned, *ptr_ival
  contains the number entered.

  At the moment number can only be non-negative.
*/
