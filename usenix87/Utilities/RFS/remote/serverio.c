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
 * $Log:	serverio.c,v $
 * Revision 2.0  85/12/07  18:22:33  toddb
 * First public release.
 * 
 */
static char	*rcsid = "$Header: serverio.c,v 2.0 85/12/07 18:22:33 toddb Rel $";
#include <errno.h>
#include <stdio.h>
#include "server.h"
#include <sys/uio.h>
#include <sys/file.h>
#include <netdb.h>
#include <signal.h>

/*
 * This routine prepares to recieve a connection from another process.  
 * To be any good it must be followed by a call to tcpaccept().
 */
extern long	errno;
extern short	current_pid;
extern char	*sys_errlist[],
		*service,
		*logfile,
		*stdlogfile,
		*getbuf();
extern boolean	i_am_gateway;
extern hosts	*host;
extern long	serviceport,
		errno;
extern short	last_sentry;

int	read(),
	write();

/*
 * Get ready to accept connections.
 */
tcppassive()
{
	int	f;
	struct sockaddr_in	sin;
	struct servent	*servp;

	if ((servp = getservbyname(service, "tcp")) == NULL)
		log_fatal("%s: unknown service\n", service);
	serviceport = servp->s_port;

	sin.sin_port = serviceport;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_family = AF_INET;

	f = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(f, SOL_SOCKET, SO_KEEPALIVE, 0, 0);
	setsockopt(f, SOL_SOCKET, SO_REUSEADDR, 0, 0);

	while (bind(f, (caddr_t)&sin, sizeof (sin)) < 0) {
		if (last_sentry && errno == EADDRINUSE)
		{
			log("shutting down old sentry (pid %d)\n",
				last_sentry);
			if (! sendsig(last_sentry, SIGTERM))
				last_sentry = 0;
			sleep(2);	/* give it time to die */
			continue;
		}
		log("cannot bind %s\n", service);
		return(-1);
	}

	listen(f, 5);
	return(f);
}

/*
 * Here we passively accept a connection from another process.
 * We return a pointer to the hosts structure.  If there is no
 * corresponding host for a given connection, we make up a new one
 * and return that.
 */
hosts *tcpaccept(f)
	int	f;
{
	hosts	*h = NULL;
	struct sockaddr_in	sin;
	int	g,
		i,
		len = sizeof (struct sockaddr_in);

	for (i = 0, g = -1; g < 0 && i < 10; i++) {
		g = accept(f, &sin, &len);
		if (g < 0)
		{
			log("accept failed\n");
			continue;
		}

		/*
		 * Ok, we have recieved a connection.  Find out what we
		 * know about this fella, and save the command file descriptor
		 * and the port number.
		 */
		h = findhostaddr(&sin.sin_addr);
		h->h_cmdfd = g;
		h->h_portnum = htons((u_short)sin.sin_port);
	}

	return(h);
}

/*
 * Call another host's server.  We assume that he is using the same service
 * as we.
 */
tcpconnect(h)
	register hosts	*h;
{
	struct sockaddr_in sin;
	register long	s;

	bzero((char *)&sin, sizeof (sin));
	bcopy(&h->h_addr, (char *)&sin.sin_addr, sizeof(struct in_addr));
	sin.sin_family = AF_INET;
	sin.sin_port = serviceport;
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
	{
		log("can't get socket to remote server\n");
		return(-1);
	}

	/*
	 * time out on connection rather quickly.
	 */
	alarm(5);
	if (connect(s, (char *)&sin, sizeof (sin)) < 0) {
		alarm(0);
		log("can't connect to server\n");
		close(s);
		return(-1);
	}
	alarm(0);
	return(s);
}

alarmsig()
{
	log("timeout\n");
	errno = EINTR;
}

log(x1, x2, x3, x4, x5, x6, x7, x8, x9)
{
	char		buf[ BUFSIZ ];
	register char	*p = buf;
	static boolean	printpid = TRUE;

	*p = '\0';
	if (printpid)
	{
		sprintf(p, "(%d)", current_pid);
		p += strlen(p);
		if (errno)
		{
			sprintf(p, "%s: ", sys_errlist[ errno ]);
			p += strlen(p);
			errno = 0;
		}
	}
	sprintf(p, x1, x2, x3, x4, x5, x6, x7, x8, x9);
	p += strlen(p);
	if (*(p-1) == '\n')
		printpid = TRUE;
	else
		printpid = FALSE;
	write(2, buf, p-buf);
}

log_fatal(x1, x2, x3, x4, x5, x6, x7, x8, x9)
{
	static boolean		entered = FALSE;

	if (! entered)
	{
		entered = TRUE;
		log ("fileserver: FATAL ERROR: ");
		log (x1, x2, x3, x4, x5, x6, x7, x8, x9);
		cleanup();
	}
	exit (1);
}

/*
 * Set up the log file for the current process.  If there is no current
 * host defined, then we are the sentry server and simply use the
 * standard logfile:  this should be done once only.  If 'host' is defined,
 * then set up a logfile for this particular connection.
 *
 * The sentry server always puts his signature at the top of the file
 * (pid number).  So as a side effect, the global variable last_sentry
 * is set to this signature (if there exists one).
 */
setlogfile()
{
	char	buf[ BUFSIZ ];
	register char	*pbuf = buf;
	register long	newstderr;
	boolean	sentry = FALSE;
	FILE	*fd;

	if (logfile)
		free (logfile);
	if (host == NULL)
	{
		sentry = TRUE;
		strcpy(pbuf, stdlogfile);
	}
	else
		sprintf(pbuf, "%s.%s.%d",
			stdlogfile, host->h_names[0], current_pid);
	logfile = malloc(strlen(pbuf)+1);
	strcpy(logfile, pbuf);

	/*
	 * Get the previous signature.
	 */
	if (sentry) 
	{
		if ((fd = fopen(logfile, "r")) != NULL)
		{
			*pbuf = '\0';
			fgets(pbuf, BUFSIZ, fd);
			last_sentry = atoi(pbuf+1);
			fclose(fd);
		}
	}

	newstderr = open(logfile, O_RDWR|O_APPEND|O_CREAT|O_TRUNC, 0444);
	if (newstderr < 0)
		log_fatal("cannot reopen stderr\n");
	dup2(newstderr, 2);
	close(newstderr);
	log("startup.\n");	/* put down our signature. */
}

cleanup()
{
	register process	*proc;

	if (i_am_gateway && host)
		for (proc=host->h_proclist; proc; proc=proc->p_next)
			if (proc->p_handler != current_pid)
				sendsig(proc->p_handler, SIGKILL);
	mourne();
}

sndmsg(fd, msg, msglen, data, len)
	register struct message	*msg;
	register char	*data;
	register int	fd, msglen, len;
{
	register int	num;
	struct iovec	iov[2];


	showmsg(msg, TRUE);
	/*
	 * set up for the write and if there is any data to send, try to send
	 * a bunch of it.
	 */
	if (len > 0 && data)
	{
		iov[0].iov_base = (char *)msg;
		iov[0].iov_len = msglen;
		iov[1].iov_base = data;
		iov[1].iov_len = len;
		if ((num = writev(fd, iov, 2)) == len + msglen)
		{
			debug8("wrote data and msg (%d)!!\n", len + msglen);
			return(TRUE);
		}
		debug8("wrote %d of %d msg + data\n", num, len+msglen);
		if (num < 0)
			return(FALSE);
		if (_rmtio(write, fd, data+num, len-num) <= 0)
			return(FALSE);
	}
	else if (_rmtio(write, fd, msg, msglen) <= 0)
		return(FALSE);
	return(TRUE);
}

/*
 * Read the next message.  Its format is null-separated hex-ascii strings.
 * In order, they are:
 *	total_length	length of this request
 *	header_length	length of the message request (minus length of data)
 *	pid		Process id on requesting machine
 *	uid		user id of process on requesting machine
 *	syscall		our internal syscall number
 *	args...		between 0 and ?
 *
 * Followed by actual data (if any).  We grab the first two fields, and
 * use them for making sure we read all that is necessary.  The remainder
 * are returned in a array of character pointers.  It there is any data
 * in this request, then it is pointed to by the last character pointer.
 *
 * The way that we read messages is as follows:
 *
 *	last message length = 0
 *	while no more messages
 *	do
 * 		if last message length > 0
 * 			gobbleup last message length bytes
 *		peek at next message
 *		if this message is for another process
 *		then
 *			wake up the other process
 *			last message length = 0
 *			go to sleep
 *			continue
 *		endif
 *		if data in this message
 *		then
 *			read the message
 *			last message length = 0
 *		else
 *			last message length = length of this message
 *		endif
 *		service request
 *	done
 */

struct message *getmsg(fd)
	register long	fd;
{
	register long	cnt,
			lastcnt = 0,
			len;
	register struct message	*msg;

	msg = (struct message *)getbuf(BIGBUF);
	/*
	 * Check to see if there is a message still in the queue, that has
	 * been processed, but not yet read.
	 */
	gobble_last_msg(fd, msg);
	for (;;)
	{
		debug8("peek at fd %d... ", fd);
		cnt = recv(fd, (char *)msg, BIGBUF, MSG_PEEK);
		if (cnt < R_MINRMSG)
		{
			if (cnt < 0 && errno == EINTR)
				continue;
			debug8("eof, cnt=%d\n", cnt);
			return(NULL);
		}
		debug8("got %d from peek\n", cnt);

		/*
		 * Now find out how long the request is (excluding data)
		 * and make sure that we have at least that amount.
		 */
#ifdef magnolia
		len = msg->m_hdlen;
#else	 /* magnolias don't need to do this garbage (MC68000) */
		msg->m_totlen = ntohl(msg->m_totlen);
		len = msg->m_hdlen = ntohs(msg->m_hdlen);
		msg->m_pid = ntohs(msg->m_pid);
		msg->m_uid = ntohs(msg->m_uid);
		msg->m_syscall = ntohs(msg->m_syscall);
		msg->m_args[0] = ntohl(msg->m_args[0]);
		msg->m_args[1] = ntohl(msg->m_args[1]);
		msg->m_args[2] = ntohl(msg->m_args[2]);
		msg->m_args[3] = ntohl(msg->m_args[3]);
#endif
		if (len > cnt)
		{
			if (cnt == lastcnt)
			{
				log("most but not all of message is in\n");
				dumpmsg(msg, cnt);
				return(NULL);
			}
			else
				debug8("didn't get all of message headder\n");
			lastcnt = cnt;
			sleep(5);
		}
		else if (len < R_MINRMSG)
		{
			log("bad soft msg len=%d, got %d\n", len, cnt);
			dumpmsg(msg, cnt);
			if (i_am_gateway)
				log_fatal("");
			say_something(S_CORRUPTED, 0);
			sendsig(current_pid, SIGSTOP);
		}
		else
			break;
	}
	showmsg(msg, FALSE);
	return(msg);
}

/*
 * Here we format the response to be sent to the client.  Note that
 * the client can never tolerate a message shorter than
 * R_MINRMSG + sizeof(long).  The declaration of x0-xf is for the
 * sake of stupid Pyramid argument conventions.
 */
sendreturn(proc, fd, data, cnt,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,xa,xb,xc,xd,xe,xf)
	process		*proc;
	register long	cnt;
	register char	*data;
	long		fd;
{
	char	buf[ BUFSIZ ];
	struct message		msgbuf;
	register struct message	*msg = &msgbuf;
	register	*argp = &x0, msglen, i;

	msg->m_errno = htons(proc->p_errno);
	msg->m_pid = htons(proc->p_pid);
	msg->m_uid = htons(proc->p_uid);
	if (proc->p_errno > 0) /* -1 is valid errno: says its a local file */
	{
		msglen = R_MINRMSG + sizeof(long);
		msg->m_totlen = htonl(msglen);
		proc->p_returnval = -1;
	}
	else
	{
		msglen = R_MINRMSG + (cnt+1)*sizeof(long);
		if (data)
			msg->m_totlen = htonl(msglen + proc->p_returnval);
		else
			msg->m_totlen = htonl(msglen);
#ifdef pyr /* Pyramid */
		msg->m_args[ R_RETVAL+1 ] = htonl(x0);
		msg->m_args[ R_RETVAL+2 ] = htonl(x1);
		msg->m_args[ R_RETVAL+3 ] = htonl(x2);
		msg->m_args[ R_RETVAL+4 ] = htonl(x3);
		msg->m_args[ R_RETVAL+5 ] = htonl(x4);
		msg->m_args[ R_RETVAL+6 ] = htonl(x5);
		msg->m_args[ R_RETVAL+7 ] = htonl(x6);
		msg->m_args[ R_RETVAL+8 ] = htonl(x7);
		msg->m_args[ R_RETVAL+9 ] = htonl(x8);
		msg->m_args[ R_RETVAL+10 ] = htonl(x9);
		msg->m_args[ R_RETVAL+11 ] = htonl(xa);
		msg->m_args[ R_RETVAL+12 ] = htonl(xb);
		msg->m_args[ R_RETVAL+13 ] = htonl(xc);
		msg->m_args[ R_RETVAL+14 ] = htonl(xd);
		msg->m_args[ R_RETVAL+15 ] = htonl(xe);
		msg->m_args[ R_RETVAL+16 ] = htonl(xf);
#else pyr
		for (i=0; i<cnt; i++)
			msg->m_args[ R_RETVAL+1+i ] = htonl(argp[ i ]);
#endif
	}
	msg->m_args[ R_RETVAL ] = htonl(proc->p_returnval);
	msg->m_hdlen = htons(msglen);
	sndmsg(fd, msg, msglen, data, proc->p_returnval);
	errno = 0;
}

_shutdown(fd)
	int	fd;
{
	log("shutdown fd %d\n", fd);
	close(fd);
}

char *getbuf(size)
{
	static char	*buf, *calloc(), *realloc();
	static int	cursize;
	static int	highwater;

	if (size > highwater)
	{
		if (buf == NULL)
			buf = calloc(cursize = size, sizeof(char));
		else
		{
			buf = realloc(buf, highwater = size);
			debug8("resize buffer to %d\n", size);
		}
		highwater = size;
		if (buf == NULL)
			log_fatal("cannot allocate buffer space\n");
	}
	return(buf);
}

char *get_data_buf(size)
{
	static char	*buf, *malloc(), *realloc();
	static int	cursize;
	static int	highwater;

	if (size > highwater)
	{
		if (buf == NULL)
			buf = malloc(cursize = size);
		else
		{
			buf = realloc(buf, highwater = size);
			debug8("resize data buffer to %d\n", size);
		}
		highwater = size;
		if (buf == NULL)
			log_fatal("cannot allocate data buffer space\n");
	}
	return(buf);
}

gobble_last_msg(fd, msg)
	register long		fd;
	register struct message	*msg;
{
	if (msg->m_totlen)
	{
		debug8("gobble up last %d byte message... ", msg->m_totlen);
		_rmtio(read, fd, msg, msg->m_totlen);
		msg->m_totlen = 0;
	}
}

_rmtio(iofunc, fd, buf, len)
	register func	iofunc;
	register int	fd, len;
	register char	*buf;
{
	register int	cnt, need = len;

	debug8("io %d bytes, fd=%d...", len, fd);
	while(need)
	{
		if ((cnt = (*iofunc)(fd, buf+len-need, need)) <= 0)
		{
			_shutdown(fd);
			return(cnt);
		}
		need -= cnt;
	}
	debug8("did %d.\n", len - need);
	return(len);
}

_rmtiov(iofunc, fd, iovec, len)
	register func	iofunc;
	register int	fd, len;
	register struct iovec	*iovec;
{

	debug8("io %d vectors, fd=%d...", len, fd);
	if ((len = (*iofunc)(fd, iovec, len)) <= 0)
		_shutdown(fd);
	debug8("did %d.\n", len);
	return(len);
}

/*
 * Show the message in characters and hex.
 */
dumpmsg(msg, len)
	char	*msg;
	long	len;
{
	char	string[ 100 ], *pstring = string,
		words[ 100 ], *pwords = words,
		dwords[ 100 ], *pdwords = dwords,
		longs[ 100 ], *plongs = longs,
		*p;
	long	cnt = 0;

	for(p = msg; cnt<len; cnt++, p++)
	{
		/* character */
		if (*p >= ' ')
			sprintf(pstring, "%c  ", *p > '~' ? '?' : *p);
		else
			sprintf(pstring, "^%c ", *p | 0x40);
		pstring += 3;
		if ((cnt & 0x1) == 0)
		{
			sprintf(pwords, "%-4.4x  ",
				*((unsigned short *)p));
			sprintf(pdwords, "%-5.5d ",
				*((unsigned short *)p));
			pwords += 6;
			pdwords += 6;
		}
		if ((cnt & 0x3) == 0)
		{
			sprintf(plongs, "%-8.8x    ", *((long *)p));
			plongs += 12;
		}
		if (pstring >= string+72)
		{	
			log("%s\n", pstring = string);
			log("%s\n", pwords = words);
			log("%s\n", pdwords = dwords);
			log("%s\n\n", plongs = longs);
		}
	}
	log("%s\n", string);
	log("%s\n", words);
	log("%s\n", dwords);
	log("%s\n", longs);
}

#ifdef RFSDEBUG

showmsg(msg, fromserver)
	register struct message	*msg;
	long	fromserver;
{
	register long	end, len, pid, uid, totlen;

	if ((remote_debug & 0x100) == 0)
		return;
	if (fromserver)
		len = ntohs(msg->m_hdlen),
		totlen = ntohl(msg->m_totlen),
		pid = ntohs(msg->m_pid),
		uid = ntohs(msg->m_uid);
	else
		len = msg->m_hdlen,
		totlen = msg->m_totlen,
		pid = msg->m_pid,
		uid = msg->m_uid;
	log("%s server: len=%d,tot=%d,pid=%d,uid=%d",
		fromserver ? "from" : "to", len, totlen, pid, uid);

	/* round up into long words */
	len = (len - R_MINRMSG + 3) >> 2;
	if (fromserver)
	{
		log(",errno=%d,retval=%d",
			htons(msg->m_errno), htonl(msg->m_args[ R_RETVAL ]));
		end = R_RETVAL+1;
	}
	else
	{
		log(",syscall=%d", msg->m_syscall);
		end = 0;
	}
	for (; end<len; end++)
		log(",0x%x", ntohl(msg->m_args[ end ]));
	log("\n");
}

/*
 * Set new debug levels.
 */
newdebug()
{
	register long	fd, cnt;
	char	*dbfile = "/usr/tmp/rfs_debug",
		buf[ BUFSIZ ];
	register char	*p = buf;

	fd = open(dbfile, O_RDONLY);
	if (fd < 0) {
		log("can't open %s\n", dbfile);
		goto out;
	}

	cnt = read(fd, buf, BUFSIZ);
	close(fd);
	if (cnt <= 0) {
		log("can't read %s\n", dbfile);
		goto out;
	}

	while ((*p >= 'a' && *p <= 'f')
	    || (*p >= 'A' && *p <= 'F')
	    || (*p >= '0' && *p <= '9'))
		p++;
	*p = '\0';
	remote_debug = atox(buf);
out:
	log("debug=%x\n", remote_debug);
}
#endif RFSDEBUG
