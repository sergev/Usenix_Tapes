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
 * $Log:	change.c,v $
 * Revision 2.0  85/12/07  18:20:57  toddb
 * First public release.
 * 
 */
static char	*rcsid = "$Header: change.c,v 2.0 85/12/07 18:20:57 toddb Rel $";
#include	"server.h"
#include	<stdio.h>
#include	<sys/time.h>

extern short	current_uid;
extern short	current_pid;
extern short	current_umask;
extern short	current_server;
extern short	gateway_server;
extern long	fds_in_use;
extern long	to_gateway;
extern long	from_servers;
extern boolean	i_am_gateway;
extern boolean	i_have_control;
extern process	*wildcard;
extern syscallmap	smap[];
extern hosts	*host;

/*
 * Given the user id # and the process id # in the message, we look up the
 * process (or allocate a new one).  If there are things to be accomplished
 * before the system call is performed, do them here.
 */
process *change_to_proc(msg)
	register struct message	*msg;
{
	register rusers	*ruser;
	register process	*proc = NULL;
	register char	*p;
	register long	syscall = msg->m_syscall,
			syscalltype = smap[ syscall ].s_type;

	/*
	 * First, we check to see that the file is not really a local file
	 * for the client.  A simple example of this is a pathname of '..'
	 * while sitting in our root directory.  If it is local, islocal()
	 * will compose a message and send it.  We simply return.
	 *
	 * Even if this is handled by another server, we may be able to serve
	 * the request;  but only if:
	 *	1. it is a system call using a path (NEED_CWD).
	 *	2. The path is starts with a '/'.
	 *	3. The system call does not generate a new file descriptor
	 *	   (like open).
	 *	4. This is not chdir().
	 * The system calls that fall in this category, are stat, lstat,
	 * rename, unlink, symlink, readlink, etc.
	 * If the system call does not match these criterion, then we must
	 * reroute it.
	 */

	proc = findprocess(msg->m_pid, msg->m_uid);
	if (syscalltype & NEED_CWD) {
		if (proc
		&& proc->p_handler == current_pid
		&& islocal(msg, syscalltype))
			return(NULL);
		if ((syscalltype & NEED_MYSERVER) == 0)
		{
			if (syscalltype & NEED_2PATH)
				p = twopath2addr(msg);
			else
				p = path1addr(msg);
			if (*p == '/')
			{
				setup_proc(proc = wildcard,
					msg->m_uid, msg->m_pid);
				debug0("using wildcard proc... ");
			}
		}
	}

	/*
	 * A fork() (but not vfork()) generates two messages, one
	 * from parent, one from the child.  If we know about it (no
	 * matter if we are the server for the parent, the child or if
	 * we are the gateway), then we already have received the
	 * first notification.  Don't do a reroute for that, just
	 * handle it locally (its redundant info).  If we don't know
	 * about it, then this is the first anyone has heard of it,
	 * so we use args[1] which is the parent pid of
	 * the fork (in both messages).
	 */
	if (proc == NULL && (syscall == RSYS_fork || syscall == RSYS_vfork))
		proc = findprocess(msg->m_args[ 1 ], msg->m_uid);

	if (proc == NULL)
	{
		/*
		 * If we are the gateway, and don't know about this process,
		 * and it is an exit() call, then just ignore it; cause if we
		 * don't know this guy, nobody does.  Otherwise,
		 * allocate a new slot for it.
		 */
		if (i_am_gateway)
		{
			if (syscall == RSYS_exit)
			{
				debug0("discard exit call for pid %d\n",
					msg->m_pid);
				return(NULL);
			}
			else
				proc = add_new_process(msg->m_uid, msg->m_pid);
		}
		/*
		 * If we are'nt the gateway, then hand this request back to
		 * the gateway.  Maybe he will know where to send the request.
		 */
		else
		{
			reroute(gateway_server, msg);
			return(NULL);
		}
	}
	/*
	 * And if we just happen to know about this process (whether we
	 * are the gateway or not), then just reroute it.
	 */
	else if (proc->p_handler != current_pid)
	{
		reroute(proc->p_handler, msg);
		if (syscall == RSYS_exit && !i_am_gateway)
		{
			deletelist(&host->h_proclist, proc);
			freeproc(proc);
		}
		return(NULL);
	}

	/*
	 * At this point, the request is for us, and there is definitely
	 * no mistake.
	 */
	if ((syscalltype & NEED_FD) || syscall == RSYS_chdir)
	{
		debug3("%d current file descriptors\n", fds_in_use);
		/*
		 * Here is where we reroute the opening of a file
		 * or a chdir() to another server.
		 */
		if (need_to_fork())
			if (! become_server(msg))
				return(NULL);
	}

	if (syscalltype & NEED_PERM)
		change_to_user( proc->p_ruser->r_user );

	debug1("pid %d: ", proc->p_pid);
	return(proc);
}

/*
 * change the current user id to 'uid'.
 * This is done with simply a setreuid
 */
change_to_user(user)
	register users	*user;
{
#ifdef RFSDEBUG
	long	gids[ NGROUPS ], ngids, i;
#endif RFSDEBUG

	if (current_uid != user->u_local_uid)
	{
		debug2("set uid to %s(%d)\n",
			user->u_name, user->u_local_uid);
		change_to_uid(0);
		if (setgroups(user->u_numgroups, user->u_local_groups) < 0)
		{
			register long	i;

			log("cannot set gids\n");
			for (i=1; i<user->u_numgroups; i++)
				log(",%d", user->u_local_groups[i]);
			return(FALSE);
		}
		change_to_uid(user->u_local_uid);
	}
	else
		debug2("already at uid %d (uid=%d/%d)\n",
			current_uid, getuid(), geteuid());
#ifdef RFSDEBUG
	if (remote_debug & 0x4)
	{
		log("%d gids should be", user->u_numgroups);
		for (i=0; i<user->u_numgroups; i++)
			log(" %d", user->u_local_groups[i]);
		ngids = getgroups(NGROUPS, gids);
		log("\n%d gids are=", ngids);
		for (i=0; i<ngids; i++)
			log(" %d", gids[i]);
		log("\n");
	}
#endif RFSDEBUG
	return (TRUE);
}

/*
 * Change to uid 'uid'.
 */
change_to_uid(uid)
	register long	uid;
{
	if (current_uid != uid)
		if (setreuid(0, uid) < 0)
			log("cannot set uid to %d\n", uid);
		else
		{
			current_uid = uid;
			debug2("uid now %d/%d\n", getuid(), geteuid());
		}
}

/*
 * Change to umask 'mask'.
 */
change_to_umask(mask)
	register long	mask;
{
	register long	oldmask;

	if (mask != current_umask)
	{
		oldmask = umask(current_umask = mask);
		debug2("umask now 0%o, was 0%o\n", current_umask, oldmask);
	}
}

/*
 * check to see if we need to fork.  We do this for two reasons:
 *	1. we are the gateway server.
 *	2. we are currently handling other processes (a chdir might mess
 *	   them up).
 * Note that we only have to ask ourselves "do we need to fork" when we are
 * opening a new file, accepting a new process (the client is doing a fork()
 * or vfork()), or changing directory.
 */
need_to_fork()
{
	register char	*fds;
	register process	*proc;
	register long	myprocs = 0;

	if (i_am_gateway)
		return(TRUE);
	for (proc = host->h_proclist; proc; proc = proc->p_next)
		if(proc->p_handler == current_pid)
			myprocs++;
	return(myprocs > 1);
}
