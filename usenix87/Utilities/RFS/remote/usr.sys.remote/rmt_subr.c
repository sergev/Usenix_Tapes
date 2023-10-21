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
 * $Header: rmt_subr.c,v 2.0 85/12/07 18:19:10 toddb Rel $
 *
 * $Log:	rmt_subr.c,v $
 * Revision 2.0  85/12/07  18:19:10  toddb
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

extern struct remoteinfo remote_info[];
extern struct remoteinfo *remote_generic;
extern struct nameserver	remote_ns;

/*
 * A fast routine for determining whether an inode is in the list of
 * remote hosts, and returning a pointer into that list.
 * Failure returns NULL.
 */
struct remoteinfo *rmt_host(ip, asysnum)
	register struct inode	*ip;
	register long		*asysnum;
{
	register struct remoteinfo	*rp = remote_info;
	register long			i;
	
	if (ip == NULL)
		printf("rmt_host: ip=0\n");
	else for(i=0; i < R_MAXSYS; i++, rp++)
		if (rp->r_mntpt == ip) {
			*asysnum = i;
			return(rp);
		}
	return(NULL);
}

/*
 * This is an extension to rmt_host() in that if rmt_host() returns a
 * pointer to a generic mount point, we return the pointer to the entry
 * that describes where we have our current working directory.
 */
struct remoteinfo *rmt_hostdir(ip, asysnum)
	register struct inode	*ip;
	register long		*asysnum;
{
	register struct remoteinfo *rp;

	rp = rmt_host(ip, asysnum);
	if (rp == NULL)
		return(NULL);
	if (rp->r_name == NULL)
		if (u.u_rmtcdir < 0)
			return(rp);
		else
			return(remote_info + (*asysnum = u.u_rmtcdir));
	return(rp);
}

/*
 * called by isremote() to figure out if there is a host implied by
 * 'path'.  Note that a user process must have ``registered'' with
 * the kernel as being willing to provide name service.
 */
struct remoteinfo *rmt_findhost(apath, asysnum)
	char	**apath;
	register long	*asysnum;
{
	label_t				qsave;
	register struct remoteinfo	*rp;
	char				savec;
	struct remoteinfo		*rmt_findaddr(),
					*rmt_findslot();
	register struct proc		*p;
	register char			*path = *apath, *cp;
	register struct mbuf		*m = NULL;
	register long			error = 0,
					i;

	/*
	 * If the path is relative, then it must be because we have done
	 * a remote chdir()... take the directory from there.
	 */
	if (*path != '/' && u.u_rmtcdir >= 0) {
		debug13("path %s==>cwd=#%d\n", path, u.u_rmtcdir);
		*asysnum = u.u_rmtcdir;
		return (remote_info + u.u_rmtcdir);
	}

	/*
	 * First try to satisfy the name from the existing table... there
	 * may have been a mount done explicitly that has the form
	 * "/hostname", or there may have been an implicit mount.  Check
	 * for both.
	 */
	rp = remote_info;
	for(i=0; i < R_MAXSYS; i++, rp++)
		if (rp->r_name && rmt_pathimplies(rp, apath)) {
			debug13("%s==>mntpt=%s(#%d)\n",
				*apath, rp->r_mntpath, i);
			*asysnum = i;
			return(rp);
		}

	/*
	 * If that fails, then give the name server a crack at it.  Note that
	 * we don't check to see if we found an open slot, because the address
	 * that we get back may match an existing address.
	 * If the nameserver is around, send him a signal.  Then wait
	 * patiently for the response.
	 */
	while (remote_ns.rn_path) /* Lock out all other nameserver action */
		sleep((caddr_t)&remote_ns.rn_path, PZERO+1);

	bcopy(&u.u_qsave, &qsave, sizeof(label_t));
	if (setjmp(&u.u_qsave)) {
		error = EINTR;
		goto out;
	}

	/*
	 * The nameserver only needs the first component.
	 */
	cp = path;
	while (*cp == '/')
		cp++;
	while (*cp && *cp != '/')
		cp++;
	savec = *cp;
	*cp = '\0';
	remote_ns.rn_pathlen = cp - path + 1;
	remote_ns.rn_path = path;
	p = remote_ns.rn_proc;
	if (server_alive(p)) {
		psignal(p, SIGURG);
		sleep((caddr_t)&remote_ns.rn_name, PZERO+1);
	}
	*cp = savec;

	/*
	 * Ok, now see what the server had to say...
	 */
	m = remote_ns.rn_name;
	remote_ns.rn_name = NULL;
	if (m == NULL)
		error = EADDRNOTAVAIL;
	else {
		rp = rmt_findaddr(m, asysnum);
		*apath = cp;
	}
	if (rp || m == NULL)
		goto out;

	if ((rp = rmt_findslot(asysnum)) == NULL)
		error = ETOOMANYREMOTE;
	else {
		if (rp->r_name) {
			debug13("findhost: reusing %d, %s\n",
				asysnum, rp->r_mntpath);
			(void) m_free(rp->r_name);
		}
		rp->r_name = m;
		bcopy (mtod(m, caddr_t) + m->m_len, rp->r_mntpath,
			MIN(R_MNTPATHLEN, MLEN - m->m_len));
		m = NULL;
	}

out:
	if (remote_ns.rn_name)
		(void) m_free(remote_ns.rn_name);
	else if (m)
		(void) m_free(m);
	remote_ns.rn_name = NULL;
	remote_ns.rn_path = NULL;
	wakeup((caddr_t)&remote_ns.rn_path);
	if (error) {
		rp = NULL;
		u.u_error = error;
	}

	/*
	 * Since we are returning and may sleep again, we must restore the
	 * setjmp info so that we don't kill ourselves.
	 */
	bcopy(&qsave, &u.u_qsave, sizeof(label_t));
	return (rp);
}

/*
 * if (index >= 0) i.e. valid remote host
 *	Set the working directory to the mount point for system 'index'.
 *	This, along with what the server does, will effect a chdir("remotedir");
 * if (index < 0)
 *	then simply decrement the number of chdir's on the current remote
 *	host, if any.
 */
remotechdir(index)
	long	index;
{
	register struct inode *ip = NULL, *oip;
	register struct remoteinfo	*rp;
	register long			error = 0;
	long				i;
	struct remoteinfo		*rmt_host();

	debug14("cd #%d\n", index);
	if (index >= R_MAXSYS)
		return(ENOENT);
	/*
	 * If we are currently cd'ed to another remote host, decrement its'
	 * reference count.
	 */
	oip = u.u_cdir;
	if (rp = rmt_hostdir(oip, &i)) {
		debug14("uncd #%d, ip=%x\n", i, oip);
		rp->r_nchdir--;
	}
	rp = remote_info + index;
	if (index >= 0) {
		/*
		 * If this is an implied mount point, find the inode for the
		 * generic mount pt.
		 */
		if (rp->r_mntpt == NULL) {
			if (remote_generic)
				ip = remote_generic->r_mntpt; 
			debug14("cd is generic, ip=%x\n", ip);
			if (ip == NULL)
				return(ENOENT);
		}
		else
			ip = rp->r_mntpt;
		rp->r_nchdir++;		/* bump the reference count */
		u.u_rmtcdir = index;
		irele(oip);
		ip->i_count++;
		u.u_cdir = ip;
	}

	return(error);
}

/*
 * See if a host is implied in a remote_info entry from the path name 'path'.
 */
rmt_pathimplies(rp, path)
	register struct remoteinfo	*rp;
	register char			**path;
{
	register char	*p1, *p2, *pend;

	p1 = rp->r_mntpath;
	while(*p1 == '/')
		p1++;
	p2 = *path;
	while(*p2 == '/')
		p2++;

	/*
	 * Compare against the mount point.
	 */
	pend = rp->r_mntpath + R_MNTPATHLEN;
	while (*p1 == *p2 && *p1 && p1 < pend)
		p1++, p2++;
	if (*p1 == *p2 || (*p1 == '\0' && *p2 == '/')) {
		*path = p2;
		return(TRUE);
	}

	return (FALSE);
}

/*
 * See if any remote entry already had address 'addr'.
 */
struct remoteinfo *rmt_findaddr(m, asysnum)
	register struct mbuf	*m;
	register long		*asysnum;
{
	register struct remoteinfo	*rp;
	register caddr_t		addr;
	register int			i, len;

	addr = mtod(m, caddr_t);
	len = m->m_len;
	for (i=0, rp = remote_info; i < R_MAXSYS; i++, rp++)
		if (rp->r_name
		&& len == rp->r_name->m_len
		&& bcmp(rp->r_name, addr, len) == 0) {
			*asysnum = i;
			debug13("%s: same as mnt %d\n", addr+len, i);
			return(rp);
		}
	return(NULL);
}

/*
 * Find an open slot in the remote info.
 */
struct remoteinfo *rmt_findslot(asysnum)
	register long	*asysnum;
{
	register struct remoteinfo	*rp,
					*frp = NULL;
	register int			i, fi;

	for (i=0, rp = remote_info; i < R_MAXSYS; i++, rp++) {
		if (rp->r_mntpt || rp ->r_sock) /* active connections */
			continue;
		if (rp->r_name == NULL) {
			frp = rp;
			fi = i;
			break;
		}
		if (frp == NULL || rp->r_age < frp->r_age) {
			frp = rp;
			fi = i;
		}
	}
	if (frp) {
		debug13("slt: %d\n", fi);
		*asysnum = fi;
		return(frp);
	}

	/*
	 * No slot... do garbage collection.
	 */
	for (i=0, rp = remote_info; i < R_MAXSYS; i++, rp++)
		if (rp->r_mntpt == NULL
		 && rp->r_nchdir == 0 && rp->r_nfile == 0) {
			debug13("fndslt: usurp %d=%s\n",
				rp - remote_info, rp->r_mntpath);
			*asysnum = i;
			rmt_closehost(rp);
			(void) m_free(rp->r_name);
			rp->r_name = NULL;
			return(rp);
		}

	return(NULL);
}
