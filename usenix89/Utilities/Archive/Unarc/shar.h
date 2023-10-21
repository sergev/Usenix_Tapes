/*
**  Header file for shar and friends.
**
**  $Header: shar.h,v 1.25 87/03/18 14:03:27 rs Exp $
*/


/*
**  Edit as necessary.
*/

/* Variances in local dialects. */
#define IDX		index		/* Maybe strchr?		*/
#define RDX		rindex		/* Maybe strrchr?		*/
/*efine NEED_MKDIR			/* Don't have mkdir(2)?		*/
/*efine NEED_QSORT			/* Don't have qsort(3)?		*/
#define NEED_GETOPT			/* Need local getopt object?	*/
#define CAN_POPEN			/* Can invoke file(1) command?	*/
/*efine USE_MY_SHELL			/* Don't popen("/bin/sh")?	*/

typedef int		*align_t;	/* Worst-case alignment, for lint */
/* typedef long		time_t		/* Needed for non-BSD sites?	*/
/* typedef long		off_t		/* Needed for non-BSD sites?	*/

#define DEF_SAVEIT	1		/* Save headers by default?	*/

/* Where is BSD-compatible directory header file?   Pick one. */
#define IN_SYS_DIR			/* <sys/dir.h>			*/
/*efine IN_SYS_NDIR			/* <sys/ndir.h>			*/
/*efine IN_DIR				/* "dir.h"			*/

/* Do you have <sys/wait.h>?  If so, then you have vfork, too. */
#define SYS_WAIT

/* Login name from environment; pick one. */
#define USER_ENV	"USER"		/* .. */
/*efine USER_ENV	"LOGNAME"	/* .. */
/*efine USER_ENV	"NAME"		/* .. */

/* What we should fopen() if stdin is not the tty. */
#define THE_TTY		"/dev/tty"	/* Maybe "con:" for MS-DOS?	*/

/* Name of the machine we're running on; pick one. */
#define GETHOSTNAME			/* Use gethostname(2) call	*/
/*efine UNAME				/* Use uname(2) call		*/
/*efine UUNAME				/* Invoke "uuname -l"		*/
/*efine	WHOAMI				/* Try /etc/whoami & <whoami.h>	*/
/*efine HOST		"SITE"		/* If all else fails		*/

/* How do we find the current working directory? */
#define GETWD				/* Use getwd(3) routine		*/
/* fine GETCWD				/* Use getcwd(3) routine	*/
/* fine PWDPOPEN			/* Invoke "pwd"			*/
/* fine PWDGETENV	"PWD"		/* Get $PWD from environment	*/

/* Prefixes for first two lines of saved NOTESFILES articles. */
#define NOTES1		"/* Written "
#define NOTES2		"/* ---"

/* Legal characters for filenames.  Note that shar doesn't do quoting... */
#define OK_CHARS	"@%_-+=.,/"

/*
**  END OF CONFIGURATION SECTION
*/

#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>

#ifdef	IN_SYS_DIR
#include <sys/dir.h>
#endif	/* IN_SYS_DIR */
#ifdef	IN_SYS_NDIR
#include <sys/ndir.h>
#endif	/* IN_SYS_NDIR */
#ifdef	IN_NDIR
#include "ndir.h"
#endif	/* IN_DIR */


/*
**  Handy shorthands.
*/
#define TRUE		1
#define FALSE		0
#define WIDTH		72
#define F_DIR		'$'		/* Something is a directory	*/
#define F_FILE		'A'		/* Something is a regular file	*/
#define S_IGNORE	'L'		/* Ignore this signal		*/
#define S_RESET		'Z'		/* Reset signal to default	*/

/* These are used by the archive parser. */
#define MAX_LINE_SIZE	200		/* Length of physical input line*/
#define MAX_VAR_NAME	 30		/* Length of a variable's name	*/
#define MAX_VAR_VALUE	128		/* Length of a variable's value	*/
#define MAX_VARS	 20		/* Number of shell vars allowed	*/
#define MAX_WORDS	 30		/* Make words in command lnes	*/


/*
**  Keep RCS stuff away from lint.
*/
#ifdef	lint
#define RCS(text)	/* NULL */
#else
#define RCS(text)	static char ID[] = text;
#endif	/* lint */


/*
**  Memory hacking.
*/
#define NEW(T, count)	((T *)getmem(sizeof(T), (unsigned int)(count)))
#define ALLOC(n)	getmem(1, (unsigned int)(n))
#define COPY(s)		strcpy(NEW(char, strlen((s)) + 1), (s))


/*
**  Macros.
*/
#define BADCHAR(c)	(iscntrl((c)) && !isspace((c)))
#define EQ(a, b)	(strcmp((a), (b)) == 0)
#define EQn(a, b, n)	(strncmp((a), (b), (n)) == 0)
#define PREFIX(a, b)	(EQn((a), (b), sizeof b - 1))
#define WHITE(c)	((c) == ' ' || (c) == '\t')


/*
**  Linked in later.
*/
extern int	 errno;
extern int	 optind;
extern char	*optarg;

/* From your C run-time library. */
extern FILE	*popen();
extern time_t	 time();
extern long	 atol();
extern char	*IDX();
extern char	*RDX();
extern char	*ctime();
extern char	*gets();
extern char	*mktemp();
extern char	*strcat();
extern char	*strcpy();
extern char	*strncpy();
extern char   	*getenv();

/* From our local library. */
extern align_t	 getmem();
extern off_t	 Fsize();
extern char	*Copy();
extern char	*Cwd();
extern char	*Ermsg();
extern char	*Host();
extern char	*User();

/* Exported by the archive parser. */
extern FILE	*Input;			/* Current input stream		*/
extern char	*File;			/* Input filename		*/
extern int	 Interactive;		/* isatty(fileno(stdin))?	*/
extern void	 SynErr();		/* Fatal syntax error		*/
