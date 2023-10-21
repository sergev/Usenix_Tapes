
/*
 * GLOBALS.C
 *
 * (c)1986 Matthew Dillon     9 October 1986
 *
 * version 2.06M (Manx Version and Additions) by Steve Drew 28-May-87
 *
 *    Most global variables.
 *
 */


#include "shell.h"

struct HIST *H_head, *H_tail;			/* HISTORY lists      */

struct PERROR Perror[] = {			/* error code->string */
   103,	 "insufficient free storage",
   105,	 "task table full",
   120,	 "argument line invalid or too long",
   121,	 "file is not an object module",
   122,	 "invalid resident library during load",
   201,	 "no default directory",
   202,	 "object in use",
   203,	 "object already exists",
   204,	 "directory not found",
   205,	 "object not found",
   206,	 "bad stream name",
   207,	 "object too large",
   209,	 "action not known",
   210,	 "invalid stream component name",
   211,	 "invalid object lock",
   212,	 "object not of required type",
   213,	 "disk not validated",
   214,	 "disk write protected",
   215,	 "rename across devices",
   216,	 "directory not empty",
   217,	 "too many levels",
   218,	 "device not mounted",
   219,	 "seek error",
   220,	 "comment too long",
   221,	 "disk full",
   222,	 "file delete protected",
   223,	 "file write protected",
   224,	 "file read protected",
   225,	 "not a DOS disk",
   226,	 "no disk",
   232,	 "no more entries in directory",

   /* custom error messages */

   500,	 "bad arguments",
   501,	 "label not found",
   502,	 "must be within source file",
   503,	 "Syntax Error",
   504,	 "redirection error",
   505,	 "pipe error",
   506,	 "too many arguments",
   507,	 "destination not a directory",
   508,	 "cannot mv a filesystem",
     0,	 NULL
};


char  *av[MAXAV];	      /* Internal argument list			*/
long  Src_base[MAXSRC];	      /* file pointers for source files		*/
long  Src_pos[MAXSRC];	      /* seek position storage for same		*/
char  If_base[MAXIF];	      /* If/Else stack for conditionals		*/
int   H_len, H_tail_base;     /* History associated stuff		*/
int   H_stack;		      /* AddHistory disable stack		*/
int   E_stack;		      /* Exception disable stack		*/
int   Src_stack, If_stack;    /* Stack Indexes				*/
int   ac;		      /* Internal argc				*/
int   debug;		      /* Debug mode				*/
int   disable;		      /* Disable com. execution (conditionals)	*/
int   Verbose;		      /* Verbose mode for source files		*/
int   Lastresult;	      /* Last return code			*/
int   Exec_abortline;	      /* flag to abort rest of line		*/
int   Exec_ignoreresult;      /* flag to ignore result			*/
int   Quit;		      /* Quit flag				*/
long  Cout, Cin;	      /* Current input and output file handles	*/
long  Cout_append;	      /* append flag for Cout			*/
long  Uniq;		      /* unique value				*/
char  *Cin_name, *Cout_name;  /* redirection input/output name or NULL	*/
char  *Pipe1, *Pipe2;	      /* the two pipe temp. files		*/
struct Process *Myprocess;
int   S_histlen = 20;	      /* Max # history entries			*/
