#
#include	"./extern.h"
#include	<error.h>
/*
Name:
	select
Function:
	To ask for and provide interactive functions.

Algorithm:
	Prompt for 'Function:', input a character, and do that function.

Parameters:
	None.

Returns:
	0 for abort or mail/send
	charactter pointer (p) to continue with input

Files and Programs:
	/tmp/hsndxxxxx
	hold_mail
	/bin/nroff
	/bin/edit
	/sys/msg/help.sndmsg



*/

select()
{
	extern  int sigfnc1(), sigfnc2(), sigfnc5();
	char	c, *p, *fnt;
	int	stat, i, j, append, pid, oldsig2, osig2;
	char	*fny, *fnx;

	normal();
	/*
	 * set up files
	 */
		for (i=0;i<15 && (b2tempfile[i] = btempfile[i]) != '\0'; i++);
		b2tempfile[8] = 't';  /* a different but unique file name*/
		fnx = btempfile;
		fny = &b2tempfile[0];
		close(fd.tb);

	oldsig2 = signal (2,&sigfnc1);
	for (;;)
	{
		signal (2,&sigfnc1);
		setexit();	/* sigfnc1 does a reset to here on Break (del) */
		puts(1,"\nFunction: ");	/* PROMPT */

		gather(bigbuf);
		if (bigbuf[0] == '!')
		{
			unix(&bigbuf[1]);	/* escape to UNIX */
			continue;
		}
		c = bigbuf[0];
		switch ( c )
		{

		case 'a':	/* ABORT */
			setexit ();	/* come back here on break */
			signal (2,&sigfnc1);	/* too late to change your mind now */
			unlink (fnx);
			close (fd.th);
			unlink (htempfile);
			aflag = 1;
			signal (2,oldsig2);
			return (0);

		case 'c':	/* CHANGE (header) */
			close(fd.th);
			unlink(htempfile);
			setuphead();
			tmpfile('\0','\0',"/tmp/hsnd00000");
			puts(1,"- - -\n");
			normal();
			continue;
		case 'p':	/* PRINT header */
			fdy = open(htempfile,2);
			goto prtlab;
		case 'd':	/* DISPLAY body */
			fdy = open (fnx,2);	/* use fdy instead of fdx because of sigfnc2 */
prtlab:
			j = 1;
			osig2 = signal (2,&sigfnc2);	/* stop output and close file on break*/
			puts(1,"- - -\n");
			while ((i = read (fdy, bigbuf, BUFLEN)) > 0 && j > 0)
				j = write (1, bigbuf, i);
			close (fdy);
			puts(1,"-----");
			signal (2,osig2);
			continue;

		case 'w':	/* WRITE */
			signal (2,1);
			fdx = open (fnx, 2);
			if(bigbuf[1] == '\0')
				fny = "hold_mail";
			else {
				p = &bigbuf[1];
				while(*p++ == ' ');
				fny = --p;
			}
			fdy = creat (fny, 0600);
			if (fdy < 0)
			{
				puts(1,fny);
				puts(1,": can't create\n");
				exit (ENOENT);
			}
			chown(fny,myuid);
			seek(fdy,0,2);
			while (i = read(fdx, bigbuf, BUFLEN))	{
				write (fdy, bigbuf, i);
			}
			close (fdx);
			close (fdy);
			continue;

		case 'i':	/* INPUT */
			signal (2,1);	/* temporarilly ignore breaks */
			fdx = open (fnx, 2);
			p = bomem;
			while ((i = read (fdx, bigbuf, BUFLEN)) > 0)
			{
				for (j=0; j<i; j++)
				{
					memchk(p);
					*p++ = bigbuf[j];
				}
				memchk(p);
				*p++ = '\n';	/* no newline on end*/
				if (*p == '\0')	p--;
			}
			close (fdx);
			puts(1,"Input More Text:\n");
			rawmode ();
			signal(2,oldsig2);	/*restore old signal status*/
			return ( p );	/* go back to input() */

		case 'j':	/* JUSTIFY */
			puts(1,"Justifying...");

		case 'n':	/* NROFF */   /* no restrictions like justify has */
			if (c == 'n')	puts(1,"Nroffing...");
			signal (2, 1);
			fdy = creat (fny, 0600);
			chown(fny,myuid);
			fdx = open (fnx,2);
			close (fdx);
			if (fork()==0)
			{
				pipe ( npipe );
				close (0);
				dup ( npipe[0] );	/* make the pipe the std. input */
				if (c == 'j')
				{
					write (npipe[1], ".c2 ~\n.cc ~\n~ec ~\n~~so ",23);
					write (npipe[1], fnx, 14);	/* .so filename of letter */
					write (npipe[1], "\n~~pl 1v\n",9);	/* inhibit blank lines at end of page*/
				}
				else	/* nroff */
				{
					write (npipe[1], ".so ",4);
					write (npipe[1], fnx, 14);	/* .so filename of letter */
					write (npipe[1], "\n.pl 1v\n",8);	/* inhibit blank lines at end of page*/
				}
				close (1);
				close (npipe[1]);
				dup (fdy);
				execl (NROFF,"nroff(sndmsg)",0);
				write (2, "Can't find nroff!\n", 18);
				exit (ENOEXEC);
			}
			wait (&stat);
			if (((stat & 0177400) >> 8) != -1)  /* if child didn't "exit(-1)" */
			{
				close (fdx);
				unlink (fnx);
				fnt = fnx;
				fnx = fny;
				fny = fnt;	/* switch names */
			}
			close (fdy);
			signal (2, &sigfnc1);	/* restore old signal value */
			continue;

		case 'r':	/* RECEIPT */
			puts(1,"Receipts will be sent by all recipients.");
			receiptall = 1;
			continue;

		case 's':	/* SEND */	/* must immediatly preceed 'mail' */
			repeat = 0;	/* exit after sending */

		case 'm':	/* MAIL */
			fdx = open(fnx, 2);
			seek (fdx, 0, 2);	/*seek end of file */
			write (fdx, "-----\n", 7);
			close (fdx);

			btempfile = fnx;
			fd.tb = open (btempfile,2);
			rawmode();
			signal(2,oldsig2);	/*restore old signal status*/
			return (0);

		case 'e':	/* XED */
			osig2 = signal (2, 1);
			fdx = open (fnx, 2); close (fdx);
			pipe (npipe);
			if ( fork() == 0 )
			{
				close (0);
				dup (npipe[0]);
				setuid(myuid);
				execl (EDIT, "edit(sndmsg)",fnx, 0);
				puts(1,"Can't find editor!\n");
				exit (ENOEXEC);
			}

			signal (2, &sigfnc5);	/* send editor a del on break */
			setexit ();	/* Drop down to command level on break */
			append = 0;	/* append mode flag */
			for (;;)	/* monitor commands to editor for 'q' */
			{
				i = read (0, bigbuf, BUFLEN);
				if (i==0)
				{
					i = 2;
					if (append)	bigbuf[0] = '.';
						else	bigbuf[0] = 'q';
					bigbuf[1] = '\n';
				}
				p = &bigbuf[0];
				while (*p == ' ' || *p == '\t')	p++;
				if (*p == '!' && append != 1)
				{
					puts(1,"Sorry, escapes are not allowed from sndmsg editor!\n* ");
					continue;
				}
				signal (2,1);
				append = parsecmd (p, append);	/* untangle ed cmd from addresses, etc. */
				signal (2, &sigfnc5);
				if (append == -1)	/*i.e. if 'q\n' and not append mode*/
				{
					/* write out file and exit */
					write (npipe[1], "w ", 2);
					write (npipe[1], fnx, 14);
					write (npipe[1], "\nq\n",3);
					signal (2,1);
					wait ();	/* let editor die before prompting "Function" */
					break;
				}
				if (append == -2)	/* o or y command */
				{
					puts(1,"Sorry, raw mode commands such as 'o' and 'y' are not allowed in sndmsg editor!\n");
					append = 0;
					continue;
				}
				write (npipe[1], bigbuf, i);	/* pipe command to editor */
			}
			close (npipe[0]);
			close (npipe[1]);
			signal (2, osig2);
			continue;

		case 'f':	/* FUTURE mail */
			quepost();	/* must precede 'q' */
		case 'q':	/* QUIT */
			qflag = 1;
			close(fd.th);
			unlink(fnx);
			unlink(htempfile);
			return(0);

		case 'h':
			osig2 = signal (2,&sigfnc2);
			fdy = open ("/sys/msg/help.sndmsg",0);
			j = 1;	/* for signal catching */
			while ((i=read(fdy, bigbuf, BUFLEN)) >0 && j > 0)	j = write (1,bigbuf,i);
			close (fdy);
			signal (2, osig2);
			continue;

		default:
			puts(1,"For help type h");
		}
	}   /* now prompt again, etc., till send */
}

/*

Name:
	parsecmd

Function:
	Determine new mode of editor from input command line.

Algorithm:
	Using current mode, look for appropriate editor commands
	or end of build mode.  Skip any leading addresses to get to
	command.

Parameters:
	Pointer to command line
	current editor mode (cmd or build).

Returns:
	-1 = quitting editor
	-2 = illegal command
	0 = command mode
	1 = build mode

Files and Programs:
	None.


*/
parsecmd (buf, appndflg)	char *buf;	int appndflg;
{
#define	ADDRESS	*p<'9'&&*p>'0'||*p=='$'||*p=='.'
	register char *p;
	p = buf;

	if (!appndflg)
	{
		/* determine if append, input or change command */
		if (ADDRESS)	/* skip over address arguments */
		{
			while (ADDRESS)	p++;
			if (*p == ',')
			{
				p++;
				while (ADDRESS)	p++;
			}
		}
		if ((*p == 'q' && p[1] == '\n') || *p == '\0')
			return (-1);	/* write out file and exit */
		if (( *p == 'a' || *p == 'i' || *p == 'c') && p[1] == '\n')
			return (1);	/* append mode */
		if ( *p == 'o' || *p == 'y' )
			return (-2);	/* open line command */
		return (0);	/* still not append mode */
	}
	/* determine if end of append mode */
	if (*p == '\0' || (*p == '.' && p[1] == '\n'))
		return (0);	/* end append mode */
	return (1);	/* still append mode */
}
/*

Name:
	normal

Function:
	put the console into normal mode .

Algorithm:
	Use previously saved tty setting for stty system call.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
normal()
{
	ttys[2] = oldmode;
	stty(0,ttys);
}
/*

Name:
	rawmode

Function:
	Put the terminal into raw mode.

Algorithm:
	Save tty setting, set raw mode, issue stty system call.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/

rawmode()
{

	oldmode = ttys[2];
	ttys[2] =| 040; /* raw mode */
	stty(0,ttys);
}
