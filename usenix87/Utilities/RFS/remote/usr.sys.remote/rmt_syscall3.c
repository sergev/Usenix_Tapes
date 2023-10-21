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
 * $Header: rmt_syscall3.c,v 2.0 85/12/07 18:19:35 toddb Rel $
 *
 * $Log:	rmt_syscall3.c,v $
 * Revision 2.0  85/12/07  18:19:35  toddb
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
#include	"../h/inode.h"
#include	"../h/mbuf.h"
#include	"../h/socket.h"
#include	"../remote/remotefs.h"
#include	"../h/file.h"
#include	"../h/stat.h"
#include	"../h/errno.h"
#include	"../netinet/in.h"

extern long	rmtrw(), rmtioctl(), rmtselect(), rmtclose();
struct 	fileops remoteops =
	{ rmtrw, rmtioctl, rmtselect, rmtclose };
rmtrw()	{ return(0); }
rmtioctl()	{ return(0); }
rmtselect()	{ return(0); }
rmtclose()	{ return(0); }

extern struct remoteinfo	remote_info[];
extern struct remoteinfo	*remote_generic;

/*
 * Remote open()
 */
rmt_open (sysindex, m)
	long	sysindex;
	struct mbuf	*m;
{
	register long	fd, error;
	register struct message *msg = mtod(m, struct message *);
	register struct file	*fp;
	struct file		*falloc();
	register struct a {
		char	*path;
		long	flags,
			mode;
	} *uap = (struct a *)u.u_ap;

	/*
	 * Initialize the file structure.
	 */
	if ((fp = falloc()) == NULL)
		return(-1);
	fp->f_type = DTYPE_REMOTE;
	fp->f_ops = &remoteops;
	fd = u.u_r.r_val1;
	fp->f_data = (caddr_t)sysindex;
	fp->f_flag = (uap->mode&FMASK) | FREMOTE;
	remote_info[ sysindex ].r_nfile++;

	if (ntohs(msg->m_syscall) == RSYS_creat) {
		uap->mode = uap->flags;
		uap->flags = FWRITE|FCREAT|FTRUNC;
		msg->m_syscall = htons(RSYS_open);
	}
	msg->m_args[ 0 ] = htonl(uap->flags);
	msg->m_args[ 1 ] = htonl(uap->mode);
	msg->m_args[ 2 ] = htonl(fd);
	msg->m_args[ 3 ] = htonl(u.u_cmask);

	/*
	 * Now send it.
	 */
	error = rmt_msgfin(sysindex, m, 0);
	if (error)
		rmt_deallocfd(fd);
	else {
		msg = &remote_info[ sysindex ].r_msg;
		fp->f_offset = msg->m_args[1];
	}
	return(error);
}

/*
 * Remote readlink()
 */
rmt_readlink (sysindex, m)
	long	sysindex;
	struct mbuf	*m;
{
	struct message	*msg = mtod(m, struct message *);
	struct a {
		char	*path;
		char	*buf;
		long	len;
	} *uap = (struct a *)u.u_ap;


	msg->m_args[ 0 ] = htonl(uap->len);
	/*
	 * Now send it.
	 */
	return( rmt_datafin(sysindex, m, RFLG_RD) );
}

/*
 * Remote stat() and lstat() and fstat()
 */
rmt_stat (sysindex, m)
	long	sysindex;
	struct mbuf	*m;
{
	long	error, i;
	struct message *msg = mtod(m, struct message *);
	struct remoteinfo *rp = remote_info + sysindex;
	struct a {
		long	fd_or_path;
		struct stat	*statp;
	} *uap = (struct a *)u.u_ap;
	struct stat	st;

	if (ntohl(msg->m_syscall) == RSYS_fstat)
		m->m_len = R_MINRMSG + sizeof(long);
	if (error = rmt_msgfin(sysindex, m, 0))
		return( error );
	msg = &rp->r_msg;
	i = R_RETVAL + 1;
	st.st_dev	  = ntohl(msg->m_args[ i++ ]);
	st.st_ino	  = ntohl(msg->m_args[ i++ ]);
	st.st_mode	  = ntohl(msg->m_args[ i++ ]);
	st.st_nlink	  = ntohl(msg->m_args[ i++ ]);
	st.st_uid	  = ntohl(msg->m_args[ i++ ]);
	st.st_gid	  = ntohl(msg->m_args[ i++ ]);
	st.st_rdev	  = ntohl(msg->m_args[ i++ ]);
	st.st_size	  = ntohl(msg->m_args[ i++ ]);
	st.st_atime	  = ntohl(msg->m_args[ i++ ]);
	st.st_mtime	  = ntohl(msg->m_args[ i++ ]);
	st.st_ctime	  = ntohl(msg->m_args[ i++ ]);
	st.st_blksize	  = ntohl(msg->m_args[ i++ ]);
	st.st_blocks 	  = ntohl(msg->m_args[ i++ ]);
	st.st_spare1 = sysindex+1;
	st.st_spare2 = 0;
	st.st_spare3 = 0;
	st.st_spare4[0] = st.st_spare4[1] = 0;
	if (msg->m_args[ i ]) { /* this is the remote root */
		if (rp->r_mntpt == NULL)
			rp = remote_generic;
		if (rp && rp->r_mntpt != NULL) {
			st.st_dev = rp->r_mntpt->i_dev;
			st.st_ino = rp->r_mntpt->i_number;
		}
	}
	(void)copyout((caddr_t)&st, (caddr_t)uap->statp, sizeof(struct stat));
	return(0);
}

/*
 * Remote truncate() and ftrucate()
 */
rmt_truncate (sysindex, m)
	long	sysindex;
	struct mbuf	*m;
{
	long	i = 0;
	struct message *msg = mtod(m, struct message *);
	struct a {
		long	fd_or_path;
		long	len;
	} *uap = (struct a *)u.u_ap;

	if (htons(msg->m_syscall) == RSYS_ftruncate)
		i++;
	msg->m_args[ i ] = htonl(uap->len);
	/*
	 * Now send it.
	 */
	return( rmt_msgfin(sysindex, m, 0) );
}

/*
 * Remote utimes()
 */
rmt_utimes (sysindex, m)
	long	sysindex;
	struct mbuf	*m;
{
	struct message	*msg = mtod(m, struct message *);
	struct a {
		char	*path;
		struct timeval	*timevalp;
	} *uap = (struct a *)u.u_ap;
	struct timeval	*tv = uap->timevalp;

	msg->m_args[ 0 ] = htonl(tv[0].tv_sec);
	msg->m_args[ 1 ] = htonl(tv[0].tv_usec);
	msg->m_args[ 2 ] = htonl(tv[1].tv_sec);
	msg->m_args[ 3 ] = htonl(tv[1].tv_usec);

	/*
	 * Now send it.
	 */
	return( rmt_msgfin(sysindex, m, 0) );
}
