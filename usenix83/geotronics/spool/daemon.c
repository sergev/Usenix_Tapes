/*
	daemon -- simple-minded despooling daemon for spooling system


	This source file may be made available to others, so long as
	Geotronics Corporation (Austin, Texas) is given proper credit.


				REVISION HISTORY

18-May-1981	D A Gwyn	Created.
09-Oct-1981	D A Gwyn	Make XFR definable.
16-Oct-1981	D A Gwyn	Set UID in case running as root.
04-May-1982	D A Gwyn	Ignore signals sooner; banner handling.


Usage:
	Invoked by spooler to start despooling to device once files are
	ready in the spool directory.  Usual name is "/etc/??d".

Protection:
	daemon: 	mode 04111, set-UID daemon
	transfer:	executable, no special privileges required
	device: 	mode 00200, owner daemon
			(handshaking device slaves require mode 00600)
	directory:	mode 00755, owner daemon
			(mode 00700 for extreme secrecy)

Method:
	The daemon tests for the presence of a "lock" file and exits if
	found, to avoid having two controllers.  Otherwise the lock is
	created, each correctly-named file is sent to the device using
	a "cp"-like file transfer utility, then the lock is removed.

	If "BANNER" is defined, each file will have its corresponding
	banner file (if any) transferred before it.

	Errors that "can't happen" exit with a characteristic status
	code; the lock file is left intact to keep things from changing
	while the problem is investigated.  Once the problem is remedied
	remove the lock file and re-invoke this despooling daemon.
*/

#include	<signal.h>
#include	<sys/types.h>
#include	<sys/dir.h>

extern char	*strcpy(), *strncpy();
extern int	_exit(), access(), chdir(), creat(), execv(), fork(),
		open(), read(), setuid(), strncmp(), unlink(), wait();
extern long	lseek();

#ifndef DEV
#define DEV	"/dev/lp"		/* output device */
#endif

#ifndef DIR
#define DIR	"/usr/lpd"		/* spool directory */
#endif

#ifndef UID
#define UID	1			/* "daemon" uid */
#endif

#ifndef XFR
#define XFR	"/bin/cp"		/* transfer utility */
#endif

#ifndef PFX
#define PFX	"spo"			/* data file prefix */
#endif

#ifdef BANNER
#ifndef BPFX
#define BPFX	"spb"			/* banner file prefix */
/* NOTE: these two prefixes must have the same length */
#endif
#endif

static int	onintr(), run();	/* later in file */

typedef int	bool;

static char		prefix[] = PFX; /* data file prefix */
#ifdef BANNER
static char		banpfx[] = BPFX;/* banner file prefix */
#endif
#define PFXLN	(sizeof prefix - 1)
static char		lock[] = "lock";/* lock file name */
static char		copy[] = XFR;	/* transfer program name */
static char		devnam[] = DEV; /* device name */
static char		file[DIRSIZ+1] = { 0 }; /* file name */
static int		dirfd = -1;	/* directory file descriptor */
static int		lockfd = -1;	/* lock file descriptor */
static struct direct	dirbuf = { 0 }; /* holds directory entry */
static char		*execargs[] = { copy, file, devnam, 0 };

main( argc, argv )
	int	argc;			/* arguments not used */
	char	**argv;
	{
#ifdef BANNER
	register bool	banex;		/* "banner exists" flag */
#endif
	register int	copied; 	/* pass improvement flag */

	signal( SIGHUP, SIG_IGN );
	signal( SIGINT, SIG_IGN );
	signal( SIGQUIT, SIG_IGN );

	if ( fork() != 0 )
		_exit( 0 );		/* detach */

	signal( SIGTERM, onintr );	/* to remove lock file */

	setuid( UID );			/* in case run by super-user */

	if ( chdir( DIR ) != 0 )
		_exit( 1 );		/* no spool directory! */

	if ( (lockfd = creat( lock, 0 )) < 0 )	/* exclusive-access */
		_exit( 0 );		/* another daemon active */

	if ( (dirfd = open( ".", 0 )) < 0 )
		_exit( 2 );		/* can't examine queue! */

	for ( ; ; )			/* until pass not useful */
		{
		copied = 0;		/* initialize pass */
		lseek( dirfd, 0L, 0 );	/* beginning of directory */

		while ( read( dirfd, (char *)&dirbuf, sizeof dirbuf )
			== sizeof dirbuf
		      ) {		/* examine queue entry */

			if ( dirbuf.d_ino == 0	/* unused entry */
			  || strncmp( dirbuf.d_name, prefix, PFXLN )
			     != 0	/* wrong name */
			   )
				continue;	/* no action */

			strncpy( file, dirbuf.d_name, DIRSIZ );
#ifdef BANNER
			/* temporary change of name */
			strncpy( file, banpfx, PFXLN );

			if ( banex = (access( file, 0 ) == 0) )
				/* banner exists; output it */
				if ( run( execargs ) != 0 )
					continue;	/* copy fail */

			/* restore data file name */
			strncpy( file, prefix, PFXLN );
#endif
			/* output the data file */
			if ( run( execargs ) == 0 )
				{
				++copied;	/* tally another */

				if ( unlink( file ) != 0 )
					_exit( 3 );	/* no delete */
#ifdef BANNER
				if ( banex )
					{
					/* delete banner, too */
					strncpy( file, banpfx, PFXLN );

					if ( unlink( file ) != 0 )
						_exit( 4 );	/* ?? */
					}
#endif
				}

			/* Uncopyable files are left in the queue. */
			}		/* next queue entry */

		if ( copied == 0 )	/* that's all for now */
			{
			unlink( lock ); /* this daemon is finished */
			_exit( 0 );
			}
		}			/* re-examine the queue */
	}

static int
run( args )				/* run a program */
	char	*args[];		/* arguments for execv() */
	{
	static int	status = -1;	/* child status from "wait" */
	register int	pid;		/* forked process ID */
	register int	w;		/* "wait" result */

	if ( (pid = fork()) == 0 )
		{
		execv( args[0], args );
		_exit( 127 );		/* somebody stole args[0]! */
		}

	while ( (w = wait( &status )) != pid )
		if ( w == -1 )
			_exit( 127 );	/* "impossible" */

	return status;
	}

static int
onintr( sig )				/* called on SIGTERM */
	int	sig;			/* not used */
	{
	if ( lockfd >= 0 )
		unlink( lock ); 	/* remove lock */

	_exit( 5 );			/* killed! */
	}
