#
/* file ned.c - main program for new RAND editor */
/*   Walt Bilofsky - 14 January 1977 */

/* arguments for editor

	ned targetfile line col

where

	targetfile is the file to be edited.
		if targetfile begins with a '-' it is a commandfile.
	line, col are the starting line and column numbers for display


*/

#include "ned.defs"

char tmpnstr[13] "/tmp/retmpx.";
char ttystr[13]  "/tmp/rettyx.";
char rstr[13]    "/tmp/resavx.";
char ttynstr[9] "/dev/tty";
char *ttyname, *ttytmp, *rfile;
int templ[4];

struct savebuf pb,db;

int sig(),igsig();

struct iobuffer
	{
	int fildes;
	int nused;
	char *nxtfree;
	char buffer[512];
	}

/* The Main Program */
main(nargs,args)
	int nargs;
	char *args[];
	{
	int i,j,k,l;
	char *cp, c, ichar;
	struct viewport screen;

	startup();

	if (nargs > 1 && *args[1] == '-')
		{
		nargs = 1;
		/* input file - open it up */
		if ((inputfile = open(args[1] + 1,0)) < 0)
			{
			cleanup();
			write(1,"Can't open command file.\n",25);
			exit(0);
			}
		if (read(inputfile,&ichar,1) == 0)
			{
			cleanup();
			write(1,"Command file is empty.\n",23);
			exit(0);
			}

		}
	else
		{
		if (nargs > 1 && (*args[1] == '+' || *args[1] == '!'))
				{
				ichar = *args[1];
				args[1]++;
				}
		else if (nargs == 1) ichar == '+';
		else ichar = ' ';
		}

	getstate(ichar);
	write(ttyfile,&ichar,1); /* for text time */

	if (nargs > 1 && *args[1] != '\0')
		{
		i = j = 1;
		if (nargs > 2) if (s2i(args[2],&i) || i <= 0)  i = 1;
		if (nargs > 3) if (s2i(args[3],&j) || j <= 0)  j = 1;
		poscursor(curwksp->ccol,curwksp->crow);
		if (editfile(args[1],i-1,j-1,1,1) <= 0)
			{
			putup(0,curport->btext);
			poscursor (curwksp->ccol,curwksp->crow);
			}
		else
			{
			writefile(CCENTER,args[1],CCSETFILE);
			if (nargs>2 && i>1) writefile(CCENTER,args[2],CCGOTO);
			if (nargs>3 && j>1)writefile(CCENTER,args[3],CCRPORT);
			if (nargs > 4)
				{
				searchkey = args[4];
				search(1);  /* could be +- */
				writefile(CCENTER,searchkey,CCPLSRCH);
				}
			}
		}

	else
		{
		putup(0,curport->btext);
		poscursor (curwksp->ccol,curwksp->crow);
		}


	mainloop();

	cleanup();
	savestate();	/* set up so re works next time from where left off */
	clearscreen();
	dumpcbuf();
	write(1,"\035",1); /* Turn ROLL back on (DATAMEDIA) */
	if (gosw)
		{
		setuid(userid);
		setgid(groupid);
		printf("\t\t/* Once more, dear friends, into the breach. */\n");
		execl("./compil","compil",0);
		execl("/bin/compil","compil",0);
		execl("/usr/bin/compil","compil",0);
		printf("compil not found\n");
		return;
		}
	}

/*
startup() - the initializing routine */

int oldttmode;

startup()
	{
	register int i;
	register char *c, *name;
	int tbuf[18];

   /*   nice(-200); /* run at high priority for now (-200 actually low!) */
	userid = getuid() & 0377;
	groupid = getgid() & 0377;
	for (i=LINEL; i;) blanks[--i] = ' ';

	if (getpw(userid,pwbuf))
		{
		printf("getpw failed!");
		exit(0);
		}
	name = pwbuf;
	for (c = name; *c != ':'; c++);
	*c = '\0';                 /* erase first colon */
				   /* N.B. - code in execr depends on this */
	*tbuf = ttystr[10] = rstr[10] = tmpnstr[10] = ttyn(0);
	tmpname = append(tmpnstr,name);
	ttytmp	= append(ttystr ,name);
	rfile	= append(rstr	,name);

	ttyname = append(ttynstr,tbuf);

	if ((ttyfile = creat(ttytmp,FILEMODE)) < 0)
		{
		write(1,"Can't open ttyfile.\n",20);
		exit(0);
		}

	if ((i = creat(tmpname,0600)) < 0)
		{
		write(1,"Can't open temporary file.\n",27);
		exit(0);
		}

/*  the following two lines are CRITICAL -- not sure why  */
	close(i);
	i = open(tmpname,2);

	openfnames[i] = tmpname;
	openwrite[i] = 1;
	tempfile = i;

	/* setup file '#' -- used to save deletebuffer and pickbuffer */
	openfnames[2] = "#";
	nlines[2] = 0;
	pickwksp = salloc(SWKSP);
	pickwksp->curfsd = openfsds[2] = salloc(SFSD);
	pickwksp->wfile = 2;
	pickbuf = &pb;
	deletebuf = &db;

	/* set up port for whole screen.  no margins,
		and it is not on portlist. */
	setupviewport(&wholescreen,0,LINEL-1,0,NLINES-1,0);

	/* set up port for parameter entry.  no margins
		and it is not on portlist  */
	setupviewport(&paramport,0,LINEL-1,NLINES-NPARAMLINES,NLINES-1,0);
	paramport.rtext--;
	paramport.redit = PARAMREDIT;

	/* set tty raw mode */
	fstat(0,tbuf);                  /* save old message status and */
	oldttmode = tbuf[2];
	chmod(ttyname,0600);		/* turn off messages */
	gtty(0,templ);
	i = templ[2];
	templ[2] = 040 | (templ[2] & ~ 072);
	stty(0,templ);
	templ[2] = i;	/* all set up for cleanup */

	/* set switch if line speed < 9600 baud */
	if ((templ[0] & 0177400) != 13 << 8) slowsw =+ 1;

	for (i=20; i; i--) signal(i,sig);
	signal(3,igsig);
	signal(2,igsig);

	curport = &wholescreen;
	clearscreen();

	putcha(030);  /* Turn off roll (DATAMEDIA) */

	return;
	}

/*
writefile -- put it out on ttyfile
*/

writefile(code1,str,code2)
int code1, code2;
char *str;
	{
	write(ttyfile,&code1,1);
	for(;*str;str++) write(ttyfile,str,1);
	write(ttyfile,&code2,1);
	}





/*
cleanup() --  cleanup before getting out  */

cleanup()
	{
	/* restore tty mode and exit */
	stty(0,templ);
	close(tempfile);
	unlink(tmpname);
	close(ttyfile);
	chmod(ttyname,07777 & oldttmode);
	}

/*
savestate() -- save it for later  */

savestate()
	{
	int i, nletters;
	register int portnum;
	register char *f1;
	char *fname;
	register struct viewport *port;
	struct iobuffer sbuff, *sbuf;

	curwksp->ccol = cursorcol;
	curwksp->crow = cursorline;
	sbuf = &sbuff;
	if (fcreat(rfile,sbuf) < 0) return;
	putw(nportlist,sbuf);
	for (portnum=0; portnum < nportlist; portnum++)
		if (portlist[portnum] == curport) break;
	putw(portnum,sbuf);
	for (i=0;i<nportlist;i++)
		{
		port = portlist[i];
		putw(port->prevport,sbuf);
		putw(port->lmarg,sbuf);
		putw(port->rmarg,sbuf);
		putw(port->tmarg,sbuf);
		putw(port->bmarg,sbuf);
		if (f1=fname=openfnames[port->altwksp->wfile])
			{
			while (*f1++);
			nletters = f1 - fname;
			putw(nletters,sbuf);
			f1 = fname;
			do putc(*f1,sbuf); while (*f1++);
			putw(port->altwksp->ulhclno,sbuf);
			putw(port->altwksp->ulhccno,sbuf);
			putw(port->altwksp->ccol,sbuf);
			putw(port->altwksp->crow,sbuf);
			}
		else putw(0,sbuf);
		f1 = fname = openfnames[port->wksp->wfile];
		while (*f1++);
		nletters = f1 - fname;
		putw(nletters,sbuf);
		f1 = fname;
		do putc(*f1,sbuf); while (*f1++);
		putw(port->wksp->ulhclno,sbuf);
		putw(port->wksp->ulhccno,sbuf);
		putw(port->wksp->ccol,sbuf);
		putw(port->wksp->crow,sbuf);
		}
	fflush(sbuf);
	close(sbuf->fildes);
	return;
	}

/*
getstate(ichar) -- get  the state
		   ichar == ' ' means use only port portnum, no files
		   ichar == '+' means use old file, for port portnum
		   ichar == '!' means use all ports and all old files
*/

getstate(ichar)
char ichar;
	{
	int nletters, lmarg, rmarg, tmarg, bmarg, row, col, portnum, gf;
	register int i,n;
	register char *f1;
	char *fname;
	struct iobuffer gbuff, *gbuf;
	struct viewport *port;

	gbuf = &gbuff;
	if (fopen(rfile,gbuf) < 0 || (nportlist = getw(gbuf)) == -1)
		{
		makestate();
		return;
		}
	portnum   = getw(gbuf);
	if (ichar != '!')
		{
		/* skip over some ports */
		for (n=0;n<portnum;n++)
			{
			for (i=0;i<5;i++) getw(gbuf);
			if (getw(gbuf))
				{
				while (getc(gbuf));
				for (i=0;i<4;i++) getw(gbuf);
				}
			getw(gbuf); while (getc(gbuf));
			for (i=0;i<4;i++) getw(gbuf);
			}
		portnum = 0;
		nportlist = 1;
		}
	for (n=0;n<nportlist; n++)
		{
		port = portlist[n] = salloc(SVIEWPORT);
		port->prevport = getw(gbuf);
		if (ichar != '!') port->prevport = 0;
		lmarg = getw(gbuf);
		rmarg = getw(gbuf);
		tmarg = getw(gbuf);
		bmarg = getw(gbuf);
		if (ichar != '!')
			{
			lmarg = 0;
			rmarg = LINEL - 1;
			tmarg = 0;
			bmarg = NLINES - NPARAMLINES - 1;
			}
		setupviewport(port,lmarg,rmarg,tmarg,bmarg,1);
		switchport(&wholescreen);
		poscursor(port->lmarg,port->tmarg);
		for (i=lmarg;i<=rmarg;i++) putch(TMCH,0);
		poscursor(port->lmarg,port->bmarg);
		for (i=lmarg;i<=rmarg;i++) putch(BMCH,0);
		poscursor(port->lmarg,port->tmarg+1);
		switchport(port);
		gf = 0;
		if (nletters=getw(gbuf))
			{
			f1 = fname = salloc(nletters);
			do *f1 = getc(gbuf); while (*f1++);
			row = getw(gbuf);
			col = getw(gbuf);
			if (ichar == '!')
			 if (editfile(fname,row,col,0,0) == 1) gf = 1;
			curwksp->ccol = getw(gbuf);
			curwksp->crow = getw(gbuf);
			poscursor(curwksp->ccol,curwksp->crow);
			}
		nletters=getw(gbuf);
		f1 = fname = salloc(nletters);
		do *f1 = getc(gbuf); while (*f1++);
		row = getw(gbuf);
		col = getw(gbuf);
		if (ichar != ' ')
		 if (editfile(fname,row,col,0,(n==portnum ? 0:1)) == 1) gf=1;
		curwksp->ccol = getw(gbuf);
		curwksp->crow = getw(gbuf);
		if (gf == 0)
			{
			if (editfile(deffile,0,0,0,(n==portnum ? 0:1)) <= 0)
			    error("Default file gone: notify sys admin.");
			curwksp->ccol = curwksp->crow = 0;
			}
		poscursor(curwksp->ccol,curwksp->crow);
		}
	switchport(portlist[portnum]);
	poscursor(curwksp->ccol,curwksp->crow);
	if (nportlist > 1)  for (i=0;i<nportlist;i++) chgport(-1);
	close(gbuf->fildes);
	return;
	}
/*
makestate() -- routine to create an initial state
*/
makestate()
		{
		register int i;
		register struct viewport *port;

		nportlist = 1;
		port = portlist[0] = salloc(SVIEWPORT);
		setupviewport(portlist[0],0,LINEL-1,0,NLINES-NPARAMLINES-1,1);
		poscursor(port->lmarg,port->tmarg);
		for (i=0;i<LINEL;i++) putch(TMCH,0);
		poscursor(port->lmarg,port->bmarg);
		for (i=0;i<LINEL;i++) putch(BMCH,0);
		poscursor(port->lmarg,port->tmarg+1);
		switchport(port);
		poscursor(0,0);
		if (editfile(deffile,0,0,0,0) <= 0)
			error("Default file gone: notify sys admin.");
		}

/*
checkpriv() - return 0 if neither read nor write allowed
		     1 if read only
		     2 if read and write
*/
struct cprv
	{
	char minor;
	char major;
	int  inumber;
	int  thebits;
	char nlinks;
	char uid;
	char gid;
	char size0;
	int  size1;
	int  addr[8];
	int  actime[2];
	int modtime[2];
	} ;

checkpriv(fildes)
int fildes;
	{
	register struct cprv *buf;
	struct cprv buffer;
	int anum, num;
	register int unum,gnum;

	if (userid == 0) return 2;   /* superuser accesses all */

	buf = &buffer;
	fstat(fildes,buf);

	unum = gnum = anum = 0;
	if (buf->uid == userid)
		{
		if (buf->thebits & 0200) unum = 2;
		else if (buf->thebits & 0400) unum = 1;
		}
	if (buf->gid == groupid)
		{
		if (buf->thebits & 020) gnum = 2;
		else if (buf->thebits & 040) gnum = 1;
		}
	if (buf->thebits & 02) anum = 2;
	else if (buf->thebits & 04) anum = 1;
	num = (unum > gnum ? unum : gnum);
	num = (num  > anum ? num  : anum);
	return(num);
	}

getpriv(fildes)
int fildes;
	{
	struct cprv buffer,*buf;

	buf = &buffer;
	fstat(fildes,buf);
	return (buf->thebits & 0777);

	}
/*
sig() - interrupt routine to trap errors */
sig()
{
fatal("Fatal signal");
}

/* igsig() - ignores quit signal */
igsig()
{
signal(2,igsig);
signal(3,igsig);
}

/*
fatal(s) -- routine to let user know of mishaps
*/

fatal(s)
char *s;
	{
	register char *ch;
	register int i;
	int *foo, j;
	register struct workspace *w;
	stty(0,templ);
	write(1,"\nFirst the bad news - the RAND editor just \035",44);
	if (s) write (1,"died\n",5);
	    else write(1,"ran out of space.\n",18);
	write(1,  "Now the good news - your editing session can be reproduced\n  from file ",71);
	for (ch=ttytmp;*ch;ch++) write(1,ch,1);
	write(1,", which you should immediately\n  preserve, and then seek help.\n",
		63);
	if (inputfile)
		{
		if (s) printf("%s\n\n",s);
		for (i = 0; i < MAXFILES; i++) if (openfsds[i])
			{
			printf("\n*** OPENFSDS[%d] - file %s\n",i,openfnames[i]);
			printfsd(openfsds[i]);
			}
		for (i = 0; i < nportlist; i++)
			{
			w = portlist[i]->wksp;
			printf("\nViewport #%d: FSD chain %d, current line %d at block %o,\n",
				i,w->wfile,w->curlno,w->curfsd);
			printf(" first line %d, ulhc (%d,%d)\n",w->curflno,w->ulhccno,
				w->ulhclno);
			}
	for (i=12; i; i--) signal(i,0);
	foo = &j;
	foo[ 32000]=foo[ 32000];
	}
close(ttyfile);
exit();
}
strcopy(a,b)
char *a,*b;
{
	while(*a++ = *b++);
}
abs(number)
int number;
	{
	return (number<0 ? -number : number);
	}
/*
 * C library -- alloc/free
 */

#define	logical	char *

struct fb {
	logical	size;
	char	*next;
};

int	freelist[] {
	0,
	-1,
};
logical	slop	2;

alloc(asize)
logical asize;
{
	register logical size;
	register logical np;
	register logical cp;

	if ((size = asize) == 0)
		return(0);
	size =+ 3;
	size =& ~01;
	for (;;) {
		cp = freelist;
		while ((np = cp->next) != -1) {
			if (np->size>=size) {
				if (size+slop >= np->size) {
					cp->next = np->next;
					return(&np->next);
				}
				cp = cp->next = np+size;
				cp->size = np->size - size;
				cp->next = np->next;
				np->size = size;
				return(&np->next);
			}
			cp = np;
		}
		asize = size<1024? 1024: size;
		if ((cp = sbrk(asize)) == -1) {
			fatal(0);

		}
		cp->size = asize;
		free(&cp->next);
	}
}

free(aptr)
char *aptr;
{
	register logical ptr;
	register logical cp;
	register logical np;

	if (aptr == 0) return;
	ptr = aptr-2;
	cp = freelist;
	while ((np = cp->next) < ptr)
		cp = np;
	if (ptr+ptr->size == np) {
		ptr->size =+ np->size;
		ptr->next = np->next;
		np = ptr;
	} else
		ptr->next = np;
	if (cp+cp->size == ptr) {
		cp->size =+ ptr->size;
		cp->next = ptr->next;
	} else
		cp->next = ptr;
}
/* salloc (smart alloc) - allocs and zeros */

char *salloc(n)
int n;
{
register char *c, *i;
c = alloc(n);
i = c + n;
while (i != c) *--i = 0;
return (c);
}
/* append(name,ext) - returns the catenation of the
	strings name and ext. */

append(name,ext)
char *name,*ext;
{
int lname;
register char *c,*d,*newname;
lname = 0;
c = name; while (*c++) ++lname;
c = ext;  while (*c++) ++lname;
newname = c = alloc(lname+1);
d = name; while (*d) *c++ = *d++;
d = ext; while (*c++ = *d++);
return newname;
}
/*
s2i(s,*i) - converts string s to int and returns value in i.
	For a series of numeric arguments, returns pointer to the next
	character past the of the numeric string, or 0 if end-of-string.
	(Thus, returns nonzero if argument was not a single number. )
*/
char *s2i(s,i)
char *s;
int *i;
{
register char lc,c;
register int val;
int sign;
char *ans;
sign = 1;
val = lc = 0;
ans = 0;
while ((c = *s++) != 0) {
	if (c >= '0' && c <= '9') val = 10*val + c - '0';
	else if (c == '-' && lc == 0) sign = -1;
	else {ans = --s; break; }
	lc = c;
	}
*i = val * sign;
return ans;
}

