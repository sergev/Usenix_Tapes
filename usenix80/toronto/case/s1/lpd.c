#
/*
 * lpd -- line-printer daemon
 *
 *      Recoded into c from assembly. Part of the code is stolen
 *      from the data-phone daemon.
 *
 *      Dennis L. Mumaugh
 *      July 29, 1976
 *
 *      11 Nov 1976
 *      Partial implementation of binary formats.
 *
 *      17 Feb 1977
 *      Add stty/gtty call to install binary format.
 *
 *      26 Sep 1977    1.2  dlm
 *      Add indent option: I card sets indent for any value
 *
 *      12 Dec 1977    1.3  njl and dlm
 *      Fix to allow file opens two work using pipes and opr
 *
 *      21 Feb 1978    1.4 dlm
 *      add in new burst page
 *
 *	20 Dec 1978	2.0 was
 *	new burst page for Case
 *	Allow file to be run through pr at print time.
 *
 *	14 Mar 1979	2.1 was
 *	modified for multiple daemons, each controlling one printer.
 *	only try 20 times on open failures.
 *
 *
 */
char daemid[]   "~|^`lpd: V2.1 14 Mar 1979";

#define	SIGHUP	1
#define	SIGINT	2
#define	SIGQIT	3
#define	DAEMUID	1
#define	EJLINE	60
#define	MAXCOL	132
#define	CAP	01		/* Set to 0 for 96-char printer, else to 01 */
#define	EJECT	02
#define	OPEN	04
#define IND	010		/* Set to 0 for no indent, else to 010 */

struct {
	int	ino;
	char	name[14];
} dbuf;

char    lpd[11] "/case/lpd\0";
char    lock[5] "lock";
char    lpr[9] "/dev/lp\0";
char    tof[2]  "\14";
#ifdef SQN
char    sfile[4] "sqn";
#endif
char    line[132];
char	title[80];
int	dfb[259];
int     fo;
int     df;
int     pid;                    /* id of process */
struct  {
	int	flag;
	char    width;          /* paper width */
	char    length;         /* paper length */
	char    indent;         /* columns to indent */
	char    pad;            /* unused byte for s/gtty */
} lpmode, nlpmode;

extern banner();
char    *logname;               /* points to user login name */
char    class[15];              /* classification field */
char    jobname[15];            /* job or file name */

main(argc, argv)
char *argv[];
{
	register char *p1, *p2;

	/* set-up controlled environment; trap bad signals */
	setuid(DAEMUID);
	signal(SIGHUP, 1);
	signal(SIGINT, 1);
	signal(SIGQIT, 1);

	aclose();   /* obtain standard environment for files */

	/* use specified printer and spool directory */
	if (argc > 1) {
		lpd[9] = *argv[1];
		lpr[7] = *argv[1];
	}

	/* opr uses short form file names */
	if ( chdir(lpd) < 0 )
		exit(printf("\r\n\7lpd:\7 Can't change directory!\7\n"));
	if( stat(lock,dfb) >= 0) exit(0);
	if ((df=creat(lock, 0644)) < 0)
		exit(0);

	/* kill the parent so the user's shell can function */
	if (fork())
		exit(0);

/*
 *      write process id for others to know
 */
	pid = getpid();
	write(df,&pid,2);
	close(df);
/*
 *       search the directory for work (file with name df which is
 *       short for daemon file
 */
again:
	df = open(lpd, 0);
	do {
		if (read(df, &dbuf, sizeof dbuf) < sizeof dbuf) {
			/* EOF => no work to do */
			unlink(lock);
			exit(0);
		}
	} while (dbuf.ino==0 || dbuf.name[0]!='d' || dbuf.name[1]!='f');
	close(df);

/*
 *     we found something to do now do it
 */
	doit( dbuf.name);
	goto again;
}

/*
 * The remaining part is the reading of the daemon control file (df)
 */
doit(file)
{
	int indent1;
	int tvec[2];
	int i;
	char *cp, *hp;
	char c;
	int sqn, sf;

again:
	aclose();
	i = 0;
	indent1 = 0;

	/* acquire lineprinter and open daemon file */
	if ((fo = open(lpr, 1)) < 0) {
		if (i++) printf("\r\n\7\7lpd:\7 Can't open %s\r\n",lpr);
		if (i > 20) {
			printf("\r\n\7\7lpd:\7 %s open failure\r\n",lpr);
			exit(-1);
		}
		sleep(30);
		goto again;
	}

	i = 0;
again1:
	if (fopen(file, dfb) < 0) {
		printf("\r\n\7\7lpd: Can't open daemon file %s\r\n",file);
		if (i++ > 20) {
			printf("\r\n\7\7lpd: %s open failure\r\n", file);
			exit(-1);
		}
		sleep(30);
		goto again1;
	}

/*
 *      read the daemon file for work to do
 *
 *      file format -- first character in the line is a command
 *      rest of the line is the argument.
 *      valid commands are:
 *
 *              L -- "literal" contains identification info from
 *                    password file.
 *              I -- "indent"changes default indents driver
 *                   must have stty/gtty avaialble
 *              B -- "binary" uses raw line printer driver
 *                   must have stty/gtty avaialble
 *              F -- "formatted file" name of file to print
 *              U -- "unlink" name of file to remove (after
 *                    we print it. (Pass 2 only).
 *		R -- "pr'ed file" print file with pr
 *		H -- "header(title)" for pr
 *
 *      getline read line and expands tabs to blanks
 *
 */

/* pass 1 */
	while (getline()) switch (line[0]) {

//***	removed V2.1
//	case 'P':
//	/* acquire lineprinter file */
//		close(fo);
//		i = 0;
//again2:
//		if( (fo = open(&line[1],1)) < 0) {
//			if (i++)
//				printf("\r\n\7\7lpd:\7 Can't open %s\r\n",
//					&line[1]);
//				sleep(30); goto again2;
//		}
//		continue;
//***
	case 'J':
		if(line[1] != '\0' )
			for( hp = line+1, cp = jobname; *hp; *cp++ = *hp++);
		else
			for( hp = "              ", cp = jobname; *hp; *cp++ = *hp++);
		*cp++ = '\0';
	continue;
	case 'C':
		if(line[1] != '\0' )
			for( hp = line+1, cp = class; *hp; *cp++ = *hp++);
		else
			for( hp = "Case Ecmp Unix", cp = class; *hp; *cp++ = *hp++);
		*cp++ = '\0';
	continue;

	case 'H':	/* header title for pr */
		for (hp = line+1, cp = title; *hp; *cp++ = *hp++);
		*cp++ = '\0';
	continue;

	case 'L':
	/*
	 *      identification line. skip over the $ident<tab>
	 *      and grab the line. Set up header with user's
	 *      id and time info.  Then print header sheet.
	 *
	 */

#ifdef SQN
		/* generate a sequence number for printout */
		if((sf = open(sfile,2))<0) {
			sf = creat(sfile,0600);
			sqn = 1;}
			else {
			read(sf,&sqn,2);
			sqn =+ 1; seek(sf,0,0); }
		write(sf,&sqn,2);
		close(sf);
#endif

		/* put in user ident information */
		cp = line+1;

		while( (c = *cp++) !=':' && c );
		logname = cp;
		time(tvec);
		cp = ctime(tvec);
		write(fo, "\n\n\n", 3);
		banner(logname,jobname);
		if ( length(class) > 0 ) {
			write(fo,"\n\n\n",3);
			scan_out(fo,class,'\0');
		}
		write(fo, "\n\n\n\n\t\t\t\t\t     Job:  ", 20);
		writef(fo, jobname);
		write(fo, "\n\t\t\t\t\t     Date: ", 17);
		write(fo, ctime(tvec), 24);
#ifdef SQN
		write(fo, "\n\t\t\t\t\t     SQN:  ", 17);
		writef(fo, itoa(sqn));
#endif
		write(fo, "\n", 1);
		write(fo ,tof  ,1);
		continue;
	case 'I':
/*
 *      set indentation for use
 *
 */
	gtty(fo,&nlpmode);
	nlpmode.flag =| IND;
	indent1 = atoi(&line[1]);
	nlpmode.indent = indent1;
	stty(fo,&nlpmode);
	continue;


	case 'B':
/*
 *      print binary file (no format)
 *
 */
	gtty(fo,&lpmode);
	nlpmode.flag = lpmode.flag & ~((indent1?0:IND)|EJECT);
	nlpmode.width = lpmode.width;
	nlpmode.length = lpmode.length;
	nlpmode.indent = indent1;
	nlpmode.pad = 0;

	stty(fo,&nlpmode);
	sascii();
	stty(fo,&lpmode);
	title[0] = '\0';
	continue;


	case 'F':
	/* print formatted file */
		sascii();
		title[0] = '\0';
		continue;

	case 'R':
	/* print file using 'pr' */
		pr();
		title[0] = '\0';	/* get rid of title */
		continue;

	case 'U':
	/* unlink deferred to pass2 */
		continue;

	}
/*
 * Second pass.
 * Unlink files
 */
	seek(dfb[0], 0, 0);
	dfb[1] = 0;
	while (getline()) switch (line[0]) {

	default:
		continue;

	case 'U':
		unlink(&line[1]);
		continue;

	}
/*
 *      clean-up incase another daemon file exists
 */
	close(dfb[0]);
	unlink(file);
	aclose();
}

sascii()
{
	register n, f;
	static char buf[512];

/*
 *      print a file. name of file is in line starting in col 2
 *      does bulk reads and writes
 */
	f = open(&line[1], 0);
	write(fo ,tof  ,1);       /* start on a fresh page */
	while ((n=read(f, buf, 512))>0)
		write(fo ,buf,n);
	close(f);
}

/*
 *	pr - print a file using 'pr'
 */
pr()
{
	write(fo, tof, 1);	/* start on a new page */
	if (fork() == 0) {
		close(1);	/* change standard output */
		dup(fo);	/* to be the line printer */
		if (title[0])
			execl("/bin/pr", "pr", "-h", title, &line[1], 0);
		else
			execl("/bin/pr", "pr", &line[1], 0);
		exit(-1);
	} else {
		wait();		/* wait for pr to finish */
	}
}

int	linel;
getline()
{
	register char *lp;
	register c;

/*
 *      reads a line from the daemon file, removes tabs, converts
 *      new-line to null and leaves it in line. returns 0 at EOF
 */
	lp = line;
	linel = 0;
	while ((c = getc(dfb)) != '\n') {
		if (c<0)
			return(0);
		if (c=='\t') {
			do {
				*lp++ = ' ';
				linel++;
			} while ((linel & 07) != 0);
			continue;
		}
		*lp++ = c;
		linel++;
	}
	*lp++ = 0;
	return(1);
}

aclose(){
	register i;
/*
 *      fix by Neil Lewis and Dennis Mumaugh
 *      When opr invoked as second half of filter,
 *      pipe will be closed, and fd of 0 will be
 *      first available, instead of 1
 *      closes all open files except standard input and output
 */
	for( i=0; i<16; close(i++));
/*
 *      must open something as standard input
 *
 */
	open("/dev/tty",0); /* standard input */
	/* use console terminal as error reporting device */
	open("/dev/tty8",1); /* standard output */
}

/*
name:
	banner

function:
	To write two lines of header for a burst page.
	This routine writes two lines on the file descriptor fo.
	The lines written are given as two arguments.

algorithm:

*/

#define HEIGHT 9        /* height of characters */
#define WIDTH 8         /* width of characters */
#define DROP 3          /* offset to drop characters with descenders */
#define linelen 132	/* length of the printable line width */


#define	bckgnd	' '

extern char scnkey[];

banner (name1, name2) char *name1, *name2;
{
	extern int fo;
	register int i;

	scan_out ( fo, name1, '\0' );
	writef(fo, "\n\n");
	scan_out ( fo, name2, '\0' );
	return fo;
}


char *scnline(key, ptr, c)
char key;
char *ptr;
char c;
{
	register char *p;
	register char scndot;
	register scnwidth;

	p = ptr;
	scndot = key;
	for( scnwidth = WIDTH; --scnwidth; )
	{
		scndot=<< 1;
		*p++ = scndot & 0200 ? c : bckgnd;
	}
	return p;
}


#define TR(q)	((q)-' ')&0177
/*
name:
	scan_out

function:
	To write one line of large characters on a page

arguments:
	scfd   file descripter to write on
	scsp   string point of string to write
	dlm    delimeter of end of string

algorithm:

*/

scan_out ( scfd, scsp, dlm )
char	*scsp;
int	scfd;
char	dlm;
{
	register char *strp;
	register nchrs;
	register j;
	char	outbuf[linelen+1];
	int	d;
	int	scnhgt;
	char	*sp;
	char	c;
	char	cc;

	for( scnhgt = 0; scnhgt++ < HEIGHT+DROP; )
	{

		strp = &outbuf[0];
		sp = scsp;
		for( nchrs = 0;; )
		{
			d = dropit(c = TR(cc = *sp++));
			if ( (!d && scnhgt>HEIGHT ) || (scnhgt<=DROP && d) )
			for(j=WIDTH; --j;)	*strp++ = bckgnd;
			else
				strp = scnline(
					scnkey[c*HEIGHT+scnhgt-1-d],/* should be 2-d array*/
					strp, cc
					);
			if( *sp==dlm || *sp=='\0' || nchrs++>=linelen/(WIDTH+1)-1 ) break;
			*strp++ = bckgnd; *strp++ = bckgnd;
		}

		while(*--strp==' ');	strp++;
		*strp++ = '\n';	
		write( scfd, outbuf, strp-outbuf );
	}
}

dropit(c)		char c;
{
	switch(c)
	{
		case TR('_'):
		case TR(';'):
		case TR(','):
		case TR('g'):
		case TR('j'):
		case TR('p'):
		case TR('q'):
		case TR('y'):

			return DROP;

		default:
			return 0;
	}
}
/*
name:
	writef and writef1 and writenl

function:
	Write a null terminated string without the terminator.  (Writef1 is
	a printf substitute which is adequate and faster when only strings are
	printed--as in this program).  (Writeln writes a new line on the
	standard output.)

algorithm:
	Determine  the  length  of  the  string.  Write  it  out.
	(Writef1 writes to standard output only.)
CAVEAT This call is not buffered, so use it carefully or pay  the
	cost  of  high  I/O  expense.  It was writted to decrease
	subroutine linkage global variable referencing and module
	size.  But  those  gains  can easily be lost if writef is
	consistantly called with small strings where buffered i/o
	would be possible.

parameters:
	int	file descriptor of output file (not present in writef1 or writenl).
	*char	pointer to null terminated string (not present in writenl).

returns:
	int	value returned by write.

globals:

calls:
	write()		(writef1 calls writef and writenl calls writef1)

called by:
	lotsa people

history:
	Initially coded by Mark Kampe 11/22/75
	Writef1 and writenl written by Rick Balocca 8/1/76
*/
writef(afd,str)		int	afd;	char*	str;
{
	register char*	r;
	register char*	s;

	s = str;
	r = s;
	while(*s++);

	return write(afd, r, --s-r);
}



writef1(str)	char*	str;
{
	return writef(1,str);
}



writenl()
{
	return writef1("\n");
}

/*
 *	length - number of chars in string
 */

length(s)
register char *s;
{
	register n;

	n = 0;
	while (*s++)
		n++;
	return(n);
}
