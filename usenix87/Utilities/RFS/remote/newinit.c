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
 * $Log:	init.c,v $
 * Revision 2.0  85/12/07  18:21:37  toddb
 * First public release.
 * 
 */
static char	*rcsid = "$Header: init.c,v 2.0 85/12/07 18:21:37 toddb Rel $";
#include	"server.h"
#include	<stdio.h>
#include	<pwd.h>
#include	<grp.h>
#include	<netdb.h>
#include	<fcntl.h>
#include	<sys/dir.h>
#include	<sys/user.h>
#include	<sys/signal.h>
#include	<sys/ioctl.h>

extern hosts	*hostlist;
extern hosts	*thishost;
extern users	*userlist;
extern users	*default_user;
extern char	hostname[];
extern char	*service;
extern short	current_uid;
extern short	current_pid;
extern process	*wildcard;
extern struct sigvec	sig_vec;
extern struct sigvec	sig_name;
extern struct sigvec	sig_alarm;
extern struct sigvec	sig_ignore;
extern struct sigvec	sig_continue;
#ifdef RFSDEBUG
extern struct sigvec	sig_debug;
#endif
extern struct stat	root;

/*
 * Initialize the host tables and user tables.
 */
init()
{
	long	tt;
	struct hostent	*gethostent();
	struct passwd	*getpwent();
	struct group	*getgrent();

	/*
	 * catch signals.
	 */
	sigvec(SIGHUP, &sig_ignore, (struct sigvec *)0);
	sigvec(SIGINT, &sig_vec, (struct sigvec *)0);
	sigvec(SIGQUIT, &sig_vec, (struct sigvec *)0);
	sigvec(SIGILL, &sig_vec, (struct sigvec *)0);
#ifdef RFSDEBUG
	sigvec(SIGTRAP, &sig_debug, (struct sigvec *)0);
#endif RFSDEBUG
	/*	SIGIOT		*/
	/*	SIGEMT		*/
	/*	SIGFPE		*/
	/*	SIGKILL		*/
	sigvec(SIGBUS, &sig_vec, (struct sigvec *)0);
	sigvec(SIGSEGV, &sig_vec, (struct sigvec *)0);
	sigvec(SIGSYS, &sig_vec, (struct sigvec *)0);
	sigvec(SIGPIPE, &sig_vec, (struct sigvec *)0);
	sigvec(SIGALRM, &sig_alarm, (struct sigvec *)0);
	sigvec(SIGTERM, &sig_vec, (struct sigvec *)0);
	sigvec(SIGURG, &sig_name, (struct sigvec *)0);
	/*	SIGSTOP		*/
	/*	SIGTSTP		*/
	/*	SIGCONT		*/
	/*	SIGCHLD		*/
	sigvec(SIGTTIN, &sig_vec, (struct sigvec *)0);
	sigvec(SIGTTOU, &sig_vec, (struct sigvec *)0);
	sigvec(SIGIO, &sig_continue, (struct sigvec *)0);
	sigvec(SIGXCPU, &sig_vec, (struct sigvec *)0);
	sigvec(SIGXFSZ, &sig_vec, (struct sigvec *)0);
	sigvec(SIGVTALRM, &sig_vec, (struct sigvec *)0);
	/*	SIGPROF		*/

	/*
	 * set up some important global values, including uid, pid,
	 * the pipe file descriptors for messages to and from the gateway
	 * server.  Register as the nameserver.  Get host name.  Get service.
	 * Get root stat info.
	 */
	if (chdir("/") == -1)
		log_fatal("cannot chdir(\"/\")\n");
	wildcard = newprocess();
	fcntl(2, F_SETFL, FAPPEND);
	close(0);
	close(1);
	change_to_uid(0);
	if (gethostname(hostname, HOSTNAMELEN) < 0 || *hostname == '\0')
		log_fatal("host name not set!\n");
	if (stat("/", &root) < 0)
		log_fatal("cannot stat /\n");
#ifdef CANREMOTE
	if (remotename(NM_SERVER, 0, 0, 0) < 0)
		log("cannot register as nameserver\n");
	/*
	 * Turn off remote access, if we have any.
	 */
	remoteoff(NULL);
#endif
	tt = open("/dev/tty", 2);

	if (tt >= 0)
	{
		ioctl(tt, TIOCNOTTY, 0);
		close(tt);
	}
	setpgrp(0,0);

	initusers();
	initgroups();
	inithosts();
	initrhosts();
}

/*
 * build the list of users on this host (where the server runs).
 */
initusers()
{
	register struct passwd	*pw;
	register users	*user;
	char		buf[ BUFSIZ ];
	register char	*pbuf = buf;

	while(pw = getpwent())
	{
		if (*pw->pw_dir == '\0' || *pw->pw_name == '\0')
		{
			log("login \"%s\" has problems, dir=\"%s\"\n",
				pw->pw_name, pw->pw_dir);
			continue;
		}
		user = newuser();
		user->u_local_uid = pw->pw_uid;
		user->u_name = copy( pw->pw_name );
		addgroup(user, pw->pw_gid);
		user->u_dir = copy( pw->pw_dir );
		sprintf(pbuf, "%s/.rhosts", pw->pw_dir);
		user->u_rhosts = copy( pbuf );
		addlist(&userlist, user);
	}
	endpwent();
	if (user = findusername(DEFAULTUSER))
		default_user = user;
	else
		log_fatal("The user \"%s\" must be in /etc/passwd (%s)\n",
			DEFAULTUSER, "for default permissions");
}

/*
 * Build the list of groups that each user belongs to.
 */
initgroups()
{
	register struct group	*gr;
	register users	*user;
	register char	**p;


	while(gr = getgrent())
	{
		for (p = gr->gr_mem; *p; p++)
			if (user = findusername(*p))
				addgroup(user, gr->gr_gid);
			else
				log("group %s: bad user=%s\n",
					gr->gr_name, *p);
	}
	endgrent();
}

/*
 * Then build the list of all hosts.
 */
inithosts()
{
	register struct hostent	*h;
	register rusers	*ruser;
	register hosts	*hst;
	register users	*user;
	register long	i;
	boolean		duplicate;

	while (h = gethostent())
	{
		/*
		 * One physical host may have more than one physical
		 * address each having a unique host name associated
		 * with it.  If we find any entry having one of its aliases
		 * match a previous alias, then simply fold all aliases
		 * into it, and continue.
		 */
		duplicate = FALSE;
		for (i = -1; i < 0 || h->h_aliases[ i ]; i++)
		{
			if (i < 0)
				hst = findhost(h->h_name);
			else
				hst = findhost(h->h_aliases[i]);
			if (hst)
			{
				duplicate = TRUE;
				break;
			}
		}

		/*
		 * If we have a redundant host... add all the names
		 * in; newname will remove redundant copies.
		 */
		if (!duplicate)
			hst = newhost();
		hst->h_names = newname(hst->h_names, h->h_name);
		for (i=0; h->h_aliases[ i ]; i++)
			hst->h_names = newname(hst->h_names,
					h->h_aliases[ i ]);
		if (duplicate)

		hst->h_addr = *((struct in_addr *)(h->h_addr));
		addlist(&hostlist, hst);

		/*
		 * now if there exists a user on this machine having
		 * the same name as the name of this host (NOT AN
		 * ALIAS!), then that will be our defaut local user
		 * to map to.  Be sure that we don't allow a machine
		 * to be mapped onto a user if the uid is real small:
		 * e.g. a machine named root, where all its user ids
		 * become root using the remote fs!
		 */
		user = findusername(hst->h_names[ 0 ]);
		if (user && user->u_local_uid <= UID_TOO_LOW)
		{
			log("host/user %s: uid %d too low for alias\n",
				hst->h_names[ 0 ], user->u_local_uid);
			user = NULL;
		}
		else if (user)
		{
			hst->h_default_user = user;
			debug2("default user for host %s (%s) is %s\n",
				hst->h_names[ 0 ],
				inet_ntoa(hst->h_addr), user->u_name);
		}
		ruser = hst->h_default_ruser = newruser();
		if (user)
			ruser->r_user = user;
		else
			ruser->r_user = default_user;
		ruser->r_uid = -1;
		ruser->r_name = copy(BOGUSUSER);
	}
	endhostent();
	if ((thishost = findhostname(hostname)) == NULL)
		log_fatal("this host (\"%s\") is not in host file\n",
			hostname);
}

/*
 * Now for each user that has a .rhosts file, assemble the
 * references and attach them to the appropriate host.
 */
initrhosts()
{
	register hosts	*hst;
	register rhost	*rh;
	register users	*user;
	char		buf[ BUFSIZ ];
	register char	*pbuf = buf;

	for (user=userlist; user; user=user->u_next)
	{
		setrhost(user->u_rhosts);
		while (rh = getrhostent(pbuf))
			if (hst = findhostname(rh->rh_host))
				addremoteuser(hst, user, rh->rh_user);
		endrhost();
	}
}

char	*copy(string)
	register char	*string;
{
	register char	*ret = malloc( strlen(string)+1 );

	if (ret == NULL)
		log_fatal("cannot allocate space\n");
	strcpy(ret, string);
	return(ret);
}

/*
 * Add a remote user to those recognized on a certain host.
 */
addremoteuser(h, user, remoteuser)
	register hosts	*h;
	register users	*user;
	register char	*remoteuser;
{
	register rusers	*ruser;
	register long	old = FALSE;

	debug2("\t%s!%s --> %s ", *h->h_names, remoteuser, user->u_name);
	if ((ruser = findrusername(&h->h_rusers, remoteuser)) == NULL)
	{
		debug2("\n");
		ruser = newruser();
	}
	else
	{
		old = TRUE;
		if (strcmp(remoteuser, user->u_name) != 0)
		{
			debug2("(old, ignored)\n");
			return;
		}
		else
			debug2("(old)\n");
	}
	ruser->r_name = copy(remoteuser);
	ruser->r_uid = -1;
	ruser->r_user = user;
	if (! old)
		addlist(&h->h_rusers, ruser);
}
