/**		defs.h			**/

/**  define file for ELM mail system.  **/

/**  (C) Copyright 1985 Dave Taylor    **/

#include "sysdefs.h"	/* system/configurable defines */

#define VERSION		"1.5" /* Version number!  WHAT_STRING should agree */

#define WHAT_STRING	"@(#) Version 1.5, January, 1987"

#define KLICK		10

#define SLEN		256	    /* long for ensuring no overwrites... */
#define SHORT		5	    /* super short strings!		  */
#define NLEN		20	    /* name length for aliases            */
#define SHORT_SLEN      40
#define STRING		100	/* reasonable string length for most..      */
#define LONG_SLEN	250	/* for mail addresses from remote machines! */
#define LONG_STRING	500	/* even longer string for group expansion   */
#define VERY_LONG_STRING 2500	/* huge string for group alias expansion    */

#define BREAK		'\0'  		/* default interrupt    */
#define BACKSPACE	'\b'     	/* backspace character  */
#define TAB		'\t'            /* tab character        */
#define RETURN		'\r'     	/* carriage return char */
#define LINE_FEED	'\n'     	/* line feed character  */
#define FORMFEED	'\f'     	/* form feed (^L) char  */
#define COMMA		','		/* comma character      */
#define SPACE		' '		/* space character      */
#define DOT		'.'		/* period/dot character */
#define BANG		'!'		/* exclaimation mark!   */
#define AT_SIGN		'@'		/* at-sign character    */
#define PERCENT		'%'		/* percent sign char.   */
#define COLON		':'		/* the colon ..		*/
#define BACKQUOTE	'`'		/* backquote character  */
#ifdef TILDE
# undef TILDE
#endif
#define TILDE		'~'		/* escape character~    */
#define ESCAPE		'\033'		/* the escape		*/

#define NO_OP_COMMAND	'\0'		/* no-op for timeouts   */

#define STANDARD_INPUT  0		/* file number of stdin */

#ifndef TRUE
#define TRUE		1
#define FALSE		0
#endif

#define NO		0
#define YES		1
#define MAYBE		2		/* a definite define, eh?  */
#define FORM		3		/*      <nevermind>        */
#define PREFORMATTED	4		/* forwarded form...       */

#define PAD		0		/* for printing name of    */
#define FULL		1		/*   the sort we're using  */

#define OUTGOING	0		/* defines for lock file   */
#define INCOMING	1		/* creation..see lock()    */

#define SH		0		/* defines for system_call */
#define USER_SHELL	1		/* to work correctly!      */

#define EXECUTE_ACCESS	01		/* These five are 	   */
#define WRITE_ACCESS	02		/*    for the calls	   */
#define READ_ACCESS	04		/*       to access()       */
#define ACCESS_EXISTS	00		/*           <etc>         */
#define EDIT_ACCESS	06		/*  (this is r+w access)   */

#define BIG_NUM		999999		/* big number!             */
#define BIGGER_NUM	9999999 	/* bigger number!          */

#define START_ENCODE	"[encode]"
#define END_ENCODE	"[clear]"

#define DONT_SAVE	"[no save]"

#define alias_file	".aliases"
#define group_file	".groups"
#define system_file	".systems"

/** some defines for the 'userlevel' variable... **/

#define RANK_AMATEUR	0
#define AMATEUR		1
#define OKAY_AT_IT	2
#define GOOD_AT_IT	3
#define EXPERT		4
#define SUPER_AT_IT	5

/** some defines for the "status" field of the header record **/

#define TAGGED		1		/* these are bit masks */
#define DELETED		2
#define EXPIRED		4
#define ACTION		8
#define NEW		16
#define PRIORITY	32
#define FORM_LETTER	64
#define VISIBLE		128

#define UNDELETE	0		/* purely for ^U function... */

/** some months... **/

#define JANUARY		0			/* months of the year */
#define FEBRUARY	1
#define MARCH		2
#define APRIL		3
#define MAY		4
#define JUNE		5
#define JULY		6
#define AUGUST		7
#define SEPTEMBER	8
#define OCTOBER		9
#define NOVEMBER	10
#define DECEMBER	11

#define equal(s,w)	(strcmp(s,w) == 0)
#define min(a,b)	a < b? a : b
#define ctrl(c)	        c - 'A' + 1	/* control character mapping */
#define plural(n)	n == 1 ? "" : "s"
#define lastch(s)	s[strlen(s)-1]
#define no_ret(s)	if (lastch(s) == '\n') lastch(s) = '\0' 
#define first_word(s,w) (strncmp(s,w, strlen(w)) == 0)
#define ClearLine(n)	MoveCursor(n,0); CleartoEOLN()
#define whitespace(c)	(c == ' ' || c == '\t')
#define quote(c)	(c == '"' || c == '\'') 
#define onoff(n)	(n == 0 ? "OFF" : "ON")

/** The next function is so certain commands can be processed from the showmsg
    routine without rewriting the main menu in between... **/

#define special(c)	(c == 'j' || c == 'k')

/** and a couple for dealing with status flags... **/

#define ison(n,mask)	(n & mask)
#define isoff(n,mask)	(~ison(n, mask))

#define setit(n,mask)		n |= mask
#define clearit(n, mask)	n &= ~mask

/** a few for the usage of function keys... **/

#define f1	1
#define f2	2
#define f3	3
#define f4	4
#define f5	5
#define f6	6
#define f7	7
#define f8	8

#define MAIN	0
#define ALIAS   1
#define YESNO	2
#define CHANGE  3
#define READ	4

#define MAIN_HELP    0
#define ALIAS_HELP   1
#define OPTIONS_HELP 2

/** some possible sort styles... **/

#define REVERSE		-		/* for reverse sorting           */
#define SENT_DATE	1		/* the date message was sent     */
#define RECEIVED_DATE	2		/* the date message was received */
#define SENDER		3		/* the name/address of sender    */
#define SIZE		4		/* the # of lines of the message */
#define SUBJECT		5		/* the subject of the message    */
#define STATUS		6		/* the status (deleted, etc)     */

/* some stuff for our own malloc call - pmalloc */

#define PMALLOC_THRESHOLD	256	/* if greater, then just use malloc */
#define PMALLOC_BUFFER_SIZE    2048	/* internal [memory] buffer size... */

/* wouldn't it be nice to have variable argument macros... */

#define dprint0(n,s)	     if (debug >= n) { \
				fprintf(debugfile, s); fflush(debugfile); }

#define dprint1(n,s,a)	     if (debug >= n) { \
				fprintf(debugfile, s, a);  fflush(debugfile); }

#define dprint2(n,s,a,b)     if (debug >= n) { \
			        fprintf(debugfile, s, a, b); fflush(debugfile);}

#define dprint3(n,s,a,b,c)   if (debug >= n) { fprintf(debugfile, \
				s, a, b, c); fflush(debugfile); }

#define dprint4(n,s,a,b,c,d)  if (debug >= n) { fprintf(debugfile, \
				s, a, b, c, d); fflush(debugfile); }

#define dprint5(n,s,a,b,c,d,e) if (debug >= n) { fprintf(debugfile, \
			        s, a, b, c, d, e); fflush(debugfile); }

#define dprint6(n,s,a,b,c,d,e,f) if (debug >= n) { fprintf(debugfile, \
			        s, a, b, c, d, e, f); fflush(debugfile); }

/* I guess this corrects it, though.  Wretched stuff though! */

/* some random records... */

struct date_rec {
	int  month;		/** this record stores a **/
	int  day;		/**   specific date and  **/
	int  year;		/**     time...		 **/
	int  hour;
	int  minute;
       };

struct header_rec {
	int  lines;		/** # of lines in the message  **/
	int  status;		/** Urgent, Deleted, Expired?  **/
	int  index_number;	/** relative loc in file...    **/
	long offset;		/** offset in bytes of message **/
	struct date_rec received; /** when elm received here   **/
	char from[STRING];	/** who sent the message?      **/
	char to[STRING];	/** who it was sent to	       **/
	char dayname[8];	/**  when the                  **/
	char month[10];		/**        message             **/
	char day[3];		/**          was 	       **/
	char year[5];		/**            sent            **/
	char time[NLEN];	/**              to you!       **/
	char subject[STRING];   /** The subject of the mail    **/
       };

struct alias_rec {
	char   name[NLEN];	/* alias name 			     */
	long   byte;		/* offset into data file for address */
       };

struct lsys_rec {
	char   name[NLEN];	/* name of machine connected to      */
	struct lsys_rec *next;	/* linked list pointer to next       */
       };

struct addr_rec {
	 char   address[NLEN];	/* machine!user you get mail as      */
	 struct addr_rec *next;	/* linked list pointer to next       */
	};

#ifdef SHORTNAMES	/* map long names to shorter ones */
# include <shortnames.h>
#endif
