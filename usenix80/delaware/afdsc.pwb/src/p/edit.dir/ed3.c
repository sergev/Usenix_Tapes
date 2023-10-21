#
#include	"./externals.h"
/*
Name:
	flushio

Function:
	Update temp files.

Algorithm:
	Create etmp.Exxxxx file if necessary.  Write changed lines to temp file.
	Reset variable fchanged to 1 (0 means 'w' was issued).

Parameters:
	None.

Returns:
	None or error reset

Files and Programs:
	etmp.Exxxxx


*/
flushio()
{
		/* this causes temporaries to be updated to most
		   current state.  Then reset fchanged to 1 if 
		   fchanged was > 0.
		*/
		int *a;
		int i;
		extern read(), write();
		if (ichanged) blkio(iblock,ibuff,write);
		ichanged = 0;
if(oblock != -1)
		blkio(oblock,obuff,write);
		if (tlpfile)
			seek(tlpfile,0,0);	/* go back to start */
		else {
			tfname[5] = 'E';
			tlpfile = creat(tfname, 0600);
			tfname[5] = 'e';
			}
		*(dol+1) = -1;	/* mark eof */
		for (a = zero+1; a < (dol-255); a =+ 256)
			if ( write(tlpfile,a,512) != 512) errfunc(IOERR);
		i = dol - a + 2;	/* length of last write */
		i =* 2;
		if ( write(tlpfile,a,i) != i ) errfunc(IOERR);
		if (fchanged) fchanged=1;
		/* don't set to 0 because 'w' command wasn't issued */
}

/*

Name:
	blkio

Function:
	Do actual block i/o.

Algorithm:
	Seek to block in file and perform function on it.

Parameters:
	Block number
	Buffer location
	I/o function

Returns:
	None or error reset.

Files and Programs:
	None.


*/
blkio(b, buf, iofcn)
int (*iofcn)();
{
	seek(tfile, b, 3);
	if ((*iofcn)(tfile, buf, 512) != 512) errfunc(TMPERR);
}

/*

Name:
	init

Function:
	Initialization of editor.

Algorithm:
	Clean up temp files, reset variable values, recreate temp files, and
	get more core.  Set dot.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	etmp.exxxxx


*/
init()
{
	close(tfile);
	rmfiles();
	fendcore = sbrk(0);
	fchanged = ichanged = 0;
	tline = 0;
	iblock = -1;
	oblock = -1;
	tfname[0] = 'e';
	tfname[4] = '.';
	tfile = creat(tfname,0600);
	if (tfile < 0) {
		/* couldn't make temp in local area, try /tmp */
		tfname[0] = '/';
		tfname[4] = '/';
		tfile = creat(tfname,0600);
		}
	/* must close and open it to get the right access! */
	close(tfile);
	open(tfname, 2);
	brk(fendcore);
	dot = zero = dol = fendcore;
	endcore = fendcore - 4;
}

/*

Name:
	rmfiles

Function:
	Remove temporary files.

Algorithm:
	Unlink 'E' and 'e' temp files.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	etmp.Exxxxx
	etmp.exxxxx


*/
rmfiles()
{
	tfname[5] = 'E';
	unlink(tfname);
	tfname[5] = 'e';
	unlink(tfname);
	if (tlpfile) close (tlpfile);
	tlpfile = 0;
	}

/*

Name:
	global

Function:
	Perform global command.

Algorithm:
	Disallow nested globals.  Check for some commands.  Compile regular search
	expression.  Read in commands.  For every line, mark lines (| 01) that match regular
	expression according to flag.  Then, for every marked line, do commands at proper
	depth (top or 'next' file).  Update temporary files.

Parameters:
	Positive/negative match flag.

Returns:
	None or error reset.

Files and Programs:
	None.


*/
global(k)
{
	register c;
	register int *a1;

	if (globp)
		ERROR;
	setall();
	nonzero();
	if ((c=getchar())=='\n')
		ERROR;
	compile(c);
	globread();
	for (a1=zero; a1<=dol; a1++) {
		*a1 =& ~01;
		if (a1>=addr1 && a1<=addr2 && execute(0, a1)==k)
			*a1 =| 01;
	}
	fchanged =| 0100000; /* prevent flushio during global change */
	for (a1=zero; a1<=dol; a1++) {
		if (*a1 & 01) {
			*a1 =& ~01;
			dot = a1;
			globp = globuf;
			if(curdepth == 2)  {
				push(&curdepth);
				commands(3);
				pop(&curdepth);
			}
			else  {
				push(&curdepth);
				commands(1);
				pop(&curdepth);
			}
			a1 = zero;
		}
	}
	fchanged =& ~0100000;
	flushio ();
}

/*

Name:
	globread

Function:
	Read in global commands.

Algorithm:
	Fill global buffer with commands after global command.

Parameters:
	None.

Returns:
	None or error reset.

Files and Programs:
	None.


*/
globread()
{
	register char *gp,c;
	gp = globuf;
	while ((c = getchar()) != '\n') {
		if (c==EOF)
			ERROR;
		if (c=='\\') {
			c = getchar();
			if (c!='\n')
				*gp++ = '\\';
		}
		*gp++ = c;
		if (gp >= &globuf[GBSIZE-2])
			errfunc(BUFERR);
	}
	*gp++ = '\n';
	*gp++ = 0;
}

/*

Name:
	substitute

Function:
	Perform substitute command.

Algorithm:
	Compile regular expression.  For range of addresses, look for regular expression
	match.  If found, perform substitution.  If global, continue executing substitution
	for rest of line.  Put line in file.  Get new text from compiled expression
	and adjust addresses.  If found by compiler, use number of times to skip
	over matches before starting to process command.  Note that first call to
	execute gets line and other calls use existing line pointers.

Parameters:
	Global substitution flag.

Returns:
	None or error reset.

Files and Programs:
	None.


*/
substitute(inglob)
{
	register gsubf, *a1, nl;
	int getsub();
	int xcnt;
	int xsskip;

	sskip = 0;
	gsubf = compsub();
	for (a1 = addr1; a1 <= addr2; a1++) {
		if (execute(0, a1)==0)
			continue;
		if(sskip) {
			xcnt = 0;
			for(xsskip = sskip;xsskip--;) {
				dosub(1);
				if(execute(1) == 0)
					break;
				xcnt++;
			}
			if(!xcnt)
				continue;
		}
		inglob =| 01;
		dosub(0);
		if (gsubf) {
			while (*loc2) {
				if (execute(1)==0)
					break;
				dosub(0);
			}
		}
		*a1 = putline();
		nostty++;
		nl = append(getsub, a1);
		a1 =+ nl;
		addr2 =+ nl;
	}
	if (inglob==0)
		ERROR;
}

/*

Name:
	compsub

Function:
	Compile substitute command.

Algorithm:
	Get delimiter of regular expression.  If potential delimiter is a number,
	save number as a count of regular expression occurrences to skip before
	processing the substitute.  If potential delimiter is illegal (a-z, A-Z,
	0-9), check 1) to see if previous values should be used, 2) for a global
	and save it and scan to end of line, and 3) for global and return global
	indicator.  If legal delimiter, compile regular expression (left hand).
	Then gather substitution string (right hand), then check for global within
	line.

Parameters:
	None.

Returns:
	Global flag or error reset.

Files and Programs:
	None.


*/
compsub()
{
	register seof, c;
	register char *p;
	int gsubf;

	if ((seof = getchar()) == '\n')
		return(0);
	if(seof >= '0' && seof <= '9') {
		sskip = seof - '0' - 1;
		if((seof = getchar()) == '\n')
			return(0);
	}
	if((seof >= 'a' && seof <= 'z') || (seof >= 'A' && seof <= 'Z')) {
		if((seof != 'g') && (seof != 'G')) peekc=seof;
		newline();
		if((seof == 'g') || (seof == 'G')) return(1);	/* signal global parm */
		else return(0);
		}
	delimit = seof;
	compile(seof);
	p = rhsbuf;
	for (;;) {
		c = getchar();
		if (c=='\\')
			c = getchar() | 0200;
		if (c=='\n')
			ERROR;
		if (c==seof)
			break;
		*p++ = c;
		if (p >= &rhsbuf[LBSIZE/2])
			errfunc(BUFERR);
	}
	*p++ = 0;
	peekc = getchar();
	if ((peekc == 'g') || (peekc == 'G')) {
		peekc = 0;
		newline();
		return(1);
	}
	newline();
	return(0);
}

/*

Name:
	getsub

Function:
	Copy substitution string into buffer.

Algorithm:
	Copy substitution string into buffer.

Parameters:
	None.

Returns:
	0 = normal
	EOF = none

Files and Programs:
	None.


*/
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

/*

Name:
	dosub

Function:
	Perform actual substitution of right hand string for regular expression.

Algorithm:
	Copy line up to matched string.  If not temporary substitution, substitute
	right hand string and copy rest of line to general buffer.  Then copy line
	buffer back to general buffer.  If temporary substitution, copy rest of matched
	string to general buffer.  Update pointers and copy rest of line to general
	buffer.  Then copy general buffer back into line buffer.

Parameters:
	None.

Returns:
	None or error reset.

Files and Programs:
	None.


*/
dosub(atmp)
int atmp;
{
	register char *lp, *sp, *rp;
	int c;

	lp = linebuf;
	sp = genbuf;
	rp = rhsbuf;
	while (lp < loc1)
		*sp++ = *lp++;
	if(atmp) {
		while(lp < loc2)
			*sp++ = *lp++;
	}
	else {
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
				errfunc(BUFERR);
		}
	}
	lp = loc2;
	loc2 = sp + linebuf - genbuf;
	while (*sp++ = *lp++)
		if (sp >= &genbuf[LBSIZE])
			errfunc(BUFERR);
	lp = linebuf;
	sp = genbuf;
	while (*lp++ = *sp++);
}

/*

Name:
	place

Function:
	Copy partial string to destination.

Algorithm:
	Copy to destination buffer, from positions l1 to l2.

Parameters:
	Destination pointer
	From pointer
	To pointer

Returns:
	Pointer to next character after last copy place.

Files and Programs:
	None.


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
			errfunc(BUFERR);
	}
	return(sp);
}

/*

Name:
	move

Function:
	Move or copy lines.

Algorithm:
	Get range of lines and target address.  If transfer, copy lines.  Otherwise,
	move them.  Adjust lines around target address.  Update temp files.

Parameters:
	Copy flag.

Returns:
	None or error reset.

Files and Programs:
	None.


*/
move(copy)
int copy;
{
	register int *adt, *ad1, *ad2;
	int getcopy();

	setdot();
	nonzero();
	if ((adt = address())==0)
		ERROR;
	newline();
	fchanged = 0100000;
	if (copy) {
		ad1 = dol+1;
		nostty++;
		append(getcopy,dol);
		/* getcopy delivers addr1 thru addr2; append changes dol */
		ad2 = dol+1;
		}
	else {
		ad1 = addr1;
		ad2 = addr2+1;
		}
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
		ERROR;
	flushio();
}

/*

Name:
	reverse

Function:
	Swap lines.

Algorithm:
	Interchange 2 address pointers.

Parameters:
	2 address pointers.

Returns:
	None.

Files and Programs:
	None.


*/
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

/*

Name:
	getcopy

Function:
	Get copy of text line.

Algorithm:
	Get copy of text line.

Parameters:
	None.

Returns:
	0 = normal
	EOF = address at end of range.

Files and Programs:
	None.


*/
getcopy()
{
	if (addr1 > addr2)
		return(EOF);
	getline(*addr1++);
	return(0);
}

/*

Name:
	compile

Function:
	Interpret regular expression.

Algorithm:
	Interpret meta and escaped characters and construct regular expression
	buffer with code characters in place of user characters.  Substitute
	paren string for column (\number) shorthand.

Parameters:
	Regular expression delimiter.

Returns:
	None or error reset.

Files and Programs:
	None.


*/
compile(aeof)
{
	register eof, c;
	register char *ep;
	char *lastep;
	char  *pt1, *pt2;
	char bracket[NBRA], *bracketp;
	int nbra;
	int cclcnt;

	ep = expbuf;
	eof = aeof;
	bracketp = bracket;
	colread = 0;
	nbra = 0;
	if ((c = getchar()) == eof) {
		if (*ep==0)
			ERROR;
		return;
	}
	else if (c == '\n') {
		peekc = c;
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
			if((c = getchar()) >= '1' && c <= '9')
			{
				colbufp = &colbuf;
				colread = 1;
				pt2 = &colbuf;
				c =- '0';
				if(c < 1)	goto cerror;
				for(; c > 0; c--)
				{
					pt1 = &colperm;
					while(*pt2++ = *pt1++);
					*--pt2 = '\t';
					pt2++;
				}
				*--pt2 = '\0';
				continue;
			}
			else	if(c == '0')	goto cerror;
			if (c == '(') {
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
			if (c=='\n')
				cerror;
			goto defchar;

		case '.':
			*ep++ = CDOT;
			continue;

		case '\n':
			*ep++ = CEOF;
			peekc = c;
			return;

		case '*':
			if (*lastep==CBRA || *lastep==CKET)
				ERROR;
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
	colread = 0;
	expbuf[0] = 0;
	ERROR;
}

/*

Name:
	execute

Function:
	Look for next regular expression match.

Algorithm:
	Either get line (flag = 0) or use existing line (flag = 1).
	Search line from current position, looking for match of regular
	expression with text line.

Parameters:
	Get line flag
	Address.

Returns:
	0 = no match
	1 = match

Files and Programs:
	None.


*/
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

/*

Name:
	advance

Function:
	Match line with regular expression.

Algorithm:
	Use compiled regular expression and scan text line for a match.

Parameters:
	Line buffer pointer
	Expression buffer pointer

Returns:
	1 = match
	0 = no match

Files and Programs:
	None.


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
		ERROR;
	}
}

/*

Name:
	cclass

Function:
	Look for character class match.

Algorithm:
	Compare character against character class set.  Return calling routine's
	flag or inverse.

Parameters:
	Pointer to character set
	Target character
	Return flag

Returns:
	Flag or inverse.

Files and Programs:
	None.


*/
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

/*

Name:
putd

Function:
	Print number.

Algorithm:
	Break down binary number by digit into decimal ascii.  Call recursively for each digit.
	No newline at end.

Parameters:
	Binary number to be printed.

Returns:
	None.

Files and Programs:
	None.


*/
putd(num)
	int   *num;
{
	register r;
	extern ldivr;

	num[1] = ldiv(num[0], num[1], 10);
	num[0] = 0;
	r = ldivr;
	if (num[1])	putd(&num[0]);
	putchar(r + '0');
}

/*

Name:
	puts

Function:
	Print string.

Algorithm:
	Print each character of string.  Append newline.

Parameters:
	String pointer.

Returns:
	None.

Files and Programs:
	None.


*/
puts(as)
{
	register char *sp;

	sp = as;
	col = 0;
	while (*sp)	putchar(*sp++);
	putchar('\n');
}

/*

Name:
	putchar

Function:
	Print character.

Algorithm:
	Buffer output characters on standard output.  If list mode, interpret
	control characters.  Flush buffer at end of line or column 64.

Parameters:
	Character to be printed.

Returns:
	None.

Files and Programs:


*/
putchar(ac)
{
	register char i, c;
	static char line[70], linx;

	i = linx;
	c = ac;
	if (listf == 1) {
		col++;
		if (col >= 72) {
			col = 0;
			line[i] = '\\';
			line[i+1] = '\n';
			i =+ 2;
		}
		if (c=='\t') {
			c = '>';
			goto esc;
		}
		if (c=='\b') {
			c = '<';
		esc:
			line[i] = '-';
			line[i+1] = '\b';
			line[i+2] = c;
			i =+ 3;
			goto out;
		}
		if ((c<' ' && c!= '\n') || c==0177) {
			line[i] = '\\';
			line[i+1] = (c>>6&07)+'0';
			line[i+2] = (c>>3&07)+'0';
			line[i+3] = (c&07)+'0';
			i =+ 4;
			col =+ 3;
			goto out;
		}
	}
	line[i++] = c;
out:
	if(c == '\n' || i >= 64) {
		write(1, line, i);
		i = 0;
	}
	linx = i;
}
