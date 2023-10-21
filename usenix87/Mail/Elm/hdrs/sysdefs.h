/**			sysdefs.h			**/

/**  System level, configurable, defines for the ELM mail system.  **/

/**  (C) Copyright 1986 Dave Taylor                                **/

/** Define the following if you think that the information in messages
    that have "Reply-To:" and/or "From:" fields with addresses will
    contain valid addressing information.  If this isn't defined, the
    calculated return address will ALWAYS be used instead.  (note that
    this doesn't necessarily preclude the use of G)roup replies).
**/

#define USE_EMBEDDED_ADDRESSES	


#define FIND_DELTA	10		/* byte region where the binary search
					   on the path alias file is fruitless 
                                           (can't be within this boundary)    */

#define MAX_SALIASES	503	/* number of system aliases allowed      */
#define MAX_UALIASES	251	/* number of user aliases allowed 	 */
#define MAX_IN_WEEDLIST 150	/* max headers to weed out               */

#define MAX_HOPS	35	/* max hops in return addr to E)veryone  */

#define MAX_ATTEMPTS	6	/* #times to attempt lock file creation */

/** see leavembox.c to determine if this should be defined or not....The 
    default is to NOT have it defined.
**/

/** #define REMOVE_AT_LAST **/

#define DEFAULT_BATCH_SUBJECT  "no subject (file transmission)"

/** If you want to have the mailer know about valid mailboxes on the
    host machine (assumes no delivery agent aliases) then you should
    undefine this (the default is to have it defined)...
**/

#define NOCHECK_VALIDNAME

/** If your machine doesn't have virtual memory (specifically the vfork() 
    command) then you should define the following....		
**/

#define NO_VM

/** If you're running sendmail, or another transport agent that can 
    handle the blind-carbon-copy list define the following
**/

#define ALLOW_BCC

/** If you have pathalias, can we get to it as a DBM file??? **/

/** #define USE_DBM **/

/** If you want the mailer to check the pathalias database BEFORE it
    looks to see if a specified machine is in the L.sys database (in
    some cases routing is preferable to direct lines) then you should
    define the following...
**/

#define LOOK_CLOSE_AFTER_SEARCH


/** If you'd rather the program automatically used the 'uuname' command
    to figure out what machines it talks to (instead of trying to get
    it from L.sys first) then define the following...
**/

/** #define USE_UUNAME **/

/** If you don't want the program to even TOUCH the addresses handed to 
    it - deferring all expansion to sendmail/smail, then define the following;
**/

#define DONT_TOUCH_ADDRESSES 

/** Perhaps you'd also like the program not to try to minimize the addresses
    stored when you 'alias current message'.  If so, define the following
**/

#define DONT_OPTIMIZE_RETURN 

/** If you'd like "newmail" to automatically go into background when you
    start it up (instead of the "newmail &" junk with the process id output,
    then define the following...
**/

#define AUTO_BACKGROUND

/** If you'd rather your mail transport agent (ie sendmail) put the From:
    line into the message, define the following...
**/

#define DONT_ADD_FROM

/** If your machine prefers the Internet notation of user@host for the
    From: line and addresses, define the following...(the default is to 
    use this rather than the USENET notation - check your pathalias file!)

**/

/** #define INTERNET_ADDRESS_FORMAT **/

/** If you're on a machine that prefers UUCP to Internet addresses, then
    define the following (the basic change is that on a machine that
    receives messages of the form <path>!user@<localhost> the displayed
    address will be <path>!user instead of user@<localhost>.

    BOGUS_INTERNET is the address that your local system appends to 
    messages occasionally.  The algorithm is simply to REMOVE the
    BOGUS_INTERNET string.  This is horrible.  *sigh*

**/

#define PREFER_UUCP
#define BOGUS_INTERNET	"@hplabs.HP.COM"

/** If you're running ACSNET and/or want to have your domain name
    attached to your hostname on outbound mail then you can define
    the following (default are not defined)
**/

/** #define USE_DOMAIN **/
#define DOMAIN		"<enter your domain here>"

/** If you are going to be running the mailer with setgid mail (or
    something similar) you'll need to define the following to ensure
    that the users mailbox in the spool directory has the correct
    group (NOT the users group)
**/

#define SAVE_GROUP_MAILBOX_ID

/** If you want a neat feature that enables scanning of the message
    body for entries to add to the users ".calendar" (or whatever)
    file, define this.
**/

#define ENABLE_CALENDAR
#define dflt_calendar_file	"calendar"	/* in HOME directory */

/** If you want to implement 'site hiding' in the mail, then you'll need to
    uncomment the following lines and set them to reasonable values.  See 
    the configuration guide for more details....
**/

/****************************************************************************

#undef    DONT_ADD_FROM		

#define   SITE_HIDING
#define   HIDDEN_SITE_NAME	"fake-machine-name"
#define   HIDDEN_SITE_USERS	"/usr/mail/lists/hidden_site_users"

****************************************************************************/

/** Do we have the 'gethostname()' call?  If not, define the following **/
#define NEED_GETHOSTNAME

/** are you stuck on a machine that has short names?  If so, define the
    following **/

/** #define SHORTNAMES **/

#define NOTES_HEADER		"/***** "
#define NOTES_FOOTER		"/* ---------- */"

#ifdef BSD
# define system_hash_file	"/usr/spool/mail/.alias_hash"
# define system_data_file	"/usr/spool/mail/.alias_data"
#else
# define system_hash_file	"/usr/mail/.alias_hash"
# define system_data_file	"/usr/mail/.alias_data"
#endif

#define pathfile		"/usr/lib/nmail.paths"
#define domains			"/usr/lib/domains"

#define Lsys			"/usr/lib/uucp/L.sys"

/** where to put the output of the elm -d command... (in home dir) **/
#define DEBUG		"ELM:debug.info"
#define OLDEBUG		"ELM:debug.last"

#define temp_file	"/tmp/snd."
#define temp_form_file	"/tmp/form."
#define temp_mbox	"/tmp/mbox."
#define temp_print      "/tmp/print."
#define temp_uuname	"/tmp/uuname."
#define mailtime_file	".last_read_mail"
#define readmsg_file	".readmsg"

#define emacs_editor	"/usr/local/bin/emacs"

#ifdef BSD
# define default_editor	"/usr/ucb/vi"
# define mailhome	"/usr/spool/mail/"
#else
# define default_editor	"/usr/bin/vi"
# define mailhome	"/usr/mail/"
#endif

# define default_pager	"builtin"

#define sendmail	"/usr/lib/sendmail"
#define smflags		"-oi -oem"	/* ignore dots and mail back errors */
#define mailer		"/bin/rmail"
#ifdef BSD
# define mailx		"/usr/ucb/Mail"
#else
# define mailx		"/usr/bin/mailx"
#endif

#define helphome	"/usr/local/lib"
#define helpfile	"elm-help"

#define ELMRC_INFO	"/usr/local/lib/elmrc-info"

#define elmrcfile	"/.elmrc"
#define mailheaders	".elmheaders"
#define dead_letter	"Cancelled.mail"

#define unedited_mail	"emergency.mbox"

#define newalias	"newalias -q 1>&2 > /dev/null"
#define readmsg		"readmsg"

#define remove		"/bin/rm -f"		/* how to remove a file */
#define cat		"/bin/cat"		/* how to display files */
#define uuname		"uuname"		/* how to get a uuname  */
