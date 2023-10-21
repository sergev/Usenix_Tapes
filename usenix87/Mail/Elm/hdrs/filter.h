/**			filter.h			**/

/** Headers for the filter program.

   (C) Copyright 1986, Dave Taylor
**/

#ifdef   BSD
# undef  tolower
# define tolower(c)	(isupper(c)?  c = c - 'A' + 'a' : c)
#endif

/** define a few handy macros for later use... **/

#define  BEEP		(audible? "\007" : "")

#define  the_same(a,b)	(strncmp(a,b,strlen(b)) == 0)

#define relationname(x)  (x == 1?"<=":x==2?"<":x==3?">=":x==4?">":x==5?"!=":"=")

#define quoteit(x)	 (x == LINES? "" : "\"")

/** some of the files we'll be using, where they are, and so on... **/

#define  filter_temp	"/tmp/filter"
#define  filterfile	".filter-rules"
#define  filterlog	".filterlog"
#define  filtersum	".filtersum"

#define  EMERGENCY_MAILBOX	"EMERGENCY_MBOX"
#define  EMERG_MBOX		"MBOX.EMERGENCY"

/** and now the hardwired constraint of the program.. **/

#define  MAXRULES	25		/* can't have more den dis, boss! */

/** some random defines for mnemonic stuff in the program... **/

#define  TO		1
#define  FROM		2
#define  LINES		3
#define  CONTAINS	4

#define  DELETE 	5
#define  SAVE		6
#define  SAVECC		7
#define  FORWARD	8
#define  LEAVE		9
#define  EXEC		10

#define  FAILED_SAVE	20

/** Some conditionals... **/

#define LE		1
#define LT		2
#define GE		3
#define GT		4
#define NE		5
#define EQ		6

/** A funky way to open a file using open() to avoid file locking hassles **/

#define  FOLDERMODE	O_WRONLY | O_APPEND | O_CREAT | O_SYNCIO

/** cheap but easy way to have two files share the same #include file **/

#ifdef MAIN_ROUTINE
# define  extern
#endif

extern char home[SLEN],				/* the users home directory */
            hostname[SLEN],			/* the machine name...      */
            username[SLEN];			/* the users login name...  */

extern char to[VERY_LONG_STRING], 
            from[SLEN], 
            subject[SLEN];			/* from current message     */

#ifdef MAIN_ROUTINE
int  total_rules = 0,				/* how many rules to check  */
     show_only = FALSE,				/* just for show?           */
     long_summary = FALSE,			/* what sorta summary??     */
     verbose   = FALSE,				/* spit out lots of stuff   */
     audible   = FALSE,				/* be noisy with output?    */
     lines     = 0,				/* lines in message..       */
     clear_logs = FALSE,			/* clear files after sum?   */
     already_been_forwarded = FALSE,		/* has this been filtered?  */
     log_actions_only = FALSE,			/* log actions | everything */
     rule_choosen;				/* which one we choose      */
#else
extern int total_rules,				/* how many rules to check  */
           show_only,				/* just for show?           */
           long_summary,			/* what sorta summary??     */
           verbose,				/* spit out lots of stuff   */
           audible,				/* be noisy with output?    */
           lines,				/* lines in message..       */
           clear_logs,			        /* clear files after sum?   */
	   already_been_forwarded,		/* has this been filtered?  */
           log_actions_only,			/* log actions | everything */
           rule_choosen;			/* which one we choose      */
#endif

/** and our ruleset record structure... **/

struct condition_rec {
	int     matchwhat;			/* type of 'if' clause      */
	int     relation;			/* type of match (eq, etc)  */
	char    argument1[SLEN];		/* match against this       */
	struct  condition_rec  *next;		/* next condition...	    */
      };

extern struct ruleset_record {
	char  	printable[SLEN];		/* straight from file...    */
	struct  condition_rec  *condition;
	int     action;				/* what action to take      */
	char    argument2[SLEN];		/* argument for action      */
      } rules[MAXRULES];


/** finally let's keep LINT happy with the return values of all these pups! ***/

unsigned short getuid();

unsigned long sleep();

char *malloc(), *strcpy(), *strcat(), *itoa();

void	exit();

#ifdef BSD
	
  FILE *popen();

  extern char  _vbuf[5*BUFSIZ];		/* space for file buffering */

# define _IOFBF		0		/* doesn't matter - ignored */

# define setvbuf(fd,a,b,c)	setbuffer(fd, _vbuf, 5*BUFSIZ)

#endif
