/*
	daemon -- simple-minded device daemon for spooling system


	This source file may be made available to others, so long as
	Geotronics Corporation (Austin, Texas) is given proper credit.


				REVISION HISTORY

18-May-1981	D A Gwyn	Created.


Usage:
	Invoked by spooler to start output to device once files are in
	spool directory.

Protection:
	daemon:		mode 4711, set-UID daemon
	device:		mode 200, owner daemon
	directory:	mode 700, owner daemon

Method:
	The daemon tests for the presence of a "lock" file and exits if
	found, to avoid having two device masters.  Otherwise the lock
	is created and each file is copied to the device.  Then the lock
	is removed.
*/

#include	<signal.h>
#include	<sys/types.h>
#include	<sys/dir.h>

#ifndef	DEV
#define	DEV	"/dev/lp"		/* output device */
#define	DIR	"/usr/lpd"		/* spool directory */
#endif
#define	LOCK	"lock"			/* lock file name */

static		quit();			// catches "kill"

static char		from[DIRSIZ+1];	// for "cp" argument
static int		dirfd ;		// directory file descriptor
static int		lockfd = -1 ;	// lock file descriptor
static int		status ;	// for "wait" status
static struct direct	dirbuf ;	// holds directory entry


main()	{				// no arguments used
	register int	copied ;	// pass improvement flag
	register int	pid ;		// forked process ID
	register int	w ;		// for "wait" PID

	if ( fork() )
		_exit( 0 );		// detach

	signal( SIGHUP , SIG_IGN );
	signal( SIGINT , SIG_IGN );	
	signal( SIGQUIT , SIG_IGN );
	signal( SIGTERM , quit );	// to remove lock, if any

	if ( chdir( DIR ) )
		_exit( 1 );		// no spool directory!

	if ( (lockfd = creat( LOCK , 0 )) < 0 )
		_exit( 0 );		// another daemon active

	if ( (dirfd = open( "." , 0 )) < 0 )
		_exit( 2 );		// can't examine queue!

	for ( ; ; )			// until finished
		{
		copied = 0 ;		// initialize pass

		lseek( dirfd , 0L , 0 );	// BOF
		while ( read( dirfd , (char *)&dirbuf , sizeof dirbuf )
						== sizeof dirbuf )
			{		// examine queue entry
			if ( dirbuf.d_ino == 0
			  || strcmp( dirbuf.d_name , "." ) == 0
			  || strcmp( dirbuf.d_name , ".." ) == 0
			  || strcmp( dirbuf.d_name , LOCK ) == 0 )
				continue ;	// not in queue

			strncpy( from , dirbuf.d_name , DIRSIZ );
			if ( (pid = fork()) == 0 )
				{
				execl( "/bin/cp" ,
						"cp" , from , DEV , 0 );
				_exit( 127 );	// somebody stole cp!
				}
			while ( (w = wait( &status )) != pid )
				if ( w == -1 )
					_exit( 3 );	// "impossible"

			if ( status == 0 )
				{
				++ copied ;	// did one

				if ( unlink( from ) )
					_exit( 4 );	// can't delete!
				}
			// Note that uncopyable files are left in queue.
			}			// next queue entry

		if ( copied == 0 )		// that's all for now
			{
			unlink( LOCK );
			_exit( 0 );
			}
		}				// re-examine queue
	}


static
quit()	{				// called on "kill"
	if ( lockfd >= 0 )
		unlink( LOCK );		// remove lock
	_exit( 5 );			// killed!
	}
