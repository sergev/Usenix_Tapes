#include <sys/file.h>
#include <sys/param.h>
#include <sys/errno.h>

#define LOCKDIR	"/usr/spool/locks/"
#define INITSLEEPTIME	((unsigned)10)
#define MAXSLEEPTIME	((unsigned)30)

extern	int	errno;
extern	char	*strcpy(), *strcat(), *sprintf();

/* mkblockf():
**	Make a lock file name for nblock() or ublock().
**	This is not declared static because the user might want to
**	use it to look at the contents of some other lock file.
*/
char *
mkblockf( buf, pref )
char	*buf,	*pref;
{
	if ( *pref == '/' )
		*buf = '\0';
	else
		(void)strcpy(buf, LOCKDIR);
	return strcat(strcat(buf, pref), ".");
}

/* nblock():
**	Block the current process until an existing lock file is
**	opened and an exclusive lock obtained on it.  The name of
**	the lock file is given by the concatenation of LOCKDIR
**	(only if prefix does not begin with "/"), the argument
**	string prefix, and a dot followed by a positive integer.
** PURPOSE:
**	To limit the number of instance of a given program which
**	can be running simultaneously.
** RETURN VALUE:
**	>= 0	An fd open for writing.  The referenced file will
**		have been truncated to zero length.
**	< 0	Errno will be set if the operation cannot be done.
**		The calling process should probably go ahead.
*/
nblock( prefix )
char	*prefix;
{
	char	namebuf[MAXPATHLEN];
	char	*numptr;

	(void)mkblockf(namebuf, prefix);
	numptr = namebuf + strlen(namebuf);
	for ( ; ; ) {
		int	sufnum,	fd = -1;
		unsigned	sleeptime = INITSLEEPTIME;

		for ( sufnum=1; fd<0; sufnum++ ) {
			(void)sprintf(numptr, "%d", sufnum);
			if ( (fd=open(namebuf, O_RDWR, 0666)) < 0 )
				if ( errno == ENOENT || sufnum == 1 )
				        break;
				else
					continue;
			if ( flock(fd, LOCK_EX|LOCK_NB) )
				(void)close(fd), fd = -1;
			errno=0;
		}
		if ( fd >= 0 )
			return fd;
		if ( errno && sufnum == 1 )
			return -1;
		sleep(sleeptime);
		if ( sleeptime < MAXSLEEPTIME )
			sleeptime += INITSLEEPTIME;
	}
	/* NOTREACHED */
}

/* ublock():
**	Block the current process until a lock file is opened
**	and an exclusive lock obtained on it.  The file will
**	be created if it does not already exist.  The name of
**	the lock file is given by the concatenation of LOCKDIR
**	(only if prefix does not begin with "/"), the argument
**	string prefix, and a dot followed by the current ruid.
**	(It would have been the username, but troff has it's
**	own atoi() that was messing up getpwuid().)
** PURPOSE:
**	To limit users to one instance of a given program
**	running at a time.
** RETURN VALUE:
**	>= 0	An fd open for writing.  The referenced file will
**		have been truncated to zero length.
**	< 0	Errno will be set if the operation cannot be done.
**		The calling process should probably go ahead.
*/
ublock( prefix )
char	*prefix;
{
	char	namebuf[MAXPATHLEN],
		uidbuf[16];
	int	fd;

	(void)strcat(mkblockf(namebuf, prefix),
		     sprintf(uidbuf, "%d", getuid()));
	fd = open(namebuf, O_RDWR|O_CREAT, 0644);
	if ( fd >= 0 ) {
		do
			errno = 0;
		while ( flock(fd, LOCK_EX) && errno == EINTR );
		if ( errno )
			(void)close(fd), fd = -1;
	}
	return fd;
}
