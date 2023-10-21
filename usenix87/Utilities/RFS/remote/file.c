/*
 * Copyright 1985, Todd Brunhoff.
 *
 * This software was written at Tektronix Computer Research Laboratories
 * as partial fulfillment of a Master's degree at the University of Denver.
 * This is not Tektronix proprietary software and should not be
 * confused with any software product sold by Tektronix.  No warranty is
 * expressed or implied on the reliability of this software; the author,
 * the University of Denver, and Tektronix, inc. accept no liability for
 * any damage done directly or indirectly by this software.  This software
 * may be copied, modified or used in any way, without fee, provided this
 * notice remains an unaltered part of the software.
 *
 * $Log:	file.c,v $
 * Revision 2.0  85/12/07  18:21:11  toddb
 * First public release.
 * 
 */
static char	*rcsid = "$Header: file.c,v 2.0 85/12/07 18:21:11 toddb Rel $";
#include	"server.h"
#include	<errno.h>

extern int	fds_in_use;
extern int	errno;

/*
 * here we allocate a file descriptor to a process and make note of our
 * total number of open file descriptors.  'fd' is the file descriptor
 * that the server itself got back, and remotefd is the file descriptor
 * that the remote process is expecting.
 */
allocate_fd(fd, proc, remotefd)
	register int	fd,
			remotefd;
	register process	*proc;
{
	if (fd != -1)
	{
		proc->p_fds[ remotefd ] = fd;
		debug3("allocate local fd %d, remote fd %d\n",
			fd, remotefd);
		fds_in_use++;
		checkfiletype(fd);
	}
	else
		remotefd = -1;
	return(remotefd);
}

deallocate_fd(proc, remotefd)
	register process	*proc;
	register long	remotefd;
{
	register char	fd;
	register long	retval;

	if ((unsigned)remotefd >= NOFILE)
	{
		errno = EBADF;
		return(-1);
	}
	fd = proc->p_fds[ remotefd ];
	proc->p_fds[ remotefd ] = -1;
	retval = close(fd);
	fds_in_use--;
	return(retval);
}
