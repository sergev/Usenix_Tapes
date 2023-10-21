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
 * $Log:	new.c,v $
 * Revision 2.0  85/12/07  18:21:48  toddb
 * First public release.
 * 
 */
static char	*rcsid = "$Header: new.c,v 2.0 85/12/07 18:21:48 toddb Rel $";
#include	"server.h"
#include	<stdio.h>

extern short	current_pid;
extern hosts	*host;

users *newuser()
{
	register users	*user;

	user = (users *)malloc(sizeof(users));
	if (user == NULL)
		log_fatal("cannot allocate space\n");
	bzero(user, sizeof(users));
	return(user);
}

hosts *newhost()
{
	register hosts	*h;

	h = (hosts *)malloc(sizeof(hosts));
	if (h == NULL)
		log_fatal("cannot allocate space\n");
	bzero(h, sizeof(hosts));
	h->h_cmdfd = -1;
	return(h);
}

rusers *newruser()
{
	register rusers	*ruser;

	ruser = (rusers *)malloc(sizeof(rusers));
	if (ruser == NULL)
		log_fatal("cannot allocate space\n");
	bzero(ruser, sizeof(rusers));
	return(ruser);
}

process *newprocess()
{
	register process	*p;

	p = (process *)malloc(sizeof(process));
	if (p == NULL)
		log_fatal("cannot allocate space\n");
	bzero(p, sizeof(process));
	return(p);
}

freeproc(p)
	register process *p;
{
	if (p->p_execfd >= 0)
		close(p->p_execfd);
	free(p);
}

char	**newname(namelist, name)
	register char	**namelist;
	register char	*name;
{
	register long	i = 0;

	if (namelist == NULL)
		namelist = (char **)malloc(sizeof(char *) * 2);
	else
	{
		/*
		 * count the elements in the list now.
		 */
		for (i=0; namelist && namelist[i]; i++) ;

		namelist = (char **)realloc(namelist, sizeof(char *) * (i+2));
	}
	namelist[ i++ ] = copy(name);
	namelist[ i ] = NULL;
	return(namelist);
}

/*
 * Add a group to 'user' unless he has exceeded the limit or the group
 * is already in his domain.
 */
addgroup(user, gid)
	register users	*user;
	register short	gid;
{
	register long	i = 0,
			*gr = user->u_local_groups;

	for (i=0; i < user->u_numgroups; i++)
		if (gr[ i ] == gid)
			return;
	if (i >= NGROUPS)
		return;
	gr[ user->u_numgroups++ ] = gid;
}

process *add_new_process(uid, pid)
	register short	uid, pid;
{
	register process	*p;
	register long	i;

	debug0("allocate new proc: pid=%d uid=%d host=%s\n",
		pid, uid, host->h_names[0]);
	setup_proc(p = newprocess(), uid, pid);
	addlist(&host->h_proclist, p);

	/*
	 * Initialize the file descriptors for this process.
	 */
	for(i=0; i<NOFILE; i++)
		p->p_fds[ i ] = 0x80;	/* -128 */

	return(p);
}

setup_proc(proc, uid, pid)
	register process	*proc;
	register short	uid, pid;
{
	register rusers	*ruser;

	proc->p_pid = pid;
	proc->p_uid = uid;
	proc->p_handler = current_pid;
	proc->p_returnval = 0;
	proc->p_execfd = -1;
	if (ruser = findremuid(&host->h_rusers, uid))
		proc->p_ruser = ruser;
	else
		proc->p_ruser = host->h_default_ruser;
}
