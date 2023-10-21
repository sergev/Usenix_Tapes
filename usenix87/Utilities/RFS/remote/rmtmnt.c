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
 * $Log:	rmtmnt.c,v $
 * Revision 2.0  85/12/07  18:21:56  toddb
 * First public release.
 * 
 */
static char	*rcsid = "$Header: rmtmnt.c,v 2.0 85/12/07 18:21:56 toddb Rel $";
#include	"server.h"
#include	<stdio.h>
#include	<sys/file.h>
#include	<netdb.h>
#include	<signal.h>
#include	<setjmp.h>
#include	<nlist.h>

extern int	errno;		/* for errors */
char		*service;	/* service name */
char		byteorder[4] = { BYTEORDER };

/*
 * for slow or dead remote hosts, we catch alarms.
 */
int		onalrm();
struct sigvec	vec = { onalrm, 0, 0 };

/*
 * Displaying current mount points requires that we read kernel space.
 */
struct nlist	nl[] = {
#ifdef magnolia
	{ "remote_info" },
#else
	{ "_remote_info" },
#endif
	{ "" },
};
#ifdef magnolia
char		*kernel = "/magix";
#else
char		*kernel = "/vmunix";
#endif

main(argc, argv)
	int	argc;
	char	**argv;
{
	long	generic = FALSE,
		unmount = FALSE;
	char	*mntpt = NULL,
		*host = NULL;

	/*
	 * Parse the args.
	 */
	for (argv++, argc--; argc; argv++, argc--)
	{
		if (**argv != '-')
			break;
		switch(argv[0][1]) {
		case 's':	/* service name */
			if (argv[0][2])
				service = argv[0]+2;
			else
				argv++, argc--, service = argv[0];
			break;
		case 'g':	/* Make this a generic mount point */
			generic = TRUE;
			break;
		case 'u':	/* unmount this mount point */
			unmount = TRUE;
			break;
		default:
			fprintf(stderr, "unknown option = %s\n", *argv);
			usage();
			exit(1);
		}
	}

	if (! generic && ! unmount && argc-- > 0)
		host = *argv++;
	if (argc-- > 0)
	{
		mntpt = *argv++;
		if (*mntpt != '/')
		{
			fprintf(stderr, "mount point must begin with '/'\n");
			mntpt = NULL;
		}
	}
	else
	{
		show();
		exit(0);
	}

	if (argc > 0
	|| (generic && unmount)
	|| (mntpt == NULL))
		usage();

	if (unmount)
		turnoff(mntpt);
	else
		turnon(mntpt, host);
}

/*
 * Display the current mount points in the kernal.
 */
show()
{
	long		index = 0,
			diff,
			now,
			kfd;
	char		buf[BUFSIZ],
			*p;
	struct sockaddr_in hostaddr;
	struct sockaddr_in	*sys;
	struct servent *servp;
	struct hostent	*hostent;
	struct remoteinfo rinfo[ R_MAXSYS ],
			*rp;
	struct mbuf	bufs[ R_MAXSYS ],
			*m;

	servp = getservbyname(REMOTE_FS_SERVER, "tcp");
	/*
	 * Get the address of the remote mount point information
	 * and read kernel memory.
	 */
	nlist(kernel, nl);
	if(nl[0].n_type == 0)
		log_fatal("no %s for namelist\n", kernel);
	kfd = open("/dev/kmem", 0);
	if(kfd < 0)
		log_fatal("cannot open /dev/kmem\n");
	lseek(kfd, (long)nl[0].n_value, 0);
	read(kfd, rinfo, sizeof(struct remoteinfo) * R_MAXSYS);

	/*
	 * Now get the mbufs on each mount point.
	 */
	m = bufs;
	time(&now);
	for (index=0, rp = rinfo; rp < rinfo+R_MAXSYS; rp++, index++)
	{
		buf[0] = '\0';
		if (rp->r_name || rp->r_mntpt)
			printf("%d: ", index);
		else
			continue;
		if (rp->r_name)
		{
			lseek(kfd, (long)rp->r_name, 0);
			read(kfd, m, sizeof(struct mbuf));
			rp->r_name = m++;
			sys = mtod(rp->r_name, struct sockaddr_in *);
			hostent = gethostbyaddr(&sys->sin_addr,
				sizeof (struct in_addr), sys->sin_family);
			if (hostent == NULL)
			{
				log("no host entry for %s\n",
					inet_ntoa(sys->sin_addr));
				continue;
			}
			bprintf(buf, "%s(%s) on ",
				hostent->h_name, inet_ntoa(sys->sin_addr));
		}
		else
			bprintf(buf, "generic mount point ");
		bprintf(buf, "%s", rp->r_mntpath);
		if (rp->r_mntpt == NULL)
			bprintf(buf, ", implied");
		if (rp->r_name && sys->sin_port != servp->s_port)
			bprintf(buf, ", port %d", sys->sin_port);
		if (rp->r_sock)
			bprintf(buf, ", connected");
		if (rp->r_close)
			bprintf(buf, ", closing");
		if (rp->r_users)
			bprintf(buf, ", %d process%s",
				rp->r_users, rp->r_users > 1 ? "es" : "");
		if (rp->r_nfile)
			bprintf(buf, ", %d open file%s",
				rp->r_nfile, rp->r_nfile > 1 ? "s" : "");
		if (rp->r_nchdir)
			bprintf(buf, ", %d chdir%s",
				rp->r_nchdir, rp->r_nchdir > 1 ? "'s" : "");
		if (rp->r_opening)
			bprintf(buf, ", opening");
		if (rp->r_failed)
		{
			bprintf(buf, ", connect failed, retry ");
			diff = rp->r_age - now;
			if (diff <= 0)
				bprintf(buf, "time reached");
			else
			{
				bprintf(buf, "in ");
				if (diff / 60)
					bprintf(buf, "%d minute%s", diff/60,
						(diff/60) > 1 ? "s" : "");
				if (diff / 60 && diff % 60)
					bprintf(buf, " and ");
				if (diff % 60)
					bprintf(buf, "%d second%s", diff%60,
						(diff%60) > 1 ? "s" : "");
			}
		}
		else if(rp->r_sock == NULL && rp->r_age)
		{
			bprintf(buf, ", last closed %s",
				ctime(&rp->r_age));
			buf[ strlen(buf)-1 ] = '\0'; /* remove newline */
		}
		printf("%s\n", buf);
	}
}

/*
 * buffer printf.  i.e. do a printf into a buffer, appending to whatever
 * is there.  Split long lines.
 */
bprintf(buf, x0,x1,x2,x3,x4,x5,x6,x7,x8,x9)
	char	*buf;
{
	char	xbuf[ BUFSIZ ],
		*pfrom, *pto, c;
	long	col;

	sprintf(xbuf, x0,x1,x2,x3,x4,x5,x6,x7,x8,x9);
	for (pto = buf; *pto; pto++) ;
	for (pfrom = xbuf, col=0; *pfrom; pfrom++, col++)
	{
		c = *pfrom;
		*pto++ = c;
		if (c == '\n')
			col = -1;
		else if (c == ' ' && col > 50)
		{
			*pto++ = '\n';
			*pto++ = '\t';
			col = 7;
		}
	}
	*pto = '\0';
}

/*
 * Do a mount.
 */
turnon(mntpt, host)
	char	*mntpt, *host;
{
	int	index, ret, fdout, fdin;
	struct message msgbuf, *msg = &msgbuf;
	struct sockaddr_in	sys;
	char	buf[ BUFSIZ ];

	if (strlen(mntpt) >= R_MNTPATHLEN)
		log_fatal("mount point must be < %d chars\n", R_MNTPATHLEN);

	/*
	 * Connect to the machine and send it our byte order and
	 * password file.
	 */
	if (host)
	{
		if ((fdout = tcpname(&sys, host)) < 0)
			log("system unreachable now...");
		index = remoteon(mntpt, strlen(mntpt)+1,
			&sys, sizeof(struct sockaddr_in));
	}
	else
		index = remoteon(mntpt, strlen(mntpt), 0, 0);
	if (index == -1)
		log_fatal("cant mount remote fs\n");
	else if (host && fdout < 0)
		log(" system mounted anyway\n");
	if (host == NULL)
		return;
	if ((fdin = open("/etc/passwd", O_RDONLY)) == -1)
		log_fatal("can't open /etc/passwd\n");
	msg->m_syscall = htons(RSYS_nosys);
	msg->m_hdlen = htons(R_MINRMSG + sizeof(long));
	msg->m_totlen = htonl(R_MINRMSG + sizeof(long));
	msg->m_args[0] = htonl(CMD_MOUNT);
	write(fdout, msg, R_MINRMSG + sizeof(long));
	write(fdout, byteorder, 4);
	while ((ret = read(fdin, buf, BUFSIZ)) > 0)
		write(fdout, buf, ret);
	close(fdout);
	close(fdin);
	return;
}

turnoff(mntpt)
	char	*mntpt;
{
	int	index, fd;

	index = remoteoff(mntpt);
	if (index == -1)
		log_fatal("can't unmount remote fs\n");
	close(fd);
}

usage()
{
	fprintf(stderr, "Usage:\t%s\n\t%s\n\t%s\n",
		"rmtmnt [-sservicename] -g path",
		"rmtmnt [-sservicename] host path",
		"rmtmnt");
	exit(1);
}

tcpname(sin, host)
	struct sockaddr_in *sin;
	char	*host;
{
	struct servent *servp;
	struct hostent *hostp;
	int s;

	servp = getservbyname(service ? service : REMOTE_FS_SERVER, "tcp");

	if (servp == NULL) {
		fprintf(stderr, "%s: unknown service\n", REMOTE_FS_SERVER);
		exit(1);
	}

	hostp = gethostbyname(host);
	if (hostp == NULL) {
		fprintf(stderr, "%s: unknown host\en", host);
		exit(1);
	}
	bzero((char *)sin, sizeof (struct sockaddr_in));
	bcopy(hostp->h_addr, (char *)&sin->sin_addr, hostp->h_length);
	sin->sin_family = hostp->h_addrtype;
	sin->sin_port = servp->s_port;

	/*
	 * Ok, now make sure that the connection will work
	 */
	if (sigvec(SIGALRM, &vec, (struct sigvec *)0) == -1) {
		perror("signals");
		return(-1);
	}
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
		return(-1);
	alarm(5);
	if(connect(s, sin, sizeof(struct sockaddr_in)) < 0) {
		alarm(0);
		return(-1);
	}
	alarm(0);
	return(s);
}

onalrm(sig)
{
	fprintf(stderr, "timeout: ");
}

log_fatal(x0,x1,x2,x3,x4,x5,x6,x7,x8,x9)
{
	log(x0,x1,x2,x3,x4,x5,x6,x7,x8,x9);
	exit(1);
}

log(x0,x1,x2,x3,x4,x5,x6,x7,x8,x9)
{
	if (errno)
		perror("rmtmnt");
	errno = 0;
	fprintf(stderr, x0,x1,x2,x3,x4,x5,x6,x7,x8,x9);
}
