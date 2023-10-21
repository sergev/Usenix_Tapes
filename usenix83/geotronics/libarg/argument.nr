.SA 1
.nr Pt 1
.TL
A Command Argument Processing Package
.AF ""
.AU "John Quarterman"
.MT 4
.H 1 "Introduction"
.P
This argument package allows the specification of formal arguments
for a UNIX*
.FS *
UNIX is a tradmark of Bell Laboratories.
.FE
C \fImain\fR routine, and the conversion of the actual arguments
of the program according to the formal arguments.  Ways are provided
to display the syntax of the command and to show the values of its arguments.
.P
The basic usage of the package is explained in the UNIX PROGRAMMER's MANUAL
entry, \fIargument\fR(3l).  That, in turn, refers to \fIintro\fR(1)
and \fIgetopt\fR(3),
which describe the UNIX System III command argument conventions.
All the System III conventions are supported by the package.
.P
This paper describes more specialized uses of the package than could
be treated in the manual entry.  The information in the manual entry
is assumed here and is not repeated in detail.
.H 1 "Definitions:  argument.h"
.P
The file <argument.h> contains the definitions needed to use the argument
package.  It requires <stdio.h> to be included first.
.DS I
Definitions:

#include <stdio.h>
#include <argument.h>
.DE
.P
Each specified formal argument has an instance of the typedef ARGUMENT
associated with it,
containing all relevant information about the formal argument.
This is the information passed by the specification routine, \fIarg_spec\fR.
Instances of ARGUMENTs should never occur outside the argument
package, as the structure may change.
.P
The \fIArg_Control\fR structure contains state information associated with
argument processing.  There is only one of these.
Usually, only argument package routines should modify it.
\fIArg_code\fR, described below under ``Control\ and\ Status,''
is used to set most parameters which should be changeable. 
.H 1 "Initialization and Actual Argument Specification"
.P
There are several routines for dealing with actual arguments.
.DS I
Initialization and Actual Argument Specification:

int \fImain\fR (argc, argv)
int argc;
char **argv;

int \fIarg_init\fR (&argc, &argv, help_main)
char *help_main[];

\fIprogname\fR = argv[0];
extern char *\fIprogname\fR;

int \fIargv_init\fR (&argc, &argv)
int \fIargv_eof\fR ()

int \fIargv_file\fR (file, prompt)
int \fIargv_line\fR (file)
FILE *file;
int (* prompt) ();

char *\fIargv_get\fR ()
char *\fIargv_unget\fR (actual)
char *actual;
.DE
.P 
Actual arguments are usually specified by \fIarg_init\fR, which
takes them from its first two arguments, which should be pointers
to argc and argv.  The third argument is a NULL-terminated array
of strings, containing a description of the command to be printed
by the "-help" standard option.  The format of this string array is
described under \fIarg_describe\fR in ``Printing functions'' below.
\fIArg_init\fR also specifies the standard options, which are
"-syn", "-help", "-show", and "--".
.P
If \fIarg_init\fR is not called, the name of the command must be given  
by setting \fIprogname\fR (declared in argument.h), usually to argv[0].
The actual arguments must be specified by \fIargv_init\fR or a similar
actual argument routine.
.P
\fIArgv_init\fR is the routine used by \fIarg_init\fR to specify
actual arguments.  Its arguments are pointers to argc and argv.
.P 
\fIArgv_eof\fR returns 1 iff there are no more actual arguments.
.P
\fIArgv_file\fR takes a standard IO (FILE *) as first argument,
a prompt function as second argument,
and initializes the actual argument routines to read arguments from the file.
These arguments are separated by white space,
which includes new lines.
Shell-style quotes and backslash escapes work, but there is no file name 
expansion.
If the file is a terminal, the prompt function will be called immediately
before each line is read from the file.  The prompt function is passed
no arguments and any value returned from it is ignored.
\fIArg_nprompt\fR (see below) is a commonly used prompt function.
.P
The \fIsformat\fR direct assignment format ARG_DSCAN should be used
with \fIargv_file\fR only with care,
as a pointer to the same buffer will be used repeatedly.
.P
\fIArgv_line\fR is like \fIargv_file\fR, except arguments are taken one per line.
The newline is stripped, but no other processing is done.
.P
\fIArgv_file\fR or \fIargv_line\fR may be used in
place of \fIargv_init\fR, or they may be used alternately to switch between
the arguments from the command line and the arguments from a file.
.P
\fIArgv_get\fR returns the next actual argument, or NULL if there are no more.
.P
\fIArgv_unget\fR ungets an actual argument, and returns it, as well.
At most one actual may be ungotten at a time.
.P
Use of \fIargv_get\fR or \fIargv_unget\fR will confuse the formal argument
routines.  Their descriptions are included for completeness.
.H 1 "Formal Argument Specification:  \fIarg_spec\fR"
.P
Formal arguments are specified by using the routine \fIarg_spec\fR.
The \fItype\fR argument is usually 0, the syntactical type of the argument
being inferred from the values of \fIflag\fR and \fIname\fR.
The value of the argument is kept in a variable pointed to by
\fIpvalue\fR, whose input conversion format is \fIsformat\fR
and whose display format is \fIpformat\fR.  \fIDesc\fR
is used in displaying the argument syntax to describe the argument.
\fICfunc\fR and \fIcfarg\fR may be a conversion function and its argument,
to be called when an actual argument is seen for the formal, if not NULL.
.DS
Formal argument specification:

int \fIarg_spec\fR (type, flag, name, sformat, pvalue
    , pformat, desc, cfunc, cfarg)
int  type;      /* syntactical argument type (usually 0) */
char *flag;     /* option flag (string starting with '-') */
char *name;     /* string naming the actual argument value */
char *sformat;  /* \fIscanf\fR format to convert the argument */
char *pvalue;   /* pointer to the formal argument variable */
char *pformat;  /* \fIprintf\fR format to display the argument */
char *desc;     /* string describing the argument */
int (* cfunc) (cfarg);  /* function to call on conversion */
char *cfarg;            /* argument for conversion function */
.DE
.P
\fIArg_spec\fR is actually a macro, so that it can cast \fIpvalue\fR
and \fIcfarg\fR to (char *), and in order to enforce the proper number 
of arguments.
.P
The kind of ordinary argument (fixed, possible, or variable)
being specified is determined
by the presence in \fIflag\fR of one of the ordinary flags,
or of the NULL pointer (which defaults to fixed type).
.P 
The syntactical type of an argument (any argument, not only ordinary
arguments) may be specified directly by using some combination
of the type defines found in argument.h.
\fIArg_spec\fR can be used to specify compound types, by oring
several types together.  An ordinary argument can be both ARG_FIXED
and ARG_VARIABLE, for example.  This would mean there must be at least
one such actual argument.
.DS
#defined argument types:

type mask       types

ARG_ORDINARY:   ARG_FIXED       ARG_POSSIBLE    ARG_VARIABLE    
ordinary flags: ARG_FFIX ""     ARG_FPOS "["    ARG_FVAR "[]"

ARG_OPTION:     ARG_SWITCH      ARG_FLAG        ARG_KEY
wild card options:      ARG_WFLAG "-?"          ARG_WKEY "?"

ARG_SPECIAL:    ARG_WILDCARD    ARG_NOCHECK     ARG_DEFAULT
.DE 
.P
The last ARGUMENT converted may be found in \fIArg_Control\fR.\fIArg_Last\fR,
as for \fIarg_each\fR.
The ARGUMENT for the last formal argument processed is
\fIArg_Control\fR.\fIArg_Last\fR, and \fIArg_Control\fR.\fIArg_LType\fR
is the type under which it was processed.
The last actual argument used is in \fIArg_Control\fR.\fIArg_Actual\fR, 
and the last actual flag argument used is in \fIArg_Control\fR.\fIArg_Flag\fR.
.P
There are several special types, masked with ARG_SPECIAL, which
may only be used with options.
.P
Any option, not just ARG_WKEY or ARG_WFLAG,
may be made a wild card by the special type ARG_WILDCARD.
An argument function must be specified for use with this,
and will be called whenever an unrecognizable switch, flag, or key is seen.
.P
The argument package cannot distinguish between unrecognized flags and switches,
and always reports errors as switches.
Thus, if option is the third argument to the ARG_WILDCARD function for
ARG_SWITCH, and (option[1] == \fIArg_Control\fR.\fIArg_Flag\fR[1]),
\fIArg_Control\fR.\fIArg_Flag\fR
could be a flag, or it could be a sequence of switches, of which 
option is the first.  Setting \fIArg_Control\fR.\fIArg_Switch\fR = NULL
will force
the package to treat it as a flag, else it will be taken to be switches.
.P
The remaining special types are not recommended, and are included
only for completeness.
.P
If the special type ARG_DEFAULT is used with an option which takes a value,
the option must be of type ARG_TTYPE, and of formats ARG_TFORMAT.
If there is no value for the option found on the command line
(the next actual is an option, or there are no more actuals), 
the value will be defaulted to ARG_TON.
.P
The special type ARG_NOCHECK prevents the usual check to see
that prevents an actual containing an option from being used as
the value of a preceding option.
.P
A table of suggested correspondences between \fIscanf\fR and \fIprintf\fR formats,
for \fIsformat\fR and \fIpformat\fR, may be useful.
.DS I
\fIPrintf\fR formats corresponding to \fIscanf\fR formats,
for use with the argument package \fIarg_spec\fR routine.

\fIscanf\fR           \fIprintf\fR

""              ""

%d              %d %u
%o              %o
%x              %x %X

%hd             %d
%ho             %o
%hx             %x %X

%ld %D          %ld %lu
%lo %O          %lo
%lx %X          %lx %lX

%s              %s
%c              %c

%f %e           %f %e %g %E %G
%lf %F %le %E   %f %e %g %E %G
.DE
.H 1 "Argument Processing:  \fIarg_process\fR"
.P
Arguments are usually processed by calling \fIarg_process\fR while
it returns a value > 0.
.DS I
Argument processing routines:

int \fIarg_process\fR ()
int \fIarg_each\fR ()
int \fIarg_all\fR ()
.DE
.P
\fIArg_each\fR processes one formal argument at a time.  It returns 1 for
ordinary successful processing, 2 for the last argument of a set, and 0
for end of processing.
.P
Since there may be more than one switch or key per actual argument, 
\fIarg_each\fR
may be called several times for the same actual argument, as it processes
one formal argument on each call.
.P
Calls on \fIarg_each\fR and \fIarg_process\fR may be interspersed.
.P 
\fIArg_all\fR returns 1 iff argument processing would be complete if there
were no more actual arguments.
.H 1 "Printing Routines (\fIarg_error\fR)"
.P
There are several printing routines.
.P
\fIArg_error\fR takes a pointer to an argument variable, a type, and
an error message, prints an appropriate message on stderr, then
exits with non-zero exit status.
.DS  I
Printing routines:

int \fIarg_error\fR (pvalue, type, message)
/* pvalue and type are as for \fIarg_spec\fR */
char *message;

int \fIarg_man\fR ()

int \fIarg_synopsis\fR ()
int \fIarg_nprompt\fR ()
int \fIarg_tprompt\fR (type) 

int \fIarg_help\fR ()
int \fIarg_describe\fR (strings)
char *strings[];

char \fIarg_replace\fR[],
     \fIarg_bracket\fR[],
     \fIarg_indirect\fR[],
     \fIarg_escape\fR[];

int \fIarg_show\fR ()   
.DE 
.P
\fIArg_init\fR sets up the "-syn", "-help", and "-show"
flags to call \fIarg_synopsis\fR, \fIarg_man\fR,
and \fIarg_show\fR, respectively.  There are also a few other printing
functions.  Those that print values of argument variables return 1,
and those that do not exit with zero status.
The ones that usually exit can be made to return 1 by setting
ARGNHEXIT with \fIarg_code\fR (see below).
.P
\fIArg_synopsis\fR prints a synopsis of the calling sequence of the command
in a format similar to that used in the SYNOPSIS sections of the manual,
and then exits.
.P
\fIArg_nprompt\fR prints a prompt, which is the same as the \fIarg_synopsis\fR 
output for the expected remaining ordinary arguments.  It is commonly
passed to \fIargv_file\fR as a prompt function.  \fIArgv_tprompt\fR
takes one argument, which is a type mask.  A synopsis is printed
of all the arguments whose types match the mask. 
Neither \fIarg_nprompt\fR nor \fIarg_tprompt\fR return any value,
and neither exits.
.P
\fIArg_man\fR uses \fIarg_synopsis\fR, \fIarg_help\fR,
and \fIarg_describe\fR to print a help message similar to a condensed
manual entry.  It then exits.
The argument is a NULL terminated array of strings to be passed to
\fIarg_describe\fR.
.P
\fIArg_help\fR prints the flag, name, format, and description of each
formal argument, and then exits.
The arguments are grouped by type, and a header 
is given before each type group.
.P
\fIArg_describe\fR prints a description of the command, taken from its
argument, which is an array of strings, i.e., an array of type (char *),
terminated by a NULL pointer.
The strings are normally output with no change.
Newlines and other formatting characters must be embedded if desired on output.
1 is returned.
.P
There are four special strings which can be used with \fIarg_describe\fR.
\fIArg_replace\fR is replaced by the value of \fIprogname\fR on output.
\fIArg_bracket\fR causes the next string to be enclosed in angle brackets,
<>, as for non-literal argument names in \fIarg_help\fR output.
\fIArg_indirect\fR causes the next array element to be treated as a pointer
to a string, and to be dereferenced before printing.
Multiple levels of indirection are possible.  An indirect string may be
bracketed using \fIarg_bracket\fR.
\fIArg_escape\fR causes the next array element to be treated as a function
pointer.  The function is called with a pointer to the following array
element as its argument.  It must return a string array (char **), which
is then used by \fIarg_describe\fR for further processing.
.P
\fIArg_show\fR is like \fIarg_help\fR, except
that the value of each formal argument is given in place of its description,
and there are no headers before the type groups.
The value 1 is returned.
.P
\fIArgv_get\fR returns NULL for EOF or error.
\fIArgv_unget\fR will print an error message
and exit if it is called twice with no intervening call on \fIargv_get\fR. 
.H 1 "Control and Status"
.P
There are several general control and status routines. 
.DS I
Control and status routines:

int \fIarg_status\fR ()

int \fIarg_code\fR (code)
int \fIarg_decode\fR (code)
int code;
codes:  ARG_ECHO    ARGNHEXIT   ARGNEEXIT

ARGUMENT *\fIarg_change\fR (arglist)
ARGUMENT *arglist;
.DE
.P
\fIArg_status\fR prints the state of the argument package, i.e., the
contents of \fIArg_Control\fR.
.P
The one element of \fIArg_Control\fR which may be directly assigned
to is \fIArg_Control\fR.\fIArg_SOpt\fR.
This is the formal argument variable for the "\-\-" standard option.
While it is 0, options are recognized wherever they are seen in
the command line.  When it is 1, options are not recognized at all.
If it is initially set to -1, options will be handled completely
compatibly with System III:  they will be recognized until "\-\-"
or the first ordinary argument is seen.
.P
\fIArg_code\fR is used to set parameters of the argument package.
Currently defined codes are ARG_ECHO, ARGNHEXIT, and ARGNEEXIT.
These codes are bit defines and may be ored together.
ARG_ECHO causes the value of the last converted argument to be printed.
ARGNHEXIT suppresses exits in \fIarg_help\fR, \fIarg_syn\fR, and \fIarg_man\fR,
making them return 1 instead.
ARGNEEXIT suppresses the exit in \fIarg_error\fR, making it return -1.
.P
\fIArg_decode\fR is normally called by the argument package after the complete
coversion of each argument, with the global special action code set by
\fIarg_code\fR from its argument.  \fIArg_decode\fR can be called directly.
.P
\fIArg_change\fR allows saving a list of formal argument specifications
for later use.  It returns the current ARGUMENT list, which is
then replaced by its argument arglist, which should be either NULL
or a previously saved argument list.  The state of the package is also
reset.  To reset the state and leave the argument list unchanged, do
\fIarg_change\fR (\fIarg_change\fR (NULL)).

