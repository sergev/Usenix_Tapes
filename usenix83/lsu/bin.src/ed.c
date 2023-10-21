#

/*
 *
 *		J H U / U N I X   T E X T   E D I T O R
 *
 *
 * Creation history
 *
 * Butler Lampson, "QED" (quick editor) on UCB project GENIE - initial
 * design.
 *
 * K. Thompson, Multics QED editor was similar to this one in almost all
 * respects
 *
 * PDP-11 versions on Unix (BTL) in assembly language.
 * Sixth edition had this (C) implementation.
 *
 *
 *
 *			R e v i s i o n
 *			 H i s t o r y
 *
 *	--Date-- -Who-		--Content--
 *
 *	10/01/75 J.D.	Prompt added for conformity with old
 *			(5th edition) editor. Interrupt and
 *			temporary file bug fixes.
 *
 *
 *	06/29/76 M.M.	Essentially all of the internal documentation
 *			and commentary was written at this time.
 *
 *	07/01/76 M.M.	New commands 'n' and 'h', numbered and
 *        		high-speed print; improvement of 'l'
 *      		command; addition of write/quit interlock.
 *			Invocation change to fake an 'e' instead of
 *			an 'r' command.
 *			Command parser (commands()) modified for changed
 *			command syntax.
 *			Output routine (putchar) modified for above
 *			reasons.
 *
 *	07/04/76 M.M.	x (execute) command and associated mechanisms
 *       		for input diversion. Fix to 'k' command so
 *			that changed lines do not destroy marks.
 *			Routine 'doappend' split off so that it
 * 			could be shared by others.
 *
 *
 *	07/08/76 M.M.	Routine 'ucompile' added for compiling
 *			'expbuf' without regular expression
 *			interpretation.
 *
 *
 *	07/10/76 M.M.	u (substitute without regular expression
 *			interpretation, j (join), < and > (upper
 *			and lower range limit) commands added.
 *			Routine 'substitute' completely rewritten
 *			for this purpose. Routine 'compsub' greatly
 *			modified.
 *
 *	07/26/76 J.D.	Code cleanups and addition of expanded
 *			address syntax.
 *			Revision of address interpretation in commands()
 *			routine and excision of some code inserted in
 *			earlier Hopkins revisions.
 *			Format revision of internal documentation
 *			and commentary.
 *
 *	08/23/76 M.M.	Minor bugs fixed in 'join', '<', and '>' commands.
 *
 *	08/29/76 M.M.	Minor bug fixed in "getchar()" concerning
 *			'x' command.
 *
 *	10/14/76 M.M.	'Setnoaddr()' call added to 'P' command.
 *			'k' and 'd' command cosmetic patches applied.
 *
 *	11/11/76 M.M.	Fixes to 'errfunc()' and 'delete()' [cosmetic]
 *
 *	01/19/77 M.M.	Patch to 'decimal' to allow ",$" command
 *			'e' interlock improved.
 *
 *	06/28/79 MJM	Various fixes:
 *			1)  Change of invocation code to set promp.
 *			2)  Cleanup of error() processing & calling.
 *			3)  Input prompting if 'promp' set.
 *			4)  Addition of PERROR(III) on errors.
 *			5)  Changed getfile() to end input line
 *			    when buffer fills (for those who sometimes
 *			    edit binary files).
 *
 *	06/29/79 MJM	More changes:
 *			1)  Conversion of block refs to FILEADDR & FILEPNTR
 *			2)  NL terminated strings in temp file
 *			3)  All calls to ttyn() replaced by gtty()
 *			    calls.  Usage was to determine if fd0
 *			    was a tty.
 *			4)  New temp file addressing scheme,
 *			    permits much larger files to be edited.
 *			5)  Fixed problem w/marking first line (GAH memorial)
 *
 *	07/01/79 MJM	More changes:
 *			1)  Fixed problem with 'n' cmnd introduced yesterday.
 *			2)  Cleaned up 'l' command's output splitting.
 *			    (GAH memorial)
 *			3)  'j' command with one address joins line
 *			    with PRECEEDING line, if present.
 *			4)  Fixed so 'x' command input would not
 *			    get a prompt for each line.  (GAH memorial)
 *			5)  Cleared output buffer before each use, so that
 *			    temp file is useful (for crash recovery, etc).
 *
 *	26-Oct-79 RNJ	Minor changes for V7 C compatibility.
 *
 *	14-Jan-82 SDK	Changed delete() so that dot set properly to
 *			next line in file rather than +1.
 *
 */

#define FILEADDR  unsigned int
#define FILEPNTR  unsigned int *
#define BYTEMASK 0177
#define EOR '\n'	/* End of record marker in temp file */
#define OBUFSIZE 80	/* size of buffer for terminal output */

/*
 *		F I L E   A D D R E S S   L A Y O U T
 *
 *
 *	|--------------------------------|
 *	|  10 bits  |  5 bits  |  1 bit  |
 *	|--------------------------------|
 *	    |          |          |
 *	    |          |          |___ Marker, for global commands
 *	    |          |
 *	    |          |__ Addr of 16 byte cluster in block
 *	    |
 *	    |_____ Block number in temporary file (0 - 1022)
 *
 */
#define MARKFLAG  01
#define TMP_BLOCK(x)	( ((x)>>6) & 01777 )
#define TMP_BYTES(x)	( ((x)<<3) & 0760 )
#define BYTES_TMP(x)	( (FILEADDR) ((((x)+15)>>3)&077776) )
#define NEXT_TMP(x)	( (FILEADDR) (((x) & ~077) + BINCR) )
#define OINCR		02		/* offset field increment */
#define BINCR		0100		/* block field increment */
#define MAXTEMP		3000		/* Max size of temp file */

/* Signals */
#define	SIGHUP	1
#define	SIGINTR	2
#define	SIGQUIT	3

/* Standard file descriptors */
#define STDINPUT 0
#define STDOUTPUT 1

/* Size declarations */
#define	FNSIZE	64	/* maximum file rname size */
#define	LBSIZE	512	/* Line buffer size */
#define	ESIZE	128	/* regular expression expansion buffer size */
#define GBSIZE	256
#define	NBRA	5	/* number of bracketed "\( \)" fields allowed */

#define FMODE	0644	/* file creation mode */
#define	EOF	-1	/* end-of-file indicator */

/*
 * Various IDs for regular expressions
 * expansion
 */
#define	CBRA	1
#define	CCHR	2
#define	CDOT	4
#define	CCL	6
#define	NCCL	8
#define	CDOL	10
#define	CEOF	11
#define	CKET	12

#define	STAR	01

#define	READ	0
#define	WRITE	1

#define WRITTEN 1
#define NOTWRITTEN	0
#define CASEMAP	04

int	peekc;	/* if nonzero, getchar() returns this character rather
		 * than reading a new one */
int	lastc;	/* last character returned by 'getchar()' */

char	savedfile[FNSIZE];
char	file[FNSIZE];



/*
 *			B U F F E R S
 *
 * 'linebuf' is the buffer for the line of text
 * being operated on.
 *
 * 'rhsbuf' is the buffer for the replacement string in
 * a substitute command
 *
 * 'expbuf' is the expanded form of the first string in
 * a substitute command
 *
 * 'genbuf' is the actual substitution work area.
 */
char	linebuf[LBSIZE];
char	rhsbuf[LBSIZE/2];
char	expbuf[ESIZE+4];
char	genbuf[LBSIZE];

/* The following three variables have the core addresses of the
 * disk locations of the
 * first, current, and last lines of text.
 */
FILEPNTR zero;
FILEPNTR dot;
FILEPNTR dol;

/*
 * The following two pointers contain the core addresses of
 * the disk pointers of the first and last lines to be acted
 * on by a command.
 */
FILEPNTR addr1;
FILEPNTR addr2;

int	*endcore;
int	*fendcore;
long	numbuf;			/* Number buffer for putd() */
char	*nextip;	/* Save area for getfile() */
char	*linebp;	/* line break point: getsub() breaking lines */
int	ninbuf;		/* # bytes left in genbuf */
int	io;		/* file I/O file descriptor */
int	pflag;		/* 'print the results' flag */
int	onhup;
int	onquit;
int	vflag = 1;	/* Verbose flag -- report # chars read/written */
int	promp = 0;	/* Prompt flag.  Print star for command */
int	circfl;
int	listf;
int	col;
char	*globp;		/* address of forced input string, or zero if none */

/*
 * Temporary file information
 */
char	*tfname;		/* temp file name */
int	tfile = -1;		/* temp file filedes */
FILEADDR tline;		/* disk addr of next availible space
			 * (bug:  always at end of file) */

/*
 * After 'execute()', loc1 and loc2 contain addresses which
 * delimit the string to be worked on in the 'substitute' command. 
 */
char	*loc1;
char	*loc2;
char	*locs;

char	*cmdprod "*";	/* command prompt character */
char	*txtprod ">";	/* Text prompt character */

/*
 * I/O buffering
 */
char	ibuff[512];	/* input block buffer */
int	iblock	-1;	/* block number of block in 'ibuff' */
char	obuff[512];	/* output block buffer */
int	oblock	-1;	/* output block buffer */
int	ichanged;	/* block number of block in 'obuff' */
int	nleft;		/* number of bytes left in current buffer before
			 * next block must be fetched */

FILEADDR names[26];	/* 'MARK' pointers */

/*
 * Start & End pointers for bracketed expressions "\(...\)"
 */
char	*braslist[NBRA];
char	*braelist[NBRA];

int	write_sw = 1;	/* 0=file not written */
int	ttystatus[3];	/* return from gtty */
int	casemap 0;	/* 1 means case mapping in effect */
int	lastput;	/* last char put out (for 'h' opt) */
int	listn;		/* numbered print */
int	listh;		/* high speed print */
int	digits;		/* number of digits in largest line number */
int	diversion  0;	/* input diversion level */
int	xfile[8];	/* diversion file descriptors */
int	docnt;		/* number of substitutions to make per line */
int	urange 0;	/* # of lines before current one */
int	lrange 22;	/* # lines after current one to be printed */

/*
 *			M A I N
 *
 */
main(argc, argv)
char **argv;
{
	register char *p1, *p2;
	extern int onintr();
	extern int hangup();

	onquit = signal(SIGQUIT, 1);
	onhup = signal(SIGHUP, 1);

	/* If invoked with name '-', then prompt, etc */
	if( argv[0][0] == '-' )  {
		puts("JHU Text Editor");
		promp++;
	}
	argv++;
	if (argc > 1 && **argv=='-') {
		vflag = 0;
		/* allow debugging quits? */
		if ((*argv)[1]=='q') {
			signal(SIGQUIT, 0);
			vflag++;
		}
		argv++;
		argc--;
	}

	fendcore = sbrk(0);	

	if (argc>1) {
		/* This code fakes out an 'e filename' rather than
		 * a 'r filename' like the original.
		 */
		p1 = *argv;
		p2 = savedfile;
		*p2++ = 'e';
		*p2++ = ' ';
		while (*p2++ = *p1++);
		*p2 = 0;
		globp = savedfile;
	} else
		init();	/* build a work space */

	xfile[0] = 0;	/* normal input is std. input */

	if ((signal(SIGINTR, 1) & 01) == 0)
		signal(SIGINTR, onintr);

	/* Check for upper/lower case mapping */
	if( gtty( STDOUTPUT, ttystatus ) >= 0 )  {
		if (( ttystatus[2] & CASEMAP ) == CASEMAP) casemap++;
	}

	signal(SIGHUP, hangup);	/* save file if hung-up */
	write_sw = WRITTEN;

	/* Set return point for RESET(), process commands */
	setexit();
	commands();

	/* Prevent EOFs in interactive edits from causing exit */
	if( gtty( STDINPUT, ttystatus ) >= 0 )
		error();
}

/*
 *			C O M M A N D S
 *
 * Interpret and process commands
 */
commands()
{
	int getfile(), gettty();
	register int c;
	register FILEPNTR a1;
	int i;

    for (;;) {
	if (pflag) {
		pflag = 0;
		addr1 = addr2 = dot;
		goto print;
	}

	/* Prompt user for a command, if requested */
	if(promp && (globp == 0) && (diversion == 0) )
		write(1, cmdprod, 1);

	/*
	 * Process any line addresses specified
	 */
	addr1 = 0;
	addr2 = 0;
	if(a1=address()) {
		/*
		 * First item encountered was an address
		 */
		addr1 = a1;
		if(((c = getchar()) == ',') || (c == ';')) {
			if(c == ';')
				dot = a1;
			if(a1 = address()) {
				addr2 = a1;
				c = getchar();
			}  else {
				addr1 = addr1 - urange;
				addr2 = addr1 + urange + lrange;
				if(addr1 <= zero) addr1=zero+1;
				if(addr2 > dol) addr2 = dol;
				c = getchar();
			}
		}
	} else  {
		/*
		 * First item encountered was NOT an address.
		 * Check for abbrreviated forms of addressing
		 */
		if ((c = getchar()) == ',') {
			addr1 = dot;
			if((i = decimal()) != -1) {
				/*
				 * Form is ",#" -- expand to
				 * ".,.+#"
				 */
				addr2 = dot + i;
				c = getchar();
			}  else {
				/*
				 * Form is simply "," -- expand to
				 * ".-urange , .+lrange"
				 */
				addr1 = dot - urange;
				if(addr1 <= zero) addr1 = zero + 1;
				addr2 = dot + lrange;
				if(addr2 > dol) addr2 = dol;
				c = getchar();
			}
		}
	}

	/* If no second address specified, make second addr same as first */
	if(addr2 == 0)
		addr2 = addr1;

	/*
	 * Process next character as command character
	 */
	switch(c) {

	case 'a':
		setdot();
		newline();
		append(gettty, addr2);
		write_sw = NOTWRITTEN;
		continue;

	case 'c':
		setdot();
		newline();
		nonzero();
		delete();
		append(gettty, addr1-1);
		write_sw = NOTWRITTEN;
		continue;

	case 'd':
		setdot();
		newline();
		nonzero();
		delete();
		write_sw = NOTWRITTEN;
		continue;

	case 'e':
		setnoaddr();	/* no args permitted */
		c = getchar();
		if ( c != '!' ) {
			peekc = c;
			if ( c != ' ' ) error();
		}
		if ( write_sw == NOTWRITTEN && c != '!' && zero < dol &&
			gtty(STDINPUT,ttystatus) >= 0 )  {
			puts("Write file!");
			error();
		}
		savedfile[0] = 0;
		init();
		addr2 = zero;
		write_sw = WRITTEN;
		goto caseread;

	case 'f':
		setnoaddr();
		if ((c = getchar()) != '\n') {
			peekc = c;
			savedfile[0] = 0;
			filename();
		}
		puts(savedfile);
		continue;

	case 'g':
		global(1);
		continue;

	case 'h':
		listh++;
		goto caseprint;

	case 'i':
		setdot();
		nonzero();
		newline();
		append(gettty, addr2-1);
		write_sw = NOTWRITTEN;
		continue;

	case 'j':
		setdot();
		if( addr1 == addr2 )
			addr1 = addr1 - 1;	/* Join to previous line */
		nonzero();
		newline();
		join();
		write_sw = NOTWRITTEN;
		continue;

	case 'k':
		if ((c = getchar()) < 'a' || c > 'z')
			error();
		newline();
		setdot();
		nonzero();
		names[c-'a'] = *addr2 | MARKFLAG;	/* ??? */
		continue;

	case 'm':
		move(0);
		write_sw = NOTWRITTEN;
		continue;

	case 'n':
		listn++;
		goto caseprint;

	case '\n':
		if((addr1 == 0) && (addr2 == 0))
			addr1 = addr2 = dot + 1;
		else if (addr2 == 0) addr2 = addr1;

		setdot();
		nonzero();
		goto print;

	case 'l':
		listf++;
	case 'p':
	caseprint:
		setdot();
		nonzero();
		newline();
	print:
		a1 = addr1;

		/* Determine number of digits in largest line number */
		c = addr2 - zero;
		digits = 0;
		do {
			c = c/10;
			digits++;
		} while ( c > 0 );

		/* Print each line, optionally preceeded w/line number */
		do {
			/* Output line numbers, if appropriate */
			if(listn) {
				zerofill( (FILEADDR) (a1-zero) );
			}
			puts(getline(*a1++));
		}  while (a1 <= addr2);

		dot = addr2;
		listf = 0;
		listh = 0;
		listn = 0;
		pflag = 0;
		continue;

	case 'q':
		setnoaddr();
		if ( (c=getchar()) != '\n') newline();
		if ( write_sw == NOTWRITTEN && c != '!' && zero < dol &&
			gtty(STDINPUT,ttystatus) >= 0 )  {
			puts("Write file!");
			error();
		}
		unlink(tfname);
		exit(0);

	case 'r':
		write_sw = NOTWRITTEN;
	caseread:
		filename();
		if ((io = open(file, 0)) < 0) {
			lastc = '\n';
			error();
		}
		setall();
		ninbuf = 0;
		append(getfile, addr2);
		exfile();
		continue;

	case 's':
		setdot();
		nonzero();
		substitute(globp,0);
		continue;

	case 't':
		move(1);
		write_sw = NOTWRITTEN;
		continue;

	case 'u':
		setdot();
		nonzero();
		substitute(globp,1);
		continue;

	case 'v':
		global(0);
		continue;

	case 'w':
		setall();
		nonzero();
		filename();
		if ((io = creat(file, FMODE)) < 0) {
			perror(file);
			error();
		}
	 	putfile();  /* this will reset the write_sw if successful */
		exfile();
		continue;

	case 'x':
		setnoaddr();	/* no line #s */
 		divert();
		continue;

	case '<':
		if( (i=decimal()) != -1 )
			urange = i;
		else {
			numbuf = urange;
			putd();
			putchar('\n');
		}
		newline();
		continue;

	case '>':
		if((i=decimal()) != -1)
			lrange = i;
		else {
			numbuf = lrange;
			putd();
			putchar('\n');
		}
		newline();
		continue;


	case '=':
		setall();
		newline();
		numbuf = addr2 - zero;
		putd();
		putchar('\n');
		continue;

	case '!':
		unixcom(0);
		continue;

	case '%':
		unixcom(1);
		continue;

	case 'P':
		setnoaddr();
		if( getchar() != '\n' )
			error();
		promp = ( promp == 0 );	/* Flip prompting status */
		continue;

	case EOF:
		return;

	}
	error();
    }	/* continue puts us here */
}

/*
 * Evaluate the address(s) on the front of each command.
 * The address is of the form:
 *	[ #1 [ { ',' or ';' } #2 ]  
 * where #n can be a numerical value, an equation of simple form,
 * or a 'regular' pattern search expression to be found.
 */
address()
{
	register FILEPNTR a1;
	register int minus;
	register int c;
	int relerr;
	int n;

	minus = 0;
	a1 = 0;
	for (;;) {
		c = getchar();
		if ('0'<=c && c<='9') {
			n = 0;
			do {
				n =* 10;
				n =+ c - '0';
			} while ((c = getchar())>='0' && c<='9');
			peekc = c;
			if (a1==0)
				a1 = zero;
			if (minus<0)
				n = -n;
			a1 =+ n;
			minus = 0;
			continue;
		}
		relerr = 0;
		if (a1 || minus)
			relerr++;
		switch(c) {
		case ' ':
		case '\t':
			continue;
	
		case '+':
			minus++;
			if (a1==0)
				a1 = dot;
			continue;


		case '-':
		case '^':
			minus--;
			if (a1==0)
				a1 = dot;
			continue;
	
		case '?':
		case '/':
			if((peekc = getchar()) != '\n') compile(c);
			else if ( *expbuf == 0 ) error();
			a1 = dot;
			for (;;) {
				if (c=='/') {
					a1++;
					if (a1 > dol)
						a1 = zero;
				} else {
					a1--;
					if (a1 < zero)
						a1 = dol;
				}
				if (execute(0, a1))
					break;
				if (a1==dot)
					error();
			}
			break;
	
		case '$':
			a1 = dol;
			break;
	
		case '.':
			a1 = dot;
			break;

		case '\'':
			/* Process a 'MARK' ref. */
			if ((c = getchar()) < 'a' || c > 'z')
				error();

			/* Search for a match */
			for (a1=zero; a1<=dol; a1++)
				if (names[c-'a'] == (*a1 | MARKFLAG))
					break;
			break;
	
		default:
			peekc = c;
			if (a1==0)
				return(0);
			a1 =+ minus;
			if (a1<zero || a1>dol)
				error();
			return(a1);
		}
		if (relerr)
			error();
	}
}

/*
 * set both addresses for the command (start and end)
 * to the value of 'dot'.
 */
setdot()
{
	if (addr2 == 0)
		addr1 = addr2 = dot;
	if (addr1 > addr2)
		error();
}

/*
 * set start address to beginning of text,
 * end address to end of text
 */
setall()
{
	if (addr2==0) {
		addr1 = zero+1;
		addr2 = dol;
		if (dol==zero)
			addr1 = zero;
	}
	setdot();
}

/*
 * Ensure that no addresses were specified on the command.
 */
setnoaddr()
{
	if (addr2)
		error();
}

/*
 * Perform a range test on the addresses.
 */
nonzero()
{
	if (addr1<=zero || addr2>dol)
		error();
}

/*
 * Process the various printing options that may occure at
 * the end of the command line.
 */
newline()
{
	register c;

	while ( (c=getchar()) != '\n') switch ( c ) {
	case 'p':
		pflag++;
		continue;
	case 'l':
		listf++;
		pflag++;
		continue;
	case 'h':
		listh++;
		pflag++;
		continue;
	case 'n':
		listn++;
		pflag++;
		continue;
	default:
		error();
	}
}

/*
 * Either return the old file name
 * or install a new one.
 */
filename()
{
	register char *p1, *p2;
	register c;

	numbuf = 0;
	c = getchar();
	if (c=='\n' || c==EOF) {
		p1 = savedfile;
		if (*p1==0) {
			puts("No name given");
			error();
		}
		p2 = file;
		while (*p2++ = *p1++);
		return;
	}
	if (c!=' ')
		error();
	while ((c = getchar()) == ' ');
	if (c=='\n')
		error();
	p1 = file;
	do {
		*p1++ = c;
		c = getchar();
	} while ( c != '\n' && c != EOF );
	*p1++ = 0;
	if (savedfile[0]==0) {
		p1 = savedfile;
		p2 = file;
		while (*p1++ = *p2++);
	}
}

/*
 * clean up after writing the file
 */
exfile()
{
	close(io);
	io = -1;
	if (vflag) {
		putd(); /* write out # bytes written */
		putchar('\n');
	}
}

/*
 * interrupts caught here.
 */
onintr()
{
	signal(SIGINTR, onintr);
	lastc = '\n';
	error();
}

/*
 * All 'error();' statements put us here.  This function
 * resets the world.  Control is transfered back to the
 * mainline.
 */
error()
{
	register c;

	for(; diversion >=1; diversion--)
		close( xfile[diversion] );
	diversion = 0;	/* Terminate any 'x' files in progress  */
	listn = 0;
	listh = 0;
	listf = 0;
	seek(0, 0, 2);
	pflag = 0;
	if (globp)
		lastc = '\n';
	globp = 0;
	peekc = lastc;
	while ((c = getchar()) != '\n' && c != EOF);
	puts("?");
	if (io > 0) {
		close(io);
		io = -1;
	}
	/*
	 * The 'reset()' function returns control to the command
	 * following the last 'setexit()' statement.  In this
	 * case, control returns to the mainline, which proceeds
	 * to analyze the next command.
	 */
	reset();
}

/*
 * This is a very special replacement for the usual 'getchar()'.
 * The last character returned in availible in the global 
 * variable 'lastc'.
 * If a routine reads a character that it was not supposed to, it can
 * place the character in the global variable 'peekc', and the next
 * call will return it again.  Also, whole lines of text may be faked
 * as input.  If nonzero, 'globp' points to a null-terminated string
 * of characters to be read.  Also, temporary input diversion
 * is processed here as well.
 */
getchar()
{
	if (lastc=peekc) {
		peekc = 0;
		return(lastc);
	}

	if (globp) {
		if ((lastc = *globp++) != 0)
			return(lastc);
		globp = 0;
		return(EOF);
	}

	/*
	 * I/O diversion (for the 'x' command) is handled
	 * here.
	 */
	if (read ( xfile[diversion], &lastc, 1) <= 0) {
		if ( --diversion < 0 ) {
			diversion = 0;
			return ( lastc = EOF );
		}
		else {
			close(xfile[diversion+1]); /* CLOSE FILE! */
			return( getchar() );
		}
	}
	lastc =& BYTEMASK;
	return(lastc);
}

/*
 * for appending -
 *
 * read a line with 'getchar()' and put it in 'linebuf'
 */
gettty()
{
	register c, gf;
	register char *p;

	p = linebuf;
	gf = globp;

	/* Prompt user for text, if requested */
	/******************* REMOVED as per request WCG *****************
	if(promp && (globp == 0) && (diversion == 0) )
		write(1, txtprod, 1);
	 ****************************************************************/

	/* Fetch a line of text */
	while ((c = getchar()) != '\n') {
		if (c==EOF) {
			if (gf)
				peekc = c;
			return(c);
		}
		if ((c =& BYTEMASK) == 0)
			continue;
		*p++ = c;
		if (p >= &linebuf[LBSIZE-1])
			error();
	}
	*p++ = 0;

	/* return EOF if ".\n" encountered */
	if (linebuf[0]=='.' && linebuf[1]==0)
		return(EOF);

	/* Normal return */
	return(0);
}

/* This function is used to read text files into the editor workfile */
getfile()
{
	register c;
	register char *lp, *fp;

	lp = linebuf;
	fp = nextip;
	do {
		/* Get more chars into buffer as required */
		if (--ninbuf < 0) {
			if ((ninbuf = read(io, genbuf, LBSIZE)-1) < 0)
				return(EOF);
			fp = genbuf;
		}

		/* If line is too long, arbitrarily break it up */
		if (lp >= &linebuf[LBSIZE-1])
			c = '\n';
		else
			c = *fp++ & BYTEMASK;

		/* Ignore nulls in input file */
		if( (*lp++ = c) == 0 )  {
			lp--;
			continue;
		}

		numbuf++;	/* Increment character count */
	} while (c != '\n');

	*--lp = 0;
	nextip = fp;
	return(0);
}

/* This function is used to put the editor workfile out as a text file */
putfile()
{
	FILEPNTR a1;
	register char *fp;
	register char *lp;
	register nib;

	nib = 512;
	fp = genbuf;
	a1 = addr1;
	do {
		lp = getline(*a1++);
		for (;;) {
			if (--nib < 0) {
				if(write(io,genbuf,fp-genbuf) != fp-genbuf) {
					perror(file);
					write_sw = NOTWRITTEN;
					error();
				}
				nib = 511;
				fp = genbuf;
			}
			numbuf++;	/* Incr character count */
			if ((*fp++ = *lp++) == 0) {
				fp[-1] = '\n';
				break;
			}
		}
	} while (a1 <= addr2);
	if ( write ( io, genbuf,fp-genbuf )  != fp-genbuf )  {
		perror(file);
		write_sw = NOTWRITTEN;
		error();
	}
	write_sw = WRITTEN;
}

/* Append text to the buffer.  Lines are supplied in 'linebuf' by
 * 'f()', the first arg.
 */
append(f, a)
int (*f)();
FILEADDR a;
{
	int nline;
	struct { int integer; };

	nline = 0;
	dot = a;
	while ((*f)() == 0) {
		if (dol >= endcore) {
			if (sbrk( 512 * sizeof( FILEADDR ) ) == -1) {
				puts("Too many lines");
				error();
			}
			endcore = sbrk(0);
		}
		doappend();
		nline++;
	}
	return(nline);
}

/* Perform the actual append of a statement.  The statement
 * is assumed to be in 'linebuf'.
 * This routine separated so it could be shared
 */
doappend()
{
	register FILEPNTR a1;
	register FILEPNTR a2;
	register FILEPNTR rdot;
	FILEPNTR tl;
	extern FILEADDR putline();

	/* put line in disk buffer */
	tl = putline();

	/* move back pointer table */
	a1 = ++dol;
	a2 = a1 + 1;
	rdot = ++dot;
	while ( a1 > rdot )
		*--a2 = *--a1;

	/* insert address into table */
	*rdot = tl;
}

/* Let the user temporarily escape from the editor */
unixcom(sw)
{
	register savint, pid, rpid;
	int retcode;

	setnoaddr();

	signal(SIGINTR, 1);
	if ((pid = fork()) == 0) {
		signal(SIGHUP, onhup);
		signal(SIGQUIT, onquit);
		signal(SIGINTR, 0);
		if(sw) {
			while(getchar() != '\n');
			execl("/bin/sh","-",0);
		} else
			execl("/bin/sh", "sh", "-t", 0);
		exit();
	}
	while ((rpid = wait(&retcode)) != pid && rpid != -1);
	signal(SIGINTR, onintr);

	casemap = 0;
	if( gtty(STDOUTPUT, ttystatus) >= 0 )  {
		if ( (ttystatus[2] & CASEMAP) == CASEMAP ) casemap++;
	}
	puts("!");
}

/* delete the lines from addr1 to addr2 */
delete()
{
	register FILEPNTR a1;
	register FILEPNTR a2;
	register FILEPNTR a3;

	a1 = addr1;
	a2 = addr2+1;
	a3 = dol;
	dol =- a2 - a1;
	do
		*a1++ = *a2++;
	while (a2 <= a3);
	a1 = addr1;
	if (a1 > dol)
		a1 = dol;
	dot = a1 - 1;  /* Set dot to next line */
	if ( zero == dol )
		pflag = listf = listh = listn = 0;
}

/*
 *			G E T L I N E
 *
 *
 * Load 'linebuf' with the text line being worked on
 */
getline(tl)
FILEADDR tl;
{
	register char *bp;
	register char *lp;
	register nl;

	lp = linebuf;
	bp = getblock(tl, READ);
	nl = nleft;
	while( (*lp++ = *bp++) != EOR )  {
		if (--nl == 0) {
			tl = NEXT_TMP( tl );
			bp = getblock(tl, READ);
			nl = nleft;
		}
	}
	*--lp = 0;		/* Null terminate string */
	return(linebuf);
}

/*
 *			P U T L I N E
 *
 * Put 'linebuf' onto the disk, increment 'tline', and return the
 * address of the line.
 */
FILEADDR putline()
{
	register char *bp;
	register char *lp;
	register nl;
	FILEADDR oldline;
	FILEADDR tl;

	lp = linebuf;
	bp = getblock(tline, WRITE);
	nl = nleft;
	tl = tline;
	while (*bp = *lp++) {
		if (*bp++ == '\n') {
			/* Used to split lines in getsub() */
			*--bp = 0;		/* (moves bp back) */
			linebp = lp;		/* Note line break point */
			break;
		}
		if (--nl == 0) {
			tl = NEXT_TMP( tl );
			bp = getblock(tl, WRITE);
			nl = nleft;
		}
	}
	*bp = EOR;		/* Terminate line in buffer */
	oldline = tline;
	tline += BYTES_TMP( lp-linebuf );
	return( oldline );
}

/*
 *			G E T B L O C K
 *
 * Performs disk I/O.  Maps internal disk addresses to a block
 * number and byte displacement.
 * Given a FILEADDR and a read or write flag, return a pointer
 * to the area within a buffer which contains the beginning
 * of the line.  Global variable 'nleft' indicates how many
 * bytes are left in the current buffer.  Records which
 * span more than one buffer by convention are allocated
 * sequential blocks in the temp file.
 *
 * Two buffers are used.  'ibuff' is primarily for input,
 * although it sometimes may buffer output.  'obuff' is used
 * primarily for output, although sometimes the contents may
 * be used before the block is actually written.
 *
 * 'iof' is one of READ or WRITE.
 */
getblock(atl, iof)
FILEADDR atl;
{
	extern read(), write();
	register int bno;
	register int i;
	register int off;

	bno = TMP_BLOCK( atl );
	off = TMP_BYTES( atl );
	if (bno >= MAXTEMP) {
		puts("Work file overflow");
		error();
	}
	nleft = 512 - off;

	/* See if block is in input buffer */
	if (bno==iblock) {
		/* If writing is done on input buffer, note it */
		if( iof == WRITE )
			ichanged = 1;
		return(ibuff+off);
	}

	/* See if block is in output buffer */
	if (bno==oblock)
		return(obuff+off);

	/* Block is not in core.  Load into input buffer */
	if (iof==READ) {
		/* If input buffer was written on, flush first */
		if (ichanged)
			blkio(iblock, ibuff, write);
		ichanged = 0;
		iblock = bno;
		blkio(bno, ibuff, read);
		return(ibuff+off);
	}

	/*
	 * Write request to a block not buffered in core.
	 * Flush out previous block, if present.
	 *
	 * Q:  Why is new block not read first?
	 * A:  Because writing is only done on the last block of
	 *     the file;  hence it is either in core, or a new block.
	 */
	if (oblock>=0)
		blkio(oblock, obuff, write);
	/* Clear out buffer */
	for(i=0; i<512; i++) obuff[i]=0;
	oblock = bno;
	return(obuff+off);
}

/*
 *			B L K I O
 *
 * Perform disk I/O on one block with error notification
 */
blkio(b, buf, iofcn)
int (*iofcn)();
{
	seek(tfile, b, 3);
	if ((*iofcn)(tfile, buf, 512) != 512) {
		perror(tfname);
		error();
	}
}

/*
 *			I N I T
 *
 * Set up disk buffers and pointers
 */
init()
{
	register char *p;
	register pid;

	/*
	 * Reinitialize temp file buffering scheme
	 */
	tline = OINCR;		/* Should be at FIRST slot, not 0 */
	iblock = -1;
	oblock = -1;
	ichanged = 0;
	close(tfile);
	unlink(tfname);

	/* Build new temp file */
	tfname = "/tmp/edaXXXXX";
	mktemp( tfname );
	close(creat(tfname, 0600));	
	tfile = open(tfname, 2);
	/*
	 * Since the temporary file will never be closed,
	 * it can be unlinked here.
	 */
/***
	unlink(tfname);
***/

	/*
	 * Throw away all of the dynamic core we acquired
	 */
	brk(fendcore);
	dot = zero = dol = fendcore;
	endcore = fendcore - 2;
}

/*
 *			G L O B A L
 *
 * Perform the given commands on all lines that match the
 * regular expression.
 */
global(k)
{
	register char *gp;
	register c;
	register int *a1;
	char globuf[GBSIZE];

	if (globp)
		error();
	setall();
	nonzero();
	if ((c=getchar())=='\n')
		error();

	/* compile the regular expression into 'expbuf' */
	compile(c);

	gp = globuf;
	while ((c = getchar()) != '\n') {
		if (c==EOF)
			error();
		if (c=='\\') {
			c = getchar();
			if (c!='\n')
				*gp++ = '\\';
		}
		*gp++ = c;
		if (gp >= &globuf[GBSIZE-2])
			error();
	}
	*gp++ = '\n';
	*gp++ = 0;

	/* Set bit 0 on all lines that fit the pattern */
	for (a1=zero; a1<=dol; a1++) {
		*a1 =& ~MARKFLAG;
		if (a1>=addr1 && a1<=addr2 && execute(0, a1)==k)
			*a1 =| MARKFLAG;
	}

	/* Fake the command on all lines marked above */
	for (a1=zero; a1<=dol; a1++) {
		if (*a1 & MARKFLAG) {
			*a1 =& ~MARKFLAG;	/* reset the mark */
			dot = a1;
			globp = globuf;		/* fool getchar */

			/* execute the faked command */
			commands();

			/* Reset search to zero so moved lines don't
			 * get missed */
			a1 = zero;
		}
	}
}

/*
 * This is the master control routine in charge of substitutions.
 * It calls several routines, which call several routines, which
 * call other routines...
 */
substitute(inglob,fnsw)
{
	register skipcnt, *a1, nl, i;
	int getsub();

	skipcnt = compsub(fnsw);	/* initialize expbuf & rhsbuf */
	for (a1 = addr1; a1 <= addr2; a1++) {

		/* Call 'execute' to initialize 'linebuf' and find first
		 * occurance of the expression described in EXPBUF
		 */
		if ( execute(0, a1) == 0) continue;

		/* Skip the requested number of occurances */
		i = skipcnt;
		while( i-- > 0 ) {
			if ( execute(1) == 0 ) goto broke;
		}

		/* Perform the requested number of substitutions */
		docnt--;
		dosub();
		inglob =| 01;
		write_sw = NOTWRITTEN;
		while ( (docnt-- > 0) && (*loc2 != 0) ) {
			if( execute(1) != 0 ) dosub();
			else break;
		}


		/* This code re-marks the line just changed, if it
		 * had been marked by a 'k' command */
		for (i=0; i < 26; i++) {
			if(names[i] == (*a1 | MARKFLAG)) {
				*a1 = putline();
				names[i] = *a1 | MARKFLAG;
				goto sub_break;
			}
		}
		*a1 = putline ();

	sub_break:
		nl = append(getsub, a1);
		a1 =+ nl;
		addr2 =+ nl;
	broke:
		continue;
	}
	if (inglob==0) {
		putchar('?');	/* ?? for string not found */
		error();
	}
}

/*
 * Process the two strings in the 's' or 'u' command.
 * Calls 'compile()' or 'ucompile()' to analyze the first string,
 * does the second itself, and calls 'options()' to process
 * the various options that can occure.
 */
compsub(fnsw)
{
/* 'seof' is eof char for substitute */
	register seof, c;
	register char *p;
	int gsubf;

	if ((seof = getchar()) == '\n')
		error();

	/* Analyze first string in substitute or U command */
	if(fnsw) ucompile(seof);
	else compile(seof);

	p = rhsbuf;
	for (;;) {
		c = getchar();
		if (c=='\\')
			c = getchar() | 0200;
		if (c=='\n')
			error();
		if (c==seof)
			break;
		*p++ = c;
		if (p >= &rhsbuf[LBSIZE/2])
			error();
	}
	*p++ = 0;

	/* analyze the options (docount & skipcount) */
	return ( options() );
}

/*
 * 'compile()' takes the first string in a substitute (S) command,
 * and builds a buffer.  Since this string potentially contains
 * characters which assume special meanings ("wild-cards", etc),
 * special processing is required.  The buffer to hold the
 * 'compiled' regular expression is called 'EXPBUF' and is
 * formatted as follows:
 *   ID, char, [ID, char, ...] EOF_ID.
 */
compile(aeof)
/* buffer is set up as:  ID, char, ID, char, ... */
{
	register eof, c;
	register char *ep;
	char *lastep;
	char bracket[NBRA], *bracketp;
	int nbra;	/* number of brackets */
	int cclcnt;

	ep = expbuf;
	eof = aeof;
	bracketp = bracket;
	nbra = 0;
	if ((c = getchar()) == eof) {
		if (*ep==0)
			error();
		return;
	}
	circfl = 0;
	if (c=='^') {
		/* beginning of line */
		c = getchar();
		circfl++;
	}
	if (c=='*')
		goto cerror;
	peekc = c;
	for (;;) {
		if (ep >= &expbuf[ESIZE])
			goto cerror;
		c = getchar();
		if (c==eof) {
			*ep++ = CEOF;
			return;
		}
		if (c!='*')
			lastep = ep;
		switch (c) {

		case '\\':
			if ((c = getchar())=='(') {
				if (nbra >= NBRA)
					goto cerror;
				*bracketp++ = nbra;
				*ep++ = CBRA;
				*ep++ = nbra++;
				continue;
			}
			if (c == ')') {
				if (bracketp <= bracket)
					goto cerror;
				*ep++ = CKET;
				*ep++ = *--bracketp;
				continue;
			}
			*ep++ = CCHR;
			if (c=='\n')
				goto cerror;
			*ep++ = c;
			continue;

		case '.':
			*ep++ = CDOT;
			continue;

		case '\n':
			goto cerror;

		case '*':
			if (*lastep==CBRA || *lastep==CKET)
				error();
			*lastep =| STAR;
			continue;

		case '$':
			if ((peekc=getchar()) != eof)
				goto defchar;
			*ep++ = CDOL;
			continue;

		case '[':
			*ep++ = CCL;
			*ep++ = 0;
			cclcnt = 1;
			if ((c=getchar()) == '^') {
				c = getchar();
				ep[-2] = NCCL;
			}
			do {
				if (c=='\n')
					goto cerror;
				*ep++ = c;
				cclcnt++;
				if (ep >= &expbuf[ESIZE])
					goto cerror;
			} while ((c = getchar()) != ']');
			lastep[1] = cclcnt;
			continue;

		defchar:
		default:
			*ep++ = CCHR;
			*ep++ = c;
		}
	}
   cerror:
	expbuf[0] = 0;
	error();
}

/*
 * This routine compiles 'expbuf' for the 'u' command.
 * It is like 'compile()' except that all special processing
 * for "wild-cards" etc has been omitted.  This is
 * particularly valuable for working with equations, especially on
 * terminals without lower case.
 */
ucompile(aeof)
{
	register eof, c;
	register char *ep;

	ep = expbuf;
	eof = aeof;
	circfl = 0;

	/* if no search string is present, use last one, if it exists */
	if( (c = getchar()) == eof) {
		if( *ep == 0 ) error();
		return;
	}
	peekc = c;

	/* Take each character literally */
	while( (c=getchar()) != eof) {
		if( ep >= &expbuf[ESIZE] ) {
			expbuf[0] = 0;
			error();
		}

		*ep++ = CCHR;
		*ep++ = c;
	}

	/* terminate compiled string */
	*ep++ = CEOF;
}

/*
 * This routine returns the address of the substring to be replaced.
 * The starting address is in 'loc1' and the ending address is
 * in 'loc2'.
 */
execute(gf, addr)
int *addr;
{
	register char *p1, *p2, c;

	if (gf) {

		/* This is not the first substitution this command */
		if (circfl)
			return(0);
		p1 = linebuf;
		/* position p1 after last char replaced by last subst */
		locs = p1 = loc2;

	} else {
		if (addr==zero)
			return(0);
		p1 = getline(*addr);	/* returns pos in 'linebuf' */
		locs = 0;
	}
	p2 = expbuf;
	if (circfl) {
		loc1 = p1;
		return(advance(p1, p2));
	}

	/*
	 * Search forwards until a match is found or we run out of
	 * characters in this line of text.
	 */

	/* fast check for first character */
	if (*p2==CCHR) {
		c = p2[1];
		do {
			if (*p1!=c)
				continue;
			if (advance(p1, p2)) {
				loc1 = p1;
				return(1);
			}
		} while (*p1++);
		return(0);
	}

	/* regular algorithm */
	do {
		if (advance(p1, p2)) {
			loc1 = p1;
			return(1);
		}
	} while (*p1++);
	return(0);
}

/*
 * This routine is called by 'execute()' to find the next
 * occurance of the regular expression in 'expbuf' in the current line
 */
advance(alp, aep)
{
	register char *lp, *ep, *curlp;
	char *nextep;

	lp = alp;
	ep = aep;
	for (;;) switch (*ep++) {

	case CCHR:
		if (*ep++ == *lp++)
			continue;
		return(0);

	case CDOT:
		if (*lp++)
			continue;
		return(0);

	case CDOL:
		if (*lp==0)
			continue;
		return(0);

	case CEOF:
		loc2 = lp;
		return(1);

	case CCL:
		if (cclass(ep, *lp++, 1)) {
			ep =+ *ep;
			continue;
		}
		return(0);

	case NCCL:
		if (cclass(ep, *lp++, 0)) {
			ep =+ *ep;
			continue;
		}
		return(0);

	case CBRA:
		braslist[*ep++] = lp;
		continue;

	case CKET:
		braelist[*ep++] = lp;
		continue;

	case CDOT|STAR:
		curlp = lp;
		while (*lp++);
		goto star;

	case CCHR|STAR:
		curlp = lp;
		while (*lp++ == *ep);
		ep++;
		goto star;

	case CCL|STAR:
	case NCCL|STAR:
		curlp = lp;
		while (cclass(ep, *lp++, ep[-1]==(CCL|STAR)));
		ep =+ *ep;
		goto star;

	star:
		do {
			lp--;
			if (lp==locs)
				break;
			if (advance(lp, ep))
				return(1);
		} while (lp > curlp);
		return(0);

	default:
		error();
	}
}

/* This routine is called by 'advance()' */
cclass(aset, ac, af)
{
	register char *set, c;
	register n;

	set = aset;
	if ((c = ac) == 0)
		return(0);
	n = *set++;
	while (--n)
		if (*set++ == c)
			return(af);
	return(!af);
}

/* Perform single substitution of 'rhsbuf' to marked portion of line */
dosub()
{
	register char *lp, *sp, *rp;
	int c;

	lp = linebuf;
	sp = genbuf;
	rp = rhsbuf;

	/* copy to beginning of sub-string */
	while (lp < loc1)
		*sp++ = *lp++;

	while (c = *rp++) {
		if (c=='&') {
		/* Insert copy of first substring */
			sp = place(sp, loc1, loc2);
			continue;
		} else  {
			if (c<0 && (c =& BYTEMASK) >='1' && c < NBRA+'1') {
				/* copy [ \( ... \) ] marked field */
				sp = place(sp, braslist[c-'1'], braelist[c-'1']);
				/*                ^ start          ^ end    */
				continue;
			}
		}
		*sp++ = c&BYTEMASK;
		if (sp >= &genbuf[LBSIZE])
			error();
	}
	lp = loc2;
	/* loc2 is left at end of replacemet string in linebuf, +1 */
	loc2 = (sp - genbuf) + linebuf;

	/* Append to 'genbuf' the rest of 'linebuf' */
	while (*sp++ = *lp++)
		if (sp >= &genbuf[LBSIZE])
			error();
	/* Copy 'genbuf' onto 'linebuf' -- substitute is done */
	lp = linebuf;
	sp = genbuf;
	while (*lp++ = *sp++);
}

/* This routine copies from al1 to al2, placing the output
 * starting at asp.  It returns the updated asp.
 */
place(asp, al1, al2)
{
	register char *sp, *l1, *l2;

	sp = asp;
	l1 = al1;
	l2 = al2;
	while (l1 < l2) {
		*sp++ = *l1++;
		if (sp >= &genbuf[LBSIZE])
			error();
	}
	return(sp);
}

/*
 *			M O V E
 *
 * This routine moves or copies lines of text from one place
 * in the file to another, depending on the value of 'cflag'.
 */
move(cflag)
{
	register FILEPNTR adt;
	register FILEPNTR ad1;
	register FILEPNTR ad2;
	int getcopy();

	setdot();
	nonzero();
	if ((adt = address())==0)
		error();
	newline();

	ad1 = addr1;
	ad2 = addr2;

	/* Perform copy, if asked */
	if (cflag) {
		ad1 = dol;
		append(getcopy, ad1++);
		ad2 = dol;
	}
	ad2++;
	if (adt<ad1) {
		dot = adt + (ad2-ad1);
		if ((++adt)==ad1)
			return;
		reverse(adt, ad1);
		reverse(ad1, ad2);
		reverse(adt, ad2);
	} else  {
		if (adt >= ad2) {
			dot = adt++;
			reverse(ad1, ad2);
			reverse(ad2, adt);
			reverse(ad1, adt);
		} else
			error();
	}
}

/*
 *			R E V E R S E
 *
 * This routine exchanges two lines of text
 *
 * The second line must follow the first line
 */
reverse(a1, a2)
register FILEPNTR a1;
register FILEPNTR a2;
{
	register FILEADDR temp;

	for (;;) {
		temp = *--a2;
		if (a2 <= a1)
			return;
		*a2 = *a1;
		*a1++ = temp;
	}
}

/* This routine provides a duplicate copy of the lines between
 * addr1 and addr2 for input to 'append()'.  This effects the
 * 't' command.
 */
getcopy()
{
	if (addr1 > addr2)
		return(EOF);
	getline(*addr1++);
	return(0);
}

/* This provides input to 'append()' for splitting lines of text */
getsub()
{
	register char *p1, *p2;

	p1 = linebuf;
	if ((p2 = linebp) == 0)
		return(EOF);
	while (*p1++ = *p2++);
	linebp = 0;
	return(0);
}

/* This routine outputs in decimal the value stored in numbuf */
putd()
{
	register r;

	r = numbuf % 10;
	numbuf = numbuf / 10;
	if( numbuf )
		putd();
	putchar(r + '0');
}

/* This routine outputs the string pointed to by 'as' */
puts(as)
{
	register char *sp;

	sp = as;
	while (*sp)
		putchar(*sp++);
	putchar('\n');
}

/* Output a character on the standard output, with optional
 * special handling.
 */
char	line[OBUFSIZE];
char	*linp	line;

putchar(ac)
{
	register char *lp;
	register c;

	lp = linp;
	c = ac;
	if(listh) {
		if ( c == '\t' ) c = ' ';
		if ( c == ' ' && lastput == ' ') return;
	}
	if (listf) {
		col++;
		if (col >= 72) {
			col = 0;
			*lp++ = '\\';
			*lp++ = '\n';
		}
		if( c == '\n' )  {
			col = 0;
			*lp++ = '\n';
			goto out;
		}
		if (c=='\t') {
			c = '>';
			goto esc;
		}
		if (c=='\b') {
			c = '<';
		esc:
			*lp++ = '-';
			*lp++ = '\b';
			*lp++ = c;
			goto out;
		}
		if ( casemap != 0  && c >= 'A' && c <= 'Z' ) {
			*lp++ = '\\';
			*lp++ = c;
			col++;
			goto out;
		}
		if ( ( c<' ' || c>= 0177 ) && c != '\n' )  {
			*lp++ = '\\';
			*lp++ = (( c >> 6 ) & 03 ) + '0';
			*lp++ = (( c >> 3 ) & 07 ) + '0';
			*lp++ = (c&07)+'0';
			col =+ 3;
			goto out;
		}
	}
	*lp++ = c;
out:
	lastput = c;
	if(c == '\n' || lp >= &line[OBUFSIZE]) {
		linp = line;
		write(1, line, lp-line);
		return;
	}
	linp = lp;
}

/*
 * set up the input diversion
 * for an 'x' command.
 */
divert() {
	register char c, *p1;
	register i;
	char workarea[FNSIZE];
	c = getchar();
	if ( c== '\n' || c == EOF ) error();
	while ( (c=getchar()) == ' ');
	if ( c == '\n' ) error();
	p1 = workarea;
	do {
		*p1++ = c;
	} while ( (c = getchar() ) != '\n' && p1-workarea < FNSIZE);
	*p1++ = 0;
	if ( (i = open(workarea, 0)) < 0 ) {
		perror(workarea);
		error();
	}
	xfile[++diversion] = i;
}

/*
 * This routine will output the proper number of zeros for the 'n'
 * command, so that all line numbers will take up the same ammount 
 * of space.
 */
zerofill( tot )
register FILEADDR tot;
{
	register int i;

	/* Store number in output area for putn() */
	numbuf = tot;

	/* Leading zero pad */
	i = 0;
	do {
		tot = tot/10;
		i++;
	} while (tot > 0);
	i = digits - i;
	while ( i-- > 0)
		putchar('0');

	putd();
	putchar(':');
	putchar('\t');
}


/* Process a hangup signal  -  save the file! */
hangup() {
	if ( (io = creat("ed-hangup", FMODE)) < 0 ) {
		unlink(tfname);
		exit();
	}
	addr1 = zero+1;		/* First line of text */
	addr2 = dol;
	putfile();
	close(io);
	exit();
}

/*
 * Process the options present at the end of the 'u' or
 * 's' command specifing the range of the substitute. 
 */
options()
{
	register c, skipcnt;

	skipcnt = 0;
	docnt = 1;
	c = getchar();
	if( c == 'g' ) {
	globl:
		/* For a large file, 1,$s/x/X/g will work
		 * only on the first 32,000 substitutions.
		 * This is kludgy, but not too dangerous.
		 */
		docnt = 32000;
		newline();
		return ( skipcnt );
	}
	if( c < '1' || c > '9' ) {
	end:
		peekc = c;
		newline();
		return( skipcnt );
	}
	skipcnt = c - '1';
	c = getchar();
	if( c == 'g' ) goto globl;
	if( c != ',' ) goto end;
	if( (c = getchar()) < '1' || c > '9' ) error();
	docnt = c - '0';
	if( docnt <= skipcnt ) error();
	docnt = docnt - skipcnt;
	newline();
	return(skipcnt);
}

/*
 *			J O I N
 *
 * This routine joins all of the lines from addr1 to addr2
 * together into one line. 
 *
 * The lines will be combined in 'genbuf'
 */
join()
{
	register FILEPNTR a1;
	register char *lb;
	register char *gb;
	FILEADDR finaldot;

	gb = genbuf;
	finaldot = addr1 - 1;

	for(a1 = addr1; a1 <= addr2; a1++) {

		/* 'lb' has the address of the line */
		lb = getline(*a1);

		/* append this line onto the others */
		while(*gb = *lb++)
			if( gb++ >= &genbuf[LBSIZE-1] ) error();
	}
	delete();

	/* move the result to 'linebuf' and put the line out */
	gb = genbuf;
	lb = linebuf;
	while ( *lb++ = *gb++ );
	dot = finaldot;
	doappend();
	write_sw = NOTWRITTEN;
}

/*
 *			D E C I M A L
 *
 * Convert a number to binary and return its value.
 */
decimal()
{
	register c, tot, num;

	/* skip any leading blanks */
	while( (c = getchar()) == ' ' );
	peekc = c;

	/* collect the number */
	tot = 0;
	num = 0;
	while ( (c = getchar()) >= '0' && c <= '9' ) {
			num++;
			tot =* 10;
			tot =+  (c - '0');
	}
	peekc = c;
	if (num) return(tot);
	if (peekc == '$') {
		getchar();
		return( dol - dot );
	}
	return(-1);
}
