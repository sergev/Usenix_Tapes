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
 * $Header: rmt_general.c,v 2.0 85/12/07 18:18:06 toddb Rel $
 *
 * $Log:	rmt_general.c,v $
 * Revision 2.0  85/12/07  18:18:06  toddb
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
#ifdef BSD4_3
#include	"../h/namei.h"
#else BSD4_3
#include	"../h/nami.h"
#endif BSD4_3
#include	"../h/inode.h"
#include	"../h/mbuf.h"
#include	"../h/socket.h"
#include	"../remote/remotefs.h"
#include	"../h/errno.h"
#include	"../netinet/in.h"
#include	"../h/file.h"

extern long	remote_sysindex;
extern struct mbuf	*remote_path;
extern u_char	remote_sysmap[];
extern syscalls	remote_syscall[];
extern struct remoteinfo	remote_info[];

/*
 * This routine is the main gateway into the remote software.  It should
 * be called from syscall().  Note that sysnum is already mapped to our
 * idea of the system call number.
 */
#ifdef pyr	/* Pyramid */
remote_startup(pass, sysnum, arg1, arg2, arg3, arg4, arg5, arg6)
#else pyr
remote_startup(pass, sysnum)
#endif
	long	sysnum, pass;
{
	struct a {
		long	arg1,
			arg2;
	} *uap = (struct a *)u.u_ap;
	long	error, sysindex;
	extern long	remote_fd();
	register syscalls	*sp = remote_syscall + sysnum;
	register struct file	*fp;
	register struct proc	*p;
	register func	general = sp->sys_gen;
	register unsigned	fd1,
				fd2;
#ifdef pyr	/* Pyramid */
	long	args[ 6 ];

	args[0] = arg1;
	args[1] = arg2;
	args[2] = arg3;
	args[3] = arg4;
	args[4] = arg5;
	args[5] = arg6;
	u.u_ap = args;
	uap = (struct a *) args;
#endif pyr

	debug1("%d strt:#%d,pass%d,%s\n",
		u.u_procp->p_pid, sysnum, pass,
		pass == 0 ? (!sp->sys_before ? "too soon" : "before") :
			    (pass == 1       ? "do it"    : "too late"));

	/*
	 * For the greatest of speed we check here for invalid or non-
	 * remote file descriptors.  Make sure that we let a dup2() go
	 * through, even if the first fd is non-remote.
	 */
	if (general == remote_fd) {
		/*
		 * Gross kludge from v7 for the bourne shell.
		 */
		fd1 = uap->arg1;
		if (sysnum == RSYS_dup && (fd1 &~ 077)) {
			fd1 &= 077;
			sysnum = RSYS_dup2;
		}
		error = 2;
		if (fd1 >= NOFILE
		|| (fp = u.u_ofile[fd1]) == NULL
		|| (fp->f_flag & FREMOTE) == 0)
			error--;
		if (sysnum != RSYS_dup2)
			error--;
		else {
			fd2 = uap->arg2;
			if (fd2 >= NOFILE
			|| (fp = u.u_ofile[fd2]) == NULL
			|| (fp->f_flag & FREMOTE) == 0)
				error--;
		}
		if (! error)
			return(FALSE);
		uap->arg1 = fd1;
	}
	/*
	 * Check to see if this is being done too soon.  Note that this is
	 * on the else side of the check for remote_fd.  i.e., this code
	 * assumes that remote_fd() routines should always be done before
	 * the real routines.
	 */
	else if (pass == 0) {
		u.u_rmtoffset[0] = -1;
		u.u_rmtoffset[1] = -1;
		if (! sp->sys_before)
			return(FALSE);
	}
	else if (pass > 1) {
		u.u_error = EISREMOTE;
		return(TRUE);
	}

	/*
	 * finally, if the user has turned off remote access for himself,
	 * then just return.
	 */
	p = u.u_procp;
	if (p->p_flag & SNOREMOTE)
		return(FALSE);
	u.u_error = 0;

	/*
	 * do the remote syscall.
	 */
	error = (*general)(sysnum);
	debug1("%d startup done: ret1=%d, ret2=%d, err=%d\n",
		u.u_procp->p_pid, u.u_r.r_val1, u.u_r.r_val2, error);
	if (error < 0)	/* call the real system call */
		return(FALSE);
	u.u_error = error;
	return(TRUE);
}

/*
 * This routine handles most system calls that have no special requirements
 * and have a single path in their first argument.
 */
remote_path1(sysnum)
	int	sysnum;
{
	struct a {
		char	*path;
	} *uap = (struct a *)u.u_ap;
	long	sysindex, len;
	struct message *msg;
	struct mbuf	*m;
	register syscalls	*sp = remote_syscall + sysnum;
	register long		error;

	/*
	 * Get the path mbuf chain and its length.  Also the remote sys #.
	 */
	if ((m = remote_path) == NULL)
		return(ENOBUFS);
	for (len=0; m; m = m->m_next)
		len += m->m_len;
	m = remote_path;
	remote_path = NULL;
	sysindex = remote_sysindex;

	/*
	 * Initialize the message and call the specific syscall handler.
	 */
	msg = mtod(m, struct message *);
	introduce(msg, sysnum);
	msg->m_hdlen = len;
	error = (*sp->sys_spec)(sysindex, m, sysnum);

	if (error < 0) {
		msg = &remote_info[ sysindex ].r_msg;
#ifdef BSD4_3
#else BSD4_3
		u.u_dirp = (caddr_t) u.u_arg[0];
#endif BSD4_3
		u.u_rmtoffset[0] = msg->m_args[0];
		return ( -1 );
	}
	return ( error );
}
/*
 * This routine handles all two-path system calls.
 *
 *	RSYS_link: the placement of path2 determines where
 *		we run the syscall because path2 is the file
 * 		that is created.  Isremote() must also resolve the
 *		path1 so that the remote link will know what file to
 *		link it to.  Both path1 and path2 must be on the same
 *		system.  Unfortunately, in kernel mode, we don't know
 *		which path was found to be remote: we must check both again.
 *	RSYS_rename: Same as RSYS_link.
 *	RSYS_symlink: path2 is the only possible remote file and
 *		path1 does not need to be resolved because symlink()
 *		blindly creats a symbolic link to it.
 */
remote_path2(sysnum)
	int	sysnum;
{
	struct a {
		char	*path1;
		char	*path2;
	} *uap = (struct a *)u.u_ap;
	long	sysindex,
		len,
		error;
	struct message	*msg;
	struct mbuf	*m;

	if ((m = remote_path) == NULL)
		return(ENOBUFS);
	/*
	 * If this is rename or link, then throw away what is in remote_path
	 * because we don't know which path it refers to .
	 */
	remote_path = NULL;
	if (sysnum != RSYS_symlink) {
		m_freem(m);
		m = NULL;
		sysindex = rmt_checkpath(uap->path2, &m, sysnum);
		if (sysindex < 0)
			return(-1);
	}
	else
		sysindex = remote_sysindex;
	/*
	 * Ok, path2 is now safely in our mbuf.  Set the PATHOFF field for
	 * the beginning of where path1 will be.
	 */
	msg = mtod(m, struct message *);
	introduce_1extra(msg, sysnum, u.u_cmask);
	msg->m_args[ R_PATHOFF ] = htonl(msg->m_hdlen);

	/*
	 * If its a symbolic link, we can just copy the path onto the
	 * end of our mbuf chain.
	 */
	if (sysnum == RSYS_symlink) {
		error = rmt_copypath(uap->path1, m, TRUE);
		if (error) {
			m_freem(m);
			return(error);
		}
	}
	/*
	 * Link and rename have to have path1 on the same system as path2.
	 */
	else if (rmt_checkpath(uap->path1, &m, sysnum) != sysindex) {
		m_freem(m);
		return(-1);
	}

	error = rmt_msgfin(sysindex, m, 0);

	if(error < 0)
		if (sysnum != RSYS_rename && sysnum != RSYS_link) {
			msg = &remote_info[ sysindex ].r_msg;
#ifdef BSD4_3
#else BSD4_3
			u.u_dirp = (caddr_t) u.u_arg[0];
#endif BSD4_3
			u.u_rmtoffset[0] = msg->m_args[0];
			u.u_rmtoffset[1] = ntohl(msg->m_args[1]);
			return ( -1 );
		}
		else
			error = EISREMOTE;
	return( error );
}

/*
 * Check a remote file for its "remoteness".  After namei() is called (in
 * the kernel) and isremote() is called as a result, we can get the result
 * from remote_path and put it on the end of 'mhead'.
 */
rmt_checkpath(path, mhead, sysnum)
	char	*path;
	struct mbuf	**mhead;
	long	sysnum;
{
	char	*psrc, *pdest;
	struct mbuf	*m;
	struct message	*msg;
	long	len;
	struct inode	*ip;

#ifdef BSD4_3
	register struct nameidata *ndp = &u.u_nd;
	long	follow = remote_syscall[ sysnum ].sys_follow ? FOLLOW : 0;

	ndp->ni_nameiop = LOOKUP | follow;
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = (caddr_t)path;
	ip = namei(ndp);
#else BSD4_3
	u.u_dirp = path;
	ip = namei(uchar, LOOKUP, remote_syscall[ sysnum ].sys_follow);
#endif BSD4_3
	if (ip != NULL || u.u_error != EISREMOTE) {
		if (ip)
			iput(ip);
		return(-1);
	}
	u.u_error = 0;

	if (remote_path == NULL)
		return(-1);
	if (*mhead == NULL)
		*mhead = remote_path;
	else {
		/*
		 * If we were handed an mbuf, then we tack the new string
		 * of mbufs on the end.  Note that we bump the offset so that
		 * the mtod(m, char *) points to the beginning of the path.
		 */
		for (m = *mhead; m->m_next; )
			m = m->m_next;
		m->m_next = remote_path;
		msg = mtod(remote_path, struct message *);
		len = msg->m_hdlen - (R_MINRMSG + R_PATHSTART*sizeof(long));
		remote_path->m_off += R_MINRMSG + R_PATHSTART*sizeof(long);
		remote_path->m_len -= R_MINRMSG + R_PATHSTART*sizeof(long);
		msg = mtod(*mhead, struct message *);
		msg->m_hdlen += len;
	}
	remote_path = NULL;
	return(remote_sysindex);
}

/*
 * Remote exit()
 */
remote_exit()
{
	struct remoteinfo	*rp, *rmt_hostdir();
	struct file	*fp;
	long	i;
	struct mbuf	*m;
	struct message *msg;

	/*
	 * Throw away remote file descriptors.
	 */
	for (i=0; i<NOFILE; i++)
		if ((fp = u.u_ofile[ i ]) && (fp->f_flag & FREMOTE))
			rmt_deallocfd(i);
	u.u_procp->p_flag &= ~SREMOTE;
	if (rp = rmt_hostdir(u.u_cdir, &i))
		rp->r_nchdir--;
	/*
	 * Send the exit message to every remote system to which we
	 * have a connection.
	 */
	for (rp = remote_info, i=0; i<R_MAXSYS; i++, rp++)
		if (rmthostused(i) && rp->r_sock) {
			/*
			 * If ours is the last usage of this connection, then
			 * shut it down.
			 */
			debug4("%d off #%d, now %d usrs\n", u.u_procp->p_pid,
				i, rp->r_users-1); 
			if (--rp->r_users <= 0 && rp->r_sock) {
				if (rp->r_close)
					rmt_shutdown(i);
				else
					rmt_closehost(rp);
				continue;
			}
			MGET(m, M_WAIT, MT_DATA);
			if (m == NULL)
				break;
			msg = mtod(m, struct message *);
			msg->m_hdlen = m->m_len = introduce(msg, RSYS_exit);
			rmt_msgfin(i, m, RFLG_INFO);
		}

	return( -1 ); /* do the real syscall, too */
}

/*
 * Remote fork()
 */
remote_fork(sysnum)
	long	sysnum;
{
	register long		i, child, pid, ppid, val1;
	long			rmtdir = u.u_rmtcdir,
				sysindex;
	struct message		*msg;
	register struct remoteinfo	*rp;
	struct remoteinfo	*rmt_hostdir();
	struct mbuf	*m;
	register struct file	*fp;
	long			rmtsys;	

	/*
	 * We may not need to even notify anyone, if this process is not
	 * doing anything interesting.  If there are no open files or
	 * a remote current working directory, then do no more.
	 */
	rmtcopyhosts(rmtsys, u.u_rmtsys);
	rmtclearhosts();
	for (i=0; i<NOFILE; i++) {
		if ((fp = u.u_ofile[ i ]) && (fp->f_flag & FREMOTE)) {
			remote_info[ (int)fp->f_data ].r_nfile++;
			rmtusehost((int)fp->f_data);
		}
	}
	if (rp = rmt_hostdir(u.u_cdir, &sysindex)) {
		rmtusehost(sysindex);
		rp->r_nchdir++;
	}

	/*
	 * Do the fork.
	 */
	if (sysnum == RSYS_vfork)
		vfork();
	else
		fork();
	val1 = u.u_r.r_val1;
	child = u.u_r.r_val2;
	if (u.u_error)
		child = FALSE;
	if (u.u_error || u.u_rmtsys == 0)
		goto done;

	if (child) {
		ppid = u.u_procp->p_ppid;
		pid = u.u_procp->p_pid;
		u.u_procp->p_flag |= SREMOTE; /* set remote flag in child */
		u.u_rmtcdir = rmtdir;
	}
	else { /* parent */
		/* "if I am the parent && this is a vfork" */
		if (sysnum == RSYS_vfork)
			goto done;
		ppid = u.u_procp->p_pid;
		pid = u.u_r.r_val1;
	}
	/*
	 * Parent (fork() only) and child tell all remote hosts.
	 * Also, bump the count of users using all connections.
	 */
	for (rp=remote_info, i=0; i<R_MAXSYS; i++, rp++)
		if (rmthostused(i) && rp->r_sock) {
			if(child)
				rp->r_users++;
			debug4("fork: %s %d notify %s(%d), users=%d\n",
				child ? "chld" : "prnt", u.u_procp->p_pid,
				rp->r_mntpath, i, rp->r_users);
			MGET(m, M_WAIT, MT_DATA);
			if (m == NULL)
				continue;
			msg = mtod(m, struct message *);
			msg->m_hdlen = m->m_len =
				introduce_2extra(msg, sysnum, pid, ppid);
			rmt_msgfin(i, m, RFLG_INFO);
		}
	/*
	 * In the kernel, the r_val[12] elements get tromped on by the
	 * io to the server hosts, so restore them here.
	 * The 'u.u_error = 0;' is so that we don't run the real syscall
	 * (already run) and so that any io errors while notifying servers
	 * don't returned to the user... otherwise he might think the fork
	 * or vfork failed.
	 */
	u.u_r.r_val1 = val1;
	u.u_r.r_val2 = child;
	u.u_error = 0;
done:
	if (! child)
		rmtcopyhosts(u.u_rmtsys, rmtsys);
	return(u.u_error);
}

/*
 * This routine handles most system calls having a file descriptor as
 * its first argument.  We are guarenteed at this point that uap->fd is
 * a valid remote file descriptor.  We optimize for reads and writes.
 */
remote_fd(sysnum)
	register long	sysnum;
{
	struct a {
		long	fd;
		char	*buf;
		long	len;
	} *uap = (struct a *)u.u_ap;
	register long	sysindex, error;
	register struct message	*msg;
	register struct mbuf	*m;
	register syscalls	*sp = remote_syscall + sysnum;
	register struct file	*fp = u.u_ofile[ uap->fd ];

	MGET(m, M_WAIT, MT_DATA);
	if (m == NULL)
		return(ENOBUFS);
	msg = mtod(m, struct message *);
	msg->m_hdlen = 0; /* rmt_datafin() or rmt_msgfin() will assign this */
	m->m_len = introduce_2extra(msg, sysnum, uap->fd, uap->len);

	sysindex = (long)fp->f_data;
	error = (*sp->sys_spec)(sysindex, m, sp->sys_flag);
	if (! error)
		switch (sysnum) {
		case RSYS_read:
		case RSYS_write:
		case RSYS_readv:
		case RSYS_writev:
			fp->f_offset += u.u_r.r_val1;
			break;
		}
	return(error);
}

/*
 * Deallocate a file descriptor.
 */
rmt_deallocfd(fd)
	int	fd;
{
	register struct file	*fp;
	register struct remoteinfo	*rp;
	register unsigned	system;

	fp = u.u_ofile[fd];
	u.u_ofile[fd] = NULL;
	if (fp == NULL
	|| ((system = (unsigned)fp->f_data) >= R_MAXSYS)) {
		debug6("dealloc: fp=%x,fd=%d,sys=%d,pid=%d\n",
			fp, fd, system, u.u_procp->p_pid);
		return;
	}

	remote_info[ (int)fp->f_data ].r_nfile--;
	closef(fp);
}
