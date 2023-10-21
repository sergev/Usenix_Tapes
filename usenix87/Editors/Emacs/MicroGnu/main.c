/*
 *		Mainline, macro commands.
 */
#include	"def.h"

int	thisflag;			/* Flags, this command		*/
int	lastflag;			/* Flags, last command		*/
int	curgoal;			/* Goal column			*/
BUFFER	*curbp;				/* Current buffer		*/
WINDOW	*curwp;				/* Current window		*/
BUFFER	*bheadp;			/* BUFFER listhead		*/
WINDOW	*wheadp = (WINDOW *)NULL;	/* WINDOW listhead		*/
KEY	kbdm[NKBDM] = {(KCTLX|')')};	/* Macro			*/
KEY	*kbdmip;			/* Input  for above		*/
KEY	*kbdmop;			/* Output for above		*/
char	pat[NPAT];			/* Pattern			*/
#ifdef	HASH
SYMBOL	*symbol[NSHASH];		/* Symbol table listhead.	*/
#else
/* should really be a *symbol, but don't want to break the hash code yet */
SYMBOL	*symbol[1];
#endif
SYMBOL	*binding[NKEYS];		/* Key bindings.		*/
#ifdef	DPROMPT
extern char prompt[], *promptp;		/* delayed prompting		*/
#endif

main(argc, argv) char *argv[]; {
	register KEY	c;
	register int	f;
	register int	n;
	register int	mflag;
#ifdef	STARTUP
	char		*sfile, *startupfile();
#endif
	char		bname[NBUFN];

#ifdef SYSINIT
	SYSINIT;				/* system dependent.	*/
#endif
	vtinit();				/* Virtual terminal.	*/
	edinit();				/* Buffers, windows.	*/
	keymapinit();				/* Symbols, bindings.	*/
	/* doing update() before reading files causes the error messages from
	 * the file I/O show up on the screen.  (and also an extra display
	 * of the mode line if there are files specified on the command line.)
	 */
	update();
#ifdef	STARTUP					/* User startup file.	*/
	if ((sfile = startupfile()) != NULL)
		(VOID) load(sfile);
#endif
	while (--argc > 0) {
		makename(bname, *++argv);
		curbp = bfind(bname, TRUE);
		(VOID) showbuffer(curbp, curwp, 0);
		(VOID) readin(*argv);
	}
	lastflag = 0;				/* Fake last flags.	*/
loop:
#ifdef	DPROMPT
	*(promptp = prompt) = '\0';
	if(epresf == KPROMPT) eerase();
#endif
	update();				/* Fix up the screen.	*/
	c = getkey(KPROMPT);
	if (epresf == TRUE) {
		eerase();
		update();
	}
	f = FALSE;
	n = 1;
	if (((KMETA|'0') <= c && c <= (KMETA|'9')) || c == (KMETA|'-')) {
		f = TRUE;
		c = c & ~KMETA;
	} else if (c == (KCTRL|'U')) {
		f = TRUE;
		n = 4;
		while ((c=getkey(KNOMAC | KPROMPT)) == (KCTRL|'U')) 
			n *= 4;
	}
	if (f == TRUE) {
		if ((c>='0' && c<='9') || c=='-') {
			if (c == '-') {
				n = 0;
				mflag = TRUE;
			} else {
				n = ((int) c) - '0';
				mflag = FALSE;
			}
			while ((c=getkey(KNOMAC | KPROMPT))>='0' && c<='9')
				n = 10*n + ((int) c) - '0';
			if (mflag != FALSE)
				n = -n;
		}
	}
	if (kbdmip != NULL) {			/* Terminate macros.	*/
		if (c!=(KCTLX|')') && kbdmip>&kbdm[NKBDM-6]) {
			(VOID) ctrlg(FALSE, 0, KRANDOM);
			goto loop;
		}
		if (f != FALSE) {
			kbdmip[-1] = (KEY) (KCTRL|'U');/* overwrite ESC */
			*kbdmip++ = (KEY) n;
			*kbdmip++ = (KEY) c;
		}
	}
	switch (execute(c, f, n)) {		/* Do it.		*/
		case TRUE: break;
		case ABORT:
			   ewprintf("Quit");	/* and fall through */
		case FALSE: default:
			   ttbeep();
			   if (kbdmip != NULL) {
				   kbdm[0] = (KEY) (KCTLX|')');
				   kbdmip = NULL;
			   }
	}
	goto loop;
}

/*
 * Command execution. Look up the binding in the the
 * binding array, and do what it says. Return a very bad status
 * if there is no binding, or if the symbol has a type that
 * is not usable (there is no way to get this into a symbol table
 * entry now). Also fiddle with the flags.
 */
execute(c, f, n) KEY c; {
	register SYMBOL	*sp;
	register int	status;

	if ((sp=binding[c]) != NULL) {
		thisflag = 0;
		status = (*sp->s_funcp)(f, n, c);
		lastflag = thisflag;
		return (status);
	}
	lastflag = 0;
	return (FALSE);
}

/*
 * Initialize all of the buffers
 * and windows. The buffer name is passed down as
 * an argument, because the main routine may have been
 * told to read in a file by default, and we want the
 * buffer name to be right.
 */
edinit() {
	register BUFFER	*bp;
	register WINDOW	*wp;

	bheadp = NULL;
	bp = bfind("*scratch*", TRUE);		/* Text buffer.		*/
	wp = (WINDOW *)malloc(sizeof(WINDOW));	/* Initial window.	*/
	if (bp==NULL || wp==NULL) panic("edinit");
	curbp  = bp;				/* Current ones.	*/
	wheadp = wp;
	curwp  = wp;
	wp->w_wndp  = NULL;			/* Initialize window.	*/
	wp->w_bufp  = bp;
	bp->b_nwnd  = 1;			/* Displayed.		*/
	wp->w_linep = bp->b_linep;
	wp->w_dotp  = bp->b_linep;
	wp->w_doto  = 0;
	wp->w_markp = NULL;
	wp->w_marko = 0;
	wp->w_toprow = 0;
	wp->w_ntrows = nrow-2;			/* 2 = mode, echo.	*/
	wp->w_force = 0;
	wp->w_flag  = WFMODE|WFHARD;		/* Full.		*/
}

/*
 * Quit command. If an argument, always
 * quit. Otherwise confirm if a buffer has been
 * changed and not written out. Normally bound
 * to "C-X C-C".
 */
/*ARGSUSED*/
quit(f, n, k) {
	register int	s;

	if ((s = anycb(FALSE)) == ABORT) return ABORT;
	if (s == FALSE
	|| eyesno("Some modified buffers exist, really exit") == TRUE) {
		vttidy();
		exit(GOOD);
	}
	return TRUE;
}

/*
 * Begin a keyboard macro.
 * Error if not at the top level
 * in keyboard processing. Set up
 * variables and return.
 */
/*ARGSUSED*/
ctlxlp(f, n, k) {
	if (kbdmip!=NULL || kbdmop!=NULL) {
		ewprintf("Already defining kbd macro!");
		return (FALSE);
	}
	ewprintf("Defining kbd macro...");
	kbdmip = &kbdm[0];
	return (TRUE);
}

/*
 * End keyboard macro. Check for
 * the same limit conditions as the
 * above routine. Set up the variables
 * and return to the caller.
 */
/*ARGSUSED*/
ctlxrp(f, n, k) {
	if (kbdmip == NULL) {
		ewprintf("Not defining kbd macro.");
		return (FALSE);
	}
	ewprintf("Keyboard macro defined");
	kbdmip = NULL;
	return (TRUE);
}

/*
 * Execute a macro.
 * The command argument is the
 * number of times to loop. Quit as
 * soon as a command gets an error.
 * Return TRUE if all ok, else
 * FALSE.
 */
/*ARGSUSED*/
ctlxe(f, n, k) {
	register KEY	c;
	register int	af;
	register int	an;
	register int	s;

	if (kbdmip!=NULL || kbdmop!=NULL) {
		ewprintf("Not now");
		return (FALSE);
	}
	if (n < 0) 
		return (TRUE);
	do {
		kbdmop = &kbdm[0];
		do {
			af = FALSE;
			an = 1;
			if ((c = *kbdmop++) == (KCTRL|'U')) {
				af = TRUE;
				an = (int) *kbdmop++;
				c  = *kbdmop++;
			}
			s = TRUE;
		} while (c!=(KCTLX|')') && (s=execute(c, af, an))==TRUE);
		kbdmop = NULL;
	} while (s==TRUE && --n);
	return (s);
}

/*
 * User abort. Should be called by any input routine that sees a C-g
 * to abort whatever C-g is aborting these days. Currently does
 * nothing.
 */
/*ARGSUSED*/
ctrlg(f, n, k) {
	return (ABORT);
}

/*
 * Display the version. All this does
 * is copy the version string onto the echo line.
 */
/*ARGSUSED*/
showversion(f, n, k) {

	ewprintf(version);
	return TRUE ;
}
