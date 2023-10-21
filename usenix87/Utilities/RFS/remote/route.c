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
 * $Log:	route.c,v $
 * Revision 2.2  86/01/05  18:14:47  toddb
 * Added a forgotten case to gateway_listen(): S_CORRUPTED.
 * 
 * Revision 2.1  85/12/19  15:53:23  toddb
 * Changed declaration of a local variable (sigmask) because it conflicts
 * with a 4.3 define.
 * 
 * Revision 2.0  85/12/07  18:22:04  toddb
 * First public release.
 * 
 */
static char	*rcsid = "$Header: route.c,v 2.2 86/01/05 18:14:47 toddb Exp $";
#include	"server.h"
#include	<sys/file.h>
#include	<sys/time.h>
#include	<setjmp.h>
#include	<errno.h>

extern short	current_pid;
extern short	current_ppid;
extern short	gateway_server;
extern short	current_server;
extern long	blocking_servers;
extern long	to_gateway;
extern long	from_servers;
extern long	errno;
extern boolean	i_am_gateway;
extern boolean	i_have_control;
extern boolean	gateway_needs_control;
extern boolean	route_to_gateway;
extern boolean	watch_for_lock;
extern boolean	i_am_asleep;
extern hosts	*host;

/*
 * Reroute to the server whose pid is 'pid'.
 */
reroute(pid, msg)
	register short	pid;
	register struct message	*msg;
{
	if (route_to_gateway)
	{
		debug5("routing changed from server %d to gateway\n", pid);
		route_to_gateway = FALSE;
		pid = gateway_server;
	}
	watch_for_lock = gateway_needs_control = FALSE;
	dont_gobble_msg(msg);

	if (pid == current_pid)
		log_fatal("reroute myself???\n");
	debug5("%d waking up %d\n", current_pid, pid);

	/*
	 * If we are the gateway, there may be some servers that are blocking
	 * on a request.  If so, then we lock file descriptor 2 with an
	 * shared lock.  This tells the server to always check to see if
	 * the lock goes up to an exclusive lock.  If so, then the server
	 * must then return control to the gateway.
	 */
	if (i_am_gateway)
	{
		set_label("reading messages");
		i_have_control = FALSE;
		if (blocking_servers)
			if (flock(2, LOCK_NB | LOCK_SH) < 0)
				log_fatal("cannot lock fd 2\n");
	}

	i_am_asleep = TRUE;
	if (pid == gateway_server)
		say_something(S_THIS_IS_YOURS, gateway_server);
	else
		wake_up(pid);
	slumber(FALSE);
	if (! i_am_gateway)
	{
		/*
		 * Check for the lock on fd 2.  But even if the gateway wants
		 * control, go ahead an serve this request.
		 */
		if (flock(2, LOCK_NB | LOCK_EX) < 0)
			if (flock(2, LOCK_NB | LOCK_SH) >= 0)
			{
				debug5("watch for lock on each request\n");
				watch_for_lock = TRUE;
				flock(2, LOCK_UN);
			}
			else
			{
				debug5("Gateway wants control\n");
				gateway_needs_control = TRUE;
			}
		else
			flock(2, LOCK_UN);
	}
}


/*
 * Tell the gateway something.
 */
say_something(cmd, arg)
	register long	cmd, arg;
{
	gtmsgs		gmsg[2];
	register gtmsgs	*g = gmsg;
	register long	len = sizeof(gtmsgs);


	if (cmd == S_NEWSERVER)	/* actually 2 messages */
	{
		g->g_server = current_ppid;
		g->g_cmd = cmd;
		g->g_pid = current_pid;
		cmd = S_NEWPROCESS;
		g++;
		len += sizeof(gtmsgs);
	}
	g->g_server = current_pid;
	g->g_cmd = cmd;
	g->g_pid = arg;

	debug5("say: cmd=%d, arg=%d\n", cmd, arg);
	if (write(to_gateway, gmsg, len) != len)
		log_fatal("pid %d: can't write to gateway!!\n",
			current_pid);
}

/*
 * Read message from servers.  We do the allocation of space and maintain it.
 */
gtmsgs *read_gtmsgs()
{
	static gtmsgs	*msgs;
	static long	current_len,
			len_needed = 10;
	register long	red;
	register gtmsgs	*g;

	/*
	 * Allocate space for the current read.
	 */
	if (current_len < len_needed)
	{
		if (msgs)
			msgs = (gtmsgs *)realloc(msgs,
				len_needed * sizeof(gtmsgs));
		else
			msgs = (gtmsgs *)malloc(len_needed * sizeof(gtmsgs));
		current_len = len_needed;
	}

	/*
	 * Now read the messages.
	 */
	red = read(from_servers, msgs, (current_len-1) * sizeof(gtmsgs));
	if (red % sizeof(gtmsgs) != 0)
		log_fatal("partial message on read = %d\n", red);
	red /= sizeof(gtmsgs);
	if (red == current_len-1)
		len_needed++;
	msgs[ red ].g_server = 0;
#ifdef RFSDEBUG
	for (g=msgs; g->g_server; g++)
		debug14("red: server %d, cmd %d, pid=%d\n",
			g->g_server, g->g_cmd, g->g_pid);
#endif RFSDEBUG
	return(msgs);
}
	
/*
 * This process is called to gather incomming messages from servers out
 * there with something interesting to say.  We return TRUE if we have
 * read a message from a process that is relinquishing control, FALSE
 * otherwise.
 */
gateway_listen()
{
	register process	*proc;
	register gtmsgs		*msgs, *g;
	short			dequeue();
	register short		cmd, i, pid, server;

	msgs = read_gtmsgs();

	errno = 0;
	for (g=msgs; g->g_server; g++)
	{
		pid    = g->g_pid;
		server = g->g_server;
		cmd    = g->g_cmd;

		switch(cmd) {
		case S_NEWSERVER: /* a new server forked by another server */
			debug5("hear: %d forks new server %d\n", server, pid);
			break;
		case S_NEWPROCESS: /* a new process is being served */
			proc = add_new_process(0, pid);
			proc->p_handler = server;
			debug5("hear: pid %d serving pid %d\n", server, pid);
			break;
		case S_PROCEXIT: /* some server's client did an exit() call */
			debug5("hear: proc exit from server %d: pid=%d\n",
				server, pid);
			if ((proc = findprocess(pid, -1)) == NULL)
				log("can't find pid %d!\n", pid);
			else
			{
				deletelist(&host->h_proclist, proc);
				freeproc(proc);
			}
			break;
		case S_I_WOULD_BLOCK: /* server will block on I/O */
			debug5("hear: server %d blocks\n", server);
			blocking_servers++;
			goto gateway_control;
		case S_ALLDONE:	/* an existing server is ready to die */
		case S_EOF:	/* a server got eof on command socket */
			mourne();
			debug5("hear: server %d %s... ", server,
				cmd == S_ALLDONE ? "dead" : "got eof");
			for (proc=host->h_proclist; proc; proc=proc->p_next)
				if (proc->p_handler == server)
				{
					debug5("free proc %d...", proc->p_pid);
					deletelist(&host->h_proclist, proc);
					freeproc(proc);
				}
			debug5("\n");
			/* fall through */
		case S_THIS_IS_YOURS: /* just relinquish control */
	gateway_control:
			/*
			 * Always unlock when we have control.
			 */
			flock(2, LOCK_UN);
			if (cmd == S_THIS_IS_YOURS)
				debug5("hear: server %d gives us control\n",
					server);
			/*
			 * Now that we have control, see about dequeing
			 * a server that is ready to go.  If there is one,
			 * Then change this message so that it looks like
			 * a message from ourself saying to reroute to
			 * 'server'.
			 */
			server = dequeue();
			if (server > 0)
			{
				debug5("server %d ready to go\n", server);
				wake_up(server);
			}
			else
			{
				debug5("gateway pid %d continuing\n",
					current_pid);
				set_label("active");
				i_have_control = TRUE;
			}
			break;
		case S_I_AM_READY:
			debug5("hear: server %d ready\n", server);
			blocking_servers--;
			if (flock(2, LOCK_EX) < 0)
				log_fatal("cannot lock fd 2\n");
			queue(server);
			break;
		case S_CORRUPTED:
			log_fatal("corrupted input stream\n");
			break;
		default:
			log("unknown message from %d = %d\n", server, cmd);
			break;
		}
	}

	return(FALSE);
}

wake_up(pid)
	long	pid;
{
	sendsig(pid, SIGIO);
}

sendsig(pid, sig)
	long	pid,
		sig;
{
	register func	logger;
	extern long	log(), log_fatal();

	change_to_uid(0);
	if (kill(pid, sig) < 0)
	{
		if (errno == ESRCH)
			logger = log;
		else
			logger = log_fatal;
		logger("couldn't signal %d w/ sig=%d\n", pid, sig);
		return(FALSE);
	}
	return(TRUE);
}

static short	*server_queue;
static short	server_len;
static short	last_server;
/*
 * Put a server on a queue to be run again.  Fifo queue.
 */
queue(pid)
	register short	pid;
{
	if (++last_server > server_len)
	{
		server_len++;
		if (server_queue == NULL)
			server_queue = (short *)malloc(sizeof(short));
		else
			server_queue = (short *)realloc(server_queue,
				server_len*sizeof(short));
	}
	server_queue[ last_server - 1 ] = pid;
}

/*
 * Get the first server off the queue.  Blech!  We have to copy all the
 * queue back one.
 */
short dequeue()
{
	register short	retval, i;

	if (last_server == 0)
		return(0);
	retval = server_queue[ 0 ];
	for (i=1; i<last_server; i++)
		server_queue[ i-1 ] = server_queue[ i ];
	last_server--;
	return( retval );
}

/*
 * Go to sleep awaiting a wakup call.  Do not return until we receive it.
 * However, if we are the gateway, we, of course MUST return and go
 * on reading messages.
 *
 * Since there is a window between testing the i_am_asleep flag and doing
 * the pause, in which we could be awakened, we must always jump around
 * this loop using longjmp() from the interrupt routine.
 */
slumber(forked)
	boolean	forked;
{
	register long	signalmask;

	if (i_am_gateway)
	{
		set_label("reading messages");
		i_have_control = FALSE;
		return;
	}
	set_label("asleep");
	signalmask = sigblock(1<<(SIGIO-1));
	while (i_am_asleep)
		sigpause(signalmask);
	sigsetmask(signalmask);

	debug5("pid %d continuing%s\n",
		current_pid, forked ? "after fork" : "");
	set_label("active");
	mourne();
}
