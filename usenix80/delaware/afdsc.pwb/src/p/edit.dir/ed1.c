#
/*
Name:
	EDIT - UNIX Text Editor

Function:
	For each command line file, read in contents and commands.
	Go on to next file when quit command is input.
	If no command line file is specified, just accept commands.

Algorithm:
	COMMAND DEPTH LEVELS:
	Depth		Description
	  0		Initial command level.
	  1		Executing a global command from level 0.
	  2		Executing commands from an x-file.
	  3		Executing a global command from level 2.

Parameters:
	-g xxx	the file xxx will be used as input commands and will be applied
		to all specified files to be edited.
	-p xxx	the prompt xxx will be used instead of "*".
	-s	do not prompt (silent running).
	files	text files on which editting is to be done.

Returns:
	EFLAG	Illegal flags

Files and Programs:
	etmp.exxxxx	temporary file (index)
	etmp.Exxxxx	temporary file (text)

Installation Instructions:

History:
 * Editor
 * Copyright 1974, Bell Telephone Laboratories, Incorporated
 * Modifications by UCB (the o command)
 * Modifications by UCLA (the b,j, & z commands; miscellaneous improvements)
 * Modifications by U of Illinois misc improvements
 * Modifications by AFDSC (the -g flag; the x command; the multiple file input)
 * Modifications by AFDSC (the -p and -s flags; the y cmd (from EM); the short
 *   a, c, and i cmds (from IBM Script); the b cmd (from M Pearson).

*/
#include	"./globals.h"
#include 	<error.h>
#define	nextarg		{++argv; --argc;}		/* function to move to next argument */



main(argc, argv)
int argc;
char **argv;
{
	int n;
	char *a2;
	register char *p1, *p2;
	register pid;
	extern int onintr();
	extern int getpid();
	extern int rmfiles();

	delimit = '/';
	expbuf[0] = '\0';
	rhsbuf[0] = '\0';

	stackptr = &deepstk[0];
	onquit = signal(SIGQUIT, 1);
	signal(SIGHUP,onhup);
	mktemp(hup);
	if(gtty(0,ttmode) != -1) {
		istty++;
		ttnorm = ttmode[2];
		}
	margin = LBSIZE - 40;
	prompt = "* ";				/* set default prompt */
	/* COMMAND LINE ARGUMENT GATHERING SECTION */
	nextarg;			/* skip over the arg with command name (arg 0)*/
	while (argc > 0 && **argv == '-')		/* while there are arguments, and */
							/* they start with a dash */
	{
		for(;;) {				/* until end of string is detected */
			switch (*++*argv)		/* interpret each flag letter */
			{
			/* global command file */
			case 'g':
				nextarg;
				globfile = *argv;		/* pt to cmd file */
				gflag++;
				goto ugh_a_goto;
			/* prompt character */
			case 'p':
				nextarg;
				prompt = *argv;
				goto ugh_a_goto;
			/* quit on 'quit' signal */
			case 'q':
				signal(SIGQUIT, 0);
				break;
			/* suppress prompts */
			case 's': 
				vflag=0;
				break;
			default:
				write(2,"Illegal flag\n",13);
				exit(EFLAG);
			case '\0':			/* end of string, go on to next argument */
				goto ugh_a_goto;
			}
		}
		ugh_a_goto:
		nextarg;
	}
	tfname = "etmp.exxxxx";
	pid = getpid();
	for (p1 = &tfname[11]; p1 > &tfname[6];) {
		*--p1 = (pid%10) + '0';
		pid =/ 10;
	}
	if ((signal(SIGINTR, 1) & 01) == 0)
		signal(SIGINTR, onintr);
	if(argc == 0) {				/* allow at least one pass */
		argc = 1;
		start--;
	}
	while(argc) {				/* for each file to be edited */
		init();
		start++;
		if(gflag) {
			close(0);
			open(globfile,2);		/* get new std input */
		}
		if(!start)	argc--;
		setexit();
		/* All Errors return to this point via reset() */
		if(start) {
			/* read in input; has to be after setexit() */
			start = 0;
			p1 = *argv;
			p2 = savedfile;
			while(*p2++ = *p1++);
			p1 = *argv;
			p2 = file;
			while (*p2++ = *p1++);
			argv++;
			argc--;
			readfile(zero);
			fchanged = 0;
		}
		push(&curdepth);
		commands(0);
		pop(&curdepth);
		rmfiles();
	}
}

/*

Name:
	commands

Function:
	set up addressed lines and interpret commands.

Algorithm:
	Parse the command line for address, command, and operands.
	Call appropriate command routine.

Parameters:
	Depth at which commands are being input (global in or out of 'next' file).

Returns:
	None.

Files and Programs:
	None.


*/
commands(depth)
	int   depth;
{

	extern int   gettty(), getfile();
	register *a1, c;
	register char *p;
	int r;
	int hfd, n;
	char *a2;
	curdepth = depth;

	for (;;) {
	if (pflag) {
		pflag = 0;
		addr1 = addr2 = dot;
		printlines();
	}
	if(vflag && !globp) {
		a2 = prompt;
		while(*a2)
			write(1,a2++,1);
	}

	addr1 = addr2 = address();		/* get first (possibly only) address */
	c = getchar();
	if (addr1) {
		if (c==';') {		/* get second */
			dot = addr1;
			addr2 = address();
			c = getchar();
			}
		else if (c==',') {		/* get second */
			addr2 = address();
			c = getchar();
			}
		}

	switch(c) {

						/* APPEND */
						/*
						long form:
							.a
							text
							.
						short forms:
							.a newtext
							.a ' newtext with lead blank
							.a " newtext with lead blank
						*/
	case 'a':
	case 'A':
		setdot();
		setmode();
		append(gettty, addr2);
		continue;

						/* BLOCK SCREEN MODE */
						/*
						.b
						*/
	case 'b':
	case 'B':
		setdot();
		edx();
		write(1,"\n",1);
		continue;

						/* CHANGE */
						/*
						long form:
							.,.c
							newtext
							.
						short forms:
							.,.c newtext
							.,.c ' newtext with lead blank
							.,.c " newtext with lead blank
						*/
	case 'c':
	case 'C':
		setdot();
		setmode();
		nonzero();
		delete();
		append(gettty, addr1-1);
		continue;

						/* DELETE */
						/*
						.,.d
						*/
	case 'd':
	case 'D':
		setdot();
		newline();
		nonzero();
		delete();
		continue;

						/* EDIT NEW FILE */
						/*
						e filename
						*/
	case 'e':
	case 'E':
		setnoaddr();
		if ((peekc = getchar()) != ' ')
			ERROR;
		if (fchanged) dontgo();
		savedfile[0] = 0;
		whichfile = savedfile;
		filename();
		init();
		readfile(zero);
		fchanged = 0;
		continue;

						/* FILE NAME */
						/*
						f filename
						f
						*/
	case 'f':
	case 'F':
		setnoaddr();
		if ((c = getchar()) != '\n') {
			peekc = c;
			savedfile[0] = 0;
			whichfile = savedfile;
			filename();
		}
		puts(savedfile);
		continue;

						/* GLOBAL */
						/*
						.,.g/expr/cmd
						.,.g/expr/cmd\
						cmd
						*/
	case 'g':
	case 'G':
		global(1);
		continue;
						/* HELP */
						/*
						h
						h cmd
						*/
	case 'h':
	case 'H':
		if((c = getchar()) == ' ')
		{
			while((c = getchar()) == ' ');
			peekc = c;
			help();
			continue;
		}
		else
		{
			peekc = c;
			newline();
		}
		if((hfd = open("/sys/msg/help.edit",0)) < 0) {
			write(2,"\nhelp file not available\n",27);
			continue;
		}
		while(n = read(hfd,linebuf,512))
			write(1,&linebuf,n);
		close(hfd);
		continue;

						/* INSERT */
						/*
						long form:
							.i
							newtext
							.
						short forms:
							.i newtext
							.i ' newtext with lead blank
							.i " newtext with lead blank
						*/
	case 'i':
	case 'I':
		setdot();
		nonzero();
		setmode();
		append(gettty, addr2-1);
		continue;

						/* JOIN */
						/*
						.,.j
						.,.j between text
						.,.j ' between text with lead blank
						.,.j " between text with lead blank
						*/
	case 'j':
	case 'J':
		setmode();
		join();
		continue;

						/* MARK */
						/*
						.kx
						*/
	case 'k':
	case 'K':
		if ((c = getchar()) < 'a' || c > 'z')
			ERROR;
		setdot();
		nonzero();
		newline();
		names[c-'a'] = *addr2 | 01;
		continue;

						/* MOVE */
						/*
						.,.m.
						*/
	case 'm':
	case 'M':
		move(0);
		continue;

						/* NUMBER PRINT */
						/*
						.,.n
						*/
	case 'n':
	case 'N':
		listf = 2;
		goto print;

						/* PRINT NEXT */
						/*
						\n
						*/
	case '\n':
		if (addr2==0)
			addr2 = dot+1;
		addr1 = addr2;
		printlines();
		continue;

						/* LIST CONTROL CHARACTER PRINT */
						/*
						.,.l
						*/
	case 'l':
	case 'L':
		listf = 1;
						/* PRINT */
						/*
						.,.p
						*/
	print:
	case 'p':
	case 'P':
		newline();
		setdot();
		printlines();
		continue;

						/* OPEN */
						/*
						.,.o
						.,.o open cmds
						.,.o ' open cmds with lead blank
						.,.o " open cmds with lead blank
						*/
	case 'o':
	case 'O':
		setdot();
		nonzero();
		if(setmode()) {
			imedmode = nostty = 0;
			ilsubst();
		}
		else {
			ttmode[2] =| 040;
			ttmode[2] =& ~010;
			stty(0,ttmode);
			write (1, "", 1);	/* put out a bell */
			ilsubst();
			ttmode[2] = ttnorm;
			stty(0,ttmode);
		}
		continue;

						/* QUIT */
						/*
						q
						*/
	case 'q':
	case 'Q':
		setnoaddr();
		if (fchanged) dontgo();
		newline();
		return;

						/* END OF FILE */
	case EOF:		/* I put this here JSK */
		switch(curdepth) {		/* see who's at eof */
		case 1:			/* global changes are done */
		case 3:
			return;
		case 2:			/* next file is done */
			close(0);		/* free std input */
			dup(savin);		/* restore old input */
			close(savin);		/* free descriptor */
			return;
		}
		setnoaddr();
		if (fchanged) {
			dontgo ();
		}
		return;

						/* READ FILE */
						/*
						.r filename
						*/
	case 'r':
	case 'R':
	caseread:
		whichfile = savedfile;
		filename();
		setall();
		readfile(addr2);
		continue;

						/* SUBSTITUTE */
						/*
						.,.s/expr/string/
						.,.s4/expr/string/
						.,.s/expr/string/g
						.,.s/expr/string/p
						.,.s[1-9]/expr/string/[g][npl]
						*/
	case 's':
	case 'S':
		setdot();
		nonzero();
		substitute(globp);
		continue;

						/* TRANSFER (COPY) */
						/*
						.,.t.
						*/
	case 't':
	case 'T':
		move(1);
		continue;
						/* UN-COMMAND (ECHO) */
						/*
						u text
						u ' text with lead blank
						u " text with lead blank
						*/

	case 'U':
	case 'u':
		uncmd();
		continue;

						/* NEGATIVE GLOBAL */
						/*
						.,.v/expr/cmd
						.,.v/expr/cmd\
						cmd
						*/
	case 'v':
	case 'V':
		global(0);
		continue;

						/* WRITE */
						/*
						.,.w
						.,.w filename
						*/
	case 'w':
	case 'W':
		setall();
		nonzero();
		whichfile = savedfile;
		filename();
		io = creat(file,0664);
		if (io < 0) errfunc("Can't create");
		seek(io,0,2);	/* go to end of file */
		putfile();
		exfile();
		fchanged = 0;
		continue;

						/* NEXT COMMAND FILE */
						/*
						x
						x filename
						*/
	case 'x':		/* input from 'next' file */
	case 'X':
		if((c = getchar()) != '\n')
		{
			nextfile[0] = 0;
		}
		peekc = c;
		whichfile = nextfile;
		filename();		/* parse file name */
		next();			/* operate on it */
		continue;

						/* AUTO CR MODE */
						/*
						y
						y+
						y-
						*/
	case 'y':
	case 'Y':
		setmargin();
		newline();
		continue;

						/* PAGE PRINT */
						/*
						.z
						.z+
						.z.
						.z-
						.z[nlp]
						*/
	case 'z':
	case 'Z':
		getzlen();
		zflag++;
		printlines();
		zflag = pflag = 0;
		continue;

						/* LINE NUMBER */
						/*
						.=
						*/
	case '=':
		setall();
		newline();
		count[1] = (addr2-zero)&077777;
		putd(&count);
		putchar('\n');
		continue;

						/* UNIX COMMAND */
						/*
						!
						!unix cmd
						*/
	case '!':
		unix();
		continue;

						/* COMMENT */
						/*
						: commentary text
						*/
	case ':':
		while(getchar() != '\n');
		continue;
	}
	ERROR;
	}
}

/*

Name:
	push

Function:
	Save depth of command execution for later restoration.

Algorithm:
	Put depth on stack or give overflow message.

Parameters:
	int pointer to depth

Returns:
	None or overflow reset

Files and Programs:
	None.


*/
push(d)
int *d;
{
	if(stackptr <= &deepstk[9])  {
		*stackptr++ = *d;
	}
	else	errfunc("Stack overflow");
}

/*

Name:
	pop

Function:
	Restore a saved depth level.

Algorithm:
	Remove depth of command from stack and put in user location.

Parameters:
	int pointer to depth location

Returns:
	None or underflow reset

Files and Programs:
	None.


*/
pop(pt)
int *pt;
{
	if(--stackptr >= &deepstk[0])  {
		*pt = *stackptr;
	}
	else	errfunc("Stack underflow");
}
/*

Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


*/
setmode() {
	char c;

	c = getchar();
	if(afterprt(c)) {
		c = getchar();
	}
	if(c == ' ') {			/* immediate input */
		imedmode++;
		nostty++;
		while((c = getchar()) == ' ');
		if(c != '"' && c != '\'')
			peekc = c;
		return(1);
	}
	if(c == '\n') {			/* normal input */
		return(0);
	}
	else				/* bad format */
		ERROR;
}
/*

Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


*/
afterprt(c)
char c;
{
	if((c == 'l') || (c == 'L'))
		listf = 1;
	else {
		if((c == 'p') || (c == 'P'));
		else {
			if((c == 'n') || (c == 'N'))
				listf = 2;
			else
				return(0);
		}
	}
	pflag++;
	return(1);
}
/*

Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


*/
newline() {
	if(setmode())
		ERROR;
}
