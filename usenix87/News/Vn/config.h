/*
** vn news reader.
**
** config.h - system configuration parameters
**
** see copyright disclaimer / history in vn.c source file
*/

#define DEF_ED "/usr/ucb/vi"	/* editor to use if no EDITOR variable */
#define DEF_PS1 "$ "		/* ! command prompt if no PS1 */
#define DEF_SAVE "vn.save"	/* save file */

/*
** mailer interface.  If INLETTER is defined, a "To:" line will be
** placed in the file being editted by the user.  Otherwise, the
** address will be an argument on the mailer's command line, with the
** user prompted for possible correction.  In either case, "Subject: "
** is included in the file.
**
** If MAILSMART is set, alternate header lines will be used instead of
** the "Path: " line to determine the address because we assume the mailer
** is intelligent enough to do routing.
**
** If ADDRMUNGE is set, it is the name of a local routine which will be
** called to make further address modifications before the address is used.
** It will be passed the address string, which it may modify.  RECLEN bytes
** of storage are available at the address passed.  It will only be called
** once for a given address.
**
** DEF_MAIL is the mailer used in absence of MAILER variable.
*/
#define INLETTER
#define MAILSMART
#define DEF_MAIL "/usr/lib/sendmail -t"

#define DEF_PRINT "/usr/ucb/lpr"		/* print command */
#define DEF_POST "/usr/lib/news/inews -h"	/* followup posting command */

#define DEF_NEWSRC ".newsrc"
#define DEF_CCFILE "author_copy"
#define DEF_KEYXLN ".vnkey"

#define SPOOLDIR "/usr/spool/news"
#define ACTFILE "/usr/lib/news/active"

/*
** foreground flag for messages.  applies only if JOBCONTROL undefined
** (SYS V). set to 1 to see newsgroup messages, etc. during reading phase,
** 0 for "silent" operation - be warned that this may suppress some
** non-fatal diagnostic messages - find all references to fgprintf to
** see what is suppressed.
*/
#define NOJOB_FG 1

/*
** arrow key treatment.  If PAGEARROW is defined, right and left arrow
** keys will be synonyms for <return> (next-page) and <backspace> (previous).
** Otherwise, the right arrow will function as down, and the left as up.
** Made configurable because while there is no lateral motion on the screen
** to associate with the right and left arrows, you might not like them
** changing pages on you.
*/
#define PAGEARROW

/*
** if USEVS is defined, terminal initialization / exit for vn will include the
** "vs"/"ve" pair as well as "ti"/"te".  This doesn't matter on a lot of
** terminals, but may make vn display behaviour closer to "vi" since vs/ve
** is vi's "visual mode" sequence.  For instance, I believe the commonly
** used definitions for these strings on multi-page concepts allows the
** program to run in the first page of the terminal, preserving the more
** recent part of your session on exit
**
** #define USEVS
*/

/*
** temp file name template for mktemp().  Used in tmpnam.c, does not apply
** if you use a system library tmpnam().   BE CAREFUL - VNTEMPNAME MUST
** contain a string of 6 X's for mktemp() (actually, a place where 6 X's
** are intended to go).  TMP_XOFFSET absolutely MUST point to the first of
** the X's.  Yes, writing into a literal string is sloppy.  To the best of
** my knowledge, tmpnam.c is the only place you'll find vn code doing it.
** We make this configurable in case you want temp files somewhere else.
*/
#define VNTEMPNAME "/usr/tmp/vnXXXXXX"
#define TMP_XOFFSET 11
