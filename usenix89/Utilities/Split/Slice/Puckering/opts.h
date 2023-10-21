
#define FALSE 0
#define TRUE 1
typedef int bool;

#define EXIT_SYNTAX 1	/* syntax error parsing commandline options */
#define EXIT_SEMANT 2	/* options are correct but meaningless */
#define EXIT_RUNERR 3	/* error opening a file, for example */
#define EXIT_INTERN 4	/* internal error -- bug!! */

#define nextstr(s,count,array,failure)	\
	{if (((count)<2) && !((array)[0][1])) {failure;}\
	else {if ((array)[0][1]) { s = &((array)[0][1]); } \
	      else {s = array[1]; --count; array++;}}}

#define DFLTNAME "slice"	/* input filename (for stdin) */
#define BUFLEN BUFSIZ	/* the maximum length of an input line (incl. "\n\0") */
#define MAXFILENAMELEN BUFSIZ	/* longer than the longest possible file name */
#define DFLTOUTNAME	"%s:%03.d"	/* o/p file name format */

