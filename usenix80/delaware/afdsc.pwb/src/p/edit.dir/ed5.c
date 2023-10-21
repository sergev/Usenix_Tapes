#
#include "./externals.h"
#include "edx.h"
/*

Name:
	edx

Function:
	Invoke block screen editor (edx).

Algorithm:
	Initialize screen editor first time only.  Until user quits screen editor,
	send control structure and block of text to edx process.  Get and interpret
	command.  If lines have been changed, replace their originals in the
	current file using delete() and/or append().  Adjust dot.  If user quits,
	reset signals and tty settings and return to line editor.  Adjust addresses
	and continue in loop otherwise.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
#define error xerrfunc()
int	xtest	0;
int	ttyinfo[3];
int	edxon;
int	xin;
int	xing;
int	xout;
int	xsavsig;

edx()
{
	register int lines, xx;
	int getpipe();

	if(globp)
		error;
	if((xx = getchar()) == '\n')
		lines = setxdot(addr1 - zero);
	else if(xx == 'x' && getchar() == '\n')
		lines = setxdot(addr2 = 0);
	else error;
	xsavsig = signal(SIGINTR, 1);
	if(!edxon)
		xinit();
	gtty(1, ttyinfo);
	for(;;) {
		xing = 1;
		write(xout, &edxio, sizeof edxio);
		if(edxio.x_count)
			putpipe();
		if(pipread(&edxio, sizeof edxio) <= 0)
			error;
		if(edxio.x_count) {
			if(lines) {
				setdot();
				nonzero();
				delete(addr1, addr2);
			}
			ninbuf = 0;
			if(lines) append(getpipe, addr1 - 1);
			else append(getpipe, addr2);
/* TEMP
			wflg++;
*/
			lines = 1;
		}
		if((dot = edxio.x_dot + zero) <= zero)
			dot = zero + 1;
		if(dot > dol)
			dot = dol;
		if(!edxio.x_more) {
			stty(1, ttyinfo);
			signal(SIGINTR, xsavsig);
			xing = 0;
			return;
		}
		if((addr1 = zero + edxio.x_base) <= zero)
			addr1 = zero + 1;
		if(addr1 > dol)
			addr1 = dol;
		addr2 = 0;
		setxdot(0);
	}
}

/*

Name:
	getpipe

Function:
	Read from the pipe.

Algorithm:
	Read character count from pipe, then read rest of pipe (line) into general
	buffer.  Copy line into line buffer.

Parameters:
	None.

Returns:
	0 = nromal.
	EOF = premature evaporation.
	Error reset.

Files and Programs:
	None.


*/
getpipe()
{
	register c;
	register char *lp, *fp;
	int xrdsize;

	lp = linebuf;
	fp = nextip;
	if((xrdsize = intread()) == 0)
		return(EOF);
	if(pipread(genbuf, xrdsize) <= 0)
		return(EOF);
	ninbuf = xrdsize;
	fp = genbuf;
	do {
		if (--ninbuf < 0) {
				return(EOF);
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

/*

Name:
	pipread

Function:
	Read from the pipe.

Algorithm:
	While the pipe has stuff in it, read it.

Parameters:
	Buffer pointer.
	Character count.

Returns:
	Final read status.

Files and Programs:
	None.


*/
pipread(l, cnt) register char *l; register int cnt;
{
	register int b;
	int t;

	b = 0;
	while(b < cnt && (t = read(xin, &l[b], cnt - b)) > 0)
		b =+ t;
	return(t);
}

/*

Name:
	setxdot

Function:
	Adjust current line pointer.

Algorithm:
	If nothing in file, adjust address pointers and variables.  Adjust line
	counts, base address, current line pointer, and character count.

Parameters:
	Current line address.

Returns:
	Character count.
	0 = no characters.

Files and Programs:
	None.


*/
setxdot(xdot) int xdot;
{
	if(dot == zero && zero == dol) {
		addr1 = addr2 = dot;
		edxio.x_dot = 0;
		edxio.x_more = edxio.x_count = 0;
		edxio.x_base = 1;
		return(0);
	}
	if(addr1 == addr2) {
		if((addr1 =- DFLTPOSN) <= zero) addr1 = zero + 1;
		if((addr2 = addr1 + DISPLAY - 1) > dol) addr2 = dol;
	} else {
		if(addr2 > addr1 && addr2 - addr1 < DISPLAY)
			addr1 = zero + ((addr1 - zero) - DFLTPOSN + ((addr2 - addr1)/2));
		if(addr1 <= zero) addr1 = zero + 1;
		if((addr2 = addr1 + DISPLAY - 1) > dol) addr2 = dol;
	}
	edxio.x_count = addr2 - addr1 + 1;
	edxio.x_base = addr1 - zero;
	if(edxio.x_dot = xdot)
		edxio.x_dot =- edxio.x_base;
	edxio.x_more = addr2 < dol;
	return(edxio.x_count > 0);
}

/*

Name:
	putpipe

Function:
	Write to the pipe.

Algorithm:
	For each addressed line, 1) get the line from the current file, 2) copy to
	to the general buffer, 3) write a character count and the line to the
	pipe, and 4) reset the pointers.  After the last line, send a zero character
	count down the pipe.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
putpipe()
{
	int *a1;
	register char *fp, *lp;
	register nib;
	int cnt;

	nib = 512;
	fp = genbuf;
	a1 = addr1;
	do {
		lp = getline(*a1++);
		for (;;) {
			if (--nib < 0)
				error;
			if((*fp++ = *lp++) == 0)
				break;
		}
		cnt = fp - genbuf;
		write(xout,&cnt,2);
		write(xout,genbuf,cnt);
		nib = 512;
		fp = genbuf;
	} while (a1 <= addr2);
	cnt = 0;
	write(xout,&cnt,2);
}

/*

Name:
	intread

Function:
	Read an integer from the pipe.

Algorithm:
	Read an integer from the pipe.  This is used for getting a character count
	from the pipe.

Parameters:
	None.

Returns:
	Value of the read integer.

Files and Programs:
	None.


*/
intread()
{
	int i;
	if(pipread(&i, sizeof i) <= 0)
		error;
	return(i);
}

/*

Name:
	xinit

Function:
	Initialize screen editor.

Algorithm:
	Create pipe, fork off child for editor.  Parent sets editor active variable,
	copies pipes, closes pipe original and returns.  Child creates a two character
	argument that shows the pipe file descriptors to edx.  Close the pipe
	remnants and execute /bin/edit.dir/edx.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	/bin/edit.dir/edx


*/
xinit()
{
	register int f;
	int pin[2], pout[2];
	char arg[3];

	if(pipe(pin) == -1 || pipe(pout) == -1)
		error;
	if((f = fork()) == 0) {	/* Child */
		arg[0] = pin[1] + '0';
		arg[1] = pout[0] + '0';
		arg[2] = 0;
		close(pin[0]);
		close(pout[1]);
		if(xtest) execl("/bin/edit.dir/edx", "display-editor", savedfile, arg, 0);
		else execl("/bin/edit.dir/edx", "display-editor", savedfile, arg, 0);
		exit();
	}

	edxon = 1;
	if(f == -1)
		error;
	xin = pin[0];
	xout = pout[1];
	close(pin[1]);
	close(pout[0]);
}

/*

Name:
	xerrfunc

Function:
	Process error condition.

Algorithm:
	If edx is invoked, close pipes, restor signals, and tty settings.
	Invoke normal error facility.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
xerrfunc()
{
	if(xing) {
		close(xin);
		close(xout);
		edxon = xing = 0;
		signal(SIGINTR, xsavsig);
		stty(1, ttyinfo);
	}
	errfunc();
}
