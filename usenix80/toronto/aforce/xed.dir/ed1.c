#
#include	"/doc/source/prog/s1.afdsc/xed.dir/globals.h"
/*
 * Editor
 * Copyright 1974, Bell Telephone Laboratories, Incorporated
 * Modifications by UCB (the o command)
 * Modifications by UCLA (the b,j, & z commands; miscellaneous improvements)
 * Modifications by U of Illinois misc improvements
 * Modifications by AFDSC (the -g flag; the x command; the multiple file input)
 */


/* COMMAND DEPTH LEVELS:
	Depth		Description
	  0		Initial command level.
	  1		Executing a global command from level 0.
	  2		Executing commands from an x-file.
	  3		Executing a global command from level 2.
*/

main(argc, argv)
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
	argv++;
	argc--;
	while (argc > 1 && **argv=='-') {
		/* allow debugging quits? */
		switch ((*argv)[1]) {
		/* global command file */
		case 'g':
			argv++;
			argc--;
			globfile = *argv;		/* pt to cmd file */
			gflag++;
			break;
		/* prompt character */
		case 'p':
			argv++;
			argc--;
			prompt = *argv;
			break;
		/* quit on 'quit' signal */
		case 'q':
			signal(SIGQUIT, 0);
			break;
		/* suppress prompts */
		case 0: 
			vflag=0;
			break;
		}
		argv++;
		argc--;
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

	addr1 = addr2 = address();
	c = getchar();
	if (addr1) {
		if (c==';') {
			dot = addr1;
			addr2 = address();
			c = getchar();
			}
		else if (c==',') {
			addr2 = address();
			c = getchar();
			}
		}

	switch(c) {

	case 'a':
	case 'A':
		setdot();
		if((c = getchar()) == ' ') {
			imedmode++;
		}
		else {
			peekc = c;
			newline();
		}
		append(gettty, addr2);
		continue;

	case 'c':
	case 'C':
		setdot();
		if((c = getchar()) == ' ') {
			imedmode++;
		}
		else {
			peekc = c;
			newline();
		}
		nonzero();
		delete();
		append(gettty, addr1-1);
		continue;

	case 'd':
	case 'D':
		setdot();
		newline();
		nonzero();
		delete();
		continue;

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

	case 'g':
	case 'G':
		global(1);
		continue;
	case 'h':
	case 'H':
		if((c = getchar()) == ' ')
		{
			help();
			continue;
		}
		else
		{
			peekc = c;
			newline();
		}
		if((hfd = open("/etc/msg/edhelp1",0)) < 0) {
			write(2,"\nhelp file 1 not available\n",27);
			continue;
		}
		while(n = read(hfd,linebuf,512))
			write(1,&linebuf,n);
		close(hfd);
		continue;

	case 'i':
	case 'I':
		setdot();
		nonzero();
		if((c = getchar()) == ' ') {
			imedmode++;
		}
		else {
			peekc = c;
			newline();
		}
		append(gettty, addr2-1);
		continue;

	case 'j':
	case 'J':
		setdot();
		nonzero();
		newline();
		join();
		continue;

	case 'k':
	case 'K':
		if ((c = getchar()) < 'a' || c > 'z')
			ERROR;
		setdot();
		nonzero();
		newline();
		names[c-'a'] = *addr2 | 01;
		continue;

	case 'm':
	case 'M':
		move(0);
		continue;

	case 'n':
	case 'N':
		listf = 2;
		goto print;

	case '\n':
		if (addr2==0)
			addr2 = dot+1;
		addr1 = addr2;
		printlines();
		continue;

	case 'l':
	case 'L':
		listf = 1;
	print:
	case 'p':
	case 'P':
		newline();
		setdot();
		printlines();
		continue;

	case 'o':
	case 'O':
		setdot();
		nonzero();
		if ((c=getchar()) != '\n')
		{
			peekc = c;
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

	case 'q':
	case 'Q':
		setnoaddr();
		if (fchanged) dontgo();
		newline();
		return;

	case EOF:		/* I put this here JSK */
		switch(curdepth) {		/* see who's at eof */
		case 1:			/* global changes are done */
		case 3:
			return;
		case 2:			/* next file is done */
			close(0);		/* free std input */
			dup(savin);		/* restore old input */
			close(savin);		/* free descriptor */
			vflag = savvflag;	/* restore prompt status */
			return;
		}
		setnoaddr();
		if (fchanged) {
			dontgo ();
		}
		return;

	case 'r':
	case 'R':
	caseread:
		whichfile = savedfile;
		filename();
		setall();
		readfile(addr2);
		continue;

	case 's':
	case 'S':
		setdot();
		nonzero();
		substitute(globp);
		continue;

	case 't':
	case 'T':
		move(1);
		continue;

	case 'v':
	case 'V':
		global(0);
		continue;

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

	case 'y':
	case 'Y':
		setmargin();
		newline();
		continue;

	case 'z':
	case 'Z':
		getzlen();
		zflag++;
		printlines();
		zflag = pflag = 0;
		continue;

	case '=':
		setall();
		newline();
		count[1] = (addr2-zero)&077777;
		putd(&count);
		putchar('\n');
		continue;

	case '!':
		unix();
		continue;

	}
	ERROR;
	}
}

push(d)
int *d;
{
	if(stackptr <= &deepstk[9])  {
		*stackptr++ = *d;
	}
	else	errfunc("Stack overflow");
}

pop(pt)
int *pt;
{
	if(--stackptr >= &deepstk[0])  {
		*pt = *stackptr;
	}
	else	errfunc("Stack underflow");
}
