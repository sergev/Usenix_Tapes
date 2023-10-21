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
 * $Log:	fileserver.c,v $
 * Revision 2.0  85/12/07  18:21:14  toddb
 * First public release.
 * 
 */
static char	*rcsid = "$Header: fileserver.c,v 2.0 85/12/07 18:21:14 toddb Rel $";
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include "server.h"
#include <sys/file.h>

extern long	errno;
extern long	from_servers;
extern long	to_gateway;
extern long	so_listen;
extern long	blocking_servers;
extern short	gateway_server;
extern short	current_ppid;
extern short	current_pid;
extern short	current_uid;
extern short	current_server;
extern char	mntpt[ MAXPATHLEN ];
extern char	*program;
extern char	*last_argaddr;
extern char	*logfile;
extern char	*service;
extern char	*syscallnames[];
extern boolean	i_am_gateway;
extern boolean	i_am_asleep;
extern boolean	i_have_control;
extern boolean	route_to_gateway;
extern boolean	watch_for_lock;
extern boolean	gateway_needs_control;
extern syscallmap	smap[];
extern process	*wildcard;
extern hosts	*host;

main(argc, argv)
	int argc;
	char **argv;
{
	register hosts	*h;

	setopts(argc, argv);
	if ((remote_debug & 0x200) == 0 && fork())
		exit(0);
	current_pid = getpid();
	setlogfile();
	if ((so_listen = tcppassive()) < 0)
		log_fatal("cannot open socket\n");
	init();
	for (;;)
	{
		if ((h = tcpaccept(so_listen)) == NULL)
			break;
		debug4("call on fd %d, portno %d, from host \"%s\"\n",
			h->h_cmdfd, h->h_portnum, h->h_names[0]);
		dumphost(h);
		if (server(h))
			exit(0);
	}
}

/*
 * This is the top lexical level for the server.  We decide here
 * when to call for a next request and whether we are running the
 * new format remote fs or not.
 *
 * We also decide whether this is a connection from the mount program
 * on a remote host or not.  If it is, then we just want to assemble the
 * info that it gives us and not become a child server.
 */

server(h)
	register hosts	*h;
{
	long		pipefd[ 2 ];
	struct message	msgbuf,
			*getmsg();
	register struct message	*msg = &msgbuf;
	register process	*proc;
	register long	cmd, len;

	/*
	 * Get the first message from this connection.
	 */
	alarm(5);
	if (! (msg = getmsg(h->h_cmdfd)))
	{
		log("connection initiation lost to \"%s\"\n", h->h_names[0]);
		close(h->h_cmdfd);
		h->h_cmdfd = -1;
		alarm(0);
		return(FALSE);
	}
	alarm(0);

	/*
	 * may be a special command
	 */
	len = msg->m_hdlen;
	if (msg->m_syscall == RSYS_nosys)
	{
		cmd = msg->m_args[0];
		debug5("new client; cmd=%d\n", cmd);
		switch(cmd) {
		case CMD_SERVICE:
			break;
		case CMD_MOUNT:
			gobble_last_msg(h->h_cmdfd, msg);
			getbyteorder(h);
			getrusers(h);
			return(FALSE);
		case CMD_NEEDMOUNT:
			sendmount(h);
			dont_gobble_msg(msg);
			return(FALSE);
		default:
			log_fatal("unknown server directive = %d\n", cmd);
		}
	}
	else
	{
		debug5("new client, not mounted by rmtmnt\n");
		dont_gobble_msg(msg);
		if (!h->h_mounted)
			getmount(h);
	}

	/*
	 * If we reach this point, then we are to be the gateway server.
	 * There may ba a server still running.  Kill it.
	 */
	if (h->h_serverpid)
		sendsig(h->h_serverpid, SIGTERM);

	if ((remote_debug & 0x200) == 0 && vfork())
	{
		wait(0);
		/*
		 * we are the parent... just return.
		 */
		close(h->h_cmdfd);
		h->h_cmdfd = -1;
		return(FALSE);
	}
	else if ((remote_debug & 0x200) == 0 && (h->h_serverpid = fork()))
		exit(0);
	host = h;
	wildcard->p_handler = gateway_server = current_pid = getpid();
	set_label("active");
	if ((remote_debug & 0x200) == 0)
	{
		setlogfile();
		close(so_listen);
	}
	if (pipe(pipefd) < 0)
		log_fatal("Can't open pipe\n");
	from_servers = pipefd[ 0 ];
	to_gateway = pipefd[ 1 ];

	/*
	 * Ok, now be a server!
	 */
	for(;;)
	{
		if (i_am_gateway && ! i_have_control)
		{
			gateway_listen();
			continue;
		}
		/*
		 * The gateway may be waiting for control.  Let's see.
		 */
		if (watch_for_lock)
			if (gateway_needs_control
			|| flock(2, LOCK_NB | LOCK_SH) < 0)
			{
				debug5("gateway wants control\n");
				reroute(gateway_server, msg);
				continue;
			}
			else
				flock(2, LOCK_UN);
			
		if ((msg = getmsg(host->h_cmdfd)) == NULL)
			break;
		proc = change_to_proc(msg);
		if (proc == NULL)
			continue;

		errno = 0;
		(*smap[ msg->m_syscall ].s_server)(msg, proc);
	}
	debug5("done.\n");
	if (i_am_gateway)
		for (proc = host->h_proclist; proc; proc=proc->p_next)
		{
			/*
			 * just hand it to the other server... he'll get eof
			 */
			if (proc->p_handler != current_pid)
				sendsig(proc->p_handler, SIGIO);
		}
	else
		say_something(S_EOF, 0);
	if ((remote_debug & 0x800) == 0)
		unlink(logfile);
	return(TRUE);
}

setopts(argc, argv)
	register int	argc;
	register char	**argv;
{
	register int	error = FALSE;
	register char	**p;
	extern char	**environ;

	program = argv[0];
	last_argaddr = argv[argc-1];
	for (argv++, argc--; argc; argv++, argc--)
	{
		if (**argv != '-')
		{
			log("arg \"%s\" is unknown\n",
				*argv);
			error = TRUE;
		}
		switch(argv[0][1]) {
		case 'v':
			if (argv[0][2])
				remote_debug = atox(argv[0] + 2);
			else if (isdigit(argv[1][0]))
				argc--, remote_debug = atox(*(++argv));
			break;
		case 's':
			if (argv[0][2])
				service = argv[0] + 2;
			else
				argc--, service = *(++argv);
			break;
		default:
			log("unknown flag = %s\n", *argv);
			error = TRUE;
		}
	}
	/*
	 * Make sure that last_argaddr points to the last possible address
	 */
	p = environ;
	while (*p)
		p++;
	if (p != environ)
		p--;
	if (*p)
		last_argaddr = *p;
	last_argaddr = (char *)ctob(btoc(last_argaddr)) - 1;
	debug5("program addr=%x, last_argaddr=%x\n", program, last_argaddr);

	if (error)
		exit(1);
}

/*
 * ascii to hex
 */
atox(buf)
	char    *buf;
{
	register char   *p;
	register unsigned       num, nibble;

	/*
	 * now, take it out checking to make sure that the number is
	 * valid.
	 */
	if (! buf)
		return(0);
	for(num=0, p = buf; *p; p++)
	{
		nibble = *p;
		if (nibble >= 'A' && nibble <= 'F')
			nibble -= 'A' - 10;
		else if (nibble >= 'a' && nibble <= 'f')
			nibble -= 'a' - 10;
		else if (nibble >= '0' && nibble <= '9')
			nibble -= '0';
		else
			return(0);
		num = (num << 4) | nibble;
	}
	return(num);
}

/*
 * fork() and give the process on the top of the list to the child.
 */
become_server(msg)
	register struct message	*msg;
{
	register long	pid, i;
	register char	*fds;
	register process	*proc = host->h_proclist;

	/*
	 * Have to change to uid 0 or we may be refused a fork
	 */
	change_to_uid(0);
	i_am_asleep = TRUE;
	if (pid = fork()) /* the parent loses control */
	{
		if (pid < 0)
			log_fatal("cannot fork\n");
		debug5("new server: pid=%d,mine=%d give him (%d)\n",
			pid, current_pid, host->h_proclist->p_pid);
		proc->p_handler = pid;
		dont_gobble_msg(msg);
		slumber(TRUE);
		if (i_am_gateway)
			current_server = pid;
		return(FALSE);
	}

	/*
	 * If we got this far, then we are no longer the gateway.  So set
	 * our pid, etc.  Also, try to dup stderr so that flock will work.
	 * If we can't do it, we are in big trouble.
	 */
	current_ppid = current_pid;
	current_pid = getpid();
	if (i_am_gateway)
	{
		close(from_servers);
		if (blocking_servers)
		{
			watch_for_lock = TRUE;
			route_to_gateway = TRUE;
		}
	}
	else
		say_something(S_NEWSERVER, proc->p_pid);
	i_am_gateway = FALSE;
	set_label("active");
	wildcard->p_handler = current_pid;
	proc->p_handler = current_pid;
	if ((i = dup(2)) < 0)
		log_fatal("cannot dup(2)\n");
	dup2(i, 2);
	close(i);
	debug5("new server: pid=%d, ppid=%d\n", current_pid, getppid());

	return(TRUE);
}

#ifdef RFSDEBUG
dumphost(h)
	register hosts	*h;
{
	register rusers	*ruser;

	if ((remote_debug & 0x10) == 0)
		return;
	log("host %s, local user = %s, ruser@%x\n",
		*h->h_names,
		h->h_default_user ? h->h_default_user->u_name : "default",
		ruser = h->h_default_ruser);
	log("\tr %s(%d)-->%s(%d)\n",
		ruser->r_name, ruser->r_uid,
		ruser->r_user->u_name, ruser->r_user->u_local_uid);
	for(ruser = h->h_rusers; ruser; ruser=ruser->r_next)
		log("\tr %s(%d)-->%s(%d)\n",
			ruser->r_name, ruser->r_uid,
			ruser->r_user->u_name, ruser->r_user->u_local_uid);
}
#endif RFSDEBUG

set_label(string)
	register char	*string;
{
	char		process_label[ 100 ];
	static char	*pend;
	static short	pid;
	register char	*pfrom, *pto;
	register long	i;

	if (pid != current_pid)
	{
		if (i_am_gateway)
			sprintf(process_label, "%s gateway server: ",
				host->h_names[0]);
		else
			sprintf(process_label, "%s server via %d: ",
				host->h_names[0], pid);
		pid = current_pid;
		pend = program + strlen(process_label);
		strncpy(program, process_label, last_argaddr - program);
		pto = pend;
		while (pto < last_argaddr)
			*pto++ = ' ';
	}
	pto = pend;
	pfrom = string;
	while (pto < last_argaddr && *pfrom)
		*pto++ = *pfrom++;
	for (i=0; pto < last_argaddr && i<10; i++)
		*pto++ = ' ';
	if (pto <= last_argaddr)
		*pto = '\0';
}
