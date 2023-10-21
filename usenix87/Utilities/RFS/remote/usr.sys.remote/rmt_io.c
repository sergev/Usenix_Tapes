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
 * $Header: rmt_io.c,v 2.1 85/12/30 16:53:01 toddb Exp $
 *
 * $Log:	rmt_io.c,v $
 * Revision 2.1  85/12/30  16:53:01  toddb
 * Changed shutdown messages for the remotefs from debug messages to 
 * printf messages.  Also fixed a bug where the return value field in
 * rp->r_msg.m_args[ R_RETVAL ] was getting cleared for process `x'
 * by process `y' when process `y' was executing an INFO_ONLY type
 * of remote system call (like close or fork).  This bug was causing
 * random system calls (like read or write) to return 0 when it should
 * have returned something non-zero.
 * 
 * Revision 2.0  85/12/07  18:18:51  toddb
 * First public release.
 * 
 */
#include "../h/param.h"
#include "../h/dir.h"
#ifndef pyr	/* Pyramid */
#include "../machine/pte.h"
#endif
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/buf.h"
#include "../h/uio.h"
#include "../h/mbuf.h"
#include "../h/socket.h"
#include "../h/socketvar.h"
#include "../h/protosw.h"
#include "../h/errno.h"
#include "../remote/remotefs.h"
#include "../h/inode.h"
#include "../h/kernel.h"
#include "../netinet/in.h"

extern struct remoteinfo	remote_info[];
extern struct remoteinfo	*remote_generic;

/*
 * Obtain a connection to 'system'.
 */
remote_getconnection(system)
	register int	system;
{
	register struct remoteinfo	*rp = remote_info + system;
	register err, s, opening = FALSE;
	short	uid;
	struct socket *so = NULL;

	if (! rp->r_name) {
		rmt_unuse(rp, system);
		return(ENOREMOTEFS);	/* no address to call. */
	}

	/*
	 * Lock out other processes from doing an open at the same time.
	 */
	if (rp->r_opening) {
		if (setjmp(&u.u_qsave)) {
			rmt_unuse(rp, system);
			return(EINTR);
		}
		while (rp->r_opening)
			sleep((caddr_t)&rp->r_sock);
		if (rp->r_sock)
			return(0);
		rmt_unuse(rp, system);
		return(rp->r_openerr);
	}
	/*
	 * We may already have a connection
	 */
	else if (rp->r_sock)
		return(0);
	/*
	 * If we have failed previously, it may be time to try again.
	 */
	else if (rp->r_failed) {
		if (rp->r_age > time.tv_sec)
			return(rp->r_openerr);
		rp->r_failed = FALSE;
	}
	rp->r_opening = TRUE;

	/*
	 * pseudo loop to avoid labels...
	 */
	do {
		/*
		 * Fortunately, there is security with ports.  Unfortunately,
		 * you must be root to do it.  So we change our uid for a
		 * brief moment.
		 */
		uid = u.u_uid;
		if (setjmp(&u.u_qsave)) {
			u.u_uid = uid;
			if (u.u_error == 0)
				err = EINTR;
			break;
		}

		/*
		 * first, make a socket for the connection; then connect.  (the
		 * connection code is basically connect(2)).
		 */
		if (err = socreate(AF_INET, &rp->r_sock, SOCK_STREAM, 0))
			break;

		so = rp->r_sock;
		debug9("connect...");
		err = soconnect(so, rp->r_name);
		u.u_uid = uid;
		if (err)
			break;

		s = splnet();
		if ((so->so_state & SS_NBIO) &&
		    (so->so_state & SS_ISCONNECTING)) {
			err = EINPROGRESS;
			splx(s);
			break;
		}
		while ((so->so_state & SS_ISCONNECTING) && so->so_error == 0) {
			debug9(".");
			sleep((caddr_t)&so->so_timeo, PZERO+1);
		}
		err = so->so_error;
		so->so_error = 0;
		rp->r_sock = so;
		rp->r_sender = rp->r_recver = -1;
		sosetopt(so, SOL_SOCKET, SO_KEEPALIVE, NULL);
		splx(s);
	} while (FALSE);

	if (err) {
		rp->r_sock = 0;
		rp->r_openerr = err;
		rp->r_failed = TRUE;
		rp->r_age = time.tv_sec + R_RETRY;
		debug9("err=%d\n", err);
		rmt_unuse(rp, system);
		if (so)
			soclose(so);
	}
	else
		debug9(" done.\n");
	rp->r_opening = FALSE;
	wakeup((caddr_t)&rp->r_sock);
	return(err);
}

/*
 * Back out of references to a remote host.
 */
rmt_unuse(rp, system)
	register struct remoteinfo *rp;
	register long	system;
{
	rp->r_users--;
	rmtunusehost(system);
	if (u.u_rmtsys == 0)
		u.u_procp->p_flag &= ~SREMOTE;
}

/*
 * Send out the message.
 */
remoteio(system, m, uio, flags)
	register int	system;
	register struct mbuf	**m;
	register struct uio *uio;
	int	flags;
{
	register int	error, signaled = 0, position;
	register struct remoteinfo	*rp = remote_info + system;
	register struct socket	*so;
	register struct iovec *iov;
	register struct proc	*p;
	long	soreceive(), sosend();
	long	oldmask;


	/*
	 * get a connection.
	 */
	if ((so = rp->r_sock) == NULL)
		if (error = remote_getconnection(system)) {
			(void) m_freem(*m);
			return(error);
		}
		else
			so = rp->r_sock;

	/*
	 * "Hold" SIGTSTP and SIGSTOP signals until we are done.
	 */
	p = u.u_procp;
	oldmask = (1 << (SIGTSTP-1))
		| (1 << (SIGSTOP-1));
	oldmask = (p->p_sigmask & oldmask);
	p->p_sigmask |= (1 << (SIGTSTP-1))
			| (1 << (SIGSTOP-1));

	/*
	 * Note that we re-do the setjmp each time we change state.
	 * Two reasons: 1) we are effectively ignoring interrupts until
	 * either the message has been completely sent or has been
	 * completely recieved. 2) setjmp() restores register variables to
	 * their state at the time of the call and since we modify them
	 * all the time, we need to re-save the state.
	 */
	position = R_NOTHINGSENT;
	rp->r_refcnt++; /* we are actively using conection */

	while(position != 0) {
		if (setjmp(&u.u_qsave)) {
			error = EINTR;
			debug10("signal to %d\n", u.u_procp->p_pid);
			if (rp->r_close)
				goto remoteio_done;
			signaled++;
			continue;
		}

		switch (position) {
		case R_NOTHINGSENT:
			/*
			 * make sure that this is not someone elses data.
			 */
			if (signaled)
				goto remoteio_done;
			while (!rp->r_close && rp->r_sender >= 0) {
				debug10("I am %d, not %d. Goodnight.\n",
					u.u_procp->p_pid, rp->r_sender);
				sleep((caddr_t)&rp->r_sender, PZERO+1);
			}
			if (rp->r_close)
				goto remoteio_done;

			/*
			 * We update position BEFORE we call tcp_usrreq()
			 * because we are guarenteed that the data will be
			 * sent just by making the call.  i.e. we will never
			 * come back to this point once we've been here.
			 */
			if (uio && (flags & RFLG_WR))
				position = R_DATANOTSENT;
			else
				position = R_MSGNOTRED;
			rp->r_sender = u.u_procp->p_pid;
			debug10("%d sending... ", rp->r_sender);
			error = (*so->so_proto->pr_usrreq)(so,
				PRU_SEND, *m, 0, 0);
			if (error) {
				printf("error=%d on sending msg\n", error);
				rp->r_close = TRUE;
				goto remoteio_done;
			}
			*m = 0;
			break;

		case R_DATANOTSENT:
			debug10("%d data\n", u.u_procp->p_pid);
			if (error = rmt_uio(rp, uio, sosend))
				flushmsg(rp, uio->uio_resid, FALSE);
			position = R_MSGNOTRED;
			break;

		case R_MSGNOTRED:
			/*
			 * Finally, read from the socket.  Also, if
			 * we have been interrupted, now is the time to
			 * notify the other side.
			 */
			rp->r_sender = -1;
			wakeup(&rp->r_sender);
			if (flags & RFLG_INFO) {
				position = 0;
				break;
			}
			debug10("%d recving\n", u.u_procp->p_pid);
			if (signaled)
				sendrsig(system);
			error = rmt_getmsg(system);
			rp->r_received = FALSE;
			if (error || rp->r_close)
				goto remoteio_done;
			if (uio && (flags & RFLG_RD))
				position = R_DATANOTRED;
			else {
				rp->r_recver = -1;
				position = 0;
				wakeup(&rp->r_recver);
			}
			break;
		case R_DATANOTRED:
			debug10("%d recving data\n", u.u_procp->p_pid);
			if (rp->r_msg.m_args[ R_RETVAL ] > uio->uio_resid) {
				printf("usr needs %d, srvr says %d\n",
					uio->uio_resid,
					rp->r_msg.m_args[ R_RETVAL ]);
				rp->r_close = TRUE;
				goto remoteio_done;
			}
					
			uio->uio_resid = rp->r_msg.m_args[ R_RETVAL ];
			if (error = rmt_uio(rp, uio, soreceive)) {
				flushmsg(rp, uio->uio_resid, TRUE);
				break;
			}
			rp->r_recver = -1;
			wakeup(&rp->r_recver);
			position = 0;
			break;
		}
	}
remoteio_done:
	/*
	 * Restore mask by first taking out SIGTSTP and SIGSTOP, whatever
	 * their values.  And then or'ing in the original value.
	 */
	p = u.u_procp;
	p->p_sigmask &= ~((1 << (SIGTSTP-1))
			| (1 << (SIGSTOP-1)));
	p->p_sigmask |= oldmask;

	rp->r_refcnt--; /* we are done with this transaction */
	if (rp->r_close)
		error = ECONNABORTED;
	if (error) {
		debug11("remoteio: err=%d, close=%s\n",
			error, rp->r_close ? "true" : "false");
		rmt_shutdown(system);
	}
	return(error);
}

/*
 * Force io to happen and consume all of uio->uio_resid.
 */
rmt_uio(rp, uio, sockfunc)
	register struct remoteinfo	*rp;
	register struct uio	*uio;
	register func		sockfunc;
{
	register struct socket	*so = rp->r_sock;
	label_t	qsave;
	register long	error = 0,
			flag = (sockfunc == soreceive)
				? SS_CANTRCVMORE : SS_CANTSENDMORE;

	bcopy(&u.u_qsave, &qsave, sizeof(label_t));
	if (setjmp(&u.u_qsave)) {
		debug11("rmt_uio: sig %d\n", u.u_procp->p_pid);
		if (rp->r_close)
			error = ECONNABORTED;
	}
	while(uio->uio_resid > 0 && ! error && (so->so_state & flag) == 0)
		error = (*sockfunc)(so, 0, uio, 0, 0);
	bcopy(&qsave, &u.u_qsave, sizeof(label_t));
	if (so->so_state & flag) {
		rp->r_close = TRUE;
		error = ECONNABORTED;
	}
	return(error);
}

/*
 * Obtain the next message from the server.  We don't return from this
 * routine until our personal message has been received or an error
 * has occured.
 */
rmt_getmsg(system)
	register int	system;
{
	struct proc	*p = NULL;
	struct remoteinfo	*rp = remote_info + system;
	struct socket *so = rp->r_sock;
	struct uio	auio;
	struct iovec	iov;
	register long	msglen, len, error = 0;
	long		soreceive();

	for(;;) {
		iov.iov_len = auio.uio_resid = R_MINRMSG+sizeof(long);
		auio.uio_segflg = 1;	/* kernel bcopy */
		auio.uio_iov = &iov;
		auio.uio_iovcnt = 1;
		auio.uio_offset = 0;
		iov.iov_base = (caddr_t)&rp->r_msg;

		/*
		 * Since we may have been usurped by this time, or a different
		 * syscall may have been repsponded to, we must make sure
		 * that we are the recipient of this message.  In fact,
		 * the message may have already arrived.
		 */
		while (rp->r_recver != u.u_procp->p_pid && rp->r_recver != -1)
			sleep((caddr_t)&rp->r_recver, PZERO+1);
		if (rp->r_recver == u.u_procp->p_pid && rp->r_received) {
			auio.uio_resid = 0;
			break;
		}
		rp->r_recver = u.u_procp->p_pid;

		/*
		 * get the message.
		 */
		if (error = rmt_uio(rp, &auio, soreceive)) {
			debug11("1st: err=%d\n", error);
			break;
		}
#ifndef magnolia
		rp->r_msg.m_totlen  = ntohl(rp->r_msg.m_totlen);
		rp->r_msg.m_hdlen   = ntohs(rp->r_msg.m_hdlen);
		rp->r_msg.m_pid     = ntohs(rp->r_msg.m_pid);
		rp->r_msg.m_uid     = ntohs(rp->r_msg.m_uid);
		rp->r_msg.m_errno = ntohs(rp->r_msg.m_errno);
		rp->r_msg.m_args[ R_RETVAL ]
			    = ntohl(rp->r_msg.m_args[ R_RETVAL ]);
#endif
		msglen = rp->r_msg.m_hdlen;
		if (msglen > R_MAXRMSG
		 || msglen < R_MINRMSG+sizeof(long)
		 || auio.uio_offset < R_MINRMSG+sizeof(long)) {
			debug11("msg len=%d, off=%d!\n",
				msglen, auio.uio_offset);
			error = ECONNABORTED;
			break;
		}

		/*
		 * We may need a few more bytes.
		 */
		if (msglen > R_MINRMSG + sizeof(long)) {
			iov.iov_len = auio.uio_resid =
				msglen - auio.uio_offset;
			auio.uio_iov = &iov;
			auio.uio_iovcnt = 1;
			debug10("getmsg2: resid=%d... cc=%d\n",
				auio.uio_resid, so->so_rcv.sb_cc);
			if (error = rmt_uio(rp, &auio, soreceive)) {
				debug11("2nd: err=%d, resid=%d, msglen=%d\n",	
					error, auio.uio_resid, msglen);
				break;
			}
		}

		/*
		 * Now find the right recipient.
		 */
		rp->r_received = TRUE;
		rp->r_recver = rp->r_msg.m_pid;
		if (rp->r_recver == u.u_procp->p_pid)
			break;
		if (rp->r_recver >= 0 && (p = pfind(rp->r_recver))) {
			debug11("%d: msg for %d@%x\n",
				u.u_procp->p_pid, rp->r_recver, p->p_wchan);
			if (p->p_wchan == (caddr_t)&rp->r_recver)
				setrun(p);
			continue;
		}
		else
			debug11("proc %d?\n", rp->r_recver);
		flushmsg(rp, rp->r_msg.m_totlen - rp->r_msg.m_hdlen, TRUE);
	}

	/*
	 * Wrap up and decide if everything is still kosher...
	 */
	if (rp->r_close || error || auio.uio_resid) {
		debug11("%d: getmsg: close=%s,err=%d,resid=%d\n",
			u.u_procp->p_pid, rp->r_close ? "true" : "false",
			error, auio.uio_resid);
		rp->r_close = TRUE;
		rmt_shutdown(system);
		error = ECONNABORTED;
	}
	return(error);
}

flushmsg(rp, len, input)
	register struct remoteinfo	*rp;
	register long	len, input;
{
	register struct socket *so = rp->r_sock;
	register long	error = 0, need;
	struct uio	auio;
	struct iovec	iov;
	char	buf[ MLEN ];
	extern long	sosend(), soreceive();
	func		ioroutine = (input ? soreceive : sosend);

	debug11("flush %d %s bytes\n", len, input ? "input" : "output");
	auio.uio_segflg = 1;	/* kernel bcopy */
	while (len > 0 && ! error) {
		need = iov.iov_len = auio.uio_resid = MIN(len, MLEN);
		auio.uio_iov = &iov;
		auio.uio_iovcnt = 1;
		iov.iov_base = (caddr_t)buf;
		debug11("flush: resid=%d... cc=%d\n",
			auio.uio_resid, so->so_rcv.sb_cc);
		error = rmt_uio(rp, &auio, ioroutine);
		len -= need - auio.uio_resid;
	}
	if (error)
		rp->r_close = TRUE;
	if (input) {
		rp->r_recver = -1;
		rp->r_received = FALSE;
		wakeup((caddr_t)&rp->r_recver);
	}
	else {
		rp->r_sender = -1;
		ioroutine = sosend;
		wakeup((caddr_t)&rp->r_sender);
	}
	debug11("flush: error=%d\n", error);
}

rmt_shutdown(system)
	register int	system;
{
	register struct remoteinfo *rp = remote_info + system;
	register struct proc	*p;

	wakeup((caddr_t)&rp->r_sender);
	wakeup((caddr_t)&rp->r_recver);
	if (rp->r_recver >= 0 && (p = pfind(rp->r_recver)) && p->p_wchan) {
		debug12("wake rder %d\n", p->p_pid);
		wakeup((caddr_t)p->p_wchan);
	}
	if (rp->r_sender >= 0 && (p = pfind(rp->r_sender)) && p->p_wchan) {
		debug12("wake wrter %d\n", p->p_pid);
		wakeup((caddr_t)p->p_wchan);
	}
	debug12("shtdwn: ref=%d, reason=", rp->r_refcnt);
	if (rp->r_close || (rp->r_sock->so_state & SS_CANTRCVMORE))
		debug12("%s\n",
			(rp->r_sock->so_state & SS_CANTRCVMORE)
			? "no more" : "closed");
	else {
		debug12("?, not done\n");
		return(EBUSY);
	}
	rp->r_close = TRUE;
	if (rp->r_refcnt || rp->r_users)
		return(EBUSY);
	rmt_closehost(rp);
	return(0);
}

/*
 * Close a host connection.
 */
rmt_closehost(rp)
	register struct remoteinfo	*rp;
{
	struct socket	*so;

	so = rp->r_sock;
	rp->r_recver = rp->r_sender = -1;
	rp->r_sock = NULL;
	rp->r_age = time.tv_sec;
	rp->r_close = 0;
	if (so)
		soclose(so);
	else
		debug12("rmt_closehost: so == 0, rp=%x\n", rp);
}

sendrsig(system)
	register int	system;
{
	debug11("would have sent sig to system %d\n", system);
}
