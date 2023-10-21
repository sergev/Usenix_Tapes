#if !defined(lint) && !defined(LINT)
static char rcsid[] = "$Header: do_command.c,v 1.4 87/05/02 17:33:35 paul Exp $";
#endif

/* $Source: /usr/src/local/vix/cron/do_command.c,v $
 * $Revision: 1.4 $
 * $Log:	do_command.c,v $
 * Revision 1.4  87/05/02  17:33:35  paul
 * baseline for mod.sources release
 * 
 * Revision 1.3  87/04/09  00:03:58  paul
 * improved data hiding, locality of declaration/references
 * fixed a rs@mirror bug by redesigning the mailto stuff completely
 * 
 * Revision 1.2  87/03/19  12:46:24  paul
 * implemented suggestions from rs@mirror (Rich $alz):
 *    MAILTO="" means no mail should be sent
 *    various fixes of bugs or lint complaints
 *    put a To: line in the mail message
 * 
 * Revision 1.1  87/01/26  23:47:00  paul
 * Initial revision
 */

/* Copyright 1987 by Vixie Enterprises
 * All rights reserved
 *
 * Distribute freely, except: don't sell it, don't remove my name from the
 * source or documentation (don't take credit for my work), mark your changes
 * (don't get me blamed for your possible bugs), don't alter or remove this
 * notice.  Commercial redistribution is negotiable; contact me for details.
 *
 * Send bug reports, bug fixes, enhancements, requests, flames, etc., and
 * I'll try to keep a version up to date.  I can be reached as follows:
 * Paul Vixie, Vixie Enterprises, 329 Noe Street, San Francisco, CA, 94114,
 * (415) 864-7013, {ucbvax!dual,ames,ucsfmis,lll-crg,sun}!ptsfa!vixie!paul.
 */


#include "cron.h"
#include <signal.h>
#include <pwd.h>
#if defined(BSD)
# include <syslog.h>
# include <sys/wait.h>
#endif /*BSD*/

void
do_command(cmd, u)
	char	*cmd;
	user	*u;
{
	extern int	fork(), _exit();
	extern char	*env_get();
	extern void	child_process();

	Debug(DPROC, ("do_command(%s, (%s,%d,%d))\n",
		cmd, env_get(USERENV, u->envp), u->uid, u->gid))

	/* fork to become asyncronous -- parent process is done immediately,
	 * and continues to run the normal cron code, which means return to
	 * tick().  the child and grandchild don't leave this function, alive.
	 *
	 * vfork() is unsuitable, since we have much to do, and the parent
	 * needs to be able to run off and fork other processes.
	 */
	if (fork() == 0)
	{
		child_process(cmd, u);
		Debug(DPROC, ("[%d] child process done, exiting\n", getpid()))
		_exit(OK_EXIT);
	}
	Debug(DPROC, ("[%d] main process returning to work\n", getpid()))
}


void
child_process(cmd, u)
	char	*cmd;
	user	*u;
{
	extern struct passwd	*getpwnam();
	extern void	sigpipe_func(), be_different();
	extern int	VFORK();
	extern char	*index(), *env_get();

	auto int	stdin_pipe[2], stdout_pipe[2];
	register char	*input_data;


	Debug(DPROC, ("[%d] child process running\n", getpid()))

	/* mark ourselves as different to PS command watchers by upshifting
	 * our program name.
	 */
	{
		register char	*pch;

		for (pch = PROGNAME;  *pch;  pch++)
			*pch = MkUpper(*pch);
	}

#if defined(BSD)
	/* our parent is watching for our death by catching SIGCHLD.  we
	 * do not care to watch for our children's deaths this way -- we
	 * use wait() explictly.  so we have to disable the signal (which
	 * was inherited from the parent.
	 *
	 * this isn't needed for system V, since our parent is already
	 * SIG_IGN on SIGCLD -- which, hopefully, will cause children to
	 * simply vanish when then die.
	 */
	(void) signal(SIGCHLD, SIG_IGN);
#endif /*BSD*/

	/* create some pipes to talk to our future child
	 */
	pipe(stdin_pipe);	/* child's stdin */
	pipe(stdout_pipe);	/* child's stdout */
	
	/* since we are a forked process, we can diddle the command string
	 * we were passed -- nobody else is going to use it again, right?
	 *
	 * if a % is present in the command, previous characters are the
	 * command, and subsequent characters are the additional input to
	 * the command.  Subsequent %'s will be transformed into newlines,
	 * but that happens later.
	 */
	if (NULL == (input_data = index(cmd, '%')))
	{
		/* no %.  point input_data at a null string.
		 */
		input_data = "";
	}
	else
	{
		/* % found.  replace with a null (remember, we're a forked
		 * process and the string won't be reused), and increment
		 * input_data to point at the following character.
		 */
		*input_data++ = '\0';
	}

	/* fork again, this time so we can exec the users' command.
	 */
	if (VFORK() == 0)
	{
		Debug(DPROC, ("[%d] grandchild process VFORK()'ed\n", getpid()))

		/* get new pgrp, void tty, etc.
		 */
		be_different();

		/* close the pipe ends that we won't use.  this doesn't affect
		 * the parent, who has to read and write them; it keeps the
		 * kernel from recording us as a potential client TWICE --
		 * which would keep it from sending SIGPIPE in otherwise
		 * appropriate circumstances.
		 */
		close(stdin_pipe[WRITE_PIPE]);
		close(stdout_pipe[READ_PIPE]);

		/* grandchild process.  make std{in,out} be the ends of
		 * pipes opened by our daddy; make stderr go to stdout.
		 */
		close(STDIN);	dup2(stdin_pipe[READ_PIPE], STDIN);
		close(STDOUT);	dup2(stdout_pipe[WRITE_PIPE], STDOUT);
		close(STDERR);	dup2(STDOUT, STDERR);

		/* close the pipes we just dup'ed.  The resources will remain,
		 * since they've been dup'ed... :-)...
		 */
		close(stdin_pipe[READ_PIPE]);
		close(stdout_pipe[WRITE_PIPE]);

		/* set our directory, uid and gid.
		 */
		setgid(u->gid);		/* set group first! */
		initgroups(env_get(USERENV, u->envp), u->gid);
		setuid(u->uid);		/* you aren't root after this... */
		chdir(env_get("HOME", u->envp));

		/* exec the command.
		 */
		{
			char	*shell = env_get("SHELL", u->envp);

			execle(shell, shell, "-c", cmd, (char *)0, u->envp);
			fprintf(stderr, "execl: couldn't exec `%s'\n", shell);
			perror("execl");
			_exit(ERROR_EXIT);
		}
	}

	/* middle process, child of original cron, parent of process running
	 * the user's command.
	 */

	Debug(DPROC, ("[%d] child continues, closing pipes\n", getpid()))

	/* close the ends of the pipe that will only be referenced in the
	 * son process...
	 */
	close(stdin_pipe[READ_PIPE]);
	close(stdout_pipe[WRITE_PIPE]);

	/*
	 * write, to the pipe connected to child's stdin, any input specified
	 * after a % in the crontab entry; we will assume that it will all
	 * fit, which means (on BSD anyway) that it should be 4096 bytes or
	 * less.  this seems reasonable.  while we copy, convert any
	 * additional %'s to newlines.  when done, if some characters were
	 * written and the last one wasn't a newline, write a newline.
	 */

	Debug(DPROC, ("[%d] child sending data to grandchild\n", getpid()))

	{
		register FILE	*out = fdopen(stdin_pipe[WRITE_PIPE], "w");
		register int	need_newline = FALSE;
		register int	escaped = FALSE;

		while (*input_data)
		{
			if (!escaped && *input_data == '%')
			{
				need_newline = FALSE;
				putc('\n', out);
			}
			else
			{
				need_newline = TRUE;
				putc(*input_data, out);
				escaped = (*input_data == '\\');
			}
			input_data++;
		}
		if (need_newline)
			putc('\n', out);

		/* write 0-length message; this is EOF in a pipe (I hope)
		 */
		fflush(out);
		write(stdin_pipe[WRITE_PIPE], "", 0);
		fclose(out);
		close(stdin_pipe[WRITE_PIPE]);

		Debug(DPROC, ("[%d] child done sending to grandchild\n", getpid()))
	}

	/*
	 * read output from the grandchild.  it's stderr has been redirected to
	 * it's stdout, which has been redirected to our pipe.  if there is any
	 * output, we'll be mailing it to the user whose crontab this is...
	 * when the grandchild exits, we'll get EOF.
	 */

	Debug(DPROC, ("[%d] child reading output from grandchild\n", getpid()))

	{
		register FILE	*in = fdopen(stdout_pipe[READ_PIPE], "r");
		register int	ch;

		if (EOF != (ch = getc(in)))
		{
			FILE	*mail;
			char	*usernm, *mailto;

			Debug(DPROC, ("[%d] got data (%x:%c) from grandchild\n",
				getpid(), ch, ch))

			/* get name of recipient.  this is MAILTO if set to a
			 * valid local username; USER otherwise.
			 */
			usernm = env_get(USERENV, u->envp);
			mailto = env_get("MAILTO", u->envp);
			if (mailto)
			{
				/* MAILTO was present in the environment
				 */
				if (!*mailto)
				{
					/* ... but it's empty. set to NULL
					 */
					mailto = NULL;
				}
				else
				{
					/* not empty -- verify it,
					 * setting to USER if not valid.
					 */
					if (NULL == getpwnam(mailto))
						mailto = usernm;
					endpwent();
				}
			}
			else
			{
				/* MAILTO not present, set to USER.
				 */
				mailto = usernm;
			}
		
			/* if we are supposed to be mailing, MAILTO will
			 * be non-NULL.  only in this case should we set
			 * up the mail command and subjects and stuff...
			 */

			if (mailto)
			{
				extern FILE	*popen();
				extern char	*sprintf();
				register char	**env;
				auto char	mailcmd[MAX_COMMAND];

				(void) sprintf(mailcmd, MAILCMD, mailto);
				if (!(mail = popen(mailcmd, "w")))
				{
					perror(MAILCMD);
					(void) _exit(ERROR_EXIT);
				}
				fprintf(mail, "To: %s\n", mailto);
				fprintf(mail, "Subject: %s\n", MAILSUBJ);
				fprintf(mail, "X-cron-cmd: <%s>\n", cmd);
				for (env = u->envp;  *env;  env++)
					fprintf(mail, "X-cron-env: <%s>\n",
						*env);
				fprintf(mail, "\n");

				/* this was the first char from the pipe
				 */
				putc(ch, mail);
			}

			/* we have to read the input pipe no matter whether
			 * we mail or not, but obviously we only write to
			 * mail pipe if we ARE mailing.
			 */

			while (EOF != (ch = getc(in)))
			{
				if (mailto)
					putc(ch, mail);
			}

			/* only close pipe if we opened it -- i.e., we're
			 * mailing...
			 */

			if (mailto)
				pclose(mail);

		} /*if data from grandchild*/

		Debug(DPROC, ("[%d] got EOF from grandchild\n", getpid()))

		fclose(in);
		close(stdout_pipe[READ_PIPE]);
	}

#if defined(BSD)
	/* wait for child to die.
	 */
	{
		int		pid;
		union wait	waiter;

		Debug(DPROC, ("[%d] waiting for grandchild to finish\n", getpid()))
		pid = wait(&waiter);
		Debug(DPROC, ("[%d] grandchild #%d finished, status=%d\n",
			getpid(), pid, waiter.w_status))
	}
#endif /*BSD*/
}
