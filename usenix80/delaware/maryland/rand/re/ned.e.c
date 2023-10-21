#
/* file ned.e.c: file manipulation functions for new RAND editor */
/*   Walt Bilofsky - 14 January 1977 */

#include "ned.defs"

int charskh,charskl,charscol;	/* global position variables for chars */

#define NBYMAX 150	/* max bytes +1 for fsdbytes */
#define CCHAR 0177   /* character which is escape for ctrl chars */

/*
file2fsd - makes a file into a chain of fsd's. */

struct fsd *file2fsd(fname)
int fname;
	{
	charsin(fname,0,0);     /* start at beginning of file */
	return temp2fsd(fname);
	}

/* temp2fsd(fname) - same as file2fsd but starts at current position of file*/

struct fsd *temp2fsd(chan)
int chan;
	{
	register struct fsd *thisfsd, *lastfsd;
	struct fsd *firstfsd;
	register int nby;
	char c, *bpt;
	char fby[NBYMAX+1];
	int i,cc,lct,nl,sh,sl,kh,kl;

	firstfsd = thisfsd = lastfsd = 0;

	/* loop to eat characters and build fsd.
		c - char, but -1 for eof and -2 for first time through. */
	c = -2;
	cc = sh = sl = 0;
	FOREVER
		{
		if ( (c < 0) || (nby >= NBYMAX) || (nl == FSDMAXL))
			{
			if (c != -2)
				{
				lastfsd = thisfsd;
				thisfsd = salloc(nby + SFSD);
				if (firstfsd == 0) firstfsd = thisfsd;
				else lastfsd->fwdptr = thisfsd;
				thisfsd->backptr = lastfsd;
				thisfsd->fwdptr = 0;
				thisfsd->fsdnlines = nl;
				nlines[chan] =+ nl;
				thisfsd->fsdfile = chan;
				thisfsd->seekhigh = sh;
				thisfsd->seeklow = sl;
				bpt = &(thisfsd->fsdbytes);
				for (i=0; i<nby; ++i) *(bpt++) = fby[i];
				}
			if (c == -1)
				{ /* put eof block and return */
				thisfsd->fwdptr = lastfsd = salloc(SFSD);
				lastfsd->backptr = thisfsd;
				return (firstfsd);
				}
			sh = charskh;
			sl = charskl;
			nl = nby = lct = 0;
			}
		kh = charskh;
		kl = charskl;
		c = chars();
		lct = (charskl - kl) + 512 * (charskh - kh);
		if (c != -1 || lct) {
			if (lct > 127)
				{
				fby[nby++] = -(lct / 128);
				lct = lct % 128;
				}
			fby[nby++] = lct;
			++nl;
			}
		}
	}


/*
charsin(file,h,l) - initializes char to start reading from file,
	at position l of record h. */

int chkl,chkh;	/* position of next read from charsfi */

int charsfi,charsi,charsn;
charsin(fi,h,l)
int fi,h,l;
{
#define LOWSEEK 1	/* used by charsin */
#define HIGHSEEK 2
#define NEWFILE 4
register char flg;
if (fi <= 0) {
	charsfi = fi;
	return;
	}
flg = (charsfi == fi ? 0 : NEWFILE);
if (chkh != h) flg =| HIGHSEEK;
if (l > chkl || l < chkl - charsn) flg =| LOWSEEK;
if (flg & (NEWFILE|HIGHSEEK)) {
	chkh = h;
	chkl = l;
	seek(fi,h,3);
	seek(fi,l,1);
	charsi = charsn = 0;
	}
else if (flg) {
	seek(fi,l-chkl,1);
	chkl = l;
	charsi = charsn = 0;
	}
else charsi = charsn + l - chkl;
ncline = 0;
charsfi = fi;
charskh = chkh;
charskl = chkl + charsi - charsn;
while (charskl < 0) {
	charskl =+ 512;
	charskh--;
	}
}

/*
chars - reads the next character from a file.
	maintains the global variables chkh, chkl which are
	the record and char position in the record of the next
	character to be read.  returns -1 for eof.
    chars fills the array cline with the characters in the
	current line (up to and including a newline or EOF (-1).
	ncline gets the number of characters in the line, including
	the end character, which is either newline or -1.

    chars also performs conversion from external to internal format.
	external format contains control characters.  chars replaces
	tabs by blanks (using the system tab convention of cols 9, 17,
	etc.) and converts control characters to the corresponding
	upper case letter prefixed by the character "ctrlchar".
	The editor treats these characters
	as it would any others.  If the two-character combinations
	appear on output, they are converted back to the external
	representation by the routine dechars, which also deletes
	trailing blanks and inserts initial tabs if possible.
*/

#define CHARBUFSZ 100		/* size of chars buffer */
char chars()
	{
	register char c;
	register int i,j;
	static char charsbuf[CHARBUFSZ];

	ncline = 0;

	if (charsfi <= 0)
		{
		if (lcline == 0) excline(1);
		ncline = 1;
		cline[0] = NEWLINE;
		return (NEWLINE);
		}

	FOREVER
		{
		if (charsi == charsn)
			{
			charsn = read(charsfi, &charsbuf, CHARBUFSZ);
			charsi = 0;
			chkl =+ charsn;
			}
		c = (charsn <= 0 ? -1 : (0177 & charsbuf[charsi++]) );
		if (c == 011)
			{
			c = ' ';
			i = 8 - (07 & ncline);
			}
		else if (c < 040 && c != CCRETURN && c >= 0)
			{
			c = CCHAR;
			i = 2;
			}
		else i = 1;

		/* check if room for these characters in cline */
		if (lcline <= ncline + i) excline(0);
		j = i;
		while (j--) cline[ncline++] = c;
		if (c == CCRETURN || c < 0)
			{
			while (chkl > 511)
				{
				chkl =- 512;
				++chkh;
				}
			charskh  = chkh;
			charskl = chkl + charsi - charsn;
			while (charskl < 0)
				{
				charskl =+ 512;
				-- charskh;
				}
			if (c < 0)
				{
				charsfi = 0;
				cline[ncline - 1] = NEWLINE;
				}
			/* take out trailing spaces */
			for (;ncline>1 && cline[ncline-2] == ' ';ncline--)
				cline[ncline-2] = cline[ncline-1];
			return (c);
			}
		else if (c == CCHAR && i == 2)
			cline[ncline-1] = charsbuf[charsi - 1] + 0100;
		}
	}
/* dechars(line,n) - performs in-place character conversion from internal
	to external format of n characters starting at line.  May
	destroy contents of line.  CAUTION - will always insert a
	newline at line[n] - so line must be n+1 long.

	note: replaces initial spaces with tabs; deletes trailing spaces
*/

int dechars(line,n)
char *line;
int n;
	{
	register char *fm,*to;  /* pointers for move */
	char cc;                /* current character */
	register int lnb;	/* 1 + last non-blank col */
	int cn;                 /* col number */

	line[n] = NEWLINE;
	fm = to = line;
	cn = -1;
	lnb = 0;
	while ((cc = *fm++) != NEWLINE)
		{
		if (cc == CCHAR && *fm != NEWLINE) cc = *fm++ & 037;
		++cn;
		if (cc != ' ')
			{
			if (lnb==0) while (8 + (lnb & 0177770) <= cn)
				{
				*to++ = (lnb & 7) == 7 ? ' ' : '\t';
				lnb =& 0177770;
				lnb =+ 8;
				}
			while (++lnb <= cn) *to++ = ' ';
			*to++ = cc;
			}
		}
	*to++ = NEWLINE;
	return (to - line);
	}

/* excline(n) - expand cline */
excline(n)
int n;
{
register int j;
register char *tmp;
	j = lcline + icline;
	if (j < n) j = n;
	tmp = alloc(j+1);
	icline =+ icline >> 1;
	while (--lcline >= 0) tmp[lcline] = cline[lcline];
	lcline = j;
	if (cline != 0) free(cline);
	cline = tmp;
	}
/* wseek(wksp,lno) - prepare for input at line lno of wksp.
	makes lno the current line.
	initializes chars(0) to start reading characters from that
		line.
	returns 1 if line is off end of workspace, else 0 if ok. */

int wseek(wksp,lno)
struct workspace *wksp;
int lno;
	{
	register char *cp;
	int h,i;
	register int j,l;
	/* first get curfsd to point to fsd containing current line */
	if (wposit(wksp,lno)) return (1);

	/* now in correct fsd - determine position of chars in file */
	h = wksp->curfsd->seekhigh;
	l = wksp->curfsd->seeklow;
	i = lno - wksp->curflno;
	cp = &(wksp->curfsd->fsdbytes);
	while (i-- != 0)
		{
		if ((j = *(cp++)) < 0)
			{
			l =- 128*j;
			j = *(cp++);
			}
		l =+ j;
		}
	while (l > 511)
		{
		l =- 512;
		++h;
		}
	charsin(wksp->curfsd->fsdfile,h,l);
	return (0);
	}
/* wposit(wksp,lno) - get curfsd of wksp to point to the fsd
	containing current line. */

wposit(wk,lno)
struct workspace *wk;
int lno;
	{
	register struct workspace *wksp;
	wksp = wk;
	if (lno < 0) fatal("WPOSIT NEG ARG");
	while (lno >= (wksp->curflno + wksp->curfsd->fsdnlines))
		{
		if (wksp->curfsd->fsdfile == 0)
			{
			wksp->curlno = wksp->curflno;
			return (1);
			}
		wksp->curflno =+ wksp->curfsd->fsdnlines;
		wksp->curfsd = wksp->curfsd->fwdptr;
		}

	while (lno < wksp->curflno)
		{
		if ((wksp->curfsd = wksp->curfsd->backptr) == 0)
			fatal("WPOSIT 0 BACKPTR");
		wksp->curflno =- wksp->curfsd->fsdnlines;
		}

	if (wksp->curflno < 0) fatal("WPOSIT LINE CT LOST");
	wksp->curlno = lno;
	return 0;
	}
/* switchfile()  go to alternate wksp if possible
*/
switchfile()
	{
	if (curport->altwksp->wfile == 0)
		{
		error("No alternate file found");
		return;
		}
	switchwksp();
	putup(0,curport->btext);
	poscursor(curwksp->ccol,curwksp->crow);
	}
switchwksp()
	{
	struct workspace *tempwksp;
	curwksp->ccol = cursorcol;
	curwksp->crow = cursorline;
	tempwksp = curwksp;
	curwksp = curport->wksp = curport->altwksp;
	curfile = curwksp->wfile;
	curport->altwksp = tempwksp;
	}

/*
openlines, openspaces and splitline
*/

openlines(from,number)
int from, number;
	{
	if (from >= nlines[curfile]) return;
	nlines[curfile] =+ number;
	insert(curwksp,blanklines(number),from);
	redisplay(0,curfile,from,from+number-1,number);
	poscursor(cursorcol,from - curwksp->ulhclno);
	}

openspaces(line,col,number, nl)
int line, col, number, nl;
	{
	register int i;
	for (i=line;i<line+nl;i++)
		{
		getline(i);
		putbks(col,number);
		fcline = 1;
		putline(0);
		putup(i - curwksp->ulhclno, i - curwksp->ulhclno);
		}
	poscursor(col - curwksp->ulhccno, line - curwksp->ulhclno);
	}

splitline(line,col)
int line,col;
	{
	register int nsave;
	register char csave;
	if (line >= nlines[curfile]) return;
	nlines[curfile]++;
	getline(line);
	if (col >= ncline - 1) openlines(line+1,1);
	else
		{
		csave = cline[col];
		cline[col] = NEWLINE;
		nsave = ncline;
		ncline = col+1;
		fcline = 1;
		putline(0);
		cline[col] = csave;
		insert(curwksp,writemp(cline+col,nsave-col),line+1);
		redisplay(0,curfile,line,line+1,1);
		}
	poscursor(col - curwksp->ulhccno, line - curwksp->ulhclno);
	return;
	}
/*
closelines, closespaces and combineline
*/

closelines(frum,number)
int frum, number;
/* frum negative => don't redisplay (for execute function) */
	{
	register int n,from;
	register struct fsd *f;

	if ((from = frum) < 0) from = ~ from;
	if (from < nlines[curfile]) if ((nlines[curfile] =- number) <= from)
		nlines[curfile] = from + 1;

	f = delete(curwksp,from,from+number-1);
	if (frum >= 0) redisplay(0,curfile,from,from+number-1,-number);

	insert(pickwksp,f,n = nlines[2]);
	redisplay(0,2,n,n,number);
	deletebuf->linenum = n;
	deletebuf->nrows = number;
	deletebuf->ncolumns = 0;
	nlines[2] =+ number;

	poscursor(cursorcol,from - curwksp->ulhclno);
	return;
	}

closespaces(line,col,number,nl)
int line, col, number, nl;
	{
	pcspaces(line,col,number,nl,1);
	}





combineline(line,col)
int line,col;
	{
	register char *temp;
	register int nsave,i;

	if (nlines[curfile] <= line-2) nlines[curfile]--;
	getline(line+1);
	temp = salloc(ncline);
	for (i=0;i<ncline;i++) temp[i] = cline[i];
	nsave = ncline;
	getline(line);
	if (col+nsave > lcline) excline(col+nsave);
	for (i=ncline-1;i<col;i++) cline[i] = ' ';
	for (i=0;i<nsave;i++) cline[col+i] = temp[i];
	ncline = col + nsave;
	fcline = 1;
	putline(0);
	free(temp);
	delete(curwksp,line+1,line+1);
	redisplay(0,curfile,line,line+1,-1);
	poscursor(col - curwksp->ulhccno, line - curwksp->ulhclno);
	}

/*
picklines, pickspaces, put
*/

picklines(from,number)
int from, number;
	{
	register int n;
	register struct fsd *f;

	f = pick(curwksp,from,from+number-1);
	redisplay(0,curfile,from,from+number-1,0); /* because of breakfsd */

	insert(pickwksp,f,n = nlines[2]);
	redisplay(0,2,n,n,number);
	pickbuf->linenum = n;
	pickbuf->nrows = number;
	pickbuf->ncolumns = 0;
	nlines[2] =+ number;

	poscursor(cursorcol,from - curwksp->ulhclno);
	return;
	}

pickspaces(line,col,number,nl)
int line, col, number, nl;
	{
	pcspaces(line,col,number,nl,0);
	}

/* pcspaces - pick/close spaces -- common routine
		flg == 0 for pick, 1 for close
*/
pcspaces(line,col,number,nl,flg)
int line, col, number, nl, flg;
	{
	register struct fsd *f1,*f2;
	char *linebuf, *bp;
	register int i;
	int j, n;
	if (charsfi == tempfile) charsfi = 0;
/* Note: this code depends on number being less than 127.  Otherwise, the
   fsd block will be incorrect due to failing to deal with two-byte line
   descriptors. wb 1/13/77 */
	f1 = salloc(nl + SFSD);
	bp = &(f1->fsdbytes);
	f2 = salloc(SFSD);
	f2->backptr = f1;
	f1->fwdptr = f2;
	f1->fsdnlines = nl;
	f1->fsdfile = tempfile;
	f1->seekhigh = tempfh;
	f1->seeklow = tempfl;
	linebuf = salloc(number+1);
	for (j=line;j<line+nl;j++)
		{
		getline(j);
		if (col+number >= ncline)
			{
			if (col+number >= lcline) excline(col+number+1);
			for (i=ncline-1;i<col+number;i++) cline[i] = ' ';
			cline[col+number] = NEWLINE;
			ncline = col + number + 1;
			}
		for (i=0;i<number;i++) linebuf[i] = cline[col+i];
		linebuf[number] = NEWLINE;
		if (tempfh)
			{
			seek(tempfile,tempfh,3);
			seek(tempfile,tempfl,1);
			}
		else seek(tempfile,tempfl,0);
		if (charsfi == tempfile) charsfi = 0;
		write(tempfile,linebuf,n = dechars(linebuf,number));
		*bp++ = n;
		tempfl =+ n;
		while (tempfl > 511)
			{
			tempfl =- 512;
			tempfh++;
			}
		}
	if (flg) for (j=line;j<line+nl;j++)
		{
		getline(j);
		if (col+number >= ncline)
			{
			if (col+number >= lcline) excline(col+number+1);
			for (i=ncline-1;i<col+number;i++) cline[i] = ' ';
			cline[col+number] = NEWLINE;
			ncline = col + number + 1;
			}
		for (i=col+number;i<ncline;i++)
			cline[i-number] = cline[i];
		ncline =- number;
		fcline = 1;
		putline(0);
		putup(j - curwksp->ulhclno, j - curwksp->ulhclno);
		}

	insert(pickwksp,f1,n = nlines[2]);
	redisplay(0,2,n,n,nl);
	if (flg)
		{
		deletebuf->linenum = n;
		deletebuf->nrows = nl;
		deletebuf->ncolumns = number;
		}
	else
		{
		pickbuf->linenum = n;
		pickbuf->nrows = nl;
		pickbuf->ncolumns = number;
		}
	nlines[2] =+ nl;
	free(linebuf);
	poscursor(col - curwksp->ulhccno, line - curwksp->ulhclno);
	}

/*    put(buf,line,col)
	if buf->ncolumns == 0, use plines; else pspaces
*/
put(buf,line,col)
struct savebuf *buf;
int line, col;
	{
	if (buf->ncolumns == 0) plines(buf,line);
	else pspaces(buf,line,col);
	}

plines(buf,line)
struct savebuf *buf;
int line;
	{
	int lbuf, cc, cl;
	struct fsd *w0, *w1;
	register struct fsd *f, *g;
	register int j;

	cc = cursorcol;
	cl = cursorline;

	breakfsd(pickwksp, buf->linenum + buf->nrows);
	w1 = pickwksp->curfsd;
	breakfsd(pickwksp, buf->linenum);
	w0 = pickwksp->curfsd;

	f = g = copyfsd(w0,w1);
	lbuf = 0;
	while (g->fsdfile)
		{
		lbuf =+ g->fsdnlines;
		g = g->fwdptr;
		}

	insert(curwksp,f,line);
	redisplay(0,curfile,line,line,lbuf);
	poscursor(cc,cl);
	if ((nlines[curfile] =+ lbuf) <= (j=line+lbuf)) nlines[curfile] = j+1;
	}

pspaces(buf,line,col)
struct savebuf *buf;
int line, col;
	{
	struct workspace *oldwksp;
	char *linebuf;
	int nc, i, j;

	linebuf = salloc(nc = buf->ncolumns);
	oldwksp = curwksp;
	for (i=0;i<buf->nrows;i++)
		{

		curwksp = pickwksp;
		getline(buf->linenum + i);
		if (ncline-1 < nc) for (j=ncline-1;j<nc;j++) cline[j] = ' ';
		for (j=0;j<nc;j++) linebuf[j] = cline[j];
		curwksp = oldwksp;

		getline(line+i);
		putbks(col,nc);
		for (j=0;j<nc;j++) cline[col+j] = linebuf[j];
		fcline = 1;
		putline(0);
		if ((j = line+i-curwksp->ulhclno) <= curport->btext)
			putup(j,j);
		}
	free(linebuf);
	poscursor(col - curwksp->ulhccno, line - curwksp->ulhclno);
	return;
	}

/* insert(wksp,f,at) inserts fsd f into workspace wksp before line at.
   Calling program MUST call redisplay with proper arguments. */

insert(wksp,f,at)
struct workspace *wksp;
struct fsd *f;
int at;
	{
	register struct fsd *w0, *wf, *ff;
	int ln;
	DEBUGCHECK;
	/* determine length of insert */
	ff = f;
	ln = 0;
	while (ff->fwdptr->fsdfile)
		{
		ln =+ ff->fsdnlines;
		ff = ff->fwdptr;
		}
	ln =+ ff->fsdnlines;
	breakfsd(wksp,at);
	wf = wksp->curfsd;
	w0 = wf->backptr;
	free(ff->fwdptr);
	ff->fwdptr = wf;
	wf->backptr = ff;

	f->backptr = w0;
	wksp->curfsd = f;
	wksp->curlno = wksp->curflno = at;
	if (openwrite[wksp->wfile]) openwrite[wksp->wfile] = EDITED;
	if (w0) w0->fwdptr = f; else openfsds[wksp->wfile] = f;
	DEBUGCHECK;
	}
/*
putbks(col,n) - inserts n blanks starting at col of cline
*/
putbks(col,n)
int col,n;
	{
	register int i;
	if (n <= 0) return;
	if (col > ncline-1)
		{
		n =+ col - (ncline-1);
		col = ncline-1;
		}
	if (lcline <= (ncline =+ n)) excline(ncline);
	for (i = ncline - (n + 1); i >= col; i--) cline[i + n] = cline[i];
	for (i = col + n - 1; i >= col; i--) cline[i] = ' ';
	}


/*
delete(wksp,from,to) - deletes indicated lines from workspace.
	Returns fsd chain that was deleted, with appended 0 block.
   Calling program MUST call redisplay with proper arguments. */

struct fsd *delete(wksp,from,to)
struct workspace *wksp;
int from,to;
	{
	struct fsd *w0;
	register struct fsd *wf,*f0,*ff;
	breakfsd(wksp,to+1);
	DEBUGCHECK;
	wf = wksp->curfsd;
	breakfsd(wksp,from);
	f0 = wksp->curfsd;
	ff = wf->backptr;
	w0 = f0->backptr;

	wksp->curfsd = wf;
	wf->backptr = w0;
	f0->backptr = 0;
	(ff->fwdptr = salloc(SFSD))->backptr = ff; /* do both in one line */
	if (w0) w0->fwdptr = wf; else openfsds[wksp->wfile] = wf;
	openwrite[wksp->wfile] = EDITED;

	DEBUGCHECK;
	return (f0);
	}

/*
pick(wksp,from,to) - returns a copy of fsds for lines from to to in wksp
	with the appended 0 block.
   Calling program MUST call redisplay with proper arguments. */

struct fsd *pick(wksp,from,to)
struct workspace *wksp;
int from,to;
	{
	struct fsd *wf;
	breakfsd(wksp,to+1);
	wf = wksp->curfsd;
	breakfsd(wksp,from);
	return(copyfsd(wksp->curfsd,wf));
	}

/* printfsd (f) - debugging routine to dump an fsd. */

printfsd(f)
struct fsd *f;
{
int i;
register char *c;
register struct {char *s0; char *s1; } *cp, *np;
extern int freelist[];
printf("\n**********");
while (f) {
	printf("\nfsdnl=%d chan=%d hi=%d lo=%d at %o",
		f->fsdnlines,f->fsdfile,f->seekhigh,
		f->seeklow,f);
	if (f->fwdptr && f != f->fwdptr->backptr) printf("\n*** next block bad backpointer ***");
	c = &(f->fsdbytes);
	for (i=0; i<f->fsdnlines; i++) {
		if ((i % 20) == 0) putchar('\n');
		printf(" %d",*(c++));
		}
	f = f->fwdptr;
	}
cp = freelist;
printf("\nFree list: ");
while (cp->s1 != -1) {
	printf("%d@%o ",cp->s1->s0,cp->s1);
		cp = cp->s1;
	};
printf("\n");
}

/* checkfsd() - debugging routine to check for fsd consistency of current
	port */
checkfsd()
{
register struct fsd *f;
register int nl;
extern int freelist[];
struct {int s0; char *s1; };
register char *g;
char *gg;
nl = 0;
f = openfsds[curfile];
while (f) {
	if (curwksp->curlno >=nl && curwksp->curlno <
	  nl + f->fsdnlines && curwksp->curfsd != f &&
	  curwksp->curfsd->backptr ) {
		fatal("CKFSD CURFSD LOST"); }
	if (f->fwdptr && f->fwdptr->backptr != f) fatal("CKFSD BAD CHAIN");
	nl =+ f->fsdnlines;
	f = f->fwdptr;
	}
g = freelist;
while (g->s1 != -1)
	if ((g + g->s0) > g->s1) fatal("CKFSD OVERLAPPING FREE BKS");
	  else g = g->s1;
/* This is extra check for use only when free list gets bollixed.  It
	should be commented out at other times. */
gg = g;
if (g->s1 != -1) for (g = freelist->s1; g != gg; g =+ g->s0)
	if (g->s0 <= 0 || g + g->s0 > gg) {
		printf("\n\rIN FREELIST AT %o\n\n\r",g);
		fatal("CKFSD FREE BLOCK COUNT CLOBBRED.");
		}
}

/* breakfsd(w,n) - breaks the fsd at line n of workspace
	w. curlno = curflno on return, and curfsd is left
	pointing to the first line after the break
	(which may be an end-of-file block).  the original
	fsd block will probably be left with unused and
	unrecoverable space at the end.  If called to break
	at the line past the end of file, will leave current
	position at the end-of-file block.  If called to break
	past that, FSD's with channel -1 (blank lines) will be
	inserted to provide sufficient lines.
CAUTION: breakfsd mucks about with pointers, and may invalidate
	the fsd pointer field of a workspace.  redisplay must
	be called before attempting to mess with workspaces other
	than those specifically fixed up in routines that call
	breakfsd. */

int breakfsd(w,n)
struct workspace *w;
int n;
	{
	int nby,i,j,jj,k,snby,offs;
	register struct fsd *f,*ff;
	struct fsd *fn;
	register char *c;
	char *cc;
	DEBUGCHECK;
	if (wposit(w,n))
		{
		f = w->curfsd;
		ff = f->backptr;
		free(f);
		fn = blanklines(n - w->curlno);
		w->curfsd = fn;
		fn->backptr = ff;
		if (ff) ff->fwdptr = fn; else openfsds[w->wfile] = fn;
		wposit(w,n);
		return (1);
		}
	f = w->curfsd;
	c = &f->fsdbytes;
	offs = 0;
	ff = f;
	nby = n - w->curflno;

	if (nby != 0)
		{
		/* get down to the nth line */
		for (i=0; i<nby; i++)
			{
			if ((j = *c++) < 0)
				{
				offs =- 128*j;
				j = *c++;
				}
			offs =+ j;
			}
		/* now make a new fsd from the remainder of f */
		i = j = jj = f -> fsdnlines - nby; /* number of lines in new fsd */
		cc = c;
		while (--i >= 0) if (*cc++ < 0) {j++; cc++; }
		ff = salloc(SFSD + j);
		ff->fsdnlines = jj;
		ff->fsdfile = f->fsdfile;
		offs =+ f->seeklow;
		ff->seeklow = offs % 512;
		ff->seekhigh = f->seekhigh + offs/512;
		cc = &ff->fsdbytes;
		for (k=0; k<jj; k++) if ((*cc++ = *c++) < 0) *cc++ = *c++;
		if ((ff->fwdptr = f->fwdptr)) ff->fwdptr->backptr = ff;
		ff->backptr = f;
		f->fwdptr = ff;
		f->fsdnlines = nby;
		}
	w->curfsd = ff;
	w->curflno = n;
	DEBUGCHECK;
	return (0);
	}
/*
writemp(buf,n) - writes a line n long from buf onto temp file.
	returns an fsd pointer describing the line. */

struct fsd *writemp(buf,n)
char *buf;
int n;
	{
	register struct fsd *f1,*f2;
	register char *p;
	if (charsfi == tempfile) charsfi = 0;
	if (tempfh)
		{
		seek(tempfile,tempfh,3);
		seek(tempfile,tempfl,1);
		}
	else seek(tempfile,tempfl,0);
	write(tempfile,buf,n = dechars(buf,n-1));

	/* now make fsd */
	f1 = salloc(2 + SFSD);
	f2 = salloc(SFSD);
	f2->backptr = f1;
	f1->fwdptr = f2;
	f1->fsdnlines = 1;
	f1->fsdfile = tempfile;
	f1->seekhigh = tempfh;
	f1->seeklow = tempfl;
	if (n<127) f1->fsdbytes = n;
	else
		{
		p = &f1->fsdbytes;
		*p++ = - (n / 128);
		*p = n % 128;
		}
	tempfl =+ n;
	while (tempfl > 511)
		{
		tempfl =- 512;
		tempfh++;
		}
	return (f1);
	}

/* getline(ln) - gets line ln of the current port into cline so
	it can be edited. */

getline(ln)
int ln;
	{
	fcline = 0;
	clineno = ln;
	if (wseek(curwksp, ln))
		{
		if (lcline == 0) excline(1);
		cline[0] = NEWLINE;
		ncline = 1;
		}
	else chars();
	}

/* putline() - inserts the line in cline in place of the current one */

putline(n)
int n;
	{
	struct fsd *w0,*cl;
	register struct fsd *wf, *wg;
	register struct workspace *w;
	int i;
	char flg;
	DEBUGCHECK;
	if (fcline == 0)
	{
		clineno = -1;
		return;
	}
	if (nlines[curfile] <= clineno) nlines[curfile] = clineno + 1;
	fcline = 0;
	cline[ncline-1] = NEWLINE;
	cl = writemp(cline+n,ncline-n);
	w = curport->wksp;             /* w s can be replaced by curwksp */
	i = clineno;
	flg = breakfsd(w,i);
	wg = w->curfsd;
	w0 = wg->backptr;
	if (flg == 0)
		{
		breakfsd(w,i+1);
		wf = w->curfsd;
		free(cl->fwdptr);
		cl->fwdptr = wf;
		wf->backptr = cl;
		}
	free(wg);
	cl->backptr = w0;
	w->curfsd = cl;
	w->curlno = w->curflno = i;
	openwrite[w->wfile] = EDITED;
	clineno = -1;
	if (w0) w0->fwdptr = cl; else openfsds[w->wfile] = cl;
	redisplay(w,w->wfile,i,i,0);
	DEBUGCHECK;
	}

/*
copyfsd(f,end) - returns a copy of fsd f up to but not including end,
	or until its final block. */

copyfsd(f,end)
struct fsd *f,*end;
	{
	struct fsd *res,*ff,*rend;
	register int i;
	register char *c1,*c2;
	res = 0;
	while (f->fsdfile && f != end)
		{
		c1 = &f->fsdbytes;
		for (i = f->fsdnlines; i; i--) if (c1++ < 0) c1++;
		c2 = f;
		i = c1 - c2;
		ff = c2 = alloc(i);
		c2 =+ i;
		while (i--) *--c2 = *--c1;
		if (res)
			{
			rend->fwdptr = ff;
			ff->backptr = rend;
			rend = ff;
			}
		else res = rend = ff;
		f = f->fwdptr;
		}
	if (res)
		{
		(rend->fwdptr = salloc(SFSD))->backptr = rend;
		rend = rend->fwdptr;
		}
	else res = rend = salloc(SFSD);
	if (f->fsdfile == 0) rend->seeklow = f->seeklow;
	return res;
	}
/*
freefsd(f) - frees all blocks in fsd */

freefsd(f)
struct fsd *f;
{
register struct fsd *g;
while (f) {
	g = f;
	f = f->fwdptr;
	free(g);
	}
}
/* blanklines(n) - returns an fsd chain of n blank lines */

struct fsd *blanklines(n)
int n;
{
int i;
register struct fsd *f,*g;
register char *c;

f = salloc(SFSD);
while (n) {
	i = n > FSDMAXL ? FSDMAXL : n;
	g = salloc(SFSD + i);
	g->fwdptr = f;
	f->backptr = g;
	g->fsdnlines = i;
	g->fsdfile = -1;
	c = &g->fsdbytes;
	n =- i;
	while (i--) *c++ = 1;
	f = g;
	}
return (f);
}
/*doreplace - handles editing functions for "Execute filter" function */
doreplace(i,m,n,j,pipef)
int i,m,n,j,*pipef;
{       register struct fsd *e,*ee;
	register int l;

	close(pipef[0]);
	breakfsd(curwksp,i);
	if (m == 0) close(pipef[1]);
	  else {m = fsdwrite(curwksp->curfsd,m,pipef[1]);
		if (m == -1) {
			error("Can't write on pipe.");
			kill(j,9);
		}       }
	while (wait(&n) != j);          /* wait for completion */
	if ((n & 0177400) == 0177400) {
		error("Can't find program to execute.");
		return; }
	if ((n & 0177400) == 0177000 || (n & 0377) != 0) {
		error("Abnormal termination of program.");
		return; }

	charsfi = -1;                   /* forget old position before fork */
	if (m) closelines(~i,m);
	charsin(tempfile,tempfh,tempfl);
	ee = e = temp2fsd(tempfile);
	tempfh = charskh;
	tempfl = charskl;
	if (e->fsdnlines) {
		l = 0;
		while (e->fsdfile) {
			l =+ e->fsdnlines;
			e = e->fwdptr;
			}
		insert(curwksp,ee,i);
		nlines[curfile] =+ l;
		}
	redisplay(0,curfile,i,i+m,l-m);
	poscursor(cursorcol,cursorline);
	}


execr(args)     /* using code from sh.c swiped 10 August 1976 */
char **args;
{       register int i;
	register char *s, *t;
	char *u;

	execv(*args,args);              /* try plain file name first */
	if (**args != '/') {            /* then user's bin directory */
		t = pwbuf;  i = 4;      /* ignore 4 ':' - setup erased one */
		do while (*t++ != ':') ; while (--i);
		if (*t != ':' && *t != 0) {
		      u = t;
		      while (*t != ':') t++;         /* space over name */
		      for (s = "/bin/"; *s; *t++ = *s++) ;  /* add on /bin/ */
		      for (s = *args; *t++ = *s++;) ;  /* add on file name */
		      execv(u,args);
		      }
	  nopw: u = append("/usr/bin/",*args);
		execv(4 + u,args);
		execv(u,args);
		}
}


