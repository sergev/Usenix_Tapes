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
 * $Log:	find.c,v $
 * Revision 2.0  85/12/07  18:21:23  toddb
 * First public release.
 * 
 */
static char	*rcsid = "$Header: find.c,v 2.0 85/12/07 18:21:23 toddb Rel $";
#include	"server.h"
#include	<stdio.h>
#include	<signal.h>

extern hosts	*hostlist;
extern hosts	*host;
extern users	*userlist;
extern boolean	i_am_gateway;
extern short	current_pid;

process *findprocess(pid, uid)
	register short	pid;
	register short	uid;
{
	register process	*p;
	register rusers	*ruser;

	debug0("findproc: ", dumpprocs(host->h_proclist));
	for(p = host->h_proclist; p; p=p->p_next)
		if (p->p_pid == pid)
		{
			debug0("found pid %d\n", pid);
			/*
			 * If the user changes uid, then change with him.
			 */
			if (uid >= 0 && uid != p->p_uid)
			{
				debug2("pid %d changes uid %d->%d\n",
					pid, p->p_uid, uid);
				p->p_uid = uid;
				if (ruser = findremuid(&host->h_rusers, uid))
					p->p_ruser = ruser;
				else
					p->p_ruser = host->h_default_ruser;
				debug2(" locally mapped to %s(%d)\n",
					p->p_ruser->r_user->u_name,
					p->p_ruser->r_user->u_local_uid);
			}
			toplist(&host->h_proclist, p);
			return(p);
		}
	return(NULL);
}

/*
 * find the user structure whose name is 'name'.
 */
users *findusername(name)
	register char	*name;
{
	register users	*user;

	for(user=userlist; user; user=user->u_next)
		if (strcmp(user->u_name, name) == 0)
		{
			toplist(&userlist, user);
			return(user);
		}
	return(NULL);
}

hosts *findhostname(name)
	register char	*name;
{
	register hosts	*h;
	register int	i;
	register char	**hnames;

	for(h=hostlist; h; h=h->h_next)
		for (i=0, hnames=h->h_names; hnames[ i ]; i++)
			if (strcmp(hnames[ i ], name) == 0)
			{
				toplist(&hostlist, h);
				return(h);
			}
	return(NULL);
}

hosts *findhostaddr(addr)
	register struct in_addr	*addr;
{
	register hosts	*h;

	debug4("find %s...\n", inet_ntoa(*addr));
	for(h=hostlist; h; h=h->h_next)
		if (bcmp(addr, &h->h_addr, sizeof(struct in_addr)) == 0)
		{
			toplist(&hostlist, h);
			debug4("\tis %s (%s)\n",
				h->h_names[0], inet_ntoa(h->h_addr));
			return(h);
		}
		else
			debug4("\tnot %s (%s)\n",
				h->h_names[0], inet_ntoa(h->h_addr));
	log("no host entry for %s, continuing anyway.\n", inet_ntoa(*addr));
	/*
	 * Kludge up a hosts structure for this guy
	 */
	h = newhost();
	h->h_names = newname(NULL, BOGUSHOST);
	bcopy(addr, &h->h_addr, sizeof(struct in_addr));
	addlist(&hostlist, h);
	return(h);
}

rusers *findremuid(list, uid)
	register rusers	**list;
	register int	uid;
{
	register rusers	*ruser;

	for (ruser = *list; ruser; ruser=ruser->r_next)
		if (ruser->r_uid == uid)
		{
			toplist(list, ruser);
			return(ruser);
		}
	return(NULL);
}

/*
 * find the ruser structure whose name is 'name'.
 */
rusers *findrusername(list, name)
	register rusers	**list;
	register char	*name;
{
	register rusers	*ruser;

	for(ruser = *list; ruser; ruser=ruser->r_next)
		if (strcmp(ruser->r_name, name) == 0)
		{
			toplist(list, ruser);
			return(ruser);
		}
	return(NULL);
}

#ifdef RFSDEBUG
dumpprocs(p)
	register process	*p;
{
	register long	i, fd;

	while(p)
	{
		log("proc@%x,pid=%d,uid=%d,next@%x,prev@%x,handler=%d\n",
			p, p->p_pid, p->p_uid, p->p_next, p->p_prev,
			p->p_handler);
		log("\t%s(%d)->%s(%d),fds=",
			p->p_ruser->r_name, p->p_ruser->r_uid,
			p->p_ruser->r_user->u_name,
			p->p_ruser->r_user->u_local_uid);
		for (i=0; i<NOFILE; i++)
			if ((fd = p->p_fds[ i ]) >= 0)
				log("%d->%d ", i, fd);
		log("\n");
		p=p->p_next;
	}
}
#endif RFSDEBUG
