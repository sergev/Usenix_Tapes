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
 * $Header: rmt_syscall2.c,v 2.0 85/12/07 18:19:28 toddb Rel $
 *
 * $Log:	rmt_syscall2.c,v $
 * Revision 2.0  85/12/07  18:19:28  toddb
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
#include	"../remote/remotefs.h"
#include	"../h/file.h"
#include	"../h/stat.h"
#include	"../h/errno.h"
#include	"../netinet/in.h"
#include	"../h/uio.h"

extern struct remoteinfo	remote_info[];
extern syscalls			remote_syscall[];

/*
 * Remote fcntl()
 */

rmt_fcntl (sysindex, m)
	int	sysindex;
	struct mbuf	*m;
{
	register long		fd,
				error;
	register struct message *msg = mtod(m, struct message *);
	register struct a {
		long	fd,
			command,
			arg;
	} *uap = (struct a *)u.u_ap;
	register struct file	*fp;

	if (uap->command == F_DUPFD) {
		fp = u.u_ofile[ uap->fd ];
		fd = ufalloc(uap->arg);
		if (fd < 0)
			return(-1);
		remote_info[ (int)fp->f_data ].r_nfile++;
		dupit(fd, fp, u.u_pofile[uap->fd]);
		msg->m_args[ 1 ] = htonl(fd);
		msg->m_syscall = htons(RSYS_dup);
	}
	else {
		msg->m_args[ 1 ] = htonl(uap->command);
		msg->m_args[ 2 ] = htonl(uap->arg);
		m->m_len += sizeof(long);
	}

	/*
	 * Now send it
	 */
	error = rmt_msgfin(sysindex, m, 0);
	if (error && uap->command == F_DUPFD)
		rmt_deallocfd(fd);

	return(error);
}

/*
 * Remote flock()
 */
rmt_flock (sysindex, m)
	int	sysindex;
	struct mbuf	*m;
{
	struct message *msg = mtod(m, struct message *);
	struct a {
		long	fd;
		long	operation;
	} *uap = (struct a *)u.u_ap;

	msg->m_args[ 1 ] = htonl(uap->operation);
	/*
	 * Now send it.
	 */
	return( rmt_msgfin(sysindex, m, 0) );
}

/*
 * Remote ioctl()
 */

rmt_ioctl (sysindex, m, request, argp)
	int	sysindex,
		request;
	struct mbuf	*m;
	char	*argp;
{

	/*
	 * for now always fail.
	 */
	return(EINVAL);
}

/*
 * Remote lseek()
 */
rmt_lseek (sysindex, m)
	long	sysindex;
	struct mbuf	*m;
{
	struct message	*msg = mtod(m, struct message *);
	struct a {
		long	fd,
			offset,
			whence;
	} *uap = (struct a *)u.u_ap;
	register long	flags = RFLG_INFO;
	register long	twhence = uap->whence;
	register long	error;
	register struct file	*fp = u.u_ofile[ uap->fd ];

	/*
	 * As a special case for the sake of speed, L_INCR and L_SET can be
	 * done locally and then the message can be sent as
	 * info only (no reply).
	 */
	switch (twhence) {
	case L_INCR:
		/*
		 * Very special case: lseek(fd, 0, L_INCR) is a noop.
		 */
		if (uap->offset == 0) {
			m_free(m);
			u.u_r.r_off = fp->f_offset;
			return(0);
		}
		fp->f_offset += uap->offset;
		break;
	case L_XTND:
		flags = 0;
		break;
	case L_SET:
		fp->f_offset = uap->offset;
		break;
	default:
		m_free(m);
		u.u_error = EINVAL;
		return(0);
	}
	msg->m_args[ 1 ] = htonl(uap->offset);
	msg->m_args[ 2 ] = htonl(twhence);
	m->m_len += sizeof(long);
	if (flags == RFLG_INFO)
		msg->m_syscall = htons(RSYS_qlseek);

	/*
	 * Now send it.
	 */
	error = rmt_msgfin(sysindex, m, flags);
	if (flags != RFLG_INFO)
		fp->f_offset = u.u_r.r_val1;
	else
		u.u_r.r_off = fp->f_offset;
	return( error );
}

/*
 * Remote mknod()
 */
rmt_mknod (sysindex, m, mode, dev)
	long	sysindex,
		mode, dev;
	struct mbuf	*m;
{
	struct message	*msg = mtod(m, struct message *);
	struct a {
		char	*path;
		long	mode,
			dev;
	} *uap = (struct a *)u.u_ap;

	msg->m_args[ 0 ] = htonl(uap->mode);
	msg->m_args[ 1 ] = htonl(uap->dev);
	msg->m_args[ 2 ] = htonl(u.u_cmask);

	/*
	 * Now send it.
	 */
	return( rmt_msgfin(sysindex, m, 0) );
}

rmt_noop() {	return;	}

/*
 * Remote mkdir() and rmdir() and unlink() and fsync() and close()
 */
rmt_onearg (sysindex, m)
	int	sysindex;
	struct mbuf	*m;
{
	struct message	*msg = mtod(m, struct message *);
	short	syscall = ntohs(msg->m_syscall);
	long	error;

	if (syscall == RSYS_close) {
		struct a {
			long	fd;
		} *uap = (struct a *)u.u_ap;
		rmt_deallocfd(uap->fd);
		m->m_len = R_MINRMSG + sizeof(long);
	}
	else if (syscall == RSYS_fsync)
		m->m_len = R_MINRMSG + sizeof(long);
	else
		msg->m_args[ 0 ] = htonl(u.u_cmask);

	error = rmt_msgfin(sysindex, m, remote_syscall[ syscall ].sys_flag);
	return(error);
}
