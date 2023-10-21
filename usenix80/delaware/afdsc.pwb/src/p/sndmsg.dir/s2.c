#
#include	"./extern.h"
#include	<error.h>
/*
Name:
	gather

Function:
	Read a single line of console input into the user specified
	buffer.  Standard unix editing is in effect.

Algorithm:
	Read a character:
	If it is an unescaped new line, null terminate the buffer and return.
	Otherwise, stash the character into the buffer.
	If the length of the buffer has been reached, echo a new line and
	return.

Parameters:
	pointer to user buffer of length STRINGLEN

Returns:
	None.

Files and Programs:
	None.


*/
gather(sp)	char	*sp;
{
	register int	c;
	register int	i;
	register char	*p;
	extern  char  getchar();

	p = sp;
	for(i=STRINGLEN+1; --i;)
	{
		c = getchar(0);
		if (c<0 || c=='\n') break;
		else
			*p++ = c=='\\'? slashc(0) : c;
	}
	*p = '\0';
}
/*

Name:
	input

Function:
	To accept the text of the message while doing all of the sexy
	sndmsg type editing.

Algorithm:
	Allocate a buffer of size BUFLEN if more space is needed.
	Read characters (editing sexily as we go)
	Return when we get a ctl-d or ctl-z and l_flag == 1
	Prompt for "Function" (call select) and do-function when we get ctl-d/z and not l_flag
	If l_flag==1 also return when we get newline.

Parameters:
	input file descriptor.
	integer which indicates a letter if it is 0, a destination
			list if 1

Returns:
	Pointer to null terminated input buffer.  "Lttr" is
	set to same value.

Files and Programs:
	/tmp/hsndxxxxx	header text.


*/
input(infd,l_flag)	int infd, l_flag; /* 0 indicates a letter, 1 a destination list */
{
	extern int savlttr;
	extern int ansflag;
	register char	*p;	/* pointer to last character read */
	register int	i;	/* used as a counter */
	register char	*c;	/* miscelanieous pointer and variable */

	char	filename[STRINGLEN];	/* filename typed after control-F */
	int	tmpfd;		/* file descriptor for the temporary file used
				 * for the letter (/tmp/send00000)
				 */
	int	column;		/* current column--not used at present */
	char	*r, eline[132];	/*ptr to and bufferfor escape line*/
	int	firsttime;
	extern	int sigfnc3();
	extern  char   getchar();

	if (l_flag!=1)
	{
		signal (1,&sigfnc3);	/* write text into sndmsg-hup on hang up */
	}
	firsttime = 1;
	/* add column accounting */
	p = lttr = bomem; /* establish the beginning of "expandable" memory */

	/*	rawmode is assumed at this point...	*/
	loop
	{
		c = getchar(infd);
		memchk(p);
		if (p >= eomem)	brk(eomem =+ BUFLEN);
		switch (c)
		{
		case cnt 'a':	/* char delete for terminals without backspace*/
			if (p > lttr)
			{
				puts(1,"\\");		/* echo backslash */
				write(1, --p, 1);	/* then the char */
			}
			continue;

		case cnt 'b':		/* input a file at this point */
		case cnt 'f':
			normal();	/* let the user kill us; save us some overhead also */
			/* the following line comes after the call on normal for
			 * syncrony--the stty() which is inside of normal()
			 * flushes the system's tty buffers
			 */
			puts(1,"\nInput file: ");
			gather(filename);	/* should be recursive call on input */
			if ((i=secure_open(filename,0)) >= 0)
			{
				c = i;	/* use other register (more on this later */
				loop
				{
					memchk(p+BUFLEN);
					if(p+BUFLEN>eomem) brk(eomem =+ BUFLEN);
					if ( (i=read(c,p,BUFLEN)) <= 0 ) break;
					p =+ i;
				}
				close(c);
				rawmode();
				puts(1,filename); puts(1," has been included\n");
			}
			else {
				puts(1,filename); puts(1," cannot be opened\n");
				rawmode();	/* at least there will be syncrony if the file is included */
			}
			continue;

		case cnt 'c':		/* breakout */
			normal();
			if (!firsttime)
			{
				unlink (htempfile);
				unlink (btempfile);
			}
			exit(EBRK);

		case cnt 'y':		/* repetitive file input (not from function level) */
			if(!l_flag) {
				bomem = p;
				return p;
			}
			*p++ = c;
			continue;
		case del:
		case 0/* IOERROR */:
			puts(1,"-BREAK-\n");
		case cnt 'd':		/* normal end of message */
		case cnt 'z':
			if (l_flag)
			{
				*p++ = '\0';	/* terminate string */
				bomem = p; /* points to next available byte */
				return lttr;
			}
			/* Guarrantee msg ends with a new line */
			if (*(p-1) != '\n')
			{
				*p++ = '\n';
				write(1,"\n",1);
			}
			*p = '\0';	/* terminate string */
			ptend = p;	/* for hang-up */
			if(ansflag)
				lttr = savlttr;
			puts(1,"-----");
			/* make the tempfile */
			if (firsttime)
			{
				tmpfile('\0','\0',"/tmp/hsnd00000"); /*write out header to tmpfile*/
				tmpfile(lttr,p,"/tmp/send00000"); /* NOTE: p points to the last byte of the string */
				firsttime = 0;
				puts(1,"\nType h<CR> for help");
			}
			else
			{
				fd.tb = creat(btempfile, 0600);
				write (fd.tb, lttr, p - lttr);
			}
			bomem = ++p; /* points to the next available byte */
			if ((p =  select()) == 0 )	/* allow Nroff, Xed, etc. of letter */
			{
				return lttr;
			}
			else	/*more input desired*/
			{
				ptend = p;	/* for hang-up */
				lttr = bomem;
			}
			continue;

		case cnt 'h': if (flag.erase && p > lttr) puts(1, " \b");

		case '#': if (p > lttr) --p;
			continue;

		case cnt 'r':		/* retype current line */
			normal(); /* so that the user can interrupt a long line */
			for(c = p; c > lttr && *--c != '\n';);
			/* c==lttr or *c=='\n' */
			*p = '\0';
			if (c == lttr) write(1,"\n",1); puts(1,c);
			rawmode();
			continue;

		case cnt 's':		/* retype entire message */
			normal(); /* so that the user can interrupt a long message */
			*p = '\0';
			write(1,"\n",1);
			write(1, lttr, p-lttr);
			rawmode();
			continue;

		case cnt 'w':		/* delete the last word typed */
			puts(1,"_");
			while
			(
			  --p >= lttr && /* back up over control w, too */
				valid(*p) /* !terminator and namestr */
			)
				/*puts(1, flag.erase ? "\b \b" : "\b" )*/;
			while (p>lttr &&
				!valid(*p) /* terminator or !namestr */
			)
			{
				/*puts(1,"\b");*/
				--p;
			}
			*++p = '\0';
			continue;

		case cnt 'x':		/* line delete */
			puts(1,"XXX");

		case '@':
			write(1,"\n",1);
			/* back up over '@', at least */
			while(--p >= lttr && *p != '\n');
			*++p = '\0';	/* terminate the string before the line in order to erase it */
			continue;

		case abortchar: 	/* sigh */ normal(); abort();

		case '\\': *p++ = slashc(infd);
			break;

		case '\n':
			if(l_flag)
			{
				*p++='\0';
				bomem = p; /* points to next available byte */
				return lttr;
			}
			*p++ = c;
			ptend = p;	/* on hang-up, restore back to last cr. */
			continue;

		case '!':
			if (p==lttr || (p!=lttr && *(p-1) =='\n') )
			{
				normal();
				r = &eline[0];
				for (;(*r=getchar()) != '\n' && *r != '\0'; r++);
				*r = 0;
				unix(eline);
				puts(1,"\n");
				rawmode();
				continue;
			}

		default: *p++ = c;
		}
	}
}
/*

Name:
	unix

Function:
	Execute a shell command line.

Algorithm:
	Fork, change to non-super user, execute with any arguments.

Parameters:
	Pointer to command line.

Returns:
	None.

Files and Programs:
	/bin/sh


*/
unix(cmd)
char cmd[];
{
	register	pid, rpid;
	int		retcode;
	int		oldsig;

	oldsig = signal(2, 1);		/* ignore breaks */
	if ( (pid = fork() ) == 0 )
	{
		setuid (myuid);	/* Don't want a super-user running loose! */
		if (cmd[0] != 0)
			execl (SH, "-shell(sndmsg)", "-c", cmd, 0);
		else
			execl (SH, "-shell(sndmsg)", 0);
		exit(ENOEXEC);
	}

	while ( (rpid = wait(&retcode)) != pid && rpid != -1);
	signal(2, oldsig);
	puts(1,"!");
	return;
}
