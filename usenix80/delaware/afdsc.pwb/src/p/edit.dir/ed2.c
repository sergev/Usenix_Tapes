#
#include	"./externals.h"
/*
Name:
	address

Function:
	Determine an address (line number).

Algorithm:
	Scan input for number, symbol, expression, or marked line.

Parameters:
	None.

Returns:
	Binary address or error reset

Files and Programs:
	None.


*/
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
					ERROR;
			}
			break;
	
		case '$':
			a1 = dol;
			break;
	
		case '.':
			a1 = dot;
			break;

		case '\'':
			if ((c = getchar()) < 'a' || c > 'z')	/* marked line variable */
				ERROR;	/* variable name valid? no */
			for (a1=zero; a1<=dol; a1++)
				if (names[c-'a'] == (*a1|01))	/* variable allocated */
					break;
			break;
	
		default:
			peekc = c;
			if (a1==0)
				return(0);
			a1 =+ minus;
			if (a1<zero) a1=zero;
			else if (a1>dol) a1=dol;
			return(a1);
		}
		if (relerr)
			ERROR;
	}
}

/*

Name:
	setdot

Function:
	Assign value of address to that of dot.

Algorithm:
	If second address is not given, set first and second address to dot.
	Check range of dot and address.

Parameters:
	None.

Returns:
	None or error reset.

Files and Programs:
	None.


*/
setdot()
{
	if (addr2 == 0) {
		if (dot>dol) ERROR;
		addr1 = addr2 = dot;
		}
	if (addr1 > addr2)
		ERROR;
}

/*

Name:
	setall

Function:
	Set addresses to full length of file.

Algorithm:
	If end address is zero, set both addresses to beginning and end of file,
	then call setdot() to adjust current line number.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


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

Name:
	setnoaddr

Function:
	Ensure a second address has not been entered.

Algorithm:
	If a second address has been entered, reset with error message.

Parameters:
	None.

Returns:
	None or error reset

Files and Programs:
	None.


*/
setnoaddr()
{
	if (addr2)
		ERROR;
}

/*

Name:
	nonzero

Function:
	Check for illegal addresses.

Algorithm:
	If either address is outside the size of the file, adjust it in bounds
	(unless it's the first).  Return an error if the addresses are reversed
	(may not be intentional, so don't reverse them automatically).

Parameters:
	None.

Returns:
	None or error reset.

Files and Programs:
	None.


*/
nonzero()
{
	if (addr1 > dol) ERROR;
	if (addr1 <= zero) addr1 = zero+1;
	if (addr2 > dol) addr2 = dol;
	if (addr1>addr2) ERROR;
}

/*

Name:
	oldnewline

Function:
	Scan for legal commands to end of line.

Algorithm:
	If more on line, scan for valid print commands.  Anything else is an error.

Parameters:
	None.

Returns:
	None or error reset.

Files and Programs:
	None.


*/
oldnewline()
{
	register c;

	if ((c = getchar()) == '\n')
		return;
	pflag++;
	if((c == 'l') || (c == 'L'))
		listf = 1;
	else if((c == 'p') || (c == 'P'));
	else if((c == 'n') || (c == 'N'))
		listf = 2;
	else ERROR;
	if (getchar() != '\n')
		ERROR;
	return;
}

/*

Name:
	filename

Function:
	Get either a remembered or input filename.

Algorithm:
	If no filename is input, use the one already saved earlier.  Otherwise, skip
	spaces and look for file name.  Copy it to save area.

Parameters:
	None.

Returns:
	None or error reset.

Files and Programs:
	None.


*/
filename()
{
	register char *p1, *p2;
	register c;

	count[1] = 0;
	c = getchar();
	if (c=='\n' || c==EOF) {
		p1 = whichfile;
		if (*p1==0)
			ERROR;
		p2 = file;
		while (*p2++ = *p1++);
		return;
	}
	if (c!=' ')
		ERROR;
	while ((c = getchar()) == ' ');
	if (c=='\n')
		ERROR;
	p1 = file;
	do {
		*p1++ = c;
	} while ((c = getchar()) != '\n');
	*p1++ = 0;
	if (whichfile[0]==0) {
		p1 = whichfile;
		p2 = file;
		while (*p1++ = *p2++);
	}
}

/*

Name:
	exfile

Function:
	Exit the current file.

Algorithm:
	Close the file, set its file descriptor (io) to -1, and print the character count.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
exfile()
{
	close(io);
	io = -1;
	if (vflag) {
		putd(&count);
		putchar('\n');
	}
}

/*

Name:
	getzlen

Function:
	Get 'z' command parameters.

Algorithm:
	After blanks, look for mode character.  Then look for and save page sizeat the
	end of the line.  Then adjust address according to size and mode.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
getzlen()
{
	register char c,zmode;
	register int i;
	while ((c=getchar()) == ' ');
	if (c == '.' || c == '-' || c== '+') {
		zmode = c;
		c = getchar();
		}
	else zmode = '+';
	if (c>='0' && c<= '9') {
		i = c-'0';
		while ((c=getchar()) >= '0' && c <= '9')
			i = i*10 + (c-'0');
		}
	else i = zlength;
	peekc = c;
	newline();
	zlength = i;
	if (addr2 == 0) addr1 = dot;
	switch (zmode) {
		case '.': addr1 =- zlength/2;
			break;
		case '-': addr1 =- zlength-1;
			break;
		}
	addr2 = addr1 + zlength-1;
	}

/*

Name:
	onintr

Function:
	Catch interrupt signal.

Algorithm:
	Reset signal, print error message, reset to command level.

Parameters:
	None.

Returns:
	Error reset

Files and Programs:
	None.


*/
onintr()
{
	signal(SIGINTR, onintr);
	putchar('\n');
	ERROR;
}

/*

Name:
	onhup

Function:
	Catch hangup signal and save file.

Algorithm:
	Construct write editor command with ed-hup filename.  Call commands to process
	the write.  Delete temporary files.  Exit the editor.

Parameters:
	None.

Returns:
	-1 exit code.

Files and Programs:
	None.


*/
onhup()
{
	char   hupp[20];
	char   *pt;
	
	signal(SIGHUP,1);
	signal(SIGINTR,1);
	signal(SIGQUIT,1);
	pt = &hupp;
	*pt++ = 'w';
	*pt++ = ' ';
	while(*hup)	*pt++ = *hup++;
	*pt++ = '\n';
	*pt = '\0';
	globp = &hupp;
	push(&curdepth);
	commands(1);
	pop(&curdepth);
	rmfiles();
	exit(-1);
}

/*

Name:
	dontgo

Function:
	Notify user that modified file was not saved.

Algorithm:
	Act like the file was written (so 2nd time won't come here) and tell user
	that he hasn't saved file yet.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
dontgo()
{
	register char c;
	fchanged = 0;
	errfunc("Modified file not written out");
}

/*

Name:
	errfunc

Function:
	Handle general errors.

Algorithm:
	Print error message, go to end of file, reset flags, close current file and
	reset input if 'x' (next) file is in effect.  Set terminal back to normal
	mode and reset to commad level.

Parameters:
	Error message pointer.

Returns:
	Error reset

Files and Programs:
	None.


*/
errfunc(s)
char *s;
{

	listf = 0;
	puts(s);
	count[0] = 0;
	seek(0, 0, 2);
	pflag = 0;
	globp = 0;
	if (io > 0) {
		close(io);
		io = -1;
	}
	peekc = 0;
	if(curdepth == 2 || curdepth == 3) {
		write(1,"?Error in x file?\n",18);
		close(0);
		dup(savin);
		close(savin);
	}
	ttmode[2] = ttnorm;
	stty(0,ttmode);		/* reset tty if necessary */
				/* and flush all pending input */
	stackptr = &deepstk[0];		/* reset stack pointer */
	reset();
				/* returns to top command level */
}

/*

Name:
	getchar

Function:
	Get a character from input stream.

Algorithm:
	If in column shorthand, get next character in column search string.
	If character was read (peeked at) and pushed back, get it.
	If doing global commands, get next character in global command line
	buffer.  Otherwise, read character from stadnard input.

Parameters:
	None.

Returns:
	A charcter or EOF

Files and Programs:
	None.


*/
getchar()
{
	char c;
	if(colread)
	{
		if((c = *colbufp++) == '\0')	colread = 0;
		else	return(c&0177);
	}
	if (c=peekc) {
		peekc = 0;
		return(c);
	}
	if (globp) {
		if ((c = *globp++) != 0)
			return(c);
		globp = 0;
		return(EOF);
	}
	if (read(0, &c, 1) <= 0)
		return(c = EOF);
	c =& 0177;
	return(c);
}

/*

Name:
	gettty

Function:
	Get line from input stream.

Algorithm:
	If finishing immediate mode i,c, or a, fake a period to end input mode.
	Otherwise, take in a line from getchar().  In 'y' mode, simulate the 
	character and line kill of the tty driver and handle escaping a character
	with a backslash.  Also check for going over threshold (hot zone) and start
	new line.  If any line is toolong, exit with error message.  Finish line
	with null.  If immediate mode, turn it off and set variable to finish
	immediate mode next time through.  Check for end of input mode before
	returning and adjust return value accordingly.

Parameters:

	None.
Returns:
	0 = normal
	EOF = end of input mode
	Error reset

Files and Programs:
	None.


*/
gettty()
{
	register c, gf;
	char c2;
	register char *p;
	char *threshold;

	threshold = linebuf + margin;
	p = linebuf;
	gf = globp;
	if(fakend) {
		fakend = 0;
		*p++ = '.';
	}
	else {
		while ((c = getchar()) != '\n') {
			if(yflag) {
				c2 = c & 0177;
				if(c2 == (ttmode[1] & 0177)) {
					if(p != &linebuf[0]) {
						--p;
						write(1,&c2,1);
					}
					continue;
				}
				if(c2 == ((ttmode[1] >> 8) & 0177)) {
					p = linebuf;
					write(1,&c2,1);
					write(1,"\n",1);
					continue;
				}
				if(c2 == '\\') {
					write(1,"\\",1);
					c = getchar();
					if(c != (ttmode[1] & 0177) && c != ((ttmode[1] >> 8) & 0177))		*p++ = '\\';
				}
				c2 = c;
				write(1, &c2, 1);
			}
			if (c==EOF) {
				if (gf)
					peekc = c;
				return(c);
			}
			if ((c =& 0177) == 0)
				continue;
			if((c == ' ') && (p > threshold)) {
				if(!yflag)	putchar('\n');
				break;
			}
			*p++ = c;
			if (p >= &linebuf[LBSIZE-2])
				ERROR;
		}
	}
	if(yflag && !nostty)	write(1,"\n",1);
	*p++ = 0;
	if(imedmode) {
		imedmode = 0;
		fakend++;
	}
	if (linebuf[0]=='.' && linebuf[1]==0) 
		return(EOF);
	return(0);
}

/*

Name:
	getfile

Function:
	Read next line from current file.

Algorithm:
	Use buffered input, scan to next line, and copy into line buffer.

Parameters:
	None.

Returns:
	0 = normal
	EOF
	Error reset

Files and Programs:
	None.


*/
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
			errfunc(BUFERR);
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

/*

Name:
	putfile

Function:
	Put line(s) to current file.

Algorithm:
	Copy addressed line(s) to line buffer, and write out blocks.

Parameters:
	None.

Returns:
	None or error reset.

Files and Programs:
	None.


*/
putfile()
{
	int *a1;
	register char *fp, *lp;
	register int i;
	int putsigsav;

	putsigsav = signal(SIGINTR,1);
	i = 0;
	fp = genbuf;
	a1 = addr1;
	lp = getline(*a1);
	for (;;) {
		/* i = fp-genbuf;
		   fp points to next place to put char to output
		   genbuf[0..i-1] contains chars created to output
		   a1 = line number of current line being moved
		   lp points to next character to move
		   count[0,1] contains total # of bytes moved
		*/
		if (++count[1] == 0)
			++count[0];
		i++;
		if ((*fp++ = *lp++) == 0) {
			fp[-1] = '\n';
			if (a1++ >= addr2) break;
			lp = getline(*a1);
		}
		if ( i == 512) {
			if (write(io,genbuf,i) != i) {
				signal(SIGINTR,putsigsav);
				errfunc(IOERR);
			}
			fp = genbuf;
			i = 0;
		}
	}
	if (write(io,genbuf,i) != i) {
		signal(SIGINTR,putsigsav);
		errfunc(IOERR);
	}
	signal(SIGINTR,putsigsav);
}

/*

Name:
	append

Function:
	Insert new line into core array.

Algorithm:
	If in 'y' mode, turn off echo and turn on raw mode.  Use function to
	gather line from input source, write line to current file and insert its
	pointer into the core array of line pointers.  If need be, get more core
	for the array (in 1K chunks).  If in 'y' mode, reset tty settings.

Parameters:
	Function to gather line
	Address where to insert

Returns:
	Number of lines inserted or error reset

Files and Programs:
	None.


*/
append(f, a)
int (*f)();
{
	register *a1, *a2, *rdot;
	int nline, tl;
	struct { int integer; };

	nline = 0;
	dot = a;
	if(yflag) {
		if(!nostty) {
			ttmode[2] =& ~ ECHO;		/* set no echo */
			ttmode[2] =| 040;
			stty(0,ttmode);
		}
	}
	while ((*f)() == 0) {
		if (dol >= endcore) {
			if (sbrk(1024) == -1)
				errfunc("No more core");
			endcore.integer =+ 1024;
		}
		tl = putline();
		nline++;
		a1 = ++dol;
		a2 = a1+1;
		rdot = ++dot;
		while (a1 > rdot)
			*--a2 = *--a1;
		*rdot = tl;
	}
	if(yflag) {
		if(!nostty) {
			ttmode[2] = ttnorm;
			stty(0,ttmode);
		}
	}
	nostty = 0;
	return(nline);
}

/*

Name:
	unix

Function:
	Execute UNIX shell command

Algorithm:
	Get command line, for and set recovery signals.  Execute shell alone
	or with command and wait for exit.  Save any stty changes while away.

Parameters:
	None.

Returns:
	None or error reset

Files and Programs:
	/bin/sh	shell


*/
unix()
{
	register savint, pid, rpid;
	char	*p;
	int retcode;

	setnoaddr();
	p = linebuf;
	while ( (*p = getchar()) != '\n' && *p != EOF ) 
		if (p++ >= linebuf + LBSIZE) errfunc(BUFERR);
	*p = 0;
	if ((pid = fork()) == 0) {
		signal(2,1);
		signal(3,1);
		if (*linebuf) 
			execl("/bin/sh","-shell(edit)","-c",linebuf,0);
		else 
			execl("/bin/sh","-shell(edit)",0);
		ERROR;
	}
	signal(SIGHUP, onhup);
	signal(SIGQUIT, onquit);
	savint = signal(SIGINTR, 1);
	while ((rpid = wait(&retcode)) != pid && rpid != -1);
	signal(SIGINTR, savint);
	if(gtty(0,ttmode) != -1)	ttnorm = ttmode[2];
	if(vflag)
		puts("!");
}

/*

Name:
	delete

Function:
	Delete lines.

Algorithm:
	Delete lines by moving all lines after the deleted ones toward the front of the file.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
delete()
{
	register *a1, *a2, *a3;

	a1 = addr1;
	a2 = addr2+1;
	a3 = dol;
	dol =- a2 - a1;
	do
		*a1++ = *a2++;
	while (a2 <= a3);
	a1 = addr1;
	if (a1 > dol)		/* reinserted by jsk */
		a1 = dol;
	dot = a1;
	fchanged =+ a2 - a1;
	if (fchanged>FLUSHLIM) flushio();
}
/*

Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


*/
nargs()		/* to fix I+D seperation bug */
{
	return(1);
}
/*

Name:
	join

Function:
	Concatenate a number of lines into one.

Algorithm:
	Copy the addressed lines into the current line buffer and delete the lines.
	Insert any argument between each line.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
join()
{
	register char *a1,*a2;
	int  *one, *two;
	int   genlen, len;
	char jbuf[100];
	int jblen;
	int apflag;

	apflag = 0;
	if(addr1 == 0) {
		if(dot == dol)
			ERROR;
		addr1 = dot;
		addr2 = dot + 1;
	}
	else {
		if(addr1 == addr2) {
			addr2++;
			apflag = 1;
		}
	}
	one = addr1;
	two = addr2;
	a1 = genbuf;
	a2 = getline(*one);		/* copy first line to buffer */
	while (*a1++ = *a2++);
	a1--;
	genlen = a1 - &genbuf;		/* length of new buffer full */
	one++;
	jblen = 0;
	if(imedmode) {			/* save inserted value */
		nostty = 0;
		a2 = &jbuf[0];
		while((*a2++ = getchar()) != '\n');
		a2[-1] = '\0';
		jblen = a2 - &jbuf - 1;
	}
	for(; one <= two; one++)
	{
		a2 = getline(*one);	/* get length of next line */
		while(*a2++);
		a2--;
		len = a2 - &linebuf;
		if(((genlen =+ len) >= LBSIZE-1) || ((genlen =+ jblen) >= LBSIZE-1))	/* if line too long, clean up */
		{
			a1 = &genbuf;
			a2 = &linebuf;
			while(*a2++ = *a1++);
			one--;
			*one = putline();
			addr2 = one - 1;
			delete();
			errfunc(JOINERR);
		}
		if(imedmode) {		/* copy insert */
			a2 = &jbuf[0];
			while(*a1++ = *a2++);
			a1--;
		}
		if(apflag) {		/* if just appending to line, duck */
			addr2--;
			break;
		}
		a2 = &linebuf;		/* copy next line */
		while (*a1++ = *a2++);
		a1--;
	}
	imedmode = 0;
	a1 = &genbuf;
	a2 = &linebuf;
	while(*a2++ = *a1++);
	*addr2 = putline();
	addr2 = one - 2;
	delete();
}

/*

Name:
	getline

Function:
	Get a line from the current file.

Algorithm:
	Read a block from the current file and extract a line into linebuf.
	Read another block if necessary to get a complete line.

Parameters:
	Address of temporary line buffer.

Returns:
	Address of current line buffer.

Files and Programs:
	None.


*/
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

/*

Name:
	putline

Function:
	Put a line into the current file.

Algorithm:
	Copy the line into a buffer, and write it.  Put any residue into the
	next block.  If the change limit is reached, update the current file.

Parameters:
	None.

Returns:
	Address of temp line buffer.

Files and Programs:
	None.


*/
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
	fchanged++;
	if (fchanged>FLUSHLIM) flushio();
	return(nl);
}
/*

Name:
	getblock

Function:
	Get/put block from/to current file.

Algorithm:
	Change buffer address into block number and offset into block.
	Read/write block as directed by function code.

Parameters:
	Read/write function code.
	Address of buffer.

Returns:
	Pointer to block and offset

Files and Programs:
	None.


*/
getblock(atl, iof)
{
	extern read(), write();
	register bno, off;
	
	bno = (atl>>8)&0377;
	off = (atl<<1)&0774;
	if (bno >= 255) errfunc(TMPERR);
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
