/**			elm.h				**/

/**  Main header file for ELM mail system.  **/

/**  (C) Copyright 1986, Dave Taylor   **/

#include <stdio.h>
#include <fcntl.h>

#include "../hdrs/curses.h"
#include "../hdrs/defs.h"

/******** static character string containing the version number  *******/

static char ident[] = { WHAT_STRING };

/******** and another string for the copyright notice            ********/

static char copyright[] = { "@(#)          (C) Copyright 1986, Dave Taylor" };

/******** global variables accessable by all pieces of the program *******/

int current = 0;		/* current message number  */
int header_page = 0;     	/* current header page     */
int last_header_page = -1;     	/* last header page        */
int message_count = 0;		/* max message number      */
int headers_per_page;		/* number of headers/page  */
char infile[SLEN];		/* name of current mailbox */
char hostname[SLEN];		/* name of machine we're on*/
char username[SLEN];		/* return address name!    */
char full_username[SLEN];	/* Full username - gecos   */
char home[SLEN];		/* home directory of user  */
char folders[SLEN];		/* folder home directory   */
char mailbox[SLEN];		/* mailbox name if defined */
char editor[SLEN];		/* editor for outgoing mail*/
char alternative_editor[SLEN];	/* alternative editor...   */
char printout[SLEN];		/* how to print messages   */
char savefile[SLEN];		/* name of file to save to */
char calendar_file[SLEN];	/* name of file for clndr  */
char prefixchars[SLEN];		/* prefix char(s) for msgs */
char shell[SLEN];		/* current system shell    */
char pager[SLEN];		/* what pager to use       */
char batch_subject[SLEN];	/* subject buffer for batchmail */
char local_signature[SLEN];	/* local msg signature file     */
char remote_signature[SLEN];	/* remote msg signature file    */

char backspace,			/* the current backspace char */
     kill_line;			/* the current kill-line char */

char up[SHORT], down[SHORT];	/* cursor control seq's    */
int  cursor_control = FALSE;	/* cursor control avail?   */

char start_highlight[SHORT],
     end_highlight[SHORT];	/* stand out mode...       */

int  has_highlighting = FALSE;	/* highlighting available? */

char *weedlist[MAX_IN_WEEDLIST];
int  weedcount;

int allow_forms = NO;		/* flag: are AT&T Mail forms okay?  */
int file_changed = 0;		/* flag: true if infile changed     */
int mini_menu = 1;		/* flag: menu specified?	    */
int mbox_specified = 0;		/* flag: specified alternate mbox?  */
int check_first = 1;		/* flag: verify mail to be sent!    */
int auto_copy = 0;		/* flag: automatically copy source? */
int filter = 1;			/* flag: weed out header lines?	    */
int resolve_mode = 1;		/* flag: delete saved mail?	    */
int auto_cc = 0;		/* flag: mail copy to user?	    */
int noheader = 1;		/* flag: copy + header to file?     */
int title_messages = 1;		/* flag: title message display?     */
int edit_outbound = 0;		/* flag: edit outbound headers?	    */
int hp_terminal = 0;		/* flag: are we on HP term?	    */
int hp_softkeys = 0;		/* flag: are there softkeys?        */
int save_by_name = 1;		/* flag: save mail by login name?   */
int mail_only = 0;		/* flag: send mail then leave?      */
int check_only = 0;		/* flag: check aliases then leave?  */
int move_when_paged = 0;	/* flag: move when '+' or '-' used? */
int point_to_new = 1;		/* flag: start pointing at new msg? */
int bounceback = 0;		/* flag: bounce copy off remote?    */
int signature = 0;		/* flag: include $home/.signature?  */
int always_leave = 0;		/* flag: always leave msgs pending? */
int always_del = 0;		/* flag: always delete marked msgs? */
int arrow_cursor = 0;		/* flag: use "->" cursor regardless?*/
int debug = 0; 			/* flag: default is no debug!       */
int read_in_aliases = 0;	/* flag: have we read in aliases??  */
int warnings = 1;		/* flag: output connection warnings?*/
int user_level = 0;		/* flag: how good is the user?      */
int selected = 0;		/* flag: used for select stuff      */
int names_only = 1;		/* flag: display user names only?   */
int question_me = 1;		/* flag: ask questions as we leave? */
int keep_empty_files = 0;	/* flag: leave empty mailbox files? */

#ifdef UTS
 int isatube = 0;		/* flag: are we on an IBM 3270?     */
#endif

int sortby = REVERSE SENT_DATE;	/* how to sort incoming mail...     */

long timeout = 600L;		/* timeout (secs) on main prompt    */

int mailbox_defined = 0;	/** mailbox specified?    **/

/** set up some default values for a 'typical' terminal *snicker* **/

int LINES=23;			/** lines per screen      **/
int COLUMNS=80;			/** columns per page      **/

long size_of_pathfd;		/** size of pathfile, 0 if none **/

FILE *mailfile;			/* current mailbox file     */
FILE *debugfile;		/* file for debug output    */
FILE *pathfd;			/* path alias file          */
FILE *domainfd;			/* domain file		    */

long mailfile_size;		/* size of current mailfile */

int   max_headers;		/* number of headers allocated */

struct header_rec *header_table;

struct alias_rec user_hash_table[MAX_UALIASES];
struct alias_rec system_hash_table[MAX_SALIASES];

struct date_rec last_read_mail; /* last time we read mailbox  */

struct lsys_rec *talk_to_sys;   /* what machines do we talk to? */

struct addr_rec *alternative_addresses;	/* how else do we get mail? */

int system_files = 0;		/* do we have system aliases? */
int user_files = 0;		/* do we have user aliases?   */

int system_data;		/* fileno of system data file */
int user_data;			/* fileno of user data file   */

int userid;			/* uid for current user	      */
int groupid;			/* groupid for current user   */
