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
 * $Header: rmt_syscall1.c,v 2.0 85/12/07 18:19:21 toddb Rel $
 *
 * $Log:	rmt_syscall1.c,v $
 * Revision 2.0  85/12/07  18:19:21  toddb
 * First public release.
 * 
 */
#include	"../h/param.h"
#ifndef pyr	/* Pyramid */
#include	"../machine/pte.h"
#endif
#include	"../h/systm.h"
#include	"../h/map.h"
#include	"../h/dir.h"
#include	"../h/user.h"
#include	"../h/kernel.h"
#include	"../h/proc.h"
#include	"../h/mbuf.h"
#include	"../h/socket.h"
#include	"../h/file.h"
#include	"../remote/remotefs.h"
#include	"../h/stat.h"
#include	"../h/errno.h"
#include	"../netinet/in.h"

extern struct remoteinfo	remote_info[];

/*
 * Remote access()
 */
rmt_access (sysindex, m)
	int	sysindex;
	struct mbuf	*m;
{
	struct message	*msg = mtod(m, struct message *);
	struct a {
		char	*path;
		long	mode;
	} *uap = (struct a *)u.u_ap;

	msg->m_args[ 0 ] = htonl(uap->mode);
	
	/*
	 * Now send it.
	 */
	return( rmt_msgfin(sysindex, m, 0) );
}

/*
 * Remote chdir()
 */
rmt_chdir (sysindex, m)
	int	sysindex;
	struct mbuf	*m;
{
	int	error;

	/*
	 * Now send it.
	 */
	if ((error = rmt_msgfin(sysindex, m, 0)) == 0)
		error = remotechdir(sysindex);

	return( error );
}

/*
 * Remote chmod() and fchmod()
 */
rmt_chmod (sysindex, m)
	int	sysindex;
	struct mbuf	*m;
{
	int	i = 0;
	struct message *msg = mtod(m, struct message *);
	struct a {
		long	path_or_fd;
		long	mode;
	} *uap = (struct a *)u.u_ap;

	if (htons(msg->m_syscall) == RSYS_fchmod)
		i++;
	msg->m_args[ i ] = htonl(uap->mode);
	/*
	 * Now send it.
	 */
	return( rmt_msgfin(sysindex, m, 0) );
}

/*
 * Remote chown() and fchown()
 */
rmt_chown (sysindex, m)
	int	sysindex;
	struct mbuf	*m;
{
	int	i = 0;
	struct message *msg = mtod(m, struct message *);
	struct a {
		long	path_or_fd;
		long	owner, group;
	} *uap = (struct a *)u.u_ap;

	if (htons(msg->m_syscall) == RSYS_fchown)
		i++, m->m_len = R_MINRMSG + 3*sizeof(long);
	msg->m_args[ i++ ] = htonl(uap->owner);
	msg->m_args[ i ] = htonl(uap->group);
	/*
	 * Now send it.
	 */
	return( rmt_msgfin(sysindex, m, 0) );
}

/*
 * Remote dup()
 */
rmt_dup (sysindex, m)
	long	sysindex;
	struct mbuf	*m;
{
	int	fd, error;
	struct message *msg = mtod(m, struct message *);
	struct file	*fp;
	struct a {
		long	fd;
	} *uap = (struct a *)u.u_ap;

	fp = u.u_ofile[ uap->fd ];
	fd = ufalloc(0);
	if (fd < 0)
		return(-1);
	remote_info[ (int)fp->f_data ].r_nfile++;
	dupit(fd, fp, u.u_pofile[uap->fd]);
	msg->m_args[ 1 ] = htonl(fd);

	/*
	 * Now send it.
	 */
	error = rmt_msgfin(sysindex, m, 0);
	if (error)
		rmt_deallocfd(fd);
	return(error);
}

/*
 * Remote dup2()
 */
rmt_dup2 (sysindex, m)
	long	sysindex;
	struct mbuf	*m;
{
	struct message *msg = mtod(m, struct message *);
	struct file	*oldfp, *newfp;
	struct a {
		long	oldfd,
			newfd;
	} *uap = (struct a *)u.u_ap;
	long	oldfd = uap->oldfd,
		newfd = uap->newfd,
		error;

	oldfp = u.u_ofile[ oldfd ];
	if (newfd >= NOFILE)
		return(EBADF);
	if (oldfd == newfd)
		return(newfd);
	if (newfp = u.u_ofile[ newfd ]) {
		/*
		 * If the new file descriptor (which must be closed) is
		 * remote on system 'n', then we may have to send a close
		 * message to system 'n', but only if
		 *	1. the file descriptor being duped is non-remote
		 *		or
		 *	2. the file descriptor being duped is on a different
		 *	   remote system.
		 * Note that case number 1 implies that there is no more
		 * remote work to be done.
		 */
		if (newfp->f_flag & FREMOTE) {
			if ((oldfp->f_flag & FREMOTE) == 0
			 || newfp->f_data != oldfp->f_data) {
				if ((oldfp->f_flag & FREMOTE) == 0)
					sysindex = -1;
				uap->oldfd = uap->newfd;
				remote_fd(RSYS_close);
			}
			else
				closef(newfp);
		}
		else
			closef(newfp);
	}
	dupit(newfd, oldfp, u.u_pofile[ oldfd ]);

	/*
	 * We may already be done.
	 */
	if (sysindex < 0)
		return(0);
	
	/*
	 * Now send it.
	 */
	remote_info[ sysindex ].r_nfile++;
	msg->m_args[ 1 ] = htonl(newfd);
	error = rmt_msgfin(sysindex, m, 0);
	if (error)
		rmt_deallocfd(newfd);

	return(error);
}

/*
 * routine for handling an error.  We should never get here... but if we
 * do.....
 */
rmt_error(sysnum)
	int	sysnum;
{
	debug1("error reached\n");
	return(EINVAL);
}
