#
/* file ned.t.c: terminal interface programs for new RAND editor */
/*   Walt Bilofsky - 14 January 1977 */

#include "ned.defs"

#define RETURNC 015	/* keyboard representation of ccreturn */
#define LINEFEEDC 012	/* keyboard representation of ccmovedown */

/* these are the codes for the DATAMEDIA terminals */
char mleftchar 010;
char mrightchar 034;
char mdownchar 012;
char mupchar 032;
char mcreturnchar 015;
char mclscreenchar 036;
char mhomechar 002;

/* putup(l0,lf) - puts up lines from l0 to lf.
	Special feature - if l0 is negative, only puts up line lf, and
	takes it from cline instead of disk. Also only writes from col -lo.*/

putup(lo,lf)
int lo,lf;
	{
	register int i, l0, fc;
	int j,k,l1;
	char lmc,*cp,c,rmargflg;
	l0 = lo;
	lo =+ 2;
	if (lo > 0) lo = 0;
	l1 = lo;
	lmc = (curport->lmarg == curport->ltext ? 0 :
		curwksp->ulhccno == 0 ? LMCH : MLMCH);
	rmargflg = (curport->ltext + curport->rtext < curport->rmarg);
	while (l0 <= lf)
		{
		lo = l1;
		if (l0 < 0)
			{
			l0 = lf;
			lf = -1;
			i = 0;
			}
		else
			{
			if (l0 != lf && intrup())  return;
			if (i = wseek(curwksp,curwksp->ulhclno + l0)
				&& lmc != 0) lmc = ELMCH;
			}

		if (lmc == 0 || lo < 0) poscursor(0,l0);
		else
			{
			poscursor(-1,l0);
			putch(lmc,0);
			}
		curport->lmchars[l0] = lmc;
		if (rmargflg != 0) rmargflg = RMCH;
		if (i != 0) i = 0;
		else
			{
			if (lf >= 0) c = chars();
			i = (ncline - 1) - curwksp->ulhccno;
			if (i < 0) i = 0;
			else if (i > curport->rtext)
				{
				if (i > 1 + curport->rtext && rmargflg)
					rmargflg = MRMCH;
				i = 1 + curport->rtext;
				}
			}
	/* put out the characters directly. */

		/* try to skip over initial blanks */

		if (lo == 0)
			{
			for (fc=0;cline[curwksp->ulhccno + fc]==' ';fc++);

			lo = (curport->firstcol)[l0] > fc ?
			       - fc : - (curport->firstcol)[l0];

			if (i+lo<=0) lo = 0;
			else curport->firstcol[l0] = fc;


			}

		if (lo) poscursor(-lo,l0);
		j = i + lo;
		cp = cline + curwksp->ulhccno - lo;
		while(j--) putcha(*cp++);
		dumpcbuf();
		cursorcol =+ (i + lo);
		if (curport->lastcol[l0] < cursorcol)
			curport->lastcol[l0] = cursorcol;
	/* blank out as much of remainder of line as needed. */
		k = (j = curport->lastcol[l0]) - i;
		if (k > 0)
			{
			write(1,&blanks,k);
			cursorcol =+ k;
			}
		fixcurs();
		if (rmargflg)
			{
			poscursor(curport->rmarg - curport->ltext, l0);
			putch(rmargflg,0);
			}
		else movecursor(0);
		curport->rmchars[l0] = rmargflg;

		curport->lastcol[l0] = (k > 0 ? i : j);
		++l0;
		}
	}
/* movew(nl) - moves current port n lines up/down. */

movew(nl)
int nl;
{
register int cc,cl;
if ((curwksp->ulhclno =+ nl) < 0) curwksp->ulhclno = 0;
cc = cursorcol;
cl = cursorline - nl;
putup(0,curport->btext);
if (cl < 0 || cl > curport->btext)
	{
	cl = defplline;
	cc = 0;
	}
poscursor(cc,cl);
}



/* movep(nc) - moves port nc columns right. */

movep(nc)
int nc;
{
register int cl,cc;
cl = cursorline;
cc = cursorcol;
if ((curwksp->ulhccno + nc) < 0) nc = - curwksp->ulhccno;
curwksp->ulhccno =+ nc;
putup(0,curport->btext);
if ((cc =- nc) < curport->ledit) cc = curport->ledit;
  else if (cc > curport->redit) cc = curport->redit;
poscursor(cc,cl);
}

gtfcn(number)
int number;
	{
	register int i;
	movew(number - curwksp->ulhclno - defplline);
	if ((i = number - curwksp->ulhclno) >= 0) {
		if (i > curport->btext) i = curport->btext;
		poscursor(cursorcol,i);
	}       }
/* routine to position cursor */
poscursor(col,lin)
int col,lin;
	{
	if (cursorline == lin)
		{
		if (cursorcol == col) return;
		if (cursorcol == col-1)
			{
			putchb(mrightchar);
			++cursorcol;
			return;
			}
		if (cursorcol == col+1)
			{
			putchb(mleftchar);
			--cursorcol;
			return;
			}
		}

	if (cursorcol == col)
		{
		if (cursorline == lin-1)
			{
			putchb(mdownchar);
			++cursorline;
			return;
			}
		if (cursorline == lin+1)
			{
			putchb(mupchar);
			--cursorline;
			return;
			}
		}

	cursorcol = col;
	cursorline = lin;
	col =+ curport->ltext;
	lin =+ curport->ttext;
	pcursor(col,lin);
	return;
	}


pcursor(col,lin)
int col,lin;
	{
	putchb(014);
	putchb(0140 - (0140 & col) + (037 & col));
	putchb(lin + 0140);
	}


/* routine to move cursor within boundaries of viewport w -
		according to value of argument:
       UP move up a line
       CR carriage return
       DN move down a line
       RT forward space
       LT backspace
       TB tab
       BT backtab
	0 no-op
*/
movecursor(arg)
int arg;

{
	register int lin,col,i;
	lin=cursorline;
	col=cursorcol;
	switch (arg)
	{

case 0: break;

case HO:
	col = lin = 0;
	break;

case UP:
	--lin;
	break;

case RN:
	col = curport->ledit;	    /* break not needed */

case DN:
	++lin;
	break;

case RT:
	++col;
	break;

case LT:
	--col;
	break;

case TB:
	i=0;
	col = col + curwksp->ulhccno;
	while (col >= tabstops[i]) i++;
	col = tabstops[i] - curwksp->ulhccno;
	break;

case BT:
	i=0;
	col = col + curwksp->ulhccno;
	while (col >  tabstops[i]) i++;
	col = (i ? tabstops[i-1] - curwksp->ulhccno : -1);
	break;
	}

	if (col > curport->redit) col = curport->ledit;
	else if (col < curport->ledit) col = curport->redit;

	if (lin < curport->tedit) lin = curport->bedit;
	else if (lin > curport->bedit) lin = curport->tedit;

	poscursor(col,lin);
	return;
}



/*
read another character from the input stream.
If the last readacter wasn't used up (lread1 still
>= 0) don't read after all.  If there are any
characters in the output buffer, boomp them
out. */

extern int knockdown 0;

 char
read1()
{
dumpcbuf();
if (lread1 != -1) return (lread1);
lread1 = read2();
if (ttyfile >= 0) write(ttyfile,&lread1,1);

/* convert from keyboard to internal codes for newline, linefeed */
if (knockdown && lread1 < 040) lread1 =+ 0100;
knockdown = 0;
if (lread1 == RETURNC) lread1 = CCRETURN;
else if (lread1 == LINEFEEDC) lread1 = CCMOVEDOWN;
else if (lread1 == CCCTRLQUOTE)
	{
	knockdown = 1;
	lread1 = 0377;		 /* so not to look like CCQUIT	*/
	}
return (lread1);
}

char intrupchar -1;             /* one char read ahead for intrup() */

read2()
/* return the next character from the input stream */
{
char l;
if (intrupchar >= 0) {
	l = intrupchar;
	intrupchar = -1;
	}
 else while (read(inputfile,&l,1) == 0) {
	close(inputfile);
	if (inputfile == 0) return CCQUIT;
	inputfile = 0;
	}
return l;
}

intrup()
/* decide whether to interrupt because another control fcn has been typed */
/* Interrupt only on control keys, except motion keys, INS, ARG and SRCH */
{       if (intrupchar == -1) {
		/* if (inputfile == 0	Changed for CVL system */
		if (inputfile || empty(0)>0 /* Changed for empty call */
			) return 0;
		intrupchar = read2();
		}
	if (intrupchar == 0177) return 1;
	if (intrupchar >= 040) return 0;
	if (cntlmotions[intrupchar]) return 0;
	if (intrupchar == CCENTER || intrupchar == CCPLSRCH ||
		intrupchar == CCMISRCH || intrupchar == CCINSMODE) return 0;
	return 1;
	}
/* put a character up at current cursor position */
putch(j,flg)
int flg;
char j;
	{
	register char *cp;
	register int cl;
	if (flg && lread1 != ' ')
		{
		if ((curport->firstcol)[cursorline] > cursorcol)
			(curport->firstcol)[cursorline] = cursorcol;
		if ((curport->lastcol)[cursorline] <= cursorcol)
			(curport->lastcol)[cursorline] = cursorcol+1;
		}
	++cursorcol;
	if (fixcurs() == 0) putcha(j);
	if (cursorcol <= 0) poscursor(curport->ledit,
		cursorline < curport->tedit ? curport->tedit :
		cursorline > curport->bedit ? curport->tedit :
		cursorline);
	movecursor(0);
	}

/* fixcurs() - adjust cursorcol, cursorline for edge effects of screen */
/* sets cursorcol, cursorline to correct values if they were positioned
	past right margin.  Proper use is to increment cursorcol, call
	fixcurs(), output character, and then position cursor if
	cursorcol <= 0.  fixcurs() returns 1 if cursor was incremented
	from brhc of screen - in this case do not put out a character
	since screen will scroll (but fixcurs() has homed cursor so
	pretend character was put out). */

fixcurs()
{
	if (curport->ltext + cursorcol >= LINEL) {
		cursorcol = - curport->ltext;
		if (curport->ttext + ++cursorline >= NLINES) {
			cursorline = - curport->ttext;
			putcha(mhomechar);
			return (1);
			}
		}
	return (0);
	}

/*put a character into the output buffer */

#define nputcbuf 25
char putcbuf[nputcbuf];
int iputcbuf 0;

putcha(c)
char c;
	{
	register char cr;
	cr = c;
	if(cr & 0200 || cr == 0177) {
		dumpcbuf();
		putcbuf[0] = 016;
		cr =& 0177;
		putcbuf[1] = (cr == 0177 ? '*' : cr);
		putcbuf[2] = 030;
		iputcbuf = 3;
	}
	else {
		putcbuf[iputcbuf++] = cr;
		if (iputcbuf == nputcbuf) dumpcbuf();
	}
	return;
	}

/* This is the original putcha routine...changed for blinking chars */
putchb(c)
char c;
	{
	putcbuf[iputcbuf++] = c;
	if (iputcbuf == nputcbuf) dumpcbuf();
	return;
	}

dumpcbuf()
	{
	if (iputcbuf != 0) write(1,&putcbuf,iputcbuf);
	iputcbuf = 0;
	}


/* routine to handle screen functions for switching to
	a new viewport.  changes cursorline, cursorcol to
	be relative to new ulhc. */
switchport(w)
struct viewport *w;
{
cursorcol  =- (w->ltext - curport->ltext);
cursorline =- (w->ttext - curport->ttext);
curport = w;
if (curwksp = curport->wksp) curfile = curwksp->wfile;
}
/* setupviewport - initialize the viewport .
	c = 1 if editing window -- i.e. boarders, etc.	*/

setupviewport(ww,cl,cr,lt,lb,c)
struct viewport *ww;
int cl,cr,lt,lb,c;
	{
	register int i,size;
	register struct viewport *w;
	w = ww;
	w->lmarg = cl;
	w->rmarg = cr;
	w->tmarg = lt;
	w->bmarg = lb;
	if (c)
		{
		w->ltext = cl + 1;
		w->rtext = cr - cl - 2;
		w->ttext = lt + 1;
		w->btext = lb - lt - 2;
		}
	else
		{
		w->ltext = cl;
		w->rtext = cr - cl;
		w->ttext = lt;
		w->btext = lb - lt;
		}
	w->ledit = w->tedit = 0;
	w->redit = w->rtext;
	w->bedit = w->btext;
	/* eventually this extra space may not be needed */
	w->wksp = salloc(SWKSP);
	w->altwksp = salloc(SWKSP);
	w->firstcol = salloc(size = lb - lt + 1);
	for (i=0;i<size;i++) (w->firstcol)[i] = w->rtext + 1;
	w->lastcol = salloc(size);
	w->lmchars = salloc(size);
	w->rmchars = salloc(size);
	}


/* " make a new viewport " command */

makeport(file)
char *file;
	{
	register struct viewport *oldport, *newport;
	char horiz;		/* 1 if margin horiz, 0 if vert */
	register int i;
	int j, portnum;

	/* check if room on portlist */
	if (nportlist >= MAXPORTLIST)
		{
		error("Can't make any more windows.");
		return;
		}

	if (cursorcol == 0 && cursorline > 0
			   && cursorline < curport->btext) horiz = 1;
	else if (cursorline == 0 && cursorcol > 0
			    && cursorcol < curport->rtext-1) horiz = 0;
	else
		{
		error("Can't put a window there.");
		return;
		}

	oldport = curport;
	newport = portlist[nportlist++] = salloc(SVIEWPORT);

	/* the number of curport is new prevport */
	for (portnum=0;portlist[portnum]!=curport;portnum++);
	newport->prevport = portnum;

	oldport->wksp->ccol = oldport->altwksp->ccol = 0;
	oldport->wksp->crow = oldport->altwksp->crow = 0;

	if (horiz)
		{
		setupviewport(newport,oldport->lmarg,
				      oldport->rmarg,
				      oldport->tmarg + cursorline + 1,
				      oldport->bmarg,1);
		oldport->bmarg = oldport->tmarg + cursorline + 1;
		oldport->btext = oldport->bedit = cursorline - 1;
		for (i=0;i <= newport->btext;i++)
			{
			newport->firstcol[i] =
				oldport->firstcol[i+cursorline+1];
			newport->lastcol[i] =
				oldport->lastcol[i+cursorline+1];
			}
		}
	else
		{
		setupviewport(newport,oldport->lmarg + cursorcol + 2,
				      oldport->rmarg,
				      oldport->tmarg,
				      oldport->bmarg,1);
		oldport->rmarg = oldport->lmarg + cursorcol + 1;
		oldport->rtext = oldport->redit = cursorcol - 1;
		for (i=0;i <= newport->btext;i++)
			{
			if (oldport->lastcol[i] > oldport->rtext + 1)
				{
				newport->firstcol[i] = 0;
				newport->lastcol[i] = oldport->lastcol[i] -
					cursorcol - 2;
				oldport->lastcol[i] = oldport->rtext + 1;
				oldport->rmchars[i] = MRMCH;
				}
			}
		}

	switchport(newport);
	if (editfile(file,0,0,1,1) <= 0 && editfile(deffile,0,0,0,1) <= 0)
		error("Default file gone: notify sys admin.");
	drawport(oldport,newport);
	defplline = defmiline = (newport->bmarg - newport->tmarg)/ 4 + 1;

	poscursor(0,0);
	}

/*
removeport() -- eliminates the last made port by expanding its ancestor */

removeport()
	{
	int j, pnum;
	register int i;
	register struct viewport *theport, *pport;
	if (nportlist == 1)
		{
		error ("Can't remove remaining port.");
		return;
		}
	pport = portlist[pnum = (theport = portlist[--nportlist])->prevport];

	if (pport->bmarg != theport->bmarg)
		{
		/* Vertical */
		pport->firstcol[j = pport->btext+1] = 0;
		pport->lastcol[j++] = pport->rtext+1;
		for (i=0;i<=theport->btext;i++)
			{
			pport->firstcol[j+i] = theport->firstcol[i];
			pport->lastcol[j+i] = theport->lastcol[i];
			}
		pport->bmarg = theport->bmarg;
		pport->bedit = pport->btext = pport->bmarg - pport->tmarg - 2;
		}
	else
		{
		/* Horizontal */
		for (i=0;i<=pport->btext;i++)
			{
			pport->lastcol[i] = theport->lastcol[i] +
				theport->lmarg - pport->lmarg;
			if (pport->firstcol[i] > pport->rtext)
				pport->firstcol[i] = pport->rtext;
			}
		pport->rmarg = theport->rmarg;
		pport->redit = pport->rtext = pport->rmarg - pport->lmarg - 2;
		}
	chgport(pnum);
	putup(0,curport->btext);
	poscursor(0,0);
DEBUGCHECK;
	free(theport->firstcol);
	free(theport->lastcol);
	free(theport->lmchars);
	free(theport->rmchars);
	free(theport->wksp);
	free(theport->altwksp);
	free(theport);
DEBUGCHECK;
	}
/* param() - gets a parameter
	There are three types of parameters:
		paramtype = -1 -- cursor defined
		paramtype = 0  -- just <arg> <function>
		paramtype = 1  -- string, either integer or text:
	returns pointer to the text string entered.
	the pointer is left in the global variable paramv, and
	its length in bytes (if a string) in paraml.  next
	time param is called, if paraml is nonzero, it will
	free up the space, so if you want to keep the string
	around in free storage, set paraml=0. */

#define LPARAM 20	/* length of free increment */

char *param()
{
	register char *c1;
	char *cp,c,*c2;
	register int i,pn;
	struct viewport *w;

	if (paraml != 0 && paramv != 0) free(paramv);

	paramc1 = paramc0 = cursorcol;
	paramr1 = paramr0 = cursorline;
	putch(0177,1);
	poscursor(cursorcol,cursorline);
	w = curport;

   back:
	telluser("ARG: ",0);
	switchport(&paramport);
	poscursor(5,0);

	do
		{
		lread1 = -1;
		read1();
		}
	while (lread1 == CCBACKSPACE);

	if (CONTROLCHAR && cntlmotions[lread1])
		{
		telluser("ARG: *** CURSOR DEFINED ***",0);
		switchport(w);
		poscursor(paramc0,paramr0);
	     t0:
		while (CONTROLCHAR && (i=cntlmotions[lread1]))
			{
			movecursor(i);
			if (cursorline == paramr0 &&
			    cursorcol  == paramc0)  goto back;
			lread1 = -1;
			read1();
			}
		if (CTRLCHAR && lread1 != CCBACKSPACE)
			{
			paramc1 = cursorcol;
			paramr1 = cursorline;
			paraml = paramv = 0;
			paramtype = -1;
			}
		else
			{
			error("Printing character illegal here");
			lread1 = -1;
			read1();
			goto t0;
			}
		}

	else if (CTRLCHAR)
		{
		paraml = paramv = 0;
		paramtype=0;
		}
	else
		{
		paraml = pn = 0;
	   loop:
		c = read1();
		if (pn >= paraml)
			{
			cp = paramv;
			paramv = alloc(paraml + LPARAM);
			c1 = paramv;
			c2 = cp;
			for (i=0; i<paraml; ++i) *c1++ = *c2++;
			if (paraml) free(cp);
			paraml =+ LPARAM;
			}
		/* this is where it decides what terminates parameter string*/
		if (CONTROLCHAR)
			{
			if (c == CCBACKSPACE &&  cursorcol != curport->ledit)
			/* backspace */
				{
				if (pn == 0)
					{
					lread1 = -1;
					goto loop;
					}
				movecursor(LT);
				--pn;
				paramv[pn] = 0;
				putch(' ',0);
				movecursor(LT);
				lread1 = -1;
				if (pn == 0) goto back;
				goto loop;
				}


			else c = 0;
			}

		if (c == 0177) c = 0;	 /* del is a contol code  */
		paramv[pn++] = c;
		if (c != 0)
			{
			putch(c,0);
			lread1 = -1;
			goto loop;
			}
		paramtype = 1;
		}

	switchport(w);
	putup(paramr0,paramr0);
	poscursor(paramc0,paramr0);
	return (paramv);
	}
/*
getarg() -- get the argument from the file  all characters up to space
*/
getarg()
	{
	register int col, i;
	register char *ch;
	paramv = paraml = 0;
	getline(curwksp->ulhclno + paramr0);
	if ((col = curwksp->ulhccno+paramc0) >= ncline-1  ||
	    cline[col] == ' ') return;
	for (ch=cline+col;*ch != ' ' && *ch != NEWLINE;*ch++) paraml++;
	paramv = salloc(paraml+1);
	for (i=0; i<paraml; i++) paramv[i] = cline[col+i];
	paramv[paraml] = '\0';
	}

/* chgport(portnum) - changes the current port */

chgport(portnum)
int portnum;
	{
	register struct viewport *oldport, *newport;
	oldport = curport;
	if (portnum < 0) for (portnum=0;portnum<nportlist &&
		oldport != portlist[portnum++];);
	oldport->wksp->ccol = cursorcol;
	oldport->wksp->crow = cursorline;
	newport = portlist[portnum%nportlist];
	if (newport == oldport) return;
	drawport(oldport,newport);
	defplline = defmiline = (newport->bmarg - newport->tmarg) / 4 + 1;


	poscursor(curport->wksp->ccol,curport->wksp->crow);
	}

drawport(oldp,newp)
struct viewport *newp, *oldp;
	{
	register struct viewport *oldport, *newport;
	register int i;
	int j, scol, sline;

	oldport = oldp;
	newport = newp;

	scol = cursorcol;
	sline = cursorline;

	switchport(&wholescreen);
	poscursor(oldport->lmarg,oldport->tmarg);
	for (i = oldport->lmarg;i <= oldport->rmarg;i++) putch(DOTCH,0);
	for (j = oldport->tmarg + 1;j <= oldport->bmarg - 1;j++)
		{
		poscursor(oldport->lmarg,j);
		putch(DOTCH,0);
		poscursor(oldport->rmarg,j);
		putch(DOTCH,0);
		}
	poscursor(oldport->lmarg,oldport->bmarg);
	for (i = oldport->lmarg;i <= oldport->rmarg;i++) putch(DOTCH,0);

	poscursor(newport->lmarg,newport->tmarg);
	for (i = newport->lmarg;i <= newport->rmarg;i++) putch(TMCH,0);
	for (j = newport->tmarg + 1;j <= newport->bmarg - 1;j++)
		{
		poscursor(newport->lmarg,j);
		putch(newport->lmchars[j - newport->tmarg - 1],0);
		poscursor(newport->rmarg,j);
		putch(newport->rmchars[j - newport->tmarg - 1],0);
		}
	poscursor(newport->lmarg,newport->bmarg);
	for (i = newport->lmarg;i <= newport->rmarg;i++) putch(BMCH,0);

	poscursor(scol,sline);
	switchport(newport);
	}
/*error(msg) and telluser(msg,col)    */
error(msg)
char *msg;
	{
	putcha(007); putcha(007);
	telluser("**** ",0);
	telluser(msg,5);
	errsw = 1;
	}



telluser(msg,col)
char *msg;
int col;
	{
	struct viewport *oldport;
	register int c,l,i;
	oldport = curport;
	c = cursorcol;
	l = cursorline;
	switchport(&paramport);
	if (col == 0)
		{
		poscursor(0,0);
		dumpcbuf();
		write(1,&blanks,cursorcol = paramport.redit);
		}
	poscursor(col,0);
	while (*msg) putch(*msg++,0);
	switchport(oldport);
	poscursor(c,l);
	dumpcbuf();
	}

/* redisplay(w,w->wfile,from,to,delta) - some pollyanna has just diddled the workspace
	associated with workspace w, involving lines from-to inclusive,
	with a total change of delta in the length of the workspace.
	We are supposed to:
    1. Redisplay any other viewports which are actually changed by this
	tampering, and
    2. Adjust the position in the fsd chain of any viewport which may
	have had its curfsd pointer moved out from under it, and
    3. Adjust the current line number of any port which may be pointing
	further down in the workspace than the change, and doesn't want
	to suffer any apparent motion. */

redisplay(w,fn,from,to,delta)
struct workspace *w;
int from,to,delta,fn;
	{
	register struct workspace *tw;
	register int i,j;
	int k,l,m;
	struct viewport *oport;
	for (i = 0; i < nportlist; i++)
		{
		if ((tw = portlist[i]->altwksp)->wfile == fn && tw != w)
			{
			/* repoint the file to avoid pointer muckup. */
			tw->curlno = tw->curflno = 0;
			tw->curfsd = openfsds[fn];
			/* adjust current line number, if necessary */
			j = delta >= 0 ? to : from;
			if (tw->ulhclno > j) tw->ulhclno =+ delta;
			}
		if ((tw = portlist[i]->wksp)->wfile == fn && tw != w)
			{
			/* repoint the file to avoid pointer muckup. */
			tw->curlno = tw->curflno = 0;
			tw->curfsd = openfsds[fn];
			/* adjust current line number, if necessary */
			j = delta >= 0 ? to : from;
			if (tw->ulhclno > j) tw->ulhclno =+ delta;
			/* do we have to redisplay anything? */
			j = (from > tw->ulhclno ? from : tw->ulhclno);
			if ((k =  tw->ulhclno + portlist[i]->btext ) > to) k = to;
			if (j <= k)
				{
				oport = curport;
				l = cursorcol;
				m = cursorline;
				switchport(portlist[i]);
				putup(j - tw->ulhclno, delta == 0 ?
				     k - tw->ulhclno : portlist[i]->btext);
				switchport(oport);
				poscursor(l,m);
				}
			}
		}
	}
/*
clearscreen() -- does just that */

clearscreen()
	{
	putcha(mclscreenchar);
	putcha(mhomechar);	/* needed by 9600 baud terminal  */
	}
