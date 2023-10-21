/*
 *		Extended (M-X) commands.
 */
#include	"def.h"

/*
 * This function modifies the keyboard
 * binding table, by adjusting the entries in the
 * big "bindings" array. Most of the grief deals with the
 * prompting for additional arguments.
 */
/*ARGSUSED*/
bindtokey(f, n, k) {
	register int	s;
	register SYMBOL	*sp;
	int		c;
	char		xname[NXNAME];

	if (kbdmop == NULL)
		ewprintf("Set key globally: ") ;
	c = (int) getkey(0);
	if ((s=eread("Set key %c to command: ", xname, NXNAME, EFNEW|EFFUNC,
#ifdef VARARGS
		     c
#else
		     (char *) &c, (char *) NULL
#endif
		     )) != TRUE)
		return (s);
	if ((sp=symlookup(xname)) == NULL) {
		ewprintf("[No match]");
		return (FALSE);
	}
	binding[(KEY) c] = sp;			/* rebind new.		*/
	return (TRUE);
}

/*
 * User function to unbind keys. Just call the unbind we already have. 
 */
/*ARGSUSED*/
unsetkey(f, n, k) {
	register KEY	key;
	
	if (kbdmop == NULL) ewprintf("Unset key globally: ") ;
	key = getkey(0);
	if (key == (KCTRL|'G') || key == (KCTLX|KCTRL|'G')
	|| key == (KMETA|KCTRL|'G')) {
		(VOID) ctrlg(FALSE, 1, KRANDOM);
		return ABORT;
	}
	binding[key] = NULL;
	return TRUE;
}

/*
 * Extended command. Call the message line
 * routine to read in the command name and apply autocompletion
 * to it. When it comes back, look the name up in the symbol table
 * and run the command if it is found and has the right type.
 * Print an error if there is anything wrong.
 */
/*ARGSUSED*/
extend(f, n, k) {
	register SYMBOL	*sp;
	register int	s;
	char		xname[NXNAME];

	if (f == FALSE)
		s = eread("M-x ", xname, NXNAME, EFNEW|EFFUNC
#ifndef VARARGS
			 , (char *) NULL
#endif
			 ) ;
	else
		s = eread("%d M-x ", xname, NXNAME, EFNEW|EFFUNC, 
#ifdef	VARARGS
			 n
#else
			 (char *) &n, (char *) NULL
#endif
			 ) ;
	if (s != TRUE) return (s);
	if ((sp=symlookup(xname)) != NULL)
		return ((*sp->s_funcp)(f, n, KRANDOM));
	ewprintf("[No match]");
	return FALSE;
}

/*
 * Read a key from the keyboard, and look it
 * up in the binding table. Display the name of the function
 * currently bound to the key. Say that the key is not bound
 * if it is indeed not bound, or if the type is not a
 * "builtin". This is a bit of overkill, because this is the
 * only kind of function there is.
 */
/*ARGSUSED*/
desckey(f, n, k) {
	register SYMBOL	*sp;
	register KEY	c;

	if (kbdmop == NULL) ewprintf("Describe key briefly: ");
	c = getkey(0);
	if (kbdmop != NULL) return TRUE;
	if ((sp=binding[c]) == NULL)
		ewprintf("%c is undefined", (int) c);
	else
		ewprintf("%c runs the command %s", (int) c, sp->s_name);
	return (TRUE);
}

/*
 * This function creates a table, listing all
 * of the command keys and their current bindings, and stores
 * the table in the standard pop-op buffer (the one used by the
 * directory list command, the buffer list command, etc.). This
 * lets MicroEMACS produce it's own wall chart. The bindings to
 * "ins-self" are only displayed if there is an argument.
 */
/*ARGSUSED*/
wallchart(f, n, k) {
	register int	key;
	register SYMBOL	*sp;
	register char	*cp1;
	register char	*cp2;
	BUFFER		*bp;
	char		buf[64];

	bp = bfind("*Help*", TRUE);
	if (bclear(bp) != TRUE)			/* Clear it out.	*/
		return TRUE;
	for (key=0; key<NKEYS; ++key) {		/* For all keys.	*/
		sp = binding[key];
		if (sp != NULL
		&& (f!=FALSE
		|| strcmp(sp->s_name, "self-insert-command")!=0)) {
			keyname(buf, key);
			cp1 = &buf[0];		/* Find end.		*/
			while (*cp1 != 0)
				++cp1;
			while (cp1 < &buf[32])	/* Goto column 32.	*/
				*cp1++ = ' ';				
			cp2 = sp->s_name;	/* Add function name.	*/
			while (*cp1++ = *cp2++)
				;
			if (addline(bp, buf) == FALSE)
				return (FALSE);
		}
	}
	return popbuf(bp) == NULL ? FALSE : TRUE;
}

#ifdef	STARTUP
/*
 * Define the commands needed to do startup-file processing.
 * This code is mostly a kludge just so we can get startup-file processing.
 *
 * If you're serious about having this code, you should rewrite it.
 * To wit: 
 *	It has lots of funny things in it to make the startup-file look
 *	like a GNU startup file; mostly dealing with parens and semicolons.
 *	This should all vanish.
 *
 *	It uses the same buffer as keyboard macros. The fix is easy (make
 *	a new function "execmacro" that takes a pointer to char and
 *	does what ctlxe does on it. Make ctlxe and excline both call it.)
 *	but would slow down the non-micro version.
 *
 * We define eval-expression because it's easy. It's pretty useless,
 * since it duplicates the functionality of execute-extended-command.
 * All of this is just to support startup files, and should be turned
 * off for micros.
 */

/*
 * evalexpr - get one line from the user, and run it. Identical in function
 *	to extend, but easy.
 */
/*ARGSUSED*/
evalexpr(f, n, k) {
	register int	s;
	char		exbuf[NKBDM];

	if ((s = ereply("Eval: ", exbuf, NKBDM)) != TRUE)
		return s;
	return excline(exbuf);
}
/*
 * evalbuffer - evaluate the current buffer as line commands. Useful
 *	for testing startup files.
 */
/*ARGSUSED*/
evalbuffer(f, n, k) {
	register LINE	*lp;
	register BUFFER	*bp = curbp;
	register int	s;
	static char	excbuf[NKBDM];
	char		*strncpy();

	for (lp = lforw(bp->b_linep); lp != bp->b_linep; lp = lforw(lp)) {
		if (llength(lp) >= NKBDM + 1) return FALSE ;
		(VOID) strncpy(excbuf, ltext(lp), NKBDM);
		if ((s = excline(excbuf)) != TRUE) return s;
	}
	return TRUE;
}
/*
 * evalfile - go get a file and evaluate it as line commands. You can
 *	go get your own startup file if need be.
 */
/*ARGSUSED*/
evalfile(f, n, k) {
	register int	s;
	char		fname[NFILEN];

	if ((s = ereply("Load file: ", fname, NFILEN)) != TRUE)
		return s;
	return load(fname);
}

/*
 * load - go load the file name we got passed.
 */
load(fname) char *fname; {
	register int	s;
	char		excbuf[NKBDM];

	if (ffropen(fname) == FIOERR)
		return FALSE;
	while ((s = ffgetline(excbuf, NKBDM)) == FIOSUC)
		if (excline(excbuf) != TRUE) break;
	(VOID) ffclose();
	return s == FIOEOF;
}

/*
 * excline - run a line from a load file or eval-expression.
 */
excline(line) register char *line; {
	register char	*funcp, *argp = NULL;
	char		*skipwhite(), *parsetoken(), *backquote();
	int		status;

	/* Don't know if it works; don't care - mwm */
	if (kbdmip != NULL || kbdmop != NULL) {
		ewprintf("Not now!") ;
		return FALSE;
	}

	funcp = skipwhite(line);
	if (*funcp == '\0') return TRUE;	/* No error on blank lines */
	line = parsetoken(funcp);
	if (*line != '\0') {
		*line++ = '\0';
		line = skipwhite(line);
		if ((*line >= '0' && *line <= '9') || *line == '-') {
			argp = line;
			line = parsetoken(line);
		}
	}

	kbdmip = &kbdm[0];
	if (argp != NULL) {
		*kbdmip++ = (KEY) (KCTRL|'U');
		*kbdmip++ = (KEY) atoi(argp);
	}
	*kbdmip++ = (KEY) (KMETA|'X');
	/* Pack in function */
	while (*funcp != '\0')
		if (kbdmip+1 <= &kbdm[NKBDM-3]) *kbdmip++ = (KEY) *funcp++;
		else {
			ewprintf("eval-expression macro overflow");
			ttflush();
			return FALSE;
		}
	*kbdmip++ = '\0';	/* done with function */
	/* Pack away all the args now...	*/
	while (*line != '\0') {
		argp = skipwhite(line);
		if (*argp == '\0') break ;
		line = parsetoken(argp) ;
		/* Slightly bogus for strings. But they should be SHORT! */
		if (kbdmip+(line-argp)+1 > &kbdm[NKBDM-3]) {
			ewprintf("eval-expression macro overflow");
			ttflush();
			return FALSE;
		}
		if (*line != '\0') *line++ = '\0';
		if (*argp != '"') {
			if (*argp == '\'') ++argp;
			while (*argp != '\0')
				*kbdmip++ = (KEY) *argp++;
			*kbdmip++ = '\0';
		}
		else {	/* Quoted strings special again */
			++argp;
			while (*argp != '"' && *argp != '\0')
				if (*argp != '\\') *kbdmip++ = (KEY) *argp++;
				else argp = backquote(++argp, TRUE);
			/* Quotes strings are gotkey'ed, so no trailing null */
		}
	}
	*kbdmip++ = (KEY) (KCTLX|')');
	*kbdmip++ = '\0';
	kbdmip = NULL;
	status = ctlxe(FALSE, 1, KRANDOM);
	kbdm[0] = (KCTLX|')');
	return status;
}
/*
 * a pair of utility functions for the above
 */
char *
skipwhite(s) register char *s; {

	while ((*s == ' ' || *s == '\t' || *s == ')' || *s == '(')
	    && *s != '\0')
		if (*s == ';') *s = '\0' ;
		else s++;
	return s;
}

char *
parsetoken(s) register char *s; {

	if (*s != '"')
		while (*s != ' ' && *s != '\t' && *s != '(' && *s != ')'
		    && *s != '\0') {
			if (*s == ';') *s = '\0';
			else s++;
		}
	else	/* Strings get special treatment */
		do {
			/* Beware: You can \ out the end of the string! */
			if (*s == '\\') ++s;
			if (ISLOWER(*s)) *s = TOUPPER(*s);
			} while (*++s != '"' && *s != '\0');
	return s;
}
/*
 * Put a backquoted string element into the keyboard macro. Return pointer
 * to char following backquoted stuff.
 */
char *
backquote(in, flag) char *in; {
	switch (*in++) {
	    case 'T': *kbdmip++ = (KEY) (KCTRL|'I');
		      break;
	    case 'N': *kbdmip++ = (KEY) (KCTRL|'J');
		      break; 
	    case 'R': *kbdmip++ = (KEY) (KCTRL|'M');
		      break;
	    case '^': *kbdmip = (KEY) (KCTRL|*in++);
		      if (flag != FALSE && *kbdmip == (KEY) (KCTRL|'X')) {
			      if (*in == '\\') in = backquote(++in, FALSE);
			      else *kbdmip++ = (KEY) *in++;
			      kbdmip[-1] |= (KEY) KCTLX;
		      } else ++kbdmip;
		      break;
	    case 'E':
		      if (flag != TRUE) *kbdmip++ = (KEY) (KCTRL|'[');
		      else if (*in != '\\') *kbdmip++ = (KEY) (KMETA|*in++);
		      else {
			      in = backquote(++in, FALSE);
			      kbdmip[-1] |= (KEY) KMETA;
		      }
		      break;
	}
	return in;
}
#endif	STARTUP
