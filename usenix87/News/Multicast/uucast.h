/*
**	uucast.h - Header file for uucast.
*/

/* #define	NODE	"foobar"	/* name of this node, define if system
				call uname is not available.  This name can
				be no longer than 7 characters. */
#define	MASK	026			/* umask for files */
#define	CDIR	"/usr/spool/uucp"	/* local command files directory */
#define	DDIR	"/usr/spool/uucp"	/* data files directory */
#define	XDIR	"/usr/spool/uucp"	/* remote command files directory */

/*  For HoneyDanBer users:  Grades are a single character, so use M and
 *  X (for example).
 */

#define GRADE	"MA"			/* grade to use for uucico files.
					   Should not be used by anyone else */
#define	GRADE2	"XM"			/* other grade to use for files
					   This grade should start
					   with an X. */
#define	USER	"usenet"		/* name of news user on your system */
#define	MAXSYS	20			/* maximum number of systems per cast */
