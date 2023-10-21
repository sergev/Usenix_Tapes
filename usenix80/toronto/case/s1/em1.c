#

/*
 * Unix 6
 * Editor
 * with QMC mods Feb. 76, by  George Coulouris
 * mods are:
	prompts (suppress with '-p' flag)
	",%,&, to display a screen full of context
	'x' - as 's' but interactive
	'n' flag when appended to 's' or 'x' commands prints number of replacements

 * also mods by jrh 26 Feb 76
	% == current file name in ! commands
	!! == repeat the last ! command you executed
	-e flag == "elfic" mode :-
		no "w", "r\n" commands, auto w before q

    More mods by George March 76:

	'o' command for text input with local editing via control keys
	'b' to set a threshold for automatic line breaks in 'o' mode.
	'h' displays a screen full of help with editor commands
		(the help is in /usr/lib/emhelp)

    Revised 5/77 by Steve Eisen of CUNY/UCC:
	^G (Give up) replaces ^H (Help) in 'o' mode.
	All of the following delete the preceding character in 'o' mode:
				^H	^X	#
	'fm' command sets default mode for writing.
		If the file exists, its mode is also changed.
		'fm' command without argument reports mode.
	'~' command is equivalent to '&' command, but leaves dot
		on the first line printed.
	'e' with no argument re-edits the current file.


    Revised 9/78 by Bill Shannon of Case/Ecmp:

	-w flag to suppress the automatic Write ??? message before quit
	Line numbers can be turned on/off by a # command.
	The k command is improved -- 'A => 'a,'b   etc., and k?
		prints all current line marks.
	The command, ac, appends a comment to the end of the line, and
		if followed by any ten characters, defines a new comment
		delimiter.  The default is //.
	The write append feature allows one to append text to the bottom
		of some file other than the one being edited.  The command
		is: w >filename.
	:,&,",~,% can now be used with a variable number of lines.  This is
		accomplished as "n, &n, etc. where n is an integer.
	The ':' command looks forward n lines and leaves dot set
		where it was.
	The 'u' command allows the user to define a string of commands 
		to be invoked by a u.  u? prints the current user command.
	The 'n' command takes input from a user-specified file which contains
		one or more commands.  n? prints the name of the file accessed
		by the n command.

 */

/* this file contains all of the code except that used in the 'o' command.
	that is in a second segment called em2.c */

/* screen dimensions */
#define LINES	18
#define LENGTH	79
#define SPLIT	'-'
#define PROMPT	'>'
#define CONFIRM	'.'
#define SCORE	"^"
#define	SIGHUP	1
#define	SIGINTR	2
#define	SIGQUIT	3
#define	FNSIZE	64
#define	LBSIZE	512
#define	ESIZE	128
#define	GBSIZE	256
#define USRSIZE	512
#define	NBRA	5
#define	EOF	-1

#define	CBRA	1
#define	CCHR	2
#define	CDOT	4
#define	CCL	6
#define	NCCL	8
#define	CDOL	10
#define	CEOF	11
#define	CKET	12

#define	STAR	01

#define	error	errfunc()
#define	READ	0
#define	WRITE	1


#define UNIXBUFL 100

#define	putd()	putn(10)
#define	puto()	putn(8)

extern int margin;	/* used to set threshold in 'open' */

int	mode	0640;
int	elfic	0;	/* true if "elfic" (-e) flag */
int	firstime	1;	/* ugh - used to frigg initial "read" */
int	peekc;
int	lastc;
int	nlns;
char	unixbuffer [UNIXBUFL];
int	statbuf[18];
char	savedfile[FNSIZE];
char	file[FNSIZE];
char	nsavdfile[FNSIZE];
char 	usrbuf[USRSIZE];
char	linebuf[LBSIZE];
char	rhsbuf[LBSIZE/2];
char	expbuf[ESIZE+4];
int	circfl;
int	*zero;
int	inputfd	0;
int	*dot;
int	*dol;
int	*endcore;
int	*fendcore;
int	*addr1;
int	*addr2;
int	i;
char	genbuf[LBSIZE];
int	count[2];
char	*nextip;
char	*linebp;
char	commentstr[11] "\t// " ;
int	modflag   0;
int	ninbuf;
int	io;
char	*nflptr;
int	pflag;
int	onquit;
int	vflag	0;
int	xflag	0;	/*used in 'xchange' command */
int	savflag	0;	/* for the next 'n' command */
int	nflag	0;	/* line numbers */
int	mflag	1;    /* used for quit-write??? feature */
int	initread   1;
int	listf;
int	col;
char	*globp;
char	*ucp;
int	tfile	-1;
int	tline;
char	*tfname;
char	*loc1;
char	*loc2;
char	*locs;
char	ibuff[512];
int	iblock	-1;
char	obuff[512];
int	oblock	-1;
int	ichanged;
int	nleft;
int	errfunc();
char	TMPERR[] "TMP";
char	*braslist[NBRA];
char	*braelist[NBRA];
int	*names[26], highmark;
char	namet[26]; // "pointer" to names
int	fileappend;

main(argc, argv)
char **argv;
{
	register char *p1, *p2;
	extern int onintr();

	nlns = LINES;
	onquit = signal(SIGQUIT, 1);
	if(*(*argv+1) == 'm') vflag = 1;
	if(*(*argv+1) == 'n') nflag=vflag= 1;
	argv++;
	if (argc > 1 && **argv=='-') {
		p1 = *argv+1;
		while (*p1) {
			switch (*p1++) {
		case 'q':
				signal(SIGQUIT, 0);
				break;
		case 'e':
				elfic = 1;
				break;
		case 'p':
				vflag = 0;
				break;
		case 's':
				vflag = -1;
				break;
		case 'w':
				mflag = 0;
				break;
			}
		}
		if (!(*argv)[1])
			vflag = -1;
		argv++;
		argc--;
	}
	if (argc>1) {
		p1 = *argv;
		p2 = savedfile;
		while (*p2++ = *p1++);
		if (stat(savedfile, statbuf) >= 0)
			mode = statbuf[2]&07777;
		breaks(p1-3);
		globp = "r";
	}
	fendcore = sbrk(0);
	if (vflag>0) puts("Editor");
	init();
	if ((signal(SIGINTR, 1) & 01) == 0)
		signal(SIGINTR, onintr);
	setexit();
	commands(vflag);
	unlink(tfname);
}

commands(prompt)
{
	int getfile(), gettty();
	register *a1, c;
	register char *p;
	char *p1,*p2;
	int fd, r, n;
	int m;

	for (;;) {
	if (prompt>0 && globp == 0) {
		if (!globp && !pflag && nflag) nputlin(dot,1);
	else
		if (!pflag) putch(PROMPT);
	}
	if (pflag) {
		pflag = 0;
		addr1 = addr2 = dot;
		goto print;
	}
	addr1 = 0;
	addr2 = 0;
	xflag = 0;
	do {
		addr1 = addr2;
		if ((a1 = address())==0) {
			c = getchar();
			break;
		}
		addr2 = a1;
		if ((c=getchar()) == ';') {
			c = ',';
			dot = a1;
		}
	} while (c==',');
	if (addr1==0)
		addr1 = addr2;
	if (c>= 'A' && c<= 'Z')
		c =| 040;
	switch(c) {

	case 'a':
		setdot();
		if (getchar() == 'c') {
			for (i = 0; i < 10 && ((c = getchar()) != '\n'); i++)
				commentstr[i] = c;
			if (i) commentstr[i] = 0;
			if (i >= 10) gobble();
			commedit();
			continue;
		}
		append(gettty, addr2);
		continue;

	case 'b':
			if((c=peekc=getchar())== '+' || c =='-')
				peekc = 0;
				else if(c != '\n') error;
			margin = c == '-' ? LBSIZE - 40 : LENGTH - 20;
			newline();
			continue;

	case 'c':
		setdot();
		newline();
		delete();
		append(gettty, addr1-1);
		continue;

	case 'd':
		setdot();
		newline();
		delete();
		continue;

	case 'e':
		if (elfic)
			error;
		setnoaddr();
		if (modflag && mflag) {
			puts("Write\07???");
			gobble();
			modflag = 0;
			continue;
		}
		if ((peekc = getchar()) == ' ')
			savedfile[0] = 0;
		init();
		addr2 = zero;
		goto caseread;

	case 'f':
		if (elfic)
			error;
		setnoaddr();
		if ((c = getchar()) == 'm')  {
			m = 0;
			if ((c = getchar()) != '\n')  {
				if (c != ' ') error;
				while ((c = getchar()) == ' ');
				if (c == '\n') error;
				for (m=0; c!='\n'; )  {
					if (c < '0' || c > '7')
						error;
					m = (m<<3) | c - '0';
					c = getchar();
				}
				if (savedfile[0] != 0 && stat(savedfile, statbuf) >= 0)
					if (chmod(savedfile, m) < 0)  {
						puts("chmod access denied");
						continue;
					}
				mode = m;
			}
			count[1] = mode;
			puto();
			putchar('\n');
			continue;
		}
		else if (c != '\n') {
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
		newline();
		if((fd = open("/usr/lib/emhelp",0))<0) {
			puts("/usr/lib/emhelp not found");
			continue;
		}
			while (n = read( fd, linebuf, 512))
				write(1, linebuf, n);
			close( fd);
			continue;

	case 'i':
		setdot();
		nonzero();
		newline();
		append(gettty, addr2-1);
		continue;

	case 'k':
		if ((c = getchar()) == '?') { 
			prmark();
			newline();
			continue;
		}
		if (c < 'a' || c > 'z')
			error;
		newline();
		setdot();
		nonzero();
		if (namet[c - 'a'] == 26)
			namet[c - 'a'] = highmark++;
		names[namet[c - 'a']] = addr1;
		if (addr1 != addr2) {
			if (namet[c - '`'] == 26)
				namet[c - '`'] = highmark++;
			names[namet[c - '`']] = addr2;
		}
		continue;

	case 'm':
		move(0);
		continue;

	case 'n':
		if (inputfd != 0) error;
		if ((c = getchar()) == '?') {
			puts(nsavdfile);
			newline();
			continue;
		}
		else if (c == ' '){
			for (i=0; (c = getchar()) != '\n'; i++)
				nsavdfile[i] = c;
			nsavdfile[i] = 0;
		} else
		if (c != '\n')
			error;
		savflag = prompt;
		prompt = 0;
		inputfd = open(nsavdfile,0);
		continue;

	case '\n':
		if (addr2==0)
			addr2 = dot+1;
		addr1 = addr2;
		goto print;

	case 'l':
		listf++;
	case 'p':
		newline();
	print:
		setdot();
		nonzero();
		a1 = addr1;
		do {
			nputlin(a1,0);
			puts(getline(*a1++));
		}
		while (a1 <= addr2);
		dot = addr2;
		listf = 0;
		continue;

	case 'o':
		modflag++;
		setdot();
		op(globp);
		continue;

	case 'q':
		setnoaddr();
		newline();
		if (elfic) {
			firstime = 1;
			goto writeout;
		}
		if (modflag && mflag) {
			puts("Write\07???");
			modflag = 0;
			continue;
		}
		unlink(tfname);
		exit();

	quitit:
		unlink(tfname);
		exit();

	case 'r':
		modflag++;
	caseread:
		filename();
		if ((io = open(file, 0)) < 0) {
			lastc = '\n';
			error;
		}
		setall();
		ninbuf = 0;
		append(getfile, addr2);
		if (c == 'e' || initread) {
			modflag = 0;
			initread = 0;
		}
		exfile();
		continue;

	case 'x':
		xflag = 1;
	case 's':
		setdot();
		nonzero();
		substitute(globp);
		xflag = 0;
		continue;

	case 't':
		move(1);
		continue;

	case 'u':
		setdot();
		if ((c = getchar()) == '?') {
			puts(usrbuf);
			newline();
		}
		else if (c != '\n')
			setusrcom();
		else 
			exusrcom(addr1,addr2);
		continue;

	case 'v':
		global(0);
		continue;

	case 'w':
		if (elfic)
			error;
	writeout:
		fileappend = 0;
		setall();
		nonzero();
		if (elfic) {
			p1 = savedfile;
			if (*p1==0)
				error;
			p2 = file;
			while (*p2++ = *p1++);
		}
		else
			filename();
		if (fileappend) {
			if ((io = open(file,2)) < 0)
				if ((io = creat(file,mode)) < 0)
					error;
				seek(io,0,2);   // end
		} else {
		if ((io = creat(file, mode)) < 0)
			error;
		modflag = 0;
		}
		putfile();
		exfile();
		if (elfic)
			goto quitit;
		continue;

	case ':':
	case '"':
		gnlns();
		setdot();
		dot = addr1;
		if (dot == dol) error;
		addr1 = dot+1;
		addr2 = dot + nlns;
		addr2 = addr2>dol? dol: addr2;
	outlines:
		a1 = addr1-1;
		while (++a1 <= addr2) {nputlin(a1,0); puts(getline(*a1));}
		if (c == '~')	dot = addr1;
		else if (c == ':') dot = addr1 - 1;
		else		dot = addr2;
		continue;

	case '~':
	case '&':
		gnlns();
		setdot();
		nonzero();
		dot = addr1;
		addr1 = dot - nlns; 
		addr2 = dot;
		addr1 = addr1>zero? addr1: zero+1;
		goto outlines;


	case '%':
		gnlns();
		setdot();
		nonzero();
		dot = addr1;
		addr1 = dot - (nlns/2 - 2);
		addr2 = dot + (nlns/2 - 2);
		addr1 = addr1>zero? addr1 : zero+1;
		addr2 = addr2>dol? dol : addr2;
		a1 = addr1 - 1;
		while(++a1 <= addr2) {
			if (a1 == dot) screensplit();
			nputlin(a1,0);
			puts(getline(*a1));
			if (a1 == dot) screensplit();
		}
		continue;

	case '>':
		vflag = vflag>0? 0: vflag;
		newline();
		reset();

	case '<':
		vflag = 1;
		newline();
		reset();

	case '#':
		nflag = !nflag;
		newline();
		reset();

	case '=':
		setall();
		newline();
		count[1] = (addr2-zero)&077777;
		putd();
		putchar('\n');
		continue;

	case '!':
		unix();
		continue;

	case EOF:
		if(prompt == -2 || ttyn(0) == 'x') return;
		if (inputfd != 0) {
			close(inputfd);
			inputfd = 0;
			vflag = savflag;
			reset();
		}
		continue;

	}
	error;
	}
}

address()
{
	register *a1, minus, c;
	int n, relerr;

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
			compile(c);
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
					{putchar('?'); error;}
						/* two '?'s for failed search */
			}
			break;
	
		case '$':
			a1 = dol;
			break;
	
		case '.':
			a1 = dot;
			break;

		case '`':
		case '\'':
			if ((c = getchar()) < 'a' || c > 'z' || (c=namet[c - 'a']) == 26)
	                         if (c < 'a') {  /* special range:  'A => 'a,'b; 'C => 'c,'d ... */
					c = c - 'A';
					addr1 = names[namet[c]];       // 'a
	                                a1 = names[namet[c + 1]]; // 'b
					break;
				} else
					error;
			a1 = names[c];
			break;

		default:
			peekc = c;
			if (a1==0)
				return(0);
			a1 =+ minus;
			if (a1<zero)
				error;
			a1 = (a1>dol ? dol : a1);
			return(a1);
		}
		if (relerr)
			error;
	}
}

setdot()
{
	if (addr2 == 0)
		addr1 = addr2 = dot;
	if (addr1 > addr2)
		error;
}

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

setnoaddr()
{
	if (addr2)
		error;
}

nonzero()
{
	if (addr1<=zero || addr2>dol)
		error;
}

newline()
{
	register c;

	if ((c = getchar()) == '\n')
		return;
	c = c >= 'A' && c <= 'Z' ? c + 32 : c;
	if (c=='p' || c=='l') {
		pflag++;
		if (c=='l')
			listf++;
		if (getchar() == '\n')
			return;
	}
	error;
}

filename()
{
	register char *p1, *p2;
	register c;

	count[1] = 0;
	c = getchar();
	if (c=='\n' || c==EOF) {
		if (elfic && !firstime)
			error;
		else
			firstime = 0;
		p1 = savedfile;
		if (*p1==0)
			error;
		p2 = file;
		while (*p2++ = *p1++);
		return;
	} else {
		if (c !=' ')
			error;
		while ((c = getchar()) == ' ') ;
		if (c == '>') {
			fileappend++;
			c = getchar();
		}
		if (c=='\n')
			error;
		p1 = file;
		do {
			*p1++ = c;
		} while ((c = getchar()) != '\n');
		*p1++ = 0;
		if (savedfile[0]==0) {
			p1 = savedfile;
			p2 = file;
			while (*p1++ = *p2++);
			if (stat(savedfile, statbuf) >= 0)
				mode = statbuf[2]&07777;
			breaks(p1 - 3);
		}
	}
}

breaks(p) char *p;

{
	if(*p++ == '.')
		if(*p == 'r' || *p == 'n') margin = LENGTH -20;
}

exfile()
{
	close(io);
	io = -1;
	if (vflag>=0) {
		putd();
		putchar('\n');
	}
}

onintr()
{
	signal(SIGINTR, onintr);
	putchar('\n');
	lastc = '\n';
	error;
}

errfunc()
{
	register c;

	if (inputfd != 0) {
		close(inputfd);
		inputfd = 0;
	}
	listf = 0;
	puts("?");
	count[0] = 0;
	seek(0, 0, 2);
	pflag = 0;
	if (globp)
		lastc = '\n';
	globp = 0;
	peekc = lastc;
	while ((c = getchar()) != '\n' && c != EOF);
	if (io > 0) {
		close(io);
		io = -1;
	}
	reset();
}

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
	if (read(inputfd, &lastc, 1) <= 0) {
		return(lastc = EOF);
	}
	lastc =& 0177;
	return(lastc);
}

gettty()
{
	register c, gf;
	register char *p;

	p = linebuf;
	gf = globp;
	if (!globp && !pflag && nflag) {
		putchar('\t');
		putchar(0);
	}
	while ((c = getchar()) != '\n') {
		if (c==EOF) {
			if (gf)
				peekc = c;
			return(c);
		}
		if ((c =& 0177) == 0)
			continue;
		*p++ = c;
		if (p >= &linebuf[LBSIZE-2])
			error;
	}
	*p++ = 0;
	if (linebuf[0]=='.' && linebuf[1]==0)
		return(EOF);
	return(0);
}

getfile()
{
	register c;
	register char *lp, *fp;

	lp = linebuf;
	fp = nextip;
	do {
		if (--ninbuf < 0) {
			if ((ninbuf = read(io, genbuf, LBSIZE)-1) < 0)
				return(EOF);
			fp = genbuf;
		}
		if (lp >= &linebuf[LBSIZE])
			error;
		if ((*lp++ = c = *fp++ & 0177) == 0) {
			lp--;
			continue;
		}
		if (++count[1] == 0)
			++count[0];
	} while (c != '\n');
	*--lp = 0;
	nextip = fp;
	return(0);
}
 
putfile()
{
	int *a1;
	register char *fp, *lp;
	register nib;

	nib = 512;
	fp = genbuf;
	a1 = addr1;
	do {
		lp = getline(*a1++);
		for (;;) {
			if (--nib < 0) {
				if (write(io, genbuf, fp-genbuf) != fp-genbuf) wrterr();
				nib = 511;
				fp = genbuf;
			}
			if (++count[1] == 0)
				++count[0];
			if ((*fp++ = *lp++) == 0) {
				fp[-1] = '\n';
				break;
			}
		}
	} while (a1 <= addr2);
	if (write(io, genbuf, fp-genbuf) != fp-genbuf) wrterr();
}

append(f, a)
int (*f)();
{
	register *a1, *a2, *rdot;
	int nline, tl, ntl;
	struct { int integer; };

	modflag++;
	nline = 0;
	dot = a;
	while ((*f)() == 0) {
		if (dol >= endcore) {
			if (sbrk(1024) == -1)
				error;
			endcore.integer =+ 1024;
		}
		tl = putline();
		nline++;
		a1 = ++dol;
		a2 = a1+1;
		rdot = ++dot;
		while (a1 > rdot)
			*--a2 = *--a1;
		for (ntl = 0; ntl < highmark; ntl++)
			if (names[ntl] >= rdot)
				names[ntl]++;
		*rdot = tl;
	}
	return(nline);
}

unix()
{
	register savint, pid, rpid;
	int retcode;
	char c,*lp,*fp;
	pid = 0;
	if ((c=getchar ()) != '!') {
		lp = unixbuffer;
		do {
			if (c != '%')
				*lp++ = c;
			else {
			pid = 1;
				fp = savedfile;
				while ((*lp++ = *fp++));
				lp--;
			}
			c = getchar();
		} while (c != '\n');
		*lp = '\0';
	}
	else { pid = 1;
		while (getchar () != '\n');}
	if(pid) {
		putchar('!');
		puts(unixbuffer);
	}
	setnoaddr();
	if ((pid = fork()) == 0) {
		signal(SIGQUIT, onquit);
		execl ("/bin/sh", "sh", "-c", unixbuffer, 0);
		exit();
	}
	savint = signal(SIGINTR, 1);
	while ((rpid = wait(&retcode)) != pid && rpid != -1);
	signal(SIGINTR, savint);
	puts("!");
}

delete()
{
	register *a1, *a2, *a3;
	int tl;

	modflag++;
	nonzero();
	a1 = addr1;
	a2 = addr2+1;
	for (tl=0; tl < highmark; tl++)
		if (names[tl] >= a2)
			names[tl] =- a2 - a1;
		else if (names[tl] >= a1)
			names[tl] = a1-1;
	a3 = dol;
	dol =- a2 - a1;
	do
		*a1++ = *a2++;
	while (a2 <= a3);
	a1 = addr1;
	if (a1 > dol)
		a1 = dol;
	dot = a1;
}

getline(tl)
{
	register char *bp, *lp;
	register nl;

	lp = linebuf;
	bp = getblock(tl, READ);
	nl = nleft;
	tl =& ~0377;
	while (*lp++ = *bp++)
		if (--nl == 0) {
			bp = getblock(tl=+0400, READ);
			nl = nleft;
		}
	return(linebuf);
}

putline()
{
	register char *bp, *lp;
	register nl;
	int tl;

	lp = linebuf;
	tl = tline;
	bp = getblock(tl, WRITE);
	nl = nleft;
	tl =& ~0377;
	while (*bp = *lp++) {
		if (*bp++ == '\n') {
			*--bp = 0;
			linebp = lp;
			break;
		}
		if (--nl == 0) {
			bp = getblock(tl=+0400, WRITE);
			nl = nleft;
		}
	}
	nl = tline;
	tline =+ (((lp-linebuf)+03)>>1)&077776;
	return(nl);
}

getblock(atl, iof)
{
	extern read(), write();
	register bno, off;
	
	bno = (atl>>8)&0377;
	off = (atl<<1)&0774;
	if (bno >= 255) {
		puts(TMPERR);
		error;
	}
	nleft = 512 - off;
	if (bno==iblock) {
		ichanged =| iof;
		return(ibuff+off);
	}
	if (bno==oblock)
		return(obuff+off);
	if (iof==READ) {
		if (ichanged)
			blkio(iblock, ibuff, write);
		ichanged = 0;
		iblock = bno;
		blkio(bno, ibuff, read);
		return(ibuff+off);
	}
	if (oblock>=0)
		blkio(oblock, obuff, write);
	oblock = bno;
	return(obuff+off);
}

blkio(b, buf, iofcn)
int (*iofcn)();
{
	seek(tfile, b, 3);
	if ((*iofcn)(tfile, buf, 512) != 512) {
		puts(TMPERR);
		error;
	}
}

init()
{
	register char *p;
	register pid;

	modflag = (globp ? -1 : 0);
	close(tfile);
	tline = 0;
	iblock = -1;
	oblock = -1;
	tfname = "/tmp/exxxxx";
	ichanged = 0;
	pid = getpid();
	for (p = &tfname[11]; p > &tfname[6];) {
		*--p = (pid&07) + '0';
		pid =>> 3;
	}
	close(creat(tfname, 0600));
	tfile = open(tfname, 2);
	brk(fendcore);
	dot = zero = dol = fendcore;
	endcore = fendcore - 2;
	for (pid=0; pid<26; pid++)
		namet[pid] = 26;	// init mark table
}

global(k)
{
	register char *gp;
	register c;
	register int *a1;
	char globuf[GBSIZE];

	if (globp)
		error;
	setall();
	nonzero();
	if ((c=getchar())=='\n')
		error;
	compile(c);
	gp = globuf;
	while ((c = getchar()) != '\n') {
		if (c==EOF)
			error;
		if (c=='\\') {
			c = getchar();
			if (c!='\n')
				*gp++ = '\\';
		}
		*gp++ = c;
		if (gp >= &globuf[GBSIZE-2])
			error;
	}
	*gp++ = '\n';
	*gp++ = 0;
	for (a1=zero; a1<=dol; a1++) {
		*a1 =& ~01;
		if (a1>=addr1 && a1<=addr2 && execute(0, a1)==k)
			*a1 =| 01;
	}
	for (a1=zero; a1<=dol; a1++) {
		if (*a1 & 01) {
			*a1 =& ~01;
			dot = a1;
			globp = globuf;
			commands(-2);
			a1 = zero;
		}
	}
}

setusrcom()
{
	register c;
	register int *a1;

	setall();
	nonzero();
	ucp = usrbuf;
	while ((c = getchar()) != '\n') {
		if (c == EOF)
			error;
		if (c == '\\') {
			c = getchar();
			if (c != '\n')
				*ucp++ = '\\';
		}
		*ucp++ = c;
		if (ucp >= &usrbuf[USRSIZE - 2])
			error;
	}
	*ucp++ = '\n';
	*ucp++ = 0;
}

exusrcom(addr1,addr2)
int *addr1, *addr2;
{
	register int *a1;

	for (a1=zero; a1<=dol; a1++) {
		*a1 =& ~01;
		if (a1 >= addr1 && a1 <= addr2)
			*a1 =| 01;
	}
	for (a1 = zero; a1 <= dol; a1++) {
		if (*a1 & 01) {
			*a1 =& ~01;
			dot = a1;
			globp = usrbuf;
			commands(-2);
			a1 = zero;
		}
	}
}

substitute(inglob)
{
	register gsubf, *a1, nl;
	int nflag, nn, getsub();

	gsubf = compsub();
	nflag = gsubf > 1 ? 1 : 0;
	nn = 0;
	gsubf =& 01;
	gsubf =| xflag;
	for (a1 = addr1; a1 <= addr2; a1++) {
		if (execute(0, a1)==0)
			continue;
		inglob =| 01;
		if (confirmed()) { dosub(); nn++; }
		else donothing();
		if (gsubf) {
			while (*loc2) {
				if (execute(1)==0)
					break;
		if(confirmed()) {  dosub(); nn++; }
		else donothing();
		}
	}
		*a1 = putline();
		nl = append(getsub, a1);
		a1 =+ nl;
		addr2 =+ nl;
	}
	if (inglob==0)
		{putchar('?'); error; }
			/* two queries distinguish failed match */
	if (nflag) {
		count[1] = nn;
		putd();
		putchar('\n');
	}
}

donothing() {
	char t1,t2;
			t1 = rhsbuf[0];
			t2 = rhsbuf[1];
			rhsbuf[0] = '&';
			rhsbuf[1] = 0;
			dosub();
			rhsbuf[0] = t1;
			rhsbuf[1] = t2;
}

confirmed()
{
int ch;
	if(xflag) {
		puts(linebuf);
		underline(linebuf, loc1, loc2, SCORE);
		ch = getchar();
		if ( ch != '\n') { while (getchar() != '\n');
				if ( ch != CONFIRM ) puts("? '.' to confirm");
		}
		return (ch == CONFIRM ? 1: 0);
		}
	return 1;
}


underline (line, l1, l2, score)
char *line, *l1, *l2, *score;
{
	char *ch, *ll; int i;
	register char *p;

	p = line;
	ch = " ";
	ll = l1;
	i = 2;
	while (i--) {
		while (*p && p < ll) {
			write (1, (*p == '\t' ? p : ch),1);
			p++;
		}
		ch = score;
		ll = l2;
	}
}

screensplit()
{
	register a;

		a = LENGTH;
		while(a--) putchar(SPLIT);
		putchar('\n');
}

compsub()
{
	register seof, c;
	register char *p;
	int gsubf;

	gsubf = 0;
	if ((seof = getchar()) == '\n')
		error;
	compile(seof);
	p = rhsbuf;
	for (;;) {
		c = getchar();
		if (c=='\\')
			c = getchar() | 0200;
		if (c=='\n')
			error;
		if (c==seof)
			break;
		*p++ = c;
		if (p >= &rhsbuf[LBSIZE/2])
			error;
	}
	*p++ = 0;
	if(((peekc = getchar())| 040) == 'g') {
		peekc = 0;
		gsubf =| 1;
	}
	if (((peekc = getchar())| 040)  == 'n') {
		peekc = 0;
		gsubf =| 2;
	}
	newline();
	return(gsubf);
}

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

dosub()
{
	register char *lp, *sp, *rp;
	int c;

	lp = linebuf;
	sp = genbuf;
	rp = rhsbuf;
	while (lp < loc1)
		*sp++ = *lp++;
	while (c = *rp++) {
		if (c=='&') {
			sp = place(sp, loc1, loc2);
			continue;
		} else if (c<0 && (c =& 0177) >='1' && c < NBRA+'1') {
			sp = place(sp, braslist[c-'1'], braelist[c-'1']);
			continue;
		}
		*sp++ = c&0177;
		if (sp >= &genbuf[LBSIZE])
			error;
	}
	lp = loc2;
	loc2 = sp + linebuf - genbuf;
	while (*sp++ = *lp++)
		if (sp >= &genbuf[LBSIZE])
			error;
	lp = linebuf;
	sp = genbuf;
	while (*lp++ = *sp++);
}

place(asp, al1, al2)
{
	register char *sp, *l1, *l2;

	sp = asp;
	l1 = al1;
	l2 = al2;
	while (l1 < l2) {
		*sp++ = *l1++;
		if (sp >= &genbuf[LBSIZE])
			error;
	}
	return(sp);
}

move(cflag)
{
	register int *adt, *ad1, *ad2;
	int getcopy();

	setdot();
	nonzero();
	if ((adt = address())==0)
		error;
	newline();
	ad1 = addr1;
	ad2 = addr2;
	if (cflag) {
		ad1 = dol;
		append(getcopy, ad1++);
		ad2 = dol;
	}
	modflag++;
	ad2++;
	if (adt<ad1) {
		dot = adt + (ad2-ad1);
		if ((++adt)==ad1)
			return;
		reverse(adt, ad1);
		reverse(ad1, ad2);
		reverse(adt, ad2);
	} else if (adt >= ad2) {
		dot = adt++;
		reverse(ad1, ad2);
		reverse(ad2, adt);
		reverse(ad1, adt);
	} else
		error;
}

reverse(aa1, aa2)
{
	register int *a1, *a2, t;

	a1 = aa1;
	a2 = aa2;
	for (;;) {
		t = *--a2;
		if (a2 <= a1)
			return;
		*a2 = *a1;
		*a1++ = t;
	}
}

getcopy()
{
	if (addr1 > addr2)
		return(EOF);
	getline(*addr1++);
	return(0);
}

compile(aeof)
{
	register eof, c;
	register char *ep;
	char *lastep;
	char bracket[NBRA], *bracketp;
	int nbra;
	int cclcnt;

	ep = expbuf;
	eof = aeof;
	bracketp = bracket;
	nbra = 0;
	if ((c = getchar()) == eof) {
		if (*ep==0)
			error;
		return;
	}
	circfl = 0;
	if (c=='^') {
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
				error;
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
	error;
}

execute(gf, addr)
int *addr;
{
	register char *p1, *p2, c;

	if (gf) {
		if (circfl)
			return(0);
		p1 = linebuf;
		p2 = genbuf;
		while (*p1++ = *p2++);
		locs = p1 = loc2;
	} else {
		if (addr==zero)
			return(0);
		p1 = getline(*addr);
		locs = 0;
	}
	p2 = expbuf;
	if (circfl) {
		loc1 = p1;
		return(advance(p1, p2));
	}
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
		error;
	}	
}

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

putn(n)
int n;
{
	register r;
	extern ldivr;

	count[1] = ldiv(count[0], count[1], n);
	count[0] = 0;
	r = ldivr;
	if (count[1])
		putn(n);
	putchar(r + '0');
}

puts(as)
char *as;

{
	register char *sp;

	sp = as;
	col = 0;
	while (*sp)
		putchar(*sp++);
	putchar('\n');
}

char	line[70];
char	*linp	line;

putchar(ac)
{
	register char *lp;
	register c;

	lp = linp;
	c = ac;
	if (listf) {
		col++;
		if (col >= 72) {
			col = 0;
			*lp++ = '\\';
			*lp++ = '\n';
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
		if (c<' ' && c!= '\n') {
			*lp++ = '\\';
			*lp++ = (c>>3)+'0';
			*lp++ = (c&07)+'0';
			col =+ 2;
			goto out;
		}
	}
	*lp++ = c;
out:
	if(c == '\n' || (c == 0 && lp--) || lp >= &line[64]) {
		linp = line;
		write(1, line, lp-line);
		return;
	}
	linp = lp;
}


commedit()
{
	register *a1, c;
	char *strp;
	for (a1=addr1; a1<= addr2; a1++) {
		linebp = getline(*a1);
		col = 0;
		while (*linebp)
			putchar(*linebp++);
		strp = commentstr;
		while (c = *strp++) {
			putchar(c);
			*linebp++ = c;
		}
		putchar(c);
		if ((c = getchar()) == '\n')
			continue;
		modflag++;
		*linebp++ = c;
	        while((*linebp++ = getchar()) != '\n') ;
		*linebp = 0;
		*a1 = putline();
	}
	dot = addr2;
	linebp = 0;
}

prmark()
{
	register i, c;
	int mod	0;

	mod = 0;
		for (i=0; i < 26; i++)
			if ((c=(namet[i]&0177)) < 26) {
				putchar(i + 'a');
				putchar('=');
				count[1] = (names[c] - zero) & 077777;
				putd();
				putchar(' ');
				mod++;
			}
		if (mod) putchar('\n');
}

nputlin(a1,bool)
register *a1;
int	bool;
{
	if (nflag) {
		count[1] = (a1 - zero)&077777;
		if (bool) {
			putd();
			putchar('>');
			putchar(0);
		} else {
			if (count[1] < 1000) putchar(' ');
			if (count[1] < 100) putchar(' ');
			if (count[1] < 10) putchar(' ');
			putd();
			putchar('\t');
		}
	}
}

gobble()
{
	register   c;
	do
		c = getchar();
	while (c != '\n');
}
wrterr()
{
	puts("write error! \07");
	error;
}

gnlns()
{

	register char c;

	if ((peekc = getchar()) >= '0' && peekc <= '9') {
	nlns = 0;
	c = peekc;
	peekc = 0;
		while (c >= '0' && c <= '9') {
			nlns =* 10;
			nlns =+ (c - '0');
			c = getchar();
		}
	peekc = c;
	} 
	newline();
}
