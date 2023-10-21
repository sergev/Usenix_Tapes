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
 * $Header: rmt_generic.c,v 2.2 86/01/02 13:52:32 toddb Exp $
 *
 * $Log:	rmt_generic.c,v $
 * Revision 2.2  86/01/02  13:52:32  toddb
 * Ifdef'ed calls to sockargs() for the differences in 4.2 vs. 4.3.
 * 
 * Revision 2.1  85/12/30  16:58:59  toddb
 * Isremote() was not freeing it's chain of mbufs if rmt_copypath() failed.
 * Now it does.
 * 
 * Revision 2.0  85/12/07  18:18:27  toddb
 * First public release.
 * 
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/inode.h"
#include "../h/dir.h"
#ifdef BSD4_3
#include "../h/namei.h"
#else BSD4_3
#include "../h/nami.h"
#endif BSD4_3
#ifndef pyr	/* Pyramid */
#include "../machine/pte.h"
#endif
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/buf.h"
#include "../h/file.h"
#include "../h/uio.h"
#include "../h/mbuf.h"
#include "../h/socket.h"
#include "../h/socketvar.h"
#include "../h/errno.h"
#include "../remote/remotefs.h"

extern	long		remote_sysindex;
extern struct mbuf	*remote_path;
extern struct remoteinfo remote_info[];
extern struct remoteinfo *remote_generic;
extern struct nameserver	remote_ns;

/*
 * Remote "mount point" definition.
 */
#ifdef pyr	/* Pyramid */
remoteon(arg1, arg2, arg3, arg4)
{
#ifdef BSD4_3
	register struct nameidata *ndp = &u.u_nd;
#else BSD4_3
#endif BSD4_3
	struct a {
		char	*path;
		unsigned pathlen;
		caddr_t	name;
		unsigned namelen;
	} ua;
	register struct a *uap = &ua;
	register struct inode		*ip;
	register struct remoteinfo	*rp;
	struct remoteinfo		*rmt_findslot(),
					*rmt_host();
	register int			error = 0;
	long				sysnum;
	struct mbuf *m = NULL;

	uap->path = (char *)arg1;
	uap->pathlen = (unsigned)arg2;
	uap->name = (caddr_t)arg3;
	uap->namelen = (unsigned)arg4;
#else pyr
remoteon()
{
#ifdef BSD4_3
	register struct nameidata *ndp = &u.u_nd;
#else BSD4_3
#endif BSD4_3
	register struct a {
		char	*path;
		unsigned pathlen;
		caddr_t	name;
		unsigned namelen;
	} *uap = (struct a *)u.u_ap;
	register struct inode		*ip;
	register struct remoteinfo	*rp;
	struct remoteinfo		*rmt_findslot(),
					*rmt_host();
	register int			error = 0;
	long				sysnum;
	struct mbuf *m = NULL;
#endif

	if (uap->path == NULL) {
		u.u_procp->p_flag &= ~SNOREMOTE;
		return;
	}
	if (!suser())
		return;
#ifdef BSD4_3
	ndp->ni_nameiop = LOOKUP;
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = (caddr_t)uap->path;
	ip = namei(ndp);
#else BSD4_3
	ip = namei(uchar, LOOKUP, 0);
#endif BSD4_3
	debug5("remote on ip=%x,path=%x,pthlen=%d,name=%x,nmlen=%d\n",
		ip, uap->path, uap->pathlen, uap->name, uap->namelen);
	if (ip == NULL)
		return;
	if (uap->pathlen >= R_MNTPATHLEN)
		uap->pathlen = R_MNTPATHLEN - 1;

	/*
	 * Check for all kinds of errors.
	 */
	if (rmt_host(ip, &sysnum) || (uap->name == NULL && remote_generic))
		error = EBUSY;
	else if ((ip->i_mode&IFMT) == IFDIR)
		error = EISDIR;
	else if (uap->name)
#ifdef BSD4_3
		error = sockargs(&m, uap->name, uap->namelen, MT_SONAME);
#else BSD4_3
		error = sockargs(&m, uap->name, uap->namelen);
#endif BSD4_3
	if (error)
		goto out;

	/*
	 * Everything is ok... so put it in our list.
	 */
	rp = rmt_findslot(&sysnum);
	if (rp == NULL)
		error = ETOOMANYREMOTE;
	else {
		debug5("rp=%x, m=%x ip=%x sys=%d\n", rp, m, ip, sysnum);
		rp->r_mntpt = ip;
		if ((rp->r_name = m) == NULL)
			remote_generic = rp;
		(void)copyin((caddr_t)uap->path, (caddr_t)rp->r_mntpath,
			MIN(uap->pathlen, R_MNTPATHLEN));
		u.u_r.r_val1 = sysnum;
		iunlock(ip);
		return;
	}

out:
	u.u_error = error;
	iput(ip);
}

/*
 * Turn off the remote file system.  If the path to unmount is NULL, then
 * turn off all remote access for this process.
 */
#ifdef pyr	/* Pyramid */
remoteoff(arg1)
{
#ifdef BSD4_3
	register struct nameidata *ndp = &u.u_nd;
#else BSD4_3
#endif BSD4_3
	struct a {
		char	*path;
	} ua;
	register struct a *uap = &ua;
	register struct inode *ip;
	register struct remoteinfo	*rp;
	struct remoteinfo		*rmt_host();
	long			sysnum;

	uap->path = (char *)arg1;
#else pyr
remoteoff()
{
#ifdef BSD4_3
	register struct nameidata *ndp = &u.u_nd;
#else BSD4_3
#endif BSD4_3
	register struct a {
		char	*path;
	} *uap = (struct a *)u.u_ap;
	register struct inode *ip;
	register struct remoteinfo	*rp;
	struct remoteinfo		*rmt_host();
	long			sysnum;
#endif pyr

	if (uap->path == NULL) {
		u.u_procp->p_flag |= SNOREMOTE;
		return;
	}
	if (!suser())
		return;
#ifdef BSD4_3
	ndp->ni_nameiop = LOOKUP;
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = (caddr_t)uap->path;
	ip = namei(ndp);
#else BSD4_3
	ip = namei(uchar, LOOKUP, 0);
#endif BSD4_3
	if (ip == NULL || (rp = rmt_host(ip, &sysnum)) == NULL) {
		if (ip)
			iput(ip);
		debug5("remoteoff: nope! ip=%x error=%d\n",
			ip, u.u_error);
		u.u_error = EINVAL;
		return;
	}

	debug5("remote off ip=%x\n", ip);
	iput(ip);

	/*
	 * Now try to shut it down. 
	 */
	rp->r_close = TRUE;
	if (u.u_error = rmt_shutdown(sysnum))
		return;
	u.u_r.r_val1 = remote_sysindex;
	/*
	 * reinitialize the structure for the next time,
	 * freeing the mbuf and closing the socket.
	 */
	rp->r_refcnt = 0;
	rp->r_nchdir = 0;
	rp->r_nfile = 0;
	if (rp->r_mntpt)
		irele(rp->r_mntpt);
	rp->r_mntpt = 0;
	if (rp->r_name)
		(void)m_free(rp->r_name);
	else
		remote_generic = NULL;
	rp->r_name = 0;
	return;
}

/*
 * This provides the nameserver function allowing
 * name information to pass to and from a server and the kernel.
 */
#ifdef pyr	/* Pyramid */
remotename(arg1, arg2, arg3, arg4)
{
	struct a {
		long	action;
		caddr_t	name;
		long	namelen;
		char	*path;
	} ap;
	register struct a *uap = &ap;
	register long	error = 0;
	struct mbuf		*m;
	register struct proc	*p = u.u_procp;

	uap->action = (long)arg1;
	uap->name = (caddr_t)arg2;
	uap->namelen = (long)arg3;
	uap->path = (char *)arg4;
#else pyr
remotename()
{
	register struct a {
		long	action;
		caddr_t	name;
		long	namelen;
		char	*path;
	} *uap = (struct a *)u.u_ap;
	register long	error = 0;
	struct mbuf		*m;
	register struct proc	*p = u.u_procp;
#endif pyr

	if ((uap->action == NM_WHATNAME || uap->action == NM_NAMEIS)
	&& ! server_alive(p))
		error = EPERM;
	else switch (uap->action) {
	case NM_SERVER:		/* register as name server */
	{
		struct proc	*p2;
		short		pid;

		if (!suser())
			return;
		p2 = remote_ns.rn_proc;
		pid = remote_ns.rn_pid;
		if (server_alive(p2))
			error = EBUSY;
		else {
			remote_ns.rn_proc = p;
			remote_ns.rn_pid = p->p_pid;
		}
		break;
	}
	case NM_WHATNAME:
		if (remote_ns.rn_path)
			error = copyout((caddr_t)remote_ns.rn_path,
				(caddr_t)uap->path,
				MIN(uap->namelen, remote_ns.rn_pathlen));
		else
			error = ENOREMOTEFS;
		break;
	case NM_NAMEIS:
	{
		register char	*cp;

		m = remote_ns.rn_name;
		if (m) {
			debug13("free old mbuf@%x\n", m);
			m_free(m); /* free extra mbuf */
			m = remote_ns.rn_name = NULL;
		}
#ifdef BSD4_3
		error = sockargs(&m, uap->name, uap->namelen, MT_SONAME);
#else BSD4_3
		error = sockargs(&m, uap->name, uap->namelen);
#endif BSD4_3
		if (error == 0) {
			cp = mtod(m, char *) + m->m_len;
			if (error = copyin((caddr_t)uap->path, (caddr_t)cp,
			    MIN(R_MNTPATHLEN, MLEN - m->m_len))) {
				m_free(m);
				m = NULL;
			}
			debug13("nmsrv: %s@%x\n", cp, m);
		}
		remote_ns.rn_name = m;
		wakeup(&remote_ns.rn_name);
		break;
	}
#ifdef RFSDEBUG
	case NM_DEBUG:
		remote_debug = (long)uap->name;
		printf("dbg=%d\n", remote_debug);
		break;
#endif RFSDEBUG
	default:
		error = EINVAL;
		break;
	}
	if (error)
		u.u_error = error;
}

/*
 * This is the routine called by namei() when it encounters what it thinks
 * might be a remote mount point.
 */
isremote(ip, path, base)
	register struct inode *ip;
	char		*path, *base;
{
	struct remoteinfo *rmt_host(), *rmt_findhost();
	register short	offset;
	register struct remoteinfo *rp;
	register struct mbuf	*m;
	register struct message	*msg;
	register int	error;
	long			sysnum;

	rp = rmt_host(ip, &sysnum);
	if (rp == NULL) {
		debug7("%s, ip=\"%x\" is not remote\n", path, ip);
		return(FALSE);
	}
	if (remote_path)
		m_freem(remote_path);
	remote_path = NULL;

	/*
	 * Adjust the path so that if there is a leading '/', the path
	 * will start there.
	 */
	if (path != base && *path != '/' && *(path-1) == '/')
		path--;

	/*
	 * Although we know the file is remote, it may have a loop back
	 * to this side; a simple case being if the path is '..' while
	 * in the root directory of the remote system.  If this is the case
	 * then the server will reply with errno == -1, remote_path[12]()
	 * will assign u.u_rmtoffset[0,1] and the real system call will be
	 * run again, bringing us back to here.  We know that that has
	 * happened if either of u.u_rmtoffset[0,1] is >= 0.  We then
	 * just adjust the path handed to us by namei() and return
	 * -1 which signals namei() to begin again with the new
	 * path with directory set to root (/).
	 */
	if ((u.u_procp->p_flag & SREMOTE) == 0) {
		u.u_rmtcdir = -1;
		offset = -1;
	}
	else if ((offset = u.u_rmtoffset[0]) >= 0)
		u.u_rmtoffset[0] = -1;
	else if ((offset = u.u_rmtoffset[1]) >= 0)
		u.u_rmtoffset[1] = -1;
	if (offset >= 0) {
		register char	*pstart = path + offset;
		register char	*p = path;

		debug8("restart path %s locally, offset=%d\n", path, offset);
		if (offset) /* validate offset value */
			while (p <= pstart)
				if (*p++ == '\0') {
					u.u_error = EINVAL;
					return(FALSE);
				} 
		if (base != pstart) {
			path = base;
			do {
				*path++ = *pstart;
			} while (*pstart++);
		}
		return(-1);
	}

	/*
	 * We have a remote mount point.  However, it may be in the midst of
	 * shutting down from some error.  This mount point may also be
	 * a "generic" mount point, in which case, we must figure out
	 * the host.
	 */
	if (rp->r_name == NULL) {
		rp = rmt_findhost(&path, &sysnum);
		if (rp == NULL) {
			debug8("isremote: can't map path %s\n", path);
			return(TRUE);
		}
	}
	if (rp->r_close) {
		debug8("isremote: host %s is closing\n", rp->r_mntpath);
		u.u_error = ENOREMOTEFS;
		return(TRUE);
	}
	u.u_error = EISREMOTE;
	remote_sysindex = sysnum;
	/*
	 * Set the remote flag for this user and bump the
	 * user count.
	 */
	u.u_procp->p_flag |= SREMOTE;
	if (! rmthostused(sysnum)) {
		rp->r_users++;
		rmtusehost(sysnum);
		debug4("%d uses rmt %d\n", u.u_procp->p_pid, sysnum);
	}
	rmtusehost(sysnum);
	debug7("%s, ip=%x is remote, idx=%d\n", path, ip, remote_sysindex);
	MGET(m, M_WAIT, MT_DATA);
	if (m != NULL) {
		msg = mtod(m, struct message *);
		msg->m_hdlen = R_MINRMSG + R_PATHSTART*sizeof(long);
		m->m_len = R_MINRMSG + R_PATHSTART*sizeof(long);
		if (error = rmt_copypath(path, m, FALSE)) {
			m_freem(m);
			u.u_error = error;
			return(TRUE);
		}
		remote_path = m;
	}
	else
		u.u_error = ENOBUFS;
	return(TRUE);
}
