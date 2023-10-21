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
 * $Header: rmt_final.c,v 2.1 85/12/30 16:51:01 toddb Exp $
 *
 * $Log:	rmt_final.c,v $
 * Revision 2.1  85/12/30  16:51:01  toddb
 * Made rmt_copypath() compatible between 4.2 and 4.3 by adding a c-version
 * of copystr() and copyinstr() for 4.2 versions (copystr() and copyinstr()
 * are assembler routines in 4.3).
 * 
 * Revision 2.0  85/12/07  18:20:12  toddb
 * First public release.
 * 
 */
#include	"../h/param.h"
#ifndef pyr /* Pyramid */
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
#include	"../h/errno.h"
#include	"../netinet/in.h"
#include	"../h/uio.h"

extern struct remoteinfo	remote_info[];

/*
 * rmt_msgfin() finalizes the message by adding in the header and total
 * lengths and sending the message.  It returns the return value found in the
 * reply.
 */
rmt_msgfin(sysindex, m, flags)
	long	sysindex, flags;
	struct mbuf	*m;
{
	long	error, len;
	struct message	*msg = mtod(m, struct message *);
	struct remoteinfo	*rp = remote_info + sysindex;

	/*
	 * If the length has been set (by remote_path[12]), then don't
	 * meddle, otherwise assign it.
	 */
	if (msg->m_hdlen == 0)
		len = m->m_len;
	else
		len = msg->m_hdlen;
	msg->m_hdlen = htons(len);
	msg->m_totlen = htonl(len);
		
	rmt_showmsg(msg, FALSE);
	error = remoteio(sysindex, &m, 0, flags);

	if (! error) {
		if ((flags & RFLG_INFO) == 0) {
			msg = &rp->r_msg;
			rmt_showmsg(msg, TRUE);
			if ((error = msg->m_errno) == 0)
				u.u_r.r_val1 = msg->m_args[R_RETVAL];
		}
		else
			u.u_r.r_val1 = 0;
	}
	return ( error );
}

/*
 * rmt_datafin() finalizes a message and fixes up the header and total
 * lengths (like rmt_msgfin()) and sends a message plus data.  It returns
 * the return value found in the reply.
 */
rmt_datafin(sysindex, m, flags)
	long	sysindex, flags;
	struct mbuf	*m;
{
	register long	i, error, len;
	struct message	*msg = mtod(m, struct message *);
	short	syscall = ntohs(msg->m_syscall);
	struct remoteinfo	*rp = remote_info + sysindex;
	struct uio auio, *uio;
	struct iovec aiov[16], *iov = aiov;
	struct a {
		long	fd;
		char	*databuf;
		long	datalen;
	} *uap = (struct a *)u.u_ap;

	/*
	 * Package up the data.
	 * If the length has been set (by remote_path1 for readlink),
	 * then don't meddle, otherwise assign it the length of the first
	 * (and only) mbuf.
	 */
	if (msg->m_hdlen == 0)
		msg->m_hdlen = htons(len = m->m_len);
	else
		msg->m_hdlen = htons(len = msg->m_hdlen);
	if (flags & (RFLG_RD | RFLG_WR)) {
		uio = &auio;
		uio->uio_iov = iov;
		uio->uio_segflg = 0;
		uio->uio_offset = 0;
		if (flags & RFLG_DATAV) {
			uio->uio_iovcnt = uap->datalen;
			error = copyin((caddr_t)uap->databuf, (caddr_t)iov,
			    (unsigned)(uap->datalen * sizeof (struct iovec)));
			if (error)
				goto done;
			uio->uio_resid = 0;
			for (i = 0; i < uio->uio_iovcnt; i++) {
				if (iov->iov_len < 0) {
					error = EINVAL;
					goto done;
				}
				uio->uio_resid += iov->iov_len;
				iov++;
			}
		}
		else {
			iov->iov_base = uap->databuf;
			iov->iov_len = uio->uio_resid = uap->datalen;
			uio->uio_iovcnt = 1;
		}
		if (uio->uio_resid < 0) {
			error = EINVAL;
			goto done;
		}
		if (flags & RFLG_WR)
			msg->m_totlen = htonl(len + uio->uio_resid);
		else
			msg->m_totlen = htonl(len);
	}
	else {
		uio = NULL;
		msg->m_totlen = htonl(len);
	}
	rmt_showmsg(msg, FALSE);
	error = remoteio(sysindex, &m, uio, flags);
	m = NULL;
	if (error)
		goto done;

	msg = &rp->r_msg;
	rmt_showmsg(msg, TRUE);
	if ((error = msg->m_errno) == 0)
		u.u_r.r_val1 = msg->m_args[R_RETVAL];
done:
	if (m)
		m_free(m);
	return ( error );
}

/*
 * Copy a path into a string of mbufs.  The source pointer may be in user
 * space, in which case we must use uchar() to copy.  Note that this routine
 * expects 'm' to point to the first mbuf in the current chain:  it will
 * append to the end of the chain, and update the length of the message
 * in msg->m_hdlen.
 */
rmt_copypath(psrc, m, copy_from_user)
	register char	*psrc;
	register struct mbuf	*m;
{
	register char	*pdest;
	register int	error,
			totlen,
			room;
	register struct mbuf	*m2;
	struct message	*msg = mtod(m, struct message *);
	int		copystr(),
			copyinstr(),
			got;
	func		copier = copy_from_user ? copyinstr : copystr;

	/*
	 * find the end of the mbuf chain.
	 */
	while (m->m_next)
		m = m->m_next;
	m2 = m;
	pdest = mtod(m, char *) + m->m_len;
	totlen = msg->m_hdlen;

	debug2("copy %s path @%x-->%x, len=%d\n",
		copy_from_user ? "user" : "kernel", psrc, pdest, totlen);

	room = MLEN - m->m_len;
	for(;;) {
		got = 0;	/* copy*str adds copied bytes to 'got' */
		error = (*copier)(psrc, pdest, room, (u_int *)&got);
		if (error && error != ENOENT)
			return(error);
		m2->m_len += got;
		totlen += got;
		if (got < room)
			break;
		MGET(m, M_WAIT, MT_DATA);
		if (m == NULL)
			return(ENOBUFS);
		m2 = m2->m_next = m;
		m2->m_len = 0;
		room = MLEN;
		pdest = mtod(m2, char *);
		psrc += got;
	}
	msg->m_hdlen = totlen;
	debug2("len now=%d\n", totlen);

	return(0);
}

#ifdef BSD4_3
#else BSD4_3
/*
 * C version of copyinstr() from 4.3.
 */
copyinstr(src, dest, maxlength, lencopied)
	register char	*src, *dest;
	register int	maxlength;
	register int	*lencopied;
{
	register int	c;
	int		dummy;

	if (!maxlength)
		return(EFAULT);
	if (lencopied == NULL)
		lencopied = &dummy;
	for(;;) {
		c = *dest++ = fubyte(src++);
		if (c == -1) {
			*(dest-1) = 0;
			return(EFAULT);
		}
		if (--maxlength < 0)
			return(ENOENT);
		(*lencopied)++;
		if (c == 0)
			return(0);
	}
}

/*
 * C version of copystr() from 4.3.
 */
copystr(src, dest, maxlength, lencopied)
	register char	*src, *dest;
	register int	maxlength;
	register int	*lencopied;
{
	int		dummy;

	if (!maxlength)
		return(EFAULT);
	if (lencopied == NULL)
		lencopied = &dummy;
	for(;;) {
		if (--maxlength < 0)
			return(ENOENT);
		(*lencopied)++;
		if ((*dest++ = *src++) == 0)
			return(0);
	}
}
#endif BSD4_3

#ifdef RFSDEBUG

rmt_debug(x0,x1,x2,x3,x4,x5,x6,x7,x8,x9)
{
	printf(x0,x1,x2,x3,x4,x5,x6,x7,x8,x9);
}

rmt_showmsg(msg, fromserver)
	struct message	*msg;
	int	fromserver;
{
	long	end, len, pid, uid, totlen;

	if ((remote_debug & 0x18000) == 0)
		return;
	if (fromserver)
		len = msg->m_hdlen,
		totlen = msg->m_totlen,
		pid = msg->m_pid,
		uid = msg->m_uid;
	else
		len = ntohs(msg->m_hdlen),
		totlen = ntohl(msg->m_totlen),
		pid = ntohs(msg->m_pid),
		uid = ntohs(msg->m_uid);
	debug15("%s srvr: len=%d,tot=%d,pid=%d,uid=%d",
		fromserver ? "from" : "to", len, totlen, pid, uid);

	/* round up into long words */
	len = (len - R_MINRMSG + 3) >> 2;
	if (fromserver)
	{
		debug15(",err=%d,ret=%d",
			msg->m_errno,
			msg->m_args[ R_RETVAL ]);
		end = R_RETVAL+1;
	}
	else
	{
		debug15(",syscall=%d", htons(msg->m_syscall));
		end = 0;
	}
	for (; end<len; end++)
		debug16(",0x%x", ntohl(msg->m_args[ end ]));
	rmt_debug("\n");
}
#endif RFSDEBUG
