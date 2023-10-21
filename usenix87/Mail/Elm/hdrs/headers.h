/**		headers.h		**/

/**  header file for ELM mail system.  **/

/**  (C) Copyright 1985, Dave Taylor   **/

#include <stdio.h>
#include <fcntl.h>

#include "curses.h"
#include "defs.h"

/******** global variables accessable by all pieces of the program *******/

extern int current;		/* current message number  */
extern int header_page;         /* current header page     */
extern int last_header_page;    /* last header page        */
extern int message_count;	/* max message number      */
extern int headers_per_page;	/* number of headers/page  */
extern char infile[SLEN];	/* name of current mailbox */
extern char hostname[SLEN];	/* name of machine we're on*/
extern char username[SLEN];	/* return address name!    */
extern char full_username[SLEN];/* Full username - gecos   */
extern char home[SLEN];		/* home directory of user  */
extern char folders[SLEN];	/* folder home directory   */
extern char mailbox[SLEN];	/* mailbox name if defined */
extern char editor[SLEN];	/* default editor for mail */
extern char alternative_editor[SLEN];/* the 'other' editor */
extern char printout[SLEN];	/* how to print messages   */
extern char savefile[SLEN];	/* name of file to save to */
extern char calendar_file[SLEN];/* name of file for clndr  */
extern char prefixchars[SLEN];	/* prefix char(s) for msgs */
extern char shell[SLEN];	/* default system shell    */
extern char pager[SLEN];	/* what pager to use...    */
extern char batch_subject[SLEN];/* subject buffer for batchmail */
extern char local_signature[SLEN];/* local msg signature file   */
extern char remote_signature[SLEN];/* remote msg signature file */

extern char backspace,		/* the current backspace char  */
	    kill_line;		/* the current kill_line char  */

extern char up[SHORT], 
	    down[SHORT];	/* cursor control seq's    */
extern int  cursor_control;	/* cursor control avail?   */

extern char start_highlight[SHORT],
	    end_highlight[SHORT];  /* standout mode... */

extern int  has_highlighting;	/* highlighting available? */

/** the following two are for arbitrary weedout lists.. **/

extern char *weedlist[MAX_IN_WEEDLIST];
extern int  weedcount;		/* how many headers to check?        */

extern int  allow_forms;	/* flag: are AT&T Mail forms okay?    */
extern int  file_changed;	/* flag: true iff infile changed      */
extern int  mini_menu;		/* flag: display menu?     	      */
extern int  mbox_specified;     /* flag: specified alternate mailbox? */
extern int  check_first;	/* flag: verify mail to be sent!      */
extern int  auto_copy;		/* flag: auto copy source into reply? */
extern int  filter;		/* flag: weed out header lines?	      */
extern int  resolve_mode;	/* flag: resolve before moving mode?  */
extern int  auto_cc;		/* flag: mail copy to yourself?       */
extern int  noheader;		/* flag: copy + header to file?       */
extern int  title_messages;	/* flag: title message display?       */
extern int  edit_outbound;	/* flag: edit outbound headers?       */
extern int  hp_terminal;	/* flag: are we on an hp terminal?    */
extern int  hp_softkeys;	/* flag: are there softkeys?          */
extern int  save_by_name;  	/* flag: save mail by login name?     */
extern int  mail_only;		/* flag: send mail then leave?        */
extern int  check_only;		/* flag: check aliases and leave?     */
extern int  move_when_paged;	/* flag: move when '+' or '-' used?   */
extern int  point_to_new;	/* flag: start pointing at new msgs?  */
extern int  bounceback;		/* flag: bounce copy off remote?      */
extern int  signature;		/* flag: include $home/.signature?    */
extern int  always_leave;	/* flag: always leave mail pending?   */
extern int  always_del;		/* flag: always delete marked msgs?   */
extern int  arrow_cursor;	/* flag: use "->" regardless?	      */
extern int  debug;		/* flag: debugging mode on?           */
extern int  read_in_aliases;	/* flag: have we read in aliases yet? */
extern int  warnings;		/* flag: output connection warnings?  */
extern int  user_level;		/* flag: how knowledgable is user?    */
extern int  selected;		/* flag: used for select stuff        */
extern int  names_only;		/* flag: display names but no addrs?  */
extern int  question_me;	/* flag: ask questions as we leave?   */
extern int  keep_empty_files;	/* flag: keep empty files??	      */

#ifdef UTS
extern int  isatube;		/* flag: are we on an IBM 3270 tube?  */
#endif

extern int  sortby;		/* how to sort mailboxes	      */

extern long timeout;		/* seconds for main level timeout     */

extern int mailbox_defined;	/** specified mailbox?  **/

extern int LINES;		/** lines per screen    **/
extern int COLUMNS;		/** columns per line    **/

extern long size_of_pathfd;	/** size of pathfile, 0 if none **/

extern FILE *mailfile;		/* current mailbox file     */
extern FILE *debugfile;		/* file for debut output    */
extern FILE *pathfd;		/* path alias file          */
extern FILE *domainfd;		/* domains file 	    */

extern long mailfile_size;	/* size of current mailfile */

extern int  max_headers;	/* number of headers currently allocated */

extern struct header_rec *header_table;

extern struct alias_rec user_hash_table  [MAX_UALIASES];
extern struct alias_rec system_hash_table[MAX_SALIASES];

extern struct date_rec last_read_mail;

extern struct lsys_rec *talk_to_sys;	/* who do we talk to? */

extern struct addr_rec *alternative_addresses;	/* how else do we get mail? */

extern int system_files;	/* do we have system aliases? */
extern int user_files;		/* do we have user aliases?   */

extern int system_data;		/* fileno of system data file */
extern int user_data;		/* fileno of user data file   */

extern int userid;		/* uid for current user	      */
extern int groupid;		/* groupid for current user   */
