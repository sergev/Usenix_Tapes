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
 * $Log:	info.c,v $
 * Revision 2.0  85/12/07  18:21:28  toddb
 * First public release.
 * 
 */
static char	*rcsid = "$Header: info.c,v 2.0 85/12/07 18:21:28 toddb Rel $";
#include	"server.h"
#include	<stdio.h>
#include	<sys/wait.h>
#include	<sys/file.h>
#include	<sys/stat.h>
#include	<netdb.h>
#include	<errno.h>

extern users	*default_user;
extern hosts	*host;
extern hosts	*thishost;
extern char	byteorder[];
extern char	*logfile;
extern long	serviceport;
extern long	errno;
extern short	current_pid;
extern process	*wildcard;
extern boolean	in_root_directory;
extern boolean	i_am_asleep;

getbyteorder(h)
	register hosts	*h;
{
	register char	*p;

	/*
	 * Read the byte order info.
	 */
	alarm(5);
	p = (char *)h->h_byteorder;
	if (read(h->h_cmdfd, p, 4) != 4)
	{
		log("can't read mount info from \"%s\"\n", h->h_names[0]);
		alarm(0);
		bzero(p, 4);
		close(h->h_cmdfd);
		h->h_cmdfd = -1;
		return;
	}
	alarm(0);
	if (bcmp(p, byteorder, 4) == 0)
		h->h_byteorderok = TRUE;
	else
		h->h_byteorderok = FALSE;
	debug4("byteorder=%d,%d,%d,%d: %s ours\n",
		p[0], p[1], p[2], p[3],
		h->h_byteorderok ? "same as" : "different than");
}

getrusers(h)
	register hosts	*h;
{
	char	buf[ BUFSIZ ];
	register rusers	*ruser;
	register FILE	*input;
	register char	*p;
	register int	uid;

	errno = 0;
	/*
	 * Read in the users from the remote host and squirrel them away.
	 * Actually it is the actual password file from the remote
	 * host, and we ignore most of the info, and save the user name and
	 * uid.
	 */
	if ((input = fdopen(h->h_cmdfd, "r")) == NULL)
		log_fatal("getrusers: cannot fdopen\n");

	alarm(30);
	while (fgets(buf, BUFSIZ, input))
	{
		/*
		 * First, the user name.
		 */
		for(p=buf; *p && *p != ':'; p++) ;
		*p = '\0';

		/*
		 * now the user id number
		 */
		for(p++; *p && *p != ':'; p++) ;
		uid = atoi(p+1);

		/*
		 * Now we need to add the info to our database on this remote
		 * host.  If the user is already present, just update the uid
		 * number.  If the user is not present, and there is a default
		 * local user for this remote host, use that.  If not, then
		 * use the default user entry for this host (where the server
		 * runs).
		 */
		if (ruser = findrusername(&h->h_rusers, buf))
		{
			debug2("(existing) ");
			ruser->r_uid = uid;
		}
		else
		{
			debug2("(new, ");
			ruser = newruser();
			ruser->r_name = copy(buf);
			ruser->r_uid = uid;
			if (h->h_default_user)
			{
				debug2("%s default)", *h->h_names);
				ruser->r_user = h->h_default_user;
			}
			else
			{
				ruser->r_user = default_user;
				debug2("host default)");
			}
			addlist(&h->h_rusers, ruser);
		}
		debug2("host %s: user %s (%d) -> local user %s (%d)\n",
			h->h_names[ 0 ], ruser->r_name, ruser->r_uid,
			ruser->r_user->u_name, ruser->r_user->u_local_uid);
	}
	fclose(input);
	h->h_cmdfd = -1;
	alarm(0);
	if (errno == EINTR)
		log("can't get remote users from \"%s\"\n", h->h_names[0]);
}

/*
 * Try to obtain mount information for host 'h'.
 */
getmount(h)
	register hosts	*h;
{
	long	savefd = h->h_cmdfd;
	struct message	msgbuf;
	register struct message	*msg = &msgbuf;
	register long	len;

	if (thishost == h) /* our own machine */
	{
		log("we are talking to ourselves\n");
		h->h_cmdfd = open("/etc/passwd", O_RDONLY);
		bcopy(byteorder, h->h_byteorder, 4);
		h->h_byteorderok = TRUE;
	}
	else
	{
		if ((h->h_cmdfd = tcpconnect(h)) < 0)
			goto done;
		len = R_MINRMSG + sizeof(long);
		msg->m_hdlen = htons(len);
		msg->m_totlen = htonl(len);
		msg->m_syscall = htons(RSYS_nosys);
		msg->m_args[ 0 ] = htonl(CMD_NEEDMOUNT);
		if (!sndmsg(h->h_cmdfd, msg, len, 0, 0))
		{
			log("can't ask for mount info\n");
			close(h->h_cmdfd);
			goto done;
		}
		getbyteorder(h);
	}
	getrusers(h); /* getrusers() closes the file descriptor */
done:
	h->h_cmdfd = savefd;
}

/*
 * Send mount information.  This includes a 4-byte header containing the
 * byte order for our machine,  followed by /etc/passwd.
 */
sendmount(h)
	register hosts	*h;
{
	char	buf[ BUFSIZ ];
	register long	fd = open("/etc/passwd", O_RDONLY),
			cnt;
	register char	*p = buf;

	write(h->h_cmdfd, byteorder, 4);
	while ((cnt = read(fd, p, BUFSIZ)) > 0)
		_rmtio(write, h->h_cmdfd, p, cnt);
	close(h->h_cmdfd);
	h->h_cmdfd = -1;
	close(fd);
}

/*
 * Mourne the death of any children.
 */
mourne()
{
	union wait	status;
	char		buf[ BUFSIZ ];
	register char	*p = buf;
	register long	pid;

	while ((pid = wait3(&status, WNOHANG, 0)) > 0)
	{
		sprintf(p, "server %d found dead", pid);
		p += strlen(p);
		if (status.w_termsig)
			sprintf(p, " by sig #%d", status.w_termsig);
		p += strlen(p);
		if (status.w_coredump)
			sprintf(p, " with core dump", status.w_termsig);
		p += strlen(p);
		sprintf(p, " exit=%d\n", status.w_retcode);
		debug5("%s", buf);
		p = buf;
	}
}

/*
 * Catch signals and only report.
 */
catch(sig, code, scp)
	register long	sig,
			code;
	register struct sigcontext	*scp;
{
	log("caught signal #%d...", sig);
	if (sig == SIGILL || sig == SIGSEGV || sig == SIGBUS)
	{
		change_to_uid(0);
		chdir("/usr/tmp");
		log("aborting: code=%d, scp=%x, sp=%x, pc=%x, end of scp=%x\n",
			code, scp, scp->sc_sp, scp->sc_pc, scp+1);
		sendsig(current_pid, SIGEMT);
		log_fatal("could not abort\n");
	}
	else if (sig == SIGTERM) /* quietly go away */
	{
		/*
		 * unlink the file only if we are allowed to and if it is
		 * not the sentry server's logfile.
		 */
		if ((remote_debug & 0x800) == 0 && host != NULL)
			unlink(logfile);
		cleanup();
		log("goodbye.\n");
		exit(0);
	}
	else
		log_fatal("exiting\n");
}

/*
 * Receive a wakeup call.
 */
wakeup_call()
{
	if (! i_am_asleep)
	{
		log("recieved spurious wakeup call!\n");
		return;
	}
	i_am_asleep = FALSE;
}

/*
 * Provide name server function for kernel.  At this point we have just
 * recieved a SIGURG signal because the kernel wants us to translate a
 * name.
 */
nameserver()
{
#ifdef CANREMOTE
	char		path[ BUFSIZ ],
			hostname[ BUFSIZ ];
	struct sockaddr_in	sinbuf;
	register char	*p1, *p2, *name;
	register hosts	*h = NULL;
	register struct sockaddr_in	*sin;

	if (remotename(NM_WHATNAME, 0, BUFSIZ, path) < 0)
		return;
	/*
	 * Find the end of the '/' prefix and copy up to the next '/' or
	 * null character.
	 */
	p1 = path;
	while (*p1 == '/')
		p1++;
	p2 = hostname;
	*p2++ = '/';
	while (*p1 && *p1 != '/')
		*p2++ = *p1++;
	*p2 = '\0';

	/*
	 * Now look it up.
	 */
	if (hostname[1])
		h = findhostname(hostname+1);
	if (h == NULL)
	{
		debug6("cannot find host for path \"%s\"\n", path);
		sin = NULL;
	}
	else
	{
		sin = &sinbuf;
		debug6("path %s mapped to host %s\n", path, h->h_names[0]);
		bzero((char *)sin, sizeof (struct sockaddr_in));
		bcopy(&h->h_addr, (char *)&sin->sin_addr,
			sizeof(struct in_addr));
		sin->sin_family = AF_INET;
		sin->sin_port = serviceport;
	}
	remotename(NM_NAMEIS, sin, sizeof(struct sockaddr_in), hostname);
#endif CANREMOTE
}

/*
 * Decide if a file is really a local file to the client or not.  We only
 * look for explicit references like
 *	name1/name2/../name3 ...
 * And we then check to see whether name2 is the root directory.  If it is
 * then we send the request back to the client.
 */
islocal(msg, type)
	register struct message *msg;
{
	register char	*p;
	register boolean	checktwopaths;
	register short	syscall = msg->m_syscall;
	register long	offset1,
			offset2 = -1,
			localcnt = 0;
	register process	*proc;
	char	buf[ BUFSIZ ];

	debug10("cwd=%s\n", getwd(buf));
	checktwopaths = (type & NEED_2REMOTE);

	if (checktwopaths)
		p = twopath1addr(msg);
	else
		p = path1addr(msg);
	if ((offset1 = find_dotdot(p)) >= 0)
		localcnt++;
	if (checktwopaths)
		if ((offset2 = find_dotdot(twopath2addr(msg))) >= 0)
			localcnt++;
	if (localcnt)
	{
		debug10("%d paths are remote: \"%s\" @ %d, \"%s\" @ %d\n",
			localcnt,
			checktwopaths ? twopath1addr(msg) : path1addr(msg),
			offset1, checktwopaths ? twopath2addr(msg) : "",
			offset2);
		setup_proc(proc = wildcard, msg->m_uid, msg->m_pid);
		proc->p_errno = -1;
		proc->p_returnval = offset1;
		sendreturn(proc, host->h_cmdfd, NULL, 1, offset2);
	}
	return(localcnt);
}

/*
 * kernel code stolen for speed.
 */
myaccess(st, user, perm)
	register struct stat	*st;
	register long	perm;
	register users	*user;
{
	register long	*gp, i;

	perm <<= 6;
	if (user->u_local_uid != st->st_uid) {
		perm >>= 3;
		gp = user->u_local_groups;
		for (i=0; i < user->u_numgroups; i++, gp++)
			if (st->st_gid == *gp)
				goto found;
		perm >>= 3;
found:
		;
	}
	if ((st->st_mode & perm) != 0)
		return (TRUE);
	return(FALSE);
}

/*
 * look for a component of ".." terminated by a '/' or a null character.
 * If we find one, examine the previous component to see if it is our
 * root directory.
 */
find_dotdot(path)
	register char	*path;
{
	struct stat	statb;
	register char	*p;
	register struct stat	*statp = &statb;
	register long	retval;
	extern struct stat	root;

	for (p = path; *p;)
	{
		while (*p == '/')
			p++;
		if (p[0] == '.' && p[1] == '.'
		&& (p[2] == '\0' || p[2] == '/'))
		{
			if (p == path)
				if (in_root_directory)
					return(0);
				else
					goto next_component;
			*p = '\0'; /* we know it is a '.' */
			retval = lstat(path, statp);
			*p = '.';
			if (retval < 0)
				return(retval);
			if (isroot(statp))
				return(p - path + 1);
		}
next_component:
		while (*p && *p != '/')
			p++;
	}
	return(-1);
}
