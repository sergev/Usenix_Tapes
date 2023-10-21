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
 * $Header: rmt_exec.c,v 2.1 85/12/30 16:45:30 toddb Exp $
 *
 * $Log:	rmt_exec.c,v $
 * Revision 2.1  85/12/30  16:45:30  toddb
 * fixed syntax error for 4.3.
 * 
 * Revision 2.0  85/12/07  18:17:54  toddb
 * First public release.
 * 
 */
#ifndef pyr /* Pyramid */
#include	"../machine/reg.h"
#include	"../machine/pte.h"
#include	"../machine/psl.h"
#endif

#include	"../h/param.h"
#include	"../h/systm.h"
#include	"../h/map.h"
#include	"../h/dir.h"
#include	"../h/user.h"
#include	"../h/kernel.h"
#include	"../h/proc.h"
#include	"../h/file.h"
#include	"../h/uio.h"
#include	"../remote/remotefs.h"
#include	"../h/mbuf.h"
#include	"../netinet/in.h"
#include	"../h/vm.h"
#include	"../h/errno.h"
#ifdef BSD4_3
#include	"../h/namei.h"
#include	"../h/exec.h"
#else BSD4_3
#include	"../h/nami.h"
#endif BSD4_3

#ifdef vax
#include "../vax/mtpr.h"
#endif

extern struct remoteinfo	remote_info[];
extern long			remote_sysindex;
extern long			remote_maxchunk;

#ifdef magnolia
#define	DEMAND_PAGED	0513
#define	SHARED_TEXT	0510
#else magnolia
#define	DEMAND_PAGED	0413
#define	SHARED_TEXT	0410
#endif magnolia

/*
 * Get execution info for a file that is on a remote system.  At this point
 * we know that the file is remote.  We must send one request to get the
 * header and then if all is ok, we will be asked for the file to be sent over
 * en-masse.
 */
#ifdef BSD4_3
remote_execinfo(ip, auid, agid, pexdata, fname)
	struct exec	*pexdata;
#else BSD4_3
remote_execinfo(ip, auid, agid, fname)
#endif BSD4_3
	register struct inode	**ip;
	long	*auid, *agid;
	register char	*fname;
{
	long	error,
		sysindex = remote_sysindex,
		uid, gid,
		hdrsize,
		execsize;
	struct inode	*namei();
	struct remoteinfo	*rp = remote_info + remote_sysindex;
	struct message	*msg;

	u.u_error = 0;
	error = remote_path1(RSYS_execinfo);
	if (error) {
		/*
		 * Try again if its a local file.
		 */
		if (error < 0 && fname) {
#ifdef BSD4_3
			struct nameidata *ndp = &u.u_nd;

			ndp->ni_dirp = (caddr_t) fname;
			if ((*ip = namei(ndp)) == NULL)
#else BSD4_3
			u.u_dirp = (caddr_t) fname;
			if ((*ip = namei(uchar, LOOKUP, 1)) == NULL)
#endif BSD4_3
				if (u.u_error == 0 || u.u_error == EISREMOTE)
					u.u_error = ELOOP;
			return(remote_sysindex);
		}
		else if (error < 0)
			error = ENOENT;
		u.u_error = error;
		return(-1);
	}
	/*
	 * The R_RETVAL cell contains the total size of the file.  So that
	 * had better be >= the size of the header + size of text + size
	 * of data.
	 */
	msg = &rp->r_msg;
#ifdef BSD4_3
	bcopy(msg->m_args+R_EXECDATA,
		(caddr_t)pexdata, sizeof(struct exec));
	if (pexdata->a_magic == DEMAND_PAGED)
		hdrsize = CLBYTES;
	else
		hdrsize = sizeof(sizeof(struct exec));
#else BSD4_3
	bcopy(msg->m_args+R_EXECDATA,
		(caddr_t)&u.u_exdata, sizeof (u.u_exdata));
	if (u.u_exdata.ux_mag == DEMAND_PAGED)
		hdrsize = CLBYTES;
	else
		hdrsize = sizeof(u.u_exdata.Ux_A);
#endif BSD4_3

	execsize = ntohl(msg->m_args[ R_RETVAL ]);
	debug0("obj=%d,hrd=%d,error=%d,tx=%d,dat=%d\n",
		execsize, hdrsize, u.u_error,
#ifdef BSD4_3
		pexdata->a_text, pexdata->a_text
#else BSD4_3
		u.u_exdata.ux_tsize, u.u_exdata.ux_dsize
#endif BSD4_3
		);

#ifdef BSD4_3
	if (pexdata->a_text + pexdata->a_data + hdrsize > execsize
	 && *((char *)pexdata) != '#') {
		error = ENOEXEC;
		goto bad;
	}
#else BSD4_3
	if (u.u_exdata.ux_tsize + u.u_exdata.ux_dsize + hdrsize > execsize
	 && u.u_exdata.ux_shell[0] != '#') {
		error = ENOEXEC;
		goto bad;
	}
#endif BSD4_3

	if ((u.u_procp->p_flag&STRC) && msg->m_args[ R_EXECREADONLY ]) {
		error = EACCES;
		goto bad;
	}
	if (auid) { /* first time thru for this exec... set uid/gid stuff */
		uid = ntohl(msg->m_args[ R_EXECUID ]);
		if (uid < 0)
			uid = u.u_uid;
		gid = ntohl(msg->m_args[ R_EXECGID ]);
		if (gid < 0)
			gid = u.u_gid;
		*auid = uid;
		*agid = gid;
	}
	return(sysindex);

bad:
	u.u_error = error;
	return(-1);
}

/*
 * This is the simple routine that is called by remote_path1 as the
 * specific routine for handling RSYS_execinfo.  remote_execinfo() is
 * called from exec, and remote_execinfo() calls remote_path1() who in
 * turn calls rmt_execinfo().
 */
rmt_execinfo(sysindex, m)
	long	sysindex;
	struct mbuf	*m;
{
	struct message *msg = mtod(m, struct message *);
#ifdef BSD4_3
	msg->m_args[0] = htonl(sizeof (struct exec));
#else BSD4_3
	msg->m_args[0] = htonl(sizeof (u.u_exdata));
#endif BSD4_3
	return (rmt_msgfin(sysindex, m, 0));
}

/*
 * Read into memory an executable file from a remote system.
 */
#ifdef BSD4_3
remote_execread(sysindex, oldtextsize, ep)
	register struct exec *ep;
	register int	sysindex, oldtextsize;
#else BSD4_3
remote_execread(sysindex, oldtextsize)
	register int	sysindex, oldtextsize;
#endif BSD4_3
{
	struct remoteinfo	*rp = remote_info + sysindex;
	struct uio auio;
	struct iovec aiov[2];
	struct mbuf	*m;
	struct message	*msg;
	register long	hdrsize, datasize, textsize, error, len, magic;
	register struct proc	*p = u.u_procp;
	long	iovlen;

#ifdef BSD4_3
	datasize = (int)ep->a_data;
	magic = ep->a_magic;
#define HEADERSIZE	sizeof(struct exec);
#else BSD4_3
	datasize = (int)u.u_exdata.ux_dsize;
	magic = u.u_exdata.ux_mag;
#define HEADERSIZE	sizeof(u.u_exdata.Ux_A);
#endif BSD4_3
	if (datasize <= 0) {
		u.u_error = ENOEXEC;
		return(-1);
	}

	/*
	 * We are guarenteed that at this point the text size is zero.
	 * But we know what the old text size was passed along for the sake of
	 * shared-text programs:  the binaries for these do not
	 * necessarily have the data page-aligned in the file (as we
	 * assume that demand-paged programs do), but instead,
	 * it is aligned when it is read in.  We must do the same here.
	 */
	if (oldtextsize && magic == SHARED_TEXT) {
		aiov[0].iov_len = oldtextsize;
		aiov[1].iov_len = datasize - oldtextsize;
		iovlen = 2;
		aiov[0].iov_base = (char *)ctob(tptov(p, 0));
		aiov[1].iov_base = (char *)ctob(btoc(oldtextsize));
		debug0("execrd: %d @ %x, %d @ %x ",
			aiov[0].iov_len, aiov[0].iov_base,
			aiov[1].iov_len, aiov[1].iov_base);
	}
	else {
		iovlen = 1;
		aiov[0].iov_base = (char *)ctob(dptov(p, 0)),
		aiov[0].iov_len = datasize;
		debug0("execrd: %d @ %x ", aiov[0].iov_len, aiov[0].iov_base);
	}
	auio.uio_offset = 0;
	auio.uio_segflg = 0;
	if (magic == DEMAND_PAGED)
		hdrsize = CLBYTES;
	else
		hdrsize = HEADERSIZE;

	/*
	 * Now ask for it in remote_maxchunk size chunks.
	 */
	debug0("hdsz=%d, datasz=%d, rd ", hdrsize, datasize);
	error = 0;
	while (datasize >= 0) {
		auio.uio_resid = MIN(datasize, remote_maxchunk);
		if (auio.uio_resid == 0)
			datasize = -1;
		else {
			if (aiov[0].iov_len <= 0 && iovlen == 2) {
				aiov[0] = aiov[1];
				iovlen = 1;
			}
			auio.uio_iov = aiov;
			auio.uio_iovcnt = iovlen;
			datasize -= auio.uio_resid;
		}
		MGET(m, M_WAIT, MT_DATA);
		if (m == NULL)
			return(ENOBUFS);
		msg = mtod(m, struct message *);
		len = introduce_2extra(msg, RSYS_execread, hdrsize,
			auio.uio_resid);
		m->m_len = len;
		msg->m_hdlen = htons(len);
		msg->m_totlen = htonl(len);
#ifdef RFSDEBUG
		if (auio.uio_resid) {
			debug0("\n %d: %d@%d", auio.uio_resid,
				aiov[0].iov_len, aiov[0].iov_base);
			if (iovlen == 2)
				debug0(",%d@%d.",
					aiov[1].iov_len, aiov[1].iov_base);
		}
		rmt_showmsg(msg, FALSE);
#endif RFSDEBUG
		if (datasize >= 0)
			error = remoteio(sysindex, &m, &auio, RFLG_RD);
		else
			remoteio(sysindex, &m, 0, RFLG_INFO);
#ifdef RFSDEBUG
		msg = &rp->r_msg;
		rmt_showmsg(msg, TRUE);
#endif RFSDEBUG
		/*
		 * If there are any errors, simply send the last
		 * message: we're all done.
		 */
		if (error && datasize >= 0)
			datasize = 0;
		else if (error)
			return(error);
	}

	debug0("\n");
	return ( error );
}
