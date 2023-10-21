/*
 * scriptd.c
 * Public domain script.c command.
 * Dennis Bednar
 * rlgvax!dennis (dennis@rlgvax.UUCP)
 * June 10 1986
 *
 * DOESN'T WORK: some applications which run under the shell and
 * do ioctl()'s to fd 0, 1, or 2 are really trying to do an ioctl
 * to a regular old file descriptor (for the end of a pipe), which
 * unfortunately doesn't work.
 * Examples: stty, ev, pro.
 *
 * ALSO the interrupt key is fielded by the shell, and he dies (at
 * least I think).
 *
 *
 *	KB input   ---> pipe2sh  --->  shell input
 *	SCR output <--- pipe2tty <---  shell output
 *		      /
 *	typescript <-/
 *	  file
 *
 * There are 3 processes:
 *	kbinput() reads from keyboard input until EOF,
 *		and passes through a pipe (pipe2sh) to the shell input.
 *		This process terminates when EOF is typed in.
 *	shell_proc() a UNIX shell that is reading from one pipe, and writing
 *		to another pipe.  This process terminates when EOF
 *		is read from pipe2sh, because kbinput() has gone away.
 *	kboutput() reads from pipe2tty, and writes both to the screen,
 *		and to a file. Because the shell is echoing, the file
 *		captures both input and output.  This process terminates
 *		when EOF is read because the shell has gone away.
 *
 * Note currently that the 'typescript' output file is written to by
 * 2 processes (kbinput() and kboutput()).
 * The output to the 'typescript' outfd file descriptor
 * is raw (ie uses write(), not fwrite()).  This is because there are two
 * processes competing to write to the same file.  Because of the fork(),
 * both processes are appending to the very end of the same typescript file
 * without interference (neat UNIX trick, huh?).
 *
 *
 * Also note that the kbinput() task is echoing, *NOT* the
 * shell.  If you thing about it, its not the shell that normally
 * echoes your commands when you type, its the tty driver.
 *
 *
 */

#include <stdio.h>
#include <errno.h>
#include <signal.h>

/* globals */
char	*cmd;		/* name of argv[0] for error messages */
int	pipe2sh  [2];	/* pipe from kb input to shell input */
int	pipe2tty [2];	/* pipe from sh output to screen output plus file */
char	*filename = "typescript";
int	filfp;		/* for above output file */
char	*shellname = "/bin/sh";

/* int pipefd[2] indices */
#define	P_READ	0
#define	P_WRITE	1

/* forward non-int functions */
char	*u_errmesg();	/* error string for perror() like error messages */

extern	char	*getenv();

main(argc, argv)
	int	argc;
	char	*argv[];
{
	char	*cp;

	/* save the command name in case of error */
	cmd = argv[0];

	/* if SHELL is an enviroment variable, use it, else use default */
	if ( (cp = getenv("SHELL")) != NULL)
		shellname = cp;


	/* the output file MUST be unbuffered, otherwise stdio causes
	 * collision of arbitrary() lower level writes.
	 */
	filfp = creat(filename, 0660);
	if (filfp == -1)
		{
		fprintf(stderr, "%s: %s: %s\n", cmd, filename, u_errmesg());
		exit(1);
		}

	/* create the 2 single directional pipes */
	(void) makepipe( pipe2sh );
	(void) makepipe( pipe2tty );

	/* at this point there will be 0,1,2, plus 4 pipe file descriptors,
	 * plus one file descriptor for the 'typescript' file.
	 */

	/* start the 3 tasks */
	/* the first two routines fork() a child.  The third routine
	 * loops until EOF.
	 */
	printf("Script started, file is typescript\n");
	spawn_in();
	signal(SIGINT, SIG_IGN);	/* so shell doesn't die on INTERRUPT */
	spawn_sh();
	task_output();
}

/*
 * create a unix pipe.  Added a little error checking.
 */
makepipe(pipefd)
	int	pipefd[];
{
	register	int	rtn;

	rtn = pipe( pipefd );
	if (rtn == -1)
		{
		fprintf( stderr, "%s: Cannot make interprocess channel (pipe): %s\n", cmd, u_errmesg());
		exit(1);
		}
}

/*
 * The output task reads the pipe2tty pipe (output from the shell),
 * and writes the same to both stdout, and to the 'typescript' file.
 */
task_output()
{
	register	int	in,
				out,
				numread,
				numwrote;
	char	buf [20];

	/* close unused fd's */
	close(0);
	out = 1;			/* keep 1 open */
	close(2);
	close( pipe2sh[ P_READ ] );
	close( pipe2sh[ P_WRITE ] );
	in = pipe2tty[ P_READ ];	/* keep pipe2tty[ P_READ ] open */
	close( pipe2tty[ P_WRITE ] );

	/* copy output from shell to stdout and to the file */
	while ( (numread = read(in, buf, sizeof(buf))) > 0)
		{
		write(out, buf, numread);
		write(filfp, buf, numread);
		}

	close(filfp);
	printf("Script done, file is typescript\n");
	exit(0);
}

/*
 * spawn input task.
 * create an input child task that reads from the keyboard, and writes
 * through the pipe2sh pipe.
 */
spawn_in()
{
	int	child;
	int	infd,
		outfd;

	child = do_fork();
	if (child < 0)
		{
		fprintf(stderr, "%s: spawn_in could not fork: %s", cmd, u_errmesg());
		exit(1);
		}
	if (child > 0)
		return;			/* parent returns */
	infd = 0;			/* 0 is input */
	close(1);
/*	close(2); */
	close( pipe2sh[ P_READ ] );
	outfd = pipe2sh[P_WRITE];	/* kept open */
	close( pipe2tty[ P_READ ] );
	close( pipe2tty[ P_WRITE ] );
/*	fixtty();				/* must be in raw mode */

	/* read from kb write to shell, and to file */
	do_copy(infd, outfd);		/* child does input task */
}


/*
 * copy task from fd 'in' to fd 'out'
 * PLUS write to the 'typescript' file.
 * This copy loop is used both by kb input, and screen output.
 */
do_copy(in, out)
	int	in;	/* fd for tty input */
	int	out;	/* fd for output to shell */
{
	char	buf[20];
	int	numread;
	int	numwrote;

	while ( (numread = read(in, buf, sizeof(buf)) ) > 0)
		{
		numwrote = write(out, buf, numread);
		if (numread != numwrote)
			exit(0);	/* stderr is closed */
		write(filfp, buf, numread);
		}

	exit(0);

}


/*
 * spawn a shell.  The shell is attached to 2 ends of UNIX pipes.
 */

spawn_sh()
{
	int	child;

	child = do_fork();
	if (child < 0)
		{
		fprintf(stderr, "%s: spawn_in could not fork: %s", cmd, u_errmesg());
		exit(1);
		}
	if (child > 0)
		return;		/* parent returns */

	/* close unused fd's */
	close(0);
	close(1);
	close(2);
	/* pipe2sh[ P_READ ] kept open */
	close( pipe2sh[ P_WRITE ] );
	close( pipe2tty[ P_READ ] );
	/* close(pipe2tty[P_WRITE]); */

	/* redirect all shell stdin, stdout, and stderr to the pipes */
	dup2( pipe2sh[ P_READ ], 0);
	dup2( pipe2tty [ P_WRITE ], 1);
	dup2( pipe2tty [ P_WRITE ], 2);

/*	do_copy(0, 1);		/* stub for now */

	/* the shell should die when it reads EOF, right?? */
	execl(shellname, "sh", "-i", (char *)0);
}



/*
 * can beef this up to retry in the future if desired
 */
do_fork()
{
	return fork();
}

/****** library routines ****/


/*
 * return basename of full path name
 */
char *
basename(path)
	char	*path;
{
	char	*cp;		/* general char pointer */
	char	*strrchr();

	if ((cp = strrchr(path, '/')) == NULL)	/* no rightmost slash */
		return path;
	else
		return cp+1;
}


/*
 * return UNIX error message associated with errno
 * more flexible than perror(3)
 */

char *
u_errmesg()
{
	extern	int	errno;
	extern	int	sys_nerr;
	extern	char	*sys_errlist[];
static	char	buffer[50];

	if (errno < 0 || errno >= sys_nerr)
		{
		sprintf( buffer, "errno %d undefined (%d=max)", errno, sys_nerr);
		return(buffer);
		}

	return( sys_errlist[errno] );
}
