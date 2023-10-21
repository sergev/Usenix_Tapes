/*	SCAME config.h				*/

/*	Revision 1.0.0  1985-02-09		*/

/*	Copyright 1985 by Leif Samuelsson	*/


/* Define one of VAX, PDP11, SUN, HP9000 */
# define SUN

/* Define one of V7, S5, BSD42 */
# define S5

/* Locate these directories. Note that TMPDIR must be rw to everyone. */
# define SCAMELIB "/usr/scame/lib"
# define TMPFILE "/usr/scame/tmp/S%05d%05d."
# define MAILDIR "/usr/mail"

/* Locate these programs. If a program does not exist, comment out line. */
# define GREP "/bin/grep"
# define MAIL "/usr/bin/mailx"
# define SORT "/usr/bin/sort"

/* Define format for naming backup files. */
/* Note that %s is the original filename with path removed. */
#ifdef BSD42
#  define BACKUPNAME "%s.BAK"
#else
#  define BACKUPNAME "%.9s.BAK"
#endif

/* Initial value for Mail Check Interval (0 means never) */
#ifdef HP9000
#  define MAILCHECKINTERVAL 0	/* Bug in HP9000 implementation */
#else
#  define MAILCHECKINTERVAL 100
#endif

/* These constants set maximum screen width and height */
# define SCRDIMX 80
# define SCRDIMY 24
