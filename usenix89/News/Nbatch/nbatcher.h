/* char	sccsid[] = "@(#)nbatcher.h 1.4 8/14/86"; */

/******************************************************
 *	nbatcher.h - defines for nbatcher source
 *
 *
 *	R.J. Esposito
 *	Bell of Penna.
 *	June 1986
 *
 *****************************************************/

/* things you might want to change */

#define UUCPDIR		"/usr/spool/uucp"	/* uucp spool directory */
#define COMPRESS	"/usr/bin/compress -F -q" /* where compress resides */
#define UUX		"/usr/bin/uux - -r"	/* default command */
#define DFL_BYTES	100000L			/* default max. UUCP bytes */
#define NEWSUID		47			/* USENET used id */
#define NEWSGID		80			/* USENET group id */

/* things you shouldn't change */

#define FALSE		0
#define TRUE		1
#define COLON		':'

struct file_entry {		/* structure of control file */
	char	site[30];	/* name of remote site */
	char	hour[80];	/* string for hour to batch */
	short	c_bits;		/* # of compression bits */
	long	m_bytes;	/* max. # of bytes on UUCP queue */
	char	command[128];	/* command string */
} ep;

extern FILE	*lfp,			/* lockfile pointer */
		*tfp,			/* tempfile pointer */
		*log;			/* logfile pointer */

extern long	n_bytes;		/* # of bytes already on UUCP queue */
extern long	cu_bytes;		/* cumculative bytes of batch files */

char	*mktemp(),
	*strcpy(),
	*strcat();

extern char	*tfile;			/* temp file */

extern short	vflg,			/* verbose flag */
		nfiles;			/* number of news articles per
					   UUCP batch file */

int	system();

extern int	fcnt,			/* number of files batched */
		scnt;			/* spool count */
