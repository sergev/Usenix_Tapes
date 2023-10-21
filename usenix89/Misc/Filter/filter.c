
# include <stdio.h>
# include <fcntl.h>
# include <signal.h>
# ifdef SYSV
# include <sgtty.h>
# else
# include <sys/ioctl.h>
# endif

# define TRUE 1

/* The following defines are application specific
   and may be changed or removed entirely.
*/

# define QUIT ' '		/* Character for QUIT - terminates everything */
# define TAB    '\033'		/* Redefinition of 'tab' key - no ESC */

int     pipe_term();
int     piperead();
int     abort();
FILE    *popen();


int     pd, pid;
FILE    *pd1, *pp;
struct  sgttyb  params;         /* tty parameters */
struct  sgttyb  p2;             /* tty parameters */


main()
{
        char    c,		/* character just hit */
		last;		/* last character hit - for comparison only */
        char    line[80];       /* text entered up to a newline (cr) */
        int     save, x;
        int     pos;            /* position in char array line */
        int     val;            /* value of line entered */

        signal(SIGQUIT,SIG_IGN);
        signal(SIGINT,SIG_IGN);
        signal(SIGHUP,abort);


        ioctl(0,TIOCGETP,&params);
        save = params.sg_flags;
        params.sg_flags |= CBREAK;      /* set CBREAK bit, don't wait */
					/* for newline */
        params.sg_flags &= ~ECHO;       /* clear ECHO bit */
        ioctl(0,TIOCSETP,&params);
        pid = fork();

        if (pid == 0)
		/* This is the reader process. Reads characters
		   from the pipe file and outputs to standard
		   output
		*/
                {
                pd = open("iopipe",O_RDONLY);
                pd1 = fdopen(pd,"r");
                ioctl(pd,TIOCGETP,&p2);
                p2.sg_flags |= CBREAK;
                ioctl(pd,TIOCSETP,&p2);
                setbuf(pd1,NULL);
/*              setbuf(1,NULL); */

                piperead();
                exit(0);
                }

		/* This is the input process. It reads characters
		   one at a time from the standard input.
		   It also runs the desired Unix/Xenix command.
		   (Has not been tested with running it under a shell)
		*/
        else    {
                pp = popen("runmm 30 > iopipe","w");	/* desired Unix command - the redirection MUST stay */
                putc('1',pp);                   /* select GL's */
                fflush(pp);

                sleep(2);	       /* This sets up the application */
                fprintf(pp,"5\n");     /* to the desired menu. Can be removed */
                fflush(pp);
                sleep(10);

                last = ' ';
                pos = 0;

		/* The following is an infinite loop of reading
		   characters and then checking them.
		   They are stored in a line buffer for later
		   comparison upon receiving a newline character.

		   Change the if statements to "search" or trap
		   for desired characters or words
		*/

                while ( (c=getc(stdin)))
                        {
                        if (c == last && c == TAB)
                                continue;

                        if (c == '\n')
                                {
                                line[pos] = '\0';
                                val = atoi(line);
                                pos = 0;
                                if (strlen(line) == 4 && val < 4000)
                                        {
                                        for (x=0; x<4 ; ++x)
                                                {
                                                putc('\b',pp);
                                                fflush(pp);
                                                }
                                        }
                                else  if (strlen(line) == 3 && val != 300)
                                        {
                                        for (x=0; x<3 ; ++x)
                                                {
                                                putc('\b',pp);
                                                fflush(pp);
                                                }
                                        putc('3',pp);
                                        putc('0',pp);
                                        putc('0',pp);
                                        putc('\n',pp);
                                        fflush(pp);
                                        }
                                else
                                        putc(c,pp);
                                }

			/* This is the exit code and the 'kill' call
			   MUST be retained. It will terminate the
			   other process reading from the pipe.
			   Make sure to output necessary keystrokes to
			   properly terminate the Unix command.
			*/

                        else if (c == QUIT)
                                {
				/* These are the Exit keystrokes of
				   the command being run
				*/
                                for (x=0; x<5; ++x)
                                        {
                                        putc(QUIT,pp);
                                        fflush(pp);
                                        }
                                fclose(pp);
                                kill(pid,SIGTERM);      /* signal child */
                                break;
                                }
                        else if (c == TAB)
                                {
                                putc('  ',pp);
                                fflush(pp);
                                }
                        else
                                {
                                putc(c,pp);
                                line[pos] = c;
                                ++pos;
                                }
                        
                        fflush(pp);
                        last = c;
                        }
                }
        abort();

/*      kill(pid,SIGTERM);      */
}


abort()
{
        kill(pid,SIGTERM);
        params.sg_flags &= ~CBREAK;
        params.sg_flags |= ECHO;
        ioctl(0,TIOCSETP,&params);
        setuid(getuid());		/* This depends on the use of */
					/* C program. Can be removed  */
        exit(0);
}


piperead()
{
	/* Infinite loop of reading characters from the iopipe
	   and outputting them to the standard output.
	*/

        char    c;
        int     cls;

        signal(SIGTERM,pipe_term);	/* Terminate pipe on receipt of */
					/* SIGTERM.  MUST be retained   */

        cls = 0;
        printf("One Moment Please.   Loading General Ledger programs and data.\n");
        printf("\nWill only take a few minutes.");
        while ( cls != 3)
                {
                read(pd,&c,1);
                if (c == '')
                        ++cls;
                }

        printf("%c",c);
        ioctl(1,TIOCFLUSH,&params);

	/* The above lines were for a specific application.
	   It would blank the user's screen and displayed nothing
	   until 3 clear screens were received (the input process
	   was putting the application at a particular menu -
	   all previous menus were denied to the user.
	*/

        while ( (read(pd,&c,1)) == 1)
                {
                printf("%c",c);
                }
}


pipe_term()
{
        /* terminate the whole I/O control process. Do not
           allow the upper GL menus to appear.
        */

        int     cls;
        char    c;

	/*  The while loop was to block certain information
	   from the user while the Unix command was terminating.
	   The Application had to go through upper menus which
	   the user was not allowed to see.
	   This can be modified or removed entirely.
	*/

        cls = 0;
        while ( (read(pd,&c,1) == 1))
                {
                }
        exit(1);
}

