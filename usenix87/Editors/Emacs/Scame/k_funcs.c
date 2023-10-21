/*	SCAME k_funcs.c				*/

/*	Revision 1.0.0  1985-02-09		*/

static char *cpyrid = "@(#)Copyright (C) 1985 by Leif Samuelsson";

/*	Functions normally bound to keys	*/

# include "scame.h"


k_void()
{
	pchar(BELL);
	echo("Internal error. Illegal character: ");
	outchar(cmdchar, TRUE);
}

k_ubk()
{
	errmes(UBK);
	savehpos = TRUE;
}

k_c_x_dispatch()
{
	strcat(commandprompt, "C-X ");
	(*c_x_disptab[upcasearr[inchar(FALSE)]])();
}

k_m_o_dispatch()
{
	strcat(commandprompt, "O ");
	(*m_o_disptab[inchar(FALSE)])();
}

k_pushpopmark()			/* CHECK THIS! */
{
	pushpopmark(arg);
}

k_begli()			/* Beginning of line */
{
	begofline();
}

k_backchar()
{
	if (arg > 0L) backchar(arg);
	else forwchar(-arg);
}

k_exit()
{
	pop_pending=TRUE;
}

k_exit_to_shell()
{
	exitscame(0);
}

k_delchar()
{
	if (xarg) killchar(arg);
	else {
		delchar(1L);
		killing=FALSE;
	}
}

k_endli()
{
	endofline();
}

k_forwchar()
{
	if (arg >0L)
		forwchar(arg);
	else
		backchar(-arg);
}

k_bell()
{
	pchar(BELL);
	savehpos = TRUE;
}

k_tab()
{
	if (gvars.tab_self_insert) insertc('\t',arg);
	else indent(arg);
}

k_indnewline()
{
	delhorizspace();
	if (cb.minormodes.autofill)
		breakline();
	insertc('\n',1L);
	if (*fillprefix == '\0') indent(arg);
	else insertstr(fillprefix, (long) strlen(fillprefix));
}

k_killline()
{
	killline(arg);
}

k_newwindow()
{
	newwindow(arg);
}

k_newline()
{
	if (cb.minormodes.overwrite) {
		if (cb.minormodes.autofill && (*dot == '\n' || dot == z))
			breakline();
		downline(arg);
		begofline();
	}
	else {
		if (cb.minormodes.autofill)
			breakline();
		insertc('\n', arg);
	}
}

k_downline()
{
	if (arg > 0L)
		downline(arg);
	else
		upline(-arg);
	savehpos = TRUE;
}

k_openline()
{
	if (insertc('\n', arg)) {
		backchar(arg);
		findxy();
		if (cury < screenlen-2)	/* can't scroll last line */
		    vtscroll(-min(winbot+1-cury,(int) arg), cury+1-(curx==0));
	}
}

k_upline()
{
	if (arg > 0L)
		upline(arg);
	else
		downline(-arg);
	savehpos = TRUE;
}

k_quote()
{
	strcat(commandprompt,"C-Q ");
	insertc(inchar(FALSE),arg);
}

k_risearch()
{
	isearch(TRUE);
}

k_isearch()
{
	isearch(FALSE);
}

k_transchar()
{
	transchar();
}

k_trnsword()
{
	errmes(NYI);
}

k_nextscreen()
{
	if (arg < 0L) {
		arg = -arg;
		k_prevscreen();
	}
	else if (away < z) {
		if (xarg) {
			if (arg < 2 * (winbot-wintop+1) / 3)
				vtscroll((int)arg, wintop);
			scroll(arg);
			while(dot < z && dot < home) dot++;
		}
		else nextscreen();
		killing=FALSE;
	}
	else pchar(BELL);
}

k_killregion()
{
	killregion();
}

k_listbuffers()
{
	listbuffers();
}

k_listdir()
{
	exec("ls -c");
}

k_findfile()
{
	fixbuftab(PUT);
	if (getfilename("Find File", cb.filename)) findfile();
}

k_indentregion()
{
	saveforundo(k_indentregion);
	indentregion(arg);
}

k_lowcaseregion()
{
	saveforundo(k_lowcaseregion);
	upcaseregion(FALSE);
}

k_delblanklines()
{
/*	delblanklines(); */
	errmes(NYI);
}

k_savefile()
{
	if (cb.modified) {
		if (fileexists(cb.filename)
		    && filemodified(cb.filename) > cb.mtime) {
			vfile(scamelib,"changedfile",FALSE,FALSE,NIL);
			if (yesorno("Save anyway") != 1) return;
		}
		writefile();
	}
	else echo("(No Changes)");
}


k_upcaseregion()
{
	saveforundo(k_upcaseregion);
	upcaseregion(TRUE);
}

k_visitfile()
{
	if (getfilename("Visit file", cb.filename)) {
		readfile();
		findmode();
	}
}

k_writefile()
{
	if (getfilename("Write file", cb.filename)) {
		writefile();
		findmode();
	}
}

k_exchgdotmark()
{
	exchgdotmark();
}

k_startdefkbdmac()
{
	definekbdmac(TRUE);
}

k_enddefkbdmac()
{
	definekbdmac(FALSE);
}

k_setfillprefix()
{
	setfillprefix();
}

k_onewindow()
{
	onewindow();
}

k_twowindows()
{
	twowindows();
}

k_setcommcol()
{
	if (xarg) gvars.commcol = min(75L, arg);
	else gvars.commcol = curx;
	if (!quiet) {
		echo("Comment Column = ");
		printf("%ld",gvars.commcol);
		fflush(stdout);
	}
}

k_whatcursorpos()
{
	whatcursorposition();
}

k_selectbuffer()
{
	selectbuffer();
}

k_dired()
{
	exec("dired");
}

k_executekbdmac()
{
long targ = arg;
	if (targ >= 0) {
		if (defining_kbd_mac) errmes(RKM);
		else if (fileexists(kbdmacfile))
			while (targ--)
				execfile(kbdmacfile);
		else errmes(NKM);
	}
}

k_setfcol()
{
	if (arg < 0) errmes(ILA);
	else {
		findxy();
		cb.lvars.fillcolumn = xarg ? arg : linpos;
		if (!quiet) {
			echo("Fill Column = ");
			printf("%ld",cb.lvars.fillcolumn);
			fflush(stdout);
		}
	}
}

k_markbuffer()
{
	markquant(BUFFER, 1L, TRUE);
}

k_killbuffer()
{
	kbuffer();
}

k_cntlpg()
{
register char *tdot=dot;
register int lines=0;
int beflines=0;
	closegap();
	echo("Page has ");
	while (tdot>buf && *(tdot-1) !='\f')tdot--;
	while (tdot<z && *tdot != '\f') {
		if (*tdot == '\n') lines++;
		if (tdot==dot) beflines=lines;
		tdot++;
	}
	printf("%d lines (%d+%d)",lines,beflines,lines-beflines);
	fflush(stdout);
}

k_mailbuffer()
{
	mailbuffer();
}

k_otherwindow()
{
	otherwindow();
}

k_readmail()
{
	exec("mail");
}

k_growwindow()
{
	growwindow(arg);
}

k_yank()
{
	yank();
}

k_logout()
{
	if (yesorno("Logout") == 1) kill(ppid, 9);
}

k_backtoindent()
{
	backtoindent();
}

k_queryreplace()
{
char old[SCRDIMX],new[SCRDIMX], tprompt[15];
	typing = FALSE;
	tmodlin("- Query Replace -");
	strcpy(tprompt,"Replace: ");
	if (instring(tprompt,old,0,NIL,"\r\007") != '\007' && *old !='\0') {
		if (strlen(old) <= 20)
			sprintf(&tprompt[strlen(tprompt)-2],"  %s    ",old);
		else *tprompt='\0';
		strcat(tprompt,"With: ");
		if (instring(tprompt,new,0,NIL,"\r\007") != '\007') {
			saveforundo(k_queryreplace);
			replace(old, new, TRUE);
		}
	}
	tmodlin(NIL);
	echo(NIL);
}

k_dispdate()
{
char s[26];
long t;
	time(&t);
	strcpy(s,ctime(&t));
	s[24] = '\0';
	echo(s);
}

k_scrollotherwindow()
{
	scrollotherwindow(arg);
}

k_appendnextkill()
{
	killing=TRUE;
}

k_extended()
{
char tprompt[80];
funcp fp;
	*tprompt = '\0';
	if (xarg) sprintf(tprompt,"%ld ",arg);
 	strcat(tprompt,"M-X ");
	fp = getfuncname(tprompt);
	if (fp != (funcp) 0)
		(*fp)();
}

k_help()
{
	help();
}

k_backdelchar()
{
	if (xarg) killchar(-arg);
	else delchar(backchar(1L));
}

k_shellcmd()
{
	shellcommand();
}

k_makeparens()
{
	insertstr("()",2L);
	backchar(1L);
}

k_push()
{
	push();
}

k_comment()
{
	comment();
}

k_begofbuf()
{
	pushpopmark(1L);
	gotobegin();
}

k_countlregion()
{
register char *tdot, *tmark;
register int lines=0;
int beflines=0;
	closegap();
	adjustmark();
	echo("Region has ");
	tdot = min(dot,*mark);
	tmark = max(dot, *mark);
	while (tdot<z && tdot < tmark) {
		if (*tdot == '\n') lines++;
		if (tdot==dot) beflines=lines;
		tdot++;
	}
	printf("%d lines (%d+%d)",lines,beflines,lines-beflines);
	fflush(stdout);
}

k_endofbuf()
{
	pushpopmark(1L);
	gotoend();
}

k_markword()
{
	markquant(WORD, arg, FALSE);
}

k_begse()
{
	backquant(SENTENCE,arg);
}

k_backword()
{
	if (arg > 0L)
		backquant(WORD,arg);
	else
		forwquant(WORD,-arg);
}

k_capitalizeword()
{
	upcaseword(arg,2);
}

k_killword()
{
	killquant(WORD,arg);
}

k_endse()
{
	forwquant(SENTENCE,arg);
}

k_forwword()
{
	if (arg > 0L)
		forwquant(WORD, arg);
	else
		backquant(WORD, -arg);
}

k_fillregion()
{
	saveforundo(k_fillregion);
	if (xarg) justifyregion();
	else fillregion();
}

k_instab()
{
	insertc('\t',arg);
}

k_killsentence()
{
	killquant(SENTENCE,arg);
}

k_locasw()
{
	upcaseword(arg,FALSE);
}

k_ascii()
{
int i,j;
	vt100a(TRUE);
	for (i=wintop; i <= winbot; i++)
		for (j=0; j<screenwidth; j++)
			oldscreen[i][j] = 128;
}

k_sis()
{
int i,j;
	vt100a(FALSE);
	for (i=wintop; i <= winbot; i++)
		for (j=0; j<screenwidth; j++)
			oldscreen[i][j] = 128;
}

/* EDT compatible commands */
k_edt_pf4()
{
	killing = FALSE;
	xarg = TRUE;
	killline(arg);
}

k_edt_comma()
{
	killing = FALSE;
	killchar(arg);
}

k_edt_dash()
{
	killing = FALSE;
	killquant(WORD,arg);
}

k_edt_0()
{
	passline(arg);
}

k_edt_8()
{
	passline(16L);
}

k_upcwd()
{
	upcaseword(arg,TRUE);
}

k_prevscreen()
{
char *tdot;
	if (arg < 0L) {
		arg = -arg;
		k_nextscreen();
	}
	else if (home > buf) {
		if (xarg) {
			tdot = dot;
			if (arg < 2 * (winbot-wintop+1) / 3)
				vtscroll((int)-arg, wintop);
			scroll(-arg);
			dot = home;
			upd2();
			if (tdot < away) dot = tdot;
				else { dot = away; begofline(); }
		}
		else prevscreen();
		killing=FALSE;
	}
	else pchar(BELL);
}

k_copyregion()
{
	copyregion();
}

k_delhorizspace()
{
	delhorizspace();
}

k_delindent()
{
	delindent();
}

k_notmodified()
{
	cb.modified = FALSE;
	modeline();
}

k_backkillword()
{
	killquant(WORD,-arg);
}

k_insertc()
{
	if (cmdchar == ' ' && (*dot=='\n' || dot==z)) {
		if (cb.minormodes.autofill)
			breakline();
		insertc(cmdchar, arg);
	}
	else if (cmdchar < 128) {
		if (!cb.minormodes.overwrite || *dot=='\n' || dot==z)
			insertc(cmdchar,arg);
		else {
			*dot++ = (char) cmdchar;
			updateflag = TRUE;
			cb.modified = TRUE;
		}
	}
	if (cmdchar == ')')
		showrightparen();
}

k_c_m_prefix()
{
	control_prefix = TRUE;
	meta_prefix = TRUE;
	savearg = TRUE;
}

k_c_prefix()
{
	control_prefix = TRUE;
	savearg = TRUE;
}

k_m_prefix()
{
	meta_prefix = TRUE;
	savearg = TRUE;
}

static Bool negarg;

k_univarg()
{
	if (xarg) xxarg = TRUE;
	else {
		*commandprompt = '\0';
		xarg = TRUE;
		negarg=FALSE;
	}
	arg *= 4;
	strcat(commandprompt, "C-U ");
	savearg = TRUE;
}

k_negarg()
{
	if (xarg && !xxarg) {
		negarg = TRUE;
		arg = 0;
		xxarg = TRUE;
		if (strlen(commandprompt) < 76) {
			commandprompt[strlen(commandprompt)+1]='\0';
			commandprompt[strlen(commandprompt)]='-';
			strcat(commandprompt," ");
		}
		savearg = TRUE;
	}
	else k_insertc();
}


k_digit()
{
	if (xarg) {
		if (xxarg) arg = 10 * arg + (1-2*negarg)*(cmdchar - '0');
		else {
			arg = cmdchar - '0';
			xxarg = TRUE;
		}
		if (strlen(commandprompt) < 76) {
			commandprompt[strlen(commandprompt)+1]='\0';
			commandprompt[strlen(commandprompt)]=cmdchar;
			strcat(commandprompt," ");
		}
		savearg = TRUE;
	}
	else k_insertc();
}

k_metadigit()
{
char c;
	c = cmdchar & 0177;
	if (!xxarg) {
		*commandprompt = '\0';
		xarg = TRUE;
		xxarg= TRUE;
		negarg=FALSE;
		arg=0;
	}
	arg = 10 * arg + (1-2*negarg)*(c - '0');
	if (strlen(commandprompt) < 76) {
		commandprompt[strlen(commandprompt)+1] = '\0';
		commandprompt[strlen(commandprompt)] = c;
		strcat(commandprompt," ");
	}
	savearg = TRUE;
}


k_teco()
{
#ifdef TECO
static char tecofile[FILENAMESIZE] = "";
	if (getfilename("Teco File", tecofile)) teco(tecofile);
#else
	errmes(NYI);
#endif
}
