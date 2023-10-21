#
/* file ned.m.c - main loop for new RAND editor. */
/*   Walt Bilofsky - 14 January 1977 */

#include "ned.defs"

int clrsw 0;  /* is 1 to clear paramport */
int csrsw 0;  /* is 1 if bullit is to temporarily appear after function */
int imodesw 0;/* is 1 when in insertmode  */
int oldccol 0;		 /* used to get cursor bullit on the screen */
int oldcline 0; 	 /* initialized to be sure there first time */

#define VMOTCODE 4  /* codes 1 through 4 imply the cursor has left the line */

/*
the dispatch table
*/

mainloop()
{
int i,m,n,pipef[2];
register int j,k,l;
int clsave,ccsave;
int occ, ocl;

int thiscol, thisrow;

char ich[8], *cp, **execargs, **e;

char c,rdf,bulsw;
extern int templ[4];
struct viewport *oport;

/***** main loop for editor - read and echo 1 char ***/


if (cursorline== 0) oldcline = 1;
if (cursorcol == 0) oldccol  = 1;
bulsw = 0;

goto funcdone;
FOREVER
	{
	bulsw = csrsw = clrsw = 0;
	read1();
	if (errsw)
		{
		errsw = 0;
		clrsw = 1;
		goto errclear;
		}
	if ((! CTRLCHAR) || lread1 == CCBACKSPACE || lread1 == CCDELCH)
		{
		lread1 =& 0177;

		/* process a printing character */

		if (lread1 == CCBACKSPACE  &&  cursorcol == 0)
		    {
		    lread1 = -1;
		    goto contin;
		    }

		if (openwrite[curfile] == 0) goto nowriterr;
		if (clineno != (i = curwksp->ulhclno+cursorline)) getline(i);

		if (lread1==CCDELCH || (imodesw && lread1==CCBACKSPACE) )
			{
			thiscol = cursorcol + curwksp->ulhccno;
			thisrow = cursorline;

			if (lread1 == CCBACKSPACE) thiscol--;
			if (ncline < thiscol + 2)
				{
				if (lread1 == CCBACKSPACE) movecursor(LT);
				lread1 = -1;
				goto contin;
				}
			for (i=thiscol;i<ncline-2;i++) cline[i] = cline[i+1];
			ncline--;
			thiscol =- curwksp->ulhccno;
			putup(-(1+thiscol),cursorline);
			poscursor(thiscol,thisrow);
			fcline = 1;
			lread1 = -1;
			goto contin;
			}

		fcline = 1;

		if (j = (lread1 == CCBACKSPACE))
			{
			movecursor(LT);
			lread1 = ' ';
			}

		/* margin-stick feature */
		if (cursorcol > curport->rtext) goto margerr;

		if ((i = cursorcol + curwksp->ulhccno) >=
			(lcline - 2)) excline(i+2);
		if (i >= ncline-1)
			{
			for (k=ncline-1; k<=i; k++) cline[k] = ' ';
			cline[i+1] = NEWLINE;
			ncline = i+2;
			}
		else if (imodesw)
			{
			thiscol = cursorcol + curwksp->ulhccno;
			thisrow = cursorline;
			if (ncline >= lcline) excline(ncline+1);
			for (i=ncline;i>thiscol;i--) cline[i] = cline[i-1];
			ncline++;
			thiscol =- curwksp->ulhccno;
			putup(-(1+thiscol),cursorline);
			poscursor(thiscol,thisrow);
			}

		/* margin-stick feature */
		if (cursorcol >= curport->rtext)
			curport->redit = curport->rtext + 1;

		if (cursorcol == curport->rtext - 10) putcha(007);
		cline[i] = lread1;
		putch(lread1,1);

		/* margin-stick feature */
		curport->redit = curport->rtext;

		if (j) movecursor(LT);
		lread1 = -1;
		goto contin;
		}
	/* Return on bottom line causes plus lines */
	if (lread1 == CCRETURN && cursorline == curport->btext)
	       {putline(0);
		movew(defplline);
		}
	if (lread1 != CCQUIT && (i=cntlmotions[lread1]))  movecursor(i);
	if (CONTROLCHAR && i )
		{
		if (i <= VMOTCODE) { putline(0); goto newnumber; }
		lread1 = -1;
		goto contin;
		}

	/* margin-stick feature */
	if (cursorcol > curport->rtext) poscursor(curport->rtext,cursorline);

	putline(0);
	rdf = 1;
	if (lread1 == CCQUIT)
		{
		if (endit() == 0) goto funcdone;
		gosw = 0;
		return;
		}

	switch (lread1)
		{

	case CCENTER:				/* <CTRL @> */
		goto gotarg;
	case CCLPORT:
		movep(- deflport);		/* <CTRL A> */
		goto funcdone;
	case CCSETFILE:
		switchfile();			/* <CTRL B> */
		goto funcdone;
	case CCCHPORT:
		chgport(-1);			/* <CTRL C> */
		bulsw++;
		goto funcdone;
	case CCOPEN:				/* <CTRL D> */
		if (openwrite[curfile]==0)  goto nowriterr;
		openlines(curwksp->ulhclno + cursorline,definsert);
		goto funcdone;
	case CCMISRCH:
		search(-1);			/* <CTRL E> */
		goto funcdone;
	case CCCLOSE:				/* <CTRL F> */
		if (openwrite[curfile]==0)
			goto nowriterr;
		closelines(curwksp->ulhclno + cursorline, defdelete);
		goto funcdone;
	case CCPUT:                             /* <CTRL G> */
		if (openwrite[curfile]==0)
			goto nowriterr;
		if (pickbuf->nrows == 0) goto nopickerr;
		put(pickbuf,curwksp->ulhclno+cursorline,
			curwksp->ulhccno+cursorcol);
		goto funcdone;
	case CCPICK:				/* <CTRL L> */
		picklines(curwksp->ulhclno + cursorline, defpick);
		goto funcdone;
	case CCINSMODE: 			/* <CTRL O> */
		imodesw = 1 - imodesw;	/* change it */
		goto funcdone;
	case CCGOTO:				/* <CTRL P> */
		gtfcn(0);
		goto funcdone;
	case CCMIPAGE:				/* <CTRL Q> */
		movew(- defmipage * (1+curport->btext));
		goto funcdone;
	case CCPLSRCH:
		search(1);			/* <CTRL R> */
		goto funcdone;
	case CCRPORT:
		movep(defrport);		/* <CTRL S> */
		goto funcdone;
	case CCPLLINE:
		movew(defplline);		/* <CTRL T> */
		goto funcdone;
	case CCDELCH:
		goto notimperr; 		/* <CTRL U> */
	case CCSAVEFILE:			/* <CTRL V> */
		savefile(0,curfile);
		goto funcdone;
	case CCMILINE:
		movew(-defmiline);		/* <CTRL W> */
		goto funcdone;
	case CCDOCMD:                           /* <CTRL X> */
		goto notstrerr;
	case CCPLPAGE:				/* <CTRL Y> */
		movew(defplpage * (1+curport->btext));
		goto funcdone;
	case CCMAKEPORT:			/* <CTRL Z> */
		makeport(deffile);
		bulsw++;
		goto funcdone;
	case CCTABS:                            /* <CTRL [> */
		settab(curwksp->ulhccno + cursorcol);
		goto funcdone;
	case CCMOVELEFT:			/* <CTRL H> */
	case CCTAB:				/* <CTRL I> */
	case CCMOVEDOWN:			/* <CTRL J> */
	case CCHOME:				/* <CTRL K> */
	case CCRETURN:				/* <CTRL M> */
	case CCMOVEUP:				/* <CTRL N> */
	default:
		goto badkeyerr;
		}

gotarg:
	param();

	if (lread1 == CCQUIT)
		{
		if (*paramv == 'a')
			{
			cleanup();
			gosw = 0;
			if (*(paramv+1) != 'd') return;
			inputfile = -1; /* to force a dump */
			fatal("Aborted");
			}
		if (endit() == 0) goto funcdone;
		gosw = 1;
		return;
		}

	switch (lread1)
		{

	case CCENTER:
		goto funcdone;		/* <CTRL @> */
	case CCLPORT:			/* <CTRL A> */
		if (paramtype <= 0)  goto notstrerr;
		if (s2i(paramv,&i)) goto notinterr;
		movep(-i);
		goto funcdone;
	case CCSETFILE:
					/* <CTRL B> */
		if (paramtype <  0)  goto notstrerr;
		if (paramtype == 0)  getarg();
		if (paramv == 0) goto noargerr;
		editfile(paramv,0,0,1,1);
		goto funcdone;
	case CCCHPORT:			/* <CTRL C> */
		if (paramtype <= 0)  goto notstrerr;
		if (s2i(paramv,&i)) goto notinterr;
		if (i <= 0) goto notposerr;
		chgport(i-1);
		bulsw++;
		goto funcdone;
	case CCOPEN:			/* <CTRL D> */
		if (openwrite[curfile]==0)  goto nowriterr;
		if (paramtype == 0) splitline(curwksp->ulhclno + paramr0,
			paramc0 + curwksp->ulhccno);
		else if (paramtype > 0)
			{
			if (s2i(paramv,&i)) goto notinterr;
			if (i <= 0) goto notposerr;
			openlines(curwksp->ulhclno + cursorline, i);
			}
		else
			{
			if (paramc1 == paramc0)
				{
				openlines(curwksp->ulhclno+(paramr0<paramr1 ?
					paramr0 : paramr1),
						abs(paramr1-paramr0)+1);
				}
			else openspaces(curwksp->ulhclno + (paramr0<paramr1 ?
				paramr0 : paramr1), curwksp->ulhccno +
				  (paramc0<paramc1 ? paramc0 : paramc1),
				  abs(paramc1-paramc0),
				  abs(paramr1-paramr0) + 1);
			}
		goto funcdone;
	case CCMISRCH:			/* <CTRL E> */
		if (paramtype <  0)  goto notstrerr;
		if (paramtype == 0)  getarg();
		if (paramv == 0) goto noargerr;
		if (searchkey) free(searchkey);
		searchkey = paramv;
		paraml = 0;
		search(-1);
		goto funcdone;
	case CCCLOSE:			/* <CTRL F> */
		if (openwrite[curfile]==0)  goto nowriterr;
		if (paramtype == 0) combineline(curwksp->ulhclno + paramr0,
			paramc0 + curwksp->ulhccno);
		else if (paramtype > 0)
			{
			if (s2i(paramv,&i)) goto notinterr;
			if (i <= 0) goto notposerr;
			closelines(curwksp->ulhclno + cursorline, i);
			}
		else
			{
			if (paramc1 == paramc0)
				{
				closelines(curwksp->ulhclno+(paramr0<paramr1 ?
					paramr0 : paramr1),
						abs(paramr1-paramr0)+1);
				}
			else closespaces(curwksp->ulhclno + (paramr0<paramr1 ?
				paramr0 : paramr1), curwksp->ulhccno +
				  (paramc0<paramc1 ? paramc0 : paramc1),
				  abs(paramc1-paramc0),
				  abs(paramr1-paramr0) + 1);
			}
		goto funcdone;
	case CCPUT:
					 /* <CTRL G> */
		if (paramtype >  0)  goto notimperr;
		if (paramtype <  0)  goto notstrerr;
		if (openwrite[curfile]==0)
			goto nowriterr;
		if (deletebuf->nrows == 0) goto nodelerr;
		put(deletebuf,curwksp->ulhclno+cursorline,
			curwksp->ulhccno+cursorcol);
		goto funcdone;
	case CCMOVELEFT:		/* <CTRL H> */
	case CCTAB:			/* <CTRL I> */
	case CCMOVEDOWN:		/* <CTRL J> */
	case CCHOME:			/* <CTRL K> */
	case CCRETURN:			/* <CTRL M> */
	case CCMOVEUP:			/* <CTRL N> */
	case CCMOVERIGHT:               /* <CTRL _> */
	case CCBACKTAB:                 /* <CTRL ]> */
		if (s2i(paramv,&i)) goto notinterr;
		if (i <= 0) goto notposerr;
		m = cntlmotions[lread1];
		while (--i >= 0) movecursor(m);
		goto funcdone;
	case CCPICK:			/* <CTRL L> */
		if (paramtype == 0) goto notimperr; /* ????????? */
		else if (paramtype > 0)
			{
			if (s2i(paramv,&i)) goto notinterr;
			if (i <= 0) goto notposerr;
			picklines(curwksp->ulhclno + cursorline, i);
			}
		else
			{
			if (paramc1 == paramc0)
				{
				picklines(curwksp->ulhclno+(paramr0<paramr1 ?
					paramr0 : paramr1),
						abs(paramr1-paramr0)+1);
				}
			else pickspaces(curwksp->ulhclno + (paramr0<paramr1 ?
				paramr0 : paramr1), curwksp->ulhccno +
				  (paramc0<paramc1 ? paramc0 : paramc1),
				  abs(paramc1-paramc0),
				  abs(paramr1-paramr0) + 1);
			}
		goto funcdone;
	case CCINSMODE: 		/* <CTRL O> */
		imodesw = 1 - imodesw;	/* change it */
		goto funcdone;
	case CCGOTO:			/* <CTRL P> */
		if (paramtype == 0) gtfcn(nlines[curfile]);
		else if (paramtype > 0)
			{
			if (s2i(paramv,&i)) goto notinterr;
			gtfcn(i-1);
			}
		else gtfcn(curwksp->ulhclno + paramr0 - paramr1 + defplline);
		goto funcdone;
	case CCMIPAGE:			/* <CTRL Q> */
		if (paramtype <= 0)  goto notstrerr;
		if (s2i(paramv,&i)) goto notinterr;
		movew(- i * (1 + curport->btext));
		goto funcdone;
	case CCPLSRCH:			/* <CTRL R> */
		if (paramtype <  0)  goto notstrerr;
		if (paramtype == 0)  getarg();
		if (paramv == 0) goto noargerr;
		if (searchkey) free(searchkey);
		searchkey = paramv;
		paraml = 0;
		search(1);
		goto funcdone;
	case CCRPORT:			/* <CTRL S> */
		if (paramtype <= 0)  goto notstrerr;
		if (s2i(paramv,&i)) goto notinterr;
		movep(i);
		goto funcdone;
	case CCPLLINE:			/* <CTRL T> */
		if (paramtype < 0)  goto notstrerr;
		else if (paramtype == 0)  movew(cursorline);
		else if (paramtype > 0)
			{
			if (s2i(paramv,&i)) goto notinterr;
			movew(i);
			}
		goto funcdone;
	case CCDELCH:
		goto notimperr; 	/* <CTRL U> */
	case CCSAVEFILE:		/* <CTRL V> */
		if (paramtype <  0)  goto notstrerr;
		if (paramtype == 0)  getarg();
		if (paramv == 0) goto noargerr;
		savefile(paramv,curfile);
		goto funcdone;
	case CCMILINE:			/* <CTRL W> */
		if (paramtype < 0)  goto notstrerr;
		else if (paramtype == 0)  movew(cursorline - curport->btext);
		else if (paramtype > 0)
			{
			if (s2i(paramv,&i)) goto notinterr;
			movew(-i);
			}
		goto funcdone;
	case CCDOCMD:                   /* <CTRL X> */
		if (openwrite[curfile] == 0) goto nowriterr;
		i = curwksp->ulhclno + cursorline;
		m = 1;
		cp = paramv;
		if (*cp == '-' || (*cp >= '0' && *cp <= '9')) {
			cp = s2i(paramv,&m);
			if (cp == 0) goto noargerr;
			}
		m = -m;                     /* default is # of paragraphs */
		if (*cp == 'l') {cp++; m = -m; }  /* nl same as -n */
		e = execargs = salloc(64);  /* prepare exec arg list */
		while (*cp == ' ') cp++;
		while (*cp != 0) {
			*e++ = cp;              /* address of arg */
			if (*e) goto noargerr; /* too many args */
			if (*cp == '"') {
				cp++;
				e[-1]++;
				while (*cp++ !=  '"')
				   if (*cp == 0) goto noargerr;
				cp[-1] = 0;
				}
			  else if (*cp++ == '\'') {
				e[-1]++;
				while (*cp++ !=  '\'')
				   if (*cp == 0) goto noargerr;
				cp[-1] = 0;
				}
			  else while (*cp != ' ' && *cp != ',' && *cp) cp++;
			while (*cp == ' ' || *cp == ',') *cp++ = 0;
			}
		*e = 0;
		if (pipe(pipef) == -1) goto nopiperr;
		if ((j = fork()) == -1) goto nopiperr;
		if (j == 0) {                   /* child process */
			close(0);               /* pipe is standard input */
			dup(pipef[0]);
			close(1);               /* tempfile is output */
			open(tmpname,1);
			seek(1,tempfh,3);
			seek(1,tempfl,1);
			j = 2;
			/* This code closes whatever it can open */
			while ((k = dup(1)) >= 0) if (k > j) j = k;
			while (j >= 2) close(j--);
			if ((i = fork()) == -1) goto nopiperr;
			if (i != 0) {           /* this process waits for   */
				while (wait(&m) != i);  /* child to die,    */
				while (read(0,pwbuf,100)); /* empties pipe, */
				exit(m >> 8);        /* and returns status.      */
				}
			execr(execargs);
			exit(-1);               /* if can't exec */
			}
		/* parent process here after fork */
		telluser("Executing ...",0);
		free(execargs);
		doreplace(i,m,n,j,pipef);
		goto funcdone;
	case CCPLPAGE:			/* <CTRL Y> */
		if (paramtype <= 0)  goto notstrerr;
		if (s2i(paramv,&i)) goto notinterr;
		movew(i * (1 + curport->btext));
		goto funcdone;
	case CCMAKEPORT:		/* <CTRL Z> */
		if (paramtype == 0)  removeport();
		  else if (paramtype <  0)  goto notstrerr;
		  else makeport(paramv);
		bulsw++;
		goto funcdone;
	case CCTABS:                    /* <CTRL [> */
		clrtab(curwksp->ulhccno + cursorcol);
		goto funcdone;

	default:
		goto badkeyerr;
		}
badkeyerr:
	error("Badkeyerr - editor error");
	goto funcdone;
nopiperr:
	error("Can not fork or write pipe.");
	goto funcdone;
notstrerr:
	error("Argument must be a string.");
	goto funcdone;
notinterr:
	error("Argument must be numeric.");
	goto funcdone;
notposerr:
	error("Argument must be positive.");
	goto funcdone;
noargerr:
	error("Invalid argument.");
	goto funcdone;
nopickerr:
	error("Nothing in the PICK buffer.");
	goto funcdone;
nodelerr:
	error ("Nothing in the CLOSE buffer.");
	goto funcdone;
notimperr:
	error("Feature not implemented yet.");
	goto funcdone;
margerr:
	error("Margin stuck; move cursor to free.");
	goto funcdone;
nfilerr:
	error("No can do:  too many files.");
	goto funcdone;
nowriterr:
	error("You cannot modify this file!");
	goto funcdone;

funcdone:
	clrsw = 1;
newnumber:
	lread1 = -1;		/* signify char read was used */
errclear:
	oport = curport;
	k = cursorline;
	j = cursorcol;
	switchport(&paramport);
	paramport.redit = PARAMRINFO;
	if (clrsw)
		{
		if (!errsw)
			{
			poscursor(0,0);
			info(blanks);
			}
		poscursor(PARAMREDIT+2,0);
		if (oport->wksp->wfile)
			{
			info("File ");
			info(openfnames[oport->wksp->wfile]);
			}
		info(" Line ");
		clsave = cursorline;
		ccsave = cursorcolumn;
		}
	poscursor(ccsave,clsave);
	i = oport->wksp->ulhclno + k + 1; /* show cursorline  */
	cp = ich + 8;
	*--cp = '\0';
	do
		(*--cp = '0' + (i % 10));
	while (i = i/10);
	info(cp);
	*cp = '\0';
	while (cp != ich) *--cp = ' ';
	info(ich);
	switchport(oport);
	paramport.redit = PARAMREDIT;
	poscursor(j,k);
	if (csrsw)
		{
		putch(0177,1);
		poscursor(j,k);
		dumpcbuf();
		sleep(1);
		putup(k,k);
		poscursor(j,k);
		}

	if (imodesw && clrsw && !errsw)
		telluser("     ***** I N S E R T M O D E *****",0);

contin:
	occ = oldccol;
	oldccol = cursorcol;
	ocl = oldcline;
	oldcline = cursorline;
	if (slowsw == 0 && (clrsw || oldccol != occ)) {
		if (bulsw == 0) { poscursor(occ,-1);
				  putch(TMCH,0);
				  poscursor(occ,curport->btext+1);
				  putch(BMCH,0);
				}
		poscursor(oldccol,-1);
		putch(HCSRCH,0);
		poscursor(oldccol,curport->btext+1);
		putch(HCSRCH,0);
		}
	if (slowsw == 0 && (clrsw || oldcline != ocl)) {
		if (bulsw == 0) { poscursor(-1,ocl);
				  putch(curport->lmchars[ocl],0);
				  poscursor(curport->rtext+1,ocl);
				  putch(curport->rmchars[ocl],0);
				}
		poscursor(-1,oldcline);
		putch(VCSRCH,0);
		poscursor(curport->rtext+1,oldcline);
		putch(VCSRCH,0);
		}
	poscursor(oldccol,oldcline);
	}

}
info(s)
char *s;
{
while (*s && cursorcol < PARAMRINFO) putch(*s++,0);
}
/* endit() - finishes up */
endit()
	{
	register int i;
	for (i = 0; i < MAXFILES; i++)
		if (openfsds[i] && openwrite[i] == EDITED)
			if (savefile(openfnames[i],i) == 0) return(0);
	return(1);
	}
/* editfile(file,line,col,mkflg,puflg) -
	installs file as edited file in current port,
	starting at line line. Gives user a chance to make file
	if its not there, provided mkflg is 1.
	Returns -1 if user does not want to want
	to make one, 0 if error condition, 1 is successfull.
	Writes screen (calls putup) if puflg is 1. */

editfile(file,line,col,mkflg,puflg)
char *file;
int line, col, mkflg, puflg;
	{
	int i,j;
	register int fn;
	register char *c,*d;

	fn = -1;
	for (i=0; i<MAXFILES;++i) if (openfnames[i] != 0)
		{
		c = file;
		d = openfnames[i];
		while (*(c++) == *d) if (*(d++) == 0)
			{
			fn = i;
			break; /* stop the testing now */
			}
		}
	if (fn < 0)
		{
		fn = open(file,0);  /* is file there? */
		if (fn >= 0)
			{
			if (fn >= MAXFILES)
				{
				error("Too many files -- Editor limit!");
				close(fn);
				return(0);
				}
			if ((j = checkpriv(fn)) == 0)
				{
				error("File read protected.");
				close(fn);
				return(0);
				}
			openwrite[fn] = (j == 2 ? 1 : 0);
			telluser("USE: ",0);
			telluser(file,5);
			}
		else if (mkflg)
			{
			telluser("Hit <USE> (ctrl-K) to make: ",0);
			telluser(file,28);
			lread1 = -1;
			read1();
			if (lread1 != CCSETFILE) return(-1);
			/* find the directory */
			for (c=d=file; *c; c++)  if (*c == '/') d = c;
			if (d > file)
				{
				*d = '\0';
				i = open(file,0);
				}
			else i = open(".",0);
			if (i < 0)
				{
				error("Specified directory does not exist.");
				return(0);
				}
			if (checkpriv(i) != 2)
				{
				error("Can't write in:");
				telluser (file,21);
				return(0);
				}
			close(i);
			if (d > file) *d = '/';
			/* ok to create file, so do it */
			if ((fn = creat(file,0666)) < 0)
				{
				error("Create failed!");
				return(0);
				}
			if (fn >= MAXFILES)
				{
				close(fn);
				error("Too many files -- Editor limit!");
				return(0);
				}
			openwrite[fn] = 1;
			chown(file,userid);
			}
		else return (-1);

		paraml = 0;   /* so its kept around */
		openfnames[fn] = file;
		}


	/* output buffer cause this may take a while */
	dumpcbuf();

	switchwksp();
	if (openfsds[fn] == 0) openfsds[fn] = file2fsd(fn);
	curwksp->curfsd = openfsds[fn];
	curfile = curwksp->wfile = fn;
	curwksp->curlno = curwksp->curflno = 0;
	curwksp->ulhclno = line;
	curwksp->ulhccno = col;
	if (puflg)
		{
		putup(0,curport->btext);
		poscursor(0,defplline);
		}
	return(1);
	}
/* search(delta) - searches up/down current file for searchkey, according
	as delta is -1 or 1.  If key is not on current page, positions
	port with key on top line.  Leaves cursor under key. */

search(delta)
int delta;
	{
	register char *at,*sk,*fk;
	int ln,lkey,col,lin,slin;
	paraml = 0;
	if (searchkey == 0 || *searchkey == 0)
		{
		error("Nothing to search for.");
		return;
		}
	col = cursorcol;
	slin = lin = cursorline;
	if (delta == 1) telluser("+",0);
	else telluser("-",0);
	telluser("SEARCH: ",1);
	telluser(searchkey,9);

	putch(0377,1);
	poscursor(col,lin);
	dumpcbuf();

	lkey = 0;
	sk = searchkey;
	while (*sk++) lkey++;
	getline (ln = lin + curwksp->ulhclno);
	putline(0);
	at = cline + col + curwksp->ulhccno;
	FOREVER
		{
		at =+ delta;
		while (at <  cline || at >  cline + ncline - lkey)
			{
			if ((ln =+ delta) < 0 ||
			    (wposit(curwksp,ln) && delta == 1))
				{
				putup(lin,lin);
				poscursor(col,lin);
				error("Search key not found.");
				csrsw = 0;
				return;
				}
			getline(ln);
			putline(0);
			at = cline;
			if (delta < 0) at =+ ncline - lkey;
			}
		sk = searchkey;
		fk = at;
		while (*sk == *fk++ && *++sk);
		if (*sk == 0)
			{
			lkey = 0;
			lin  = ln - curwksp->ulhclno;
			if (lin  < 0 || lin  > curport->btext)
				{
				lkey = -1;
				lin = defplline;
				if ((curwksp->ulhclno = ln - defplline) < 0)
					{
					lin =+ curwksp->ulhclno;
					curwksp->ulhclno = 0;
					}
				}
			col = at - cline;
			col =- curwksp->ulhccno;
			if (col < 0 || col > curport->rtext)
				{
				curwksp->ulhccno =+ col;
				col = 0;
				lkey = -1;
				}
			if (lkey < 0) putup(0,curport->btext);
			else putup(slin,slin);
			poscursor(col,lin);
			csrsw = 1;	/* put up a bullit briefly */
			return;
			}
		}
	}

/* hookfsd(a,b) - returns a chained to b.  a may be 0, b can't. */
struct fsd *hookfsd(a,b)
struct fsd *a,*b;
{
register struct fsd *c;
if ((c = a) == 0 || a->fsdfile == 0) return b;
while ((c = c->fwdptr)->fsdfile);
(b->backptr = c->backptr)->fwdptr = b;
free(c);
return a;
}

/*
savefile(file,n) - writes out file on channel n with name of file.
	gives user a chance to put in "." if can't write in directory.
	(but only when file is 0 -- i.e. openfnames[n])
	normal return is 1; error is 0; user does not elect "." is -1
*/
savefile(file,n)
char *file;
int n;
	{
	register char *f1;
	char *c, *f0, *f2;
	register int i, j;
	int newf;

	/* get the directory */
	f0 = (file ? file : openfnames[n]);
	for (f1=f2=f0; *f1; f1++) if (*f1 == '/') f2 = f1;
	if (f2 > f0)
		{
		*f2 = '\0';
		i = open(f0,0);
		*f2 = '/';
		}
	else i = open (".",0);
	if (i < 0)
		{
		error ("Directory does not exist.");
		return(0);
		}
	j = checkpriv(i);
	close (i);
	if (j != 2)
		{
		if (file)
			{
			error ("Can't write in specified directory");
			return(0);
			}
		if (f2 > f0)
			{
			telluser("Hit <SAVE> to use '.'");
			lread1 = -1;
			read1();
			if (lread1 != CCSAVEFILE) return(-1);
			if ((i = open(".",0)) < 0)
				{
				error ("Directory '.' does not exist!");
				return(0);
				}
			j = checkpriv(i);
			close (i);
			if (j != 2)
				{
				error ("Can't write in '.'");
				return(0);
				}
			f0 = f2 +1;/* points to file name */
			}
		else
			{
			error ("Can't write in '.'");
			return(0);
			}
		}

	/* here means we can write file f0   */
	f1 = append (f0,".bak");
	unlink(f1);
	link (f0,f1);
	unlink (f0);
	if ((newf = creat(f0,getpriv(n))) < 0)
		 {
		 error ("Creat failed!");
		 }
	chown(f0,userid);


	/* copy from fsd chain, by fsd blocks.	Use cline as a buffer. */

	telluser("SAVE: ",0);
	telluser(f0,6);

	return (fsdwrite(openfsds[n],077777,newf) == -1 ? 0 : 1);
	}



/* fsdwrite(f,nl,newf) - write a fsd chain out to a file.
	f is a pointer to the fsd chain.
	newf is the filedescriptor for the file to be written.
	The first nl lines (nl > 0) or -nl block of text separated by one
		or more blank lines (nl < 0) are written, unless the file
		ends first.
	Returns total number lines written, or -1 if write error. */
fsdwrite(ff,nl,newf)
struct fsd *ff;
int nl, newf;
	{
	register struct fsd *f;
	register char *c;
	register int i;
	int j,k,bflag,tlines;

	if (lcline < LBUFFER) excline(LBUFFER);
	f = ff;
	bflag = 1;
	tlines = 0;
	while (f->fsdfile && nl)
		{
		if (f->fsdfile > 0)
			{
			i = 0;
			c = &f->fsdbytes;
			for (j=f->fsdnlines; j; j--)
				{
				if (nl < 0) {
					/* check blank line count */
					if (bflag && *c != 1) bflag = 0;
					else if (bflag == 0 && *c == 1) {
						bflag = 1;
						if (++nl == 0) break;
						}
					}
				if (*c < 0) i =- 128 * *c++;
				i =+ *c++;
				++tlines;
				/* check line count */
				if (nl > 0 && --nl == 0) break;
				}
			seek(f->fsdfile,f->seekhigh,3);
			seek(f->fsdfile,f->seeklow,1);
			while (i)
				{
				j = i < LBUFFER ? i : LBUFFER;
				read(f->fsdfile,cline,j);
				if (write(newf,cline,j) < 0)
					{
					error("DANGER -- WRITE ERROR");
					close(newf);
					return(-1);
					}
				i =- j;
				}
			}
		else
			{
			j = f->fsdnlines;
			if (nl < 0) {
				if (bflag == 0 && ++nl == 0) j = 0;
				bflag = 1;
				}
			else {  if (j > nl) j = nl;
				nl =- j;
				}
			k = j;
			while (k) cline[--k] = NEWLINE;
			if (j && write(newf,cline,j) < 0)
				{
				error("DANGER -- WRITE ERROR");
				close(newf);
				return(-1);
				}
			tlines =+ j;
			}
		f = f->fwdptr;
		}
	close(newf);
	return tlines;
	}

settab(tabcol)
int tabcol;
	{
	register int i, j;
	if (tabstops[NTABS-1] == BIGTAB)
		{
		error("Too many tabstops; can't set more.");
		return;
		}
	for (i=0;i<NTABS;i++)
		{
		if (tabstops[i] == tabcol) return;
		if (tabstops[i] >  tabcol)
			{
			for (j=NTABS-1;j>i;j--) tabstops[j] = tabstops[j-1];
			tabstops[i] = tabcol;
			return;
			}
		}
	error("Error in settab!");
	return;
	}

clrtab(tabcol)
int tabcol;
	{
	register int i,j;
	for (i=0;i<NTABS;i++) if (tabstops[i] == tabcol)
		{
		for (j=i;j<NTABS-1;j++) tabstops[j] = tabstops[j+1];
		tabstops[NTABS-1] = 0;
		return;
		}
	return;
	}
