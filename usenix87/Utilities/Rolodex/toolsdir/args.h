/* You must include "basics.h" to use this */

/* To use this package, first call get_args, then call the various other */
/* routines to see which options were given, etc. */

#define NIL 0                          /* null pointer  */

#define ARG_ERROR -1
#define NO_ARGS 0
#define ARGS_PRESENT 1

#define MAX_OPTIONS 52                 /* a-z A-Z */
#define NO_OPTION -1

/* an argument and the option it is associated with */

typedef struct argument {
        char *option;
        int option_index;
        struct argument *next;
} Cmd_Arg, *Ptr_Cmd_Arg;

/* all the arguments (in a list) and a toggle for every possible option */

typedef struct {
        Ptr_Cmd_Arg non_dash_arg_list; 
        int dash_options[MAX_OPTIONS];
} Cmd_Line, *Ptr_Cmd_Line;

/*--------------------------------------------------------------------------*/
        
extern char *malloc();

extern int get_args();

        /* int argc; char **argv; Bool dup_error; Bool print_msg; */
        /* returns one of ARG_ERROR, NO_ARGS, or ARGS_PRESENT */
        /* if dup_error, then having two identical options on the command */
        /* line will cause an error.  If print_msg, then  any error that */
        /* is noticed is printed out to stderr.  If !dup_error then */
        /* 'foo -a 3 4 -b -a 5' is equivalent to 'foo -a 3 4 5 -b' */
        
        
extern Bool any_option_present();

        /* no arguments */

extern Bool option_present();

        /* char achar; */

extern char * option_arg();

        /* char achar; int n; */

extern char * non_option_arg();

        /* int n; */

extern int n_option_args();

        /* char achar; */

extern int n_non_option_args();

        /* no arguments */

extern int n_non_dash_args();

        /* no arguments */

extern Bool check_option_args();        

        /* char achar; int min; int max; */

#define ALL_LEGAL 0

extern char legal_options();

        /* char *legaloptions; */
        /* legaloptions should be a string of characters all in the range */
        /* a-zA-Z.   Returns ALL_LEGAL if every option parsed in included */
        /* in the legaloptions string, otherwise returns the first option */
        /* character not in the string. */

extern set_option();

        /* char achar */

extern error_message();

        /* char *progname; char **argv; int index; char *usage; */

extern print_args();

        /* debugging routine */
