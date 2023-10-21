/*	SCAME scame2.c				*/

/*	Revision 1.0.0  1985-02-09		*/

static char *cpyrid = "@(#)Copyright (C) 1985 by Leif Samuelsson";

# include "scame.h"

/* Change Directory */

changedir()
{
char c;
char dirname[80], homedir[80];
char tprompt[80];
	strcpy(homedir,getenv("HOME"));
	sprintf(tprompt, "Change to Directory (%s): ", homedir);
	*dirname = '\0';
	do {
	    c = instring(tprompt, dirname, 0 ," \t", "\007\r\033");
	    if (c == ESC) {
			if (*dirname == '\0') strcpy(dirname,homedir);
			else pchar(BELL);
	    }
	} while (c == ESC);
	if (c == '\r') {
		if (*dirname != '\0') buildfilename(homedir, dirname);
		if (chdir(homedir) < 0) {
			echo("Couldn't change directory");
			pchar(BELL);
		}
		else {
			getwd(currentdir);
			modeline();
			echo(NIL);
		}
	}
	else echo(NIL);
}

/* View Working Directory */

viewworkingdir()
{
	echo("Working directory = ");
	strout(currentdir);
}


/* Delete Blank Lines */

# ifdef COMMENT
delblanklines()
{
char *tdot1, *tdot2;
Bool blankline;
	closegap();
	tdot1 = dot;
	do {
		blankline = TRUE;
		tdot2 = tdot1;
		while (tdot1 < z && *tdot1 != '\n') {
			if (!isblank(*tdot1)) blankline = FALSE;
			tdot1++;
		}
		if (blankline && tdot1<z) tdot1++;
	} while (blankline && tdot1 < z);
	delchar((long)(tdot2 - dot));
	killing=FALSE;
}
#endif
/* Shell Command */

char oldshellcommand[80]="";

shellcommand()
{
char c, tstr[80], tprompt[80];
	strcpy(tprompt,"Shell command");
	if (*oldshellcommand != '\0')
		sprintf(&tprompt[strlen(tprompt)]," (\"%s\")",oldshellcommand);
	strcat(tprompt,": ");
	*tstr= '\0';
	do {
		c=instring(tprompt,tstr,strlen(tstr),"","\r\007\033");
		if (c == ESC) {
			if (*tstr == '\0') {
				strcpy(tstr,oldshellcommand);
			}
			else pchar(BELL);
		}
	} while (c == ESC);
	if (c == '\r') {
 		if (*tstr != '\0') strcpy(oldshellcommand,tstr);
		exec(oldshellcommand);
	}
	else echo (NIL);
}


/* Send Mail */

char oldmailname[80]="";

mailbuffer()
{
#ifdef MAIL
char c, tstr[80], tprompt[80];
	strcpy(tprompt,"Mail Buffer to");
	if (*oldmailname != '\0')
		sprintf(&tprompt[strlen(tprompt)]," (\"%s\")",oldmailname);
	strcat(tprompt,": ");
	*tstr= '\0';
	do {
		c=instring(tprompt, tstr,strlen(tstr)," \t","\r\007\033");
		pchar('\r');
		if (c == ESC) {
			if (*tstr == '\0') {
				strcpy(tstr,oldmailname);
			}
			else pchar(BELL);
		}
	} while (c == ESC);
	if (c==13) {
		if (*tstr == '\0') strcpy(tstr,oldmailname);
		else strcpy(oldmailname,tstr);
		if (*tstr != '\0')
			pipe_to(MAIL, tstr, NIL, NIL, buf, z);
		else echo("Mail not sent - no addressee");
	}
	else echo(NIL);
#else
	errmes(NYI);
#endif
}

checkmail(report)
Bool report;
{
static time_t fm;
time_t tfm;
	tfm = filemodified(mailfile);
	if (report && tfm > fm && filesize(mailfile) > 0) {
		echo("New mail for you, ");
		strout(username); pchar(BELL);
	}
	fm = tfm;
	chkmailcnt = gvars.mail_check_interval;
}


/* Multiple windows */

twowindows() {
	if (windows == 1) {
		windows = 2;
		otherdot = dot;
		otherhome = home;
		otherbuf = bufno;
		winbot = screenlen/2 - 2;
		modeline();
		update();
		wintop = screenlen/2;
		winbot = screenlen-3;
		upd2();
		modeline();
	}
	else errmes(A2W);
	killing = FALSE;
}

onewindow() {
	if (windows > 1) {
		windows = 1;
		wintop = 0;
		winbot = screenlen-3;
		upd2();
		modeline();
		updateflag = TRUE;
	}
	else errmes(O1W);
}

otherwindow() {
int i;
char *h;
	if (windows > 1) {
		if (wintop == 0) {
			wintop = winbot + 2;
			winbot = screenlen - 3;
		}
		else {
			winbot = wintop - 2;
			wintop = 0;
		}
		if (bufno == otherbuf) {
			if (otherdot + GAPDIST > gaps) closegap();
			h = dot;
			dot = min(z,otherdot);
			otherdot = h;
			h = home;
			home = min(z,otherhome);
			otherhome = h;
		}
		else {
			closegap();
			otherdot = dot;
			otherhome = home;
			i = bufno;
			fixbuftab(PUT);
			getbuffer(buftab[otherbuf].name);
			otherbuf = i;
		}
	}
	else errmes(O1W);
}

scrollotherwindow(i)
long i;
{
	if (windows > 1) {
		otherwindow();
		upd2();
		nextscreen(i);
		update();
		modeline();
		otherwindow();
	}
	else errmes(O1W);
}

growwindow(i)
long i;
{
char *tdot;
	if (windows > 1) {
		if (wintop == 0 && winbot < screenlen-5-i) {
			winbot += i;
			upd2();
			modeline();
			updateflag = TRUE;
		}
		else if (winbot == screenlen-3 && wintop > i + 2) {
			wintop -= i;
			otherwindow();
			update();
			modeline();
			otherwindow();
			if (home > buf) {
				tdot = dot;
				scroll(-i);
				dot = tdot;
			}
			updateflag = TRUE;
		}
		else echo("\007Windows need at least two lines.");
	}
}

pipe_to(prog, arg1, arg2, arg3, from, to)
char *prog, *arg1, *arg2, *arg3, *from, *to;
{
int p[2], status;
	closegap();
	pipe(p);
	if (fork() == 0) {
		close(p[1]);
		close(0);
		dup(p[0]);
		execl(prog, "", arg1, arg2, arg3, 0);
		write(2, "Can't find ", 10); write(2, prog, strlen(prog));
		exit(1);
	}
	close(p[0]);
	write(p[1], from, to-from);
	close(p[1]);
	wait(&status);
}

pipe_through(prog, arg1, arg2, arg3, from, to)
char *prog, *arg1, *arg2, *arg3, *from, *to;
{
int p[2], status;
char tfile[FILENAMESIZE], *tdot;
	if (!fileexists(prog)) {
		echo("Can't find ");
		strout(prog);
		return;
	}
	sprintf(tfile,tempfile,ppid,uid);
	strcat(tfile,"t");
	closegap();
	pipe(p);
	if (fork() == 0) {
		close(p[1]);
		close(0);
		dup(p[0]);
		close(1);
		creat(tfile, 0600);
		execl(prog, "", arg1, arg2, arg3, 0);
		exit(1);
	}
	close(p[0]);
	write(p[1], from, to-from);
	close(p[1]);
	wait(&status);
	if (status) {
		echo("Can't run ");
		strout(prog);
	}
	else {
		tdot = dot;
		dot = from;
		delchar((long)(to-from));
		insertfile(tfile);
		dot = min(tdot, z);
		cb.modified = TRUE;
	}
}


saveforundo(fn)
int (*fn)();
{
int f;
Bool flg;
char tfile[FILENAMESIZE];
	closegap();
	flg=FALSE;
	sprintf(tfile,tempfile,ppid,uid);
	strcat(tfile,"u");
	if ((f=creat(tfile, 0600)) >= 0) {
		write(f,&fn,sizeof(fn));
		write(f,&cb.modified,sizeof(cb.modified));
		if (write(f,buf,z-buf) == z-buf) flg = TRUE;
		else echo("can't write tempfile");
		close(f);
	}
	else echo("can't create tempfile!");
	return(flg);
}

undo()
{
int f;
char tfile[FILENAMESIZE], ufile[FILENAMESIZE];
char s[80], *p;
int (*fn)();
	sprintf(tfile,tempfile,ppid,uid);
	strcat(tfile,"t");
	sprintf(ufile,tempfile,ppid,uid);
	strcat(ufile,"u");
	strcpy(s,"Undo last ");
	p = &s[strlen(s)];
	unlink(tfile);
	if ((f=open(ufile,0)) >= 0) {
		read(f, &fn, sizeof(fn));
		func_name(fn, p);
		if (yesorno(s) == 1) {
			rename(ufile,tfile);
			saveforundo(undo);
			read(f,&cb.modified,sizeof(cb.modified));
			z = buf + read(f,buf,(int) bufsize);
			dot = min(dot,z);
			updateflag = TRUE;
		}
		close(f);
		clearecho();
 	}
	else echo("Sorry, can't undo");
	modeline();
}

writeregion()
{
char tfile[FILENAMESIZE];
int c;
long howmany;
char *from= dot;
	c = instring("Write Region to file: ",tfile,0," \t\033","\r\007");
	if (c==13 && *tfile != '\0') {
		adjustmark();
		howmany= *mark - dot;
		if (howmany < 0) {
 			howmany = -howmany;
			from = *mark;
		}
		if (max(dot,*mark) >= gaps) closegap();
		if (filncpy(tfile, from, howmany, 0666)) clearecho();
		killing=FALSE;
	}
}

/* Fill & Justify */

fillregion()
{
char *tdot, *tmpCS;
	adjustmark();
	if (dot == *mark) return;
	if (dot > *mark) exchgdotmark();
	gapto(*mark + 1, GAPSIZE);
/*
	if (*(*mark - 1) == '\n') (*mark)--;
*/
	tdot = dot;
	tmpCS = CS;
	CS = NIL;
	while (dot < *mark) {
		while (dot < *mark && isblank(*dot) || *dot == '\n') {
			delchar(1L); (*mark)--;
		}
		linpos = 0;
		while (linpos < cb.lvars.fillcolumn && dot < *mark) {
			while(dot<*mark && !isblank(*dot) && *dot != '\n')
				dot++;
			if (dot == *mark-1 && *dot == '\n') dot++;
			while (dot<*mark && (isblank(*dot) || *dot == '\n')){
				delchar(1L); (*mark)--;
			}
			if (dot < *mark) {
				insertc(' ',1L);	(*mark)++;
				findxy();
			}
		}
		if (linpos > cb.lvars.fillcolumn) {
			while (dot>buf && isblank(*(dot-1))) dot--;
			while (dot>buf && !isblank(*(dot-1))) dot--;
			if (dot>buf) *(dot-1) = '\n';
		}
	}
	*mark = tdot;
	CS = tmpCS;
}

justifyregion()
{
char *tdot;
int blanks,n;
	tdot = min(dot, *mark);
	adjustmark();
	if (dot > *mark) exchgdotmark();
	while (dot < *mark - 1) {
		begofline();
		blanks = 0;
		while (dot < *mark - 1 && isblank(*dot)) {
			delchar(1L);
			(*mark)--;
		}
		while (dot < *mark - 1 && *dot != '\n'){
			while(dot<*mark - 1 && !isblank(*dot) && *dot != '\n')
				dot++;
			while (dot<*mark-1 && isblank(*dot)) {
				delchar(1L);
				(*mark)--;
			}
			if (dot < *mark-1 && *dot != '\n') {
				insertc(' ',1L);
				(*mark)++;
				blanks++;
			}
		}
		findxy();
		n = cb.lvars.fillcolumn - linpos;
		while (n > 0 && blanks > 0) {
			begofline();
			while (n > 0 && dot < *mark - 1 && *dot != '\n') {
				while(dot<*mark - 1 && !isblank(*dot)
					&& *dot != '\n') dot++;
				while(dot<*mark - 1 && isblank(*dot)
					&& *dot != '\n') dot++;
				if (dot < *mark-1 && *dot != '\n') {
					insertc(' ',1L);
					(*mark)++;
					n--;
				}
			}
		}
		passline(1L);
	}
	*mark = tdot;
}


breakline()
{
char *tdot, *tdot2;
	findxy();
	if (linpos > cb.lvars.fillcolumn) {
		tdot = dot;
		begofline();
		tdot2 = dot;
		linpos = 0;
		while (linpos <= cb.lvars.fillcolumn) {
			while(dot<tdot && isblank(*dot)) dot++;
			while(dot<tdot && !isblank(*dot)) dot++;
			findxy();
		}
		while (dot>tdot2 && !isblank(*(dot-1))) dot--;
		while (dot>tdot2 && isblank(*(dot-1))) dot--;
		while (dot<tdot && isblank(*dot)) {
			delchar(1L);
			tdot--;
		}
		if (*(dot-1) != '\n') {
			insertc('\n', 1L);
			tdot++;
		}
		insertstr(fillprefix, (long) strlen(fillprefix));
		dot = tdot + strlen(fillprefix);
	}
}

showrightparen()
{
char *tdot;
register int level;
static Bool echoed = FALSE;
	if (cb.lvars.disp_matching_paren != 0L) {
		upd2();
		tdot = dot--;
		level = 1;
		if (cb.lvars.disp_matching_paren > 0L) {
			while (level > 0) {
				while (dot > buf && *(--dot) != '(' && *dot != ')');
				if (*dot == '(') level--;
				else if (*dot == ')') level++;
				if (level > 0 && dot == buf) level = -1;
			}
			if (level == 0) {
				if (dot < home) { char *tdot2, *tdot3;
					tdot2 = dot;
					begofline();
					tdot3 = dot;
					endofline();
					echo(NIL);
					strnout(tdot3, (int) (tdot2-tdot3));
					invmod(TRUE);
					outchar(*tdot2++, FALSE);
					invmod(FALSE);
					strnout(tdot2, (int) (dot-tdot2));
					echoed = TRUE;
				}
				else {
					if (echoed) {
						echo(NIL);
						echoed = FALSE;
					}
					findxy();
					cur(cury, curx);
					suspend(cb.lvars.disp_matching_paren * 1000, TRUE);
				}
			}
			else {
				pchar(BELL);
				if (echoed) {
					echo(NIL);
					echoed = FALSE;
				}
			}
		}
		else {
			while (level > 0) {
				while (dot > home && *(--dot) != '(' && *dot != ')');
				if (*dot == '(') level--;
				else if (*dot == ')') level++;
				if (level > 0 && dot == home) level = -1;
			}
			if (level == 0) {
				findxy();
				cur(cury, curx);
				suspend(abs(cb.lvars.disp_matching_paren) * 1000, TRUE);
			}
			dot = tdot;
			findxy();
		}
		dot = tdot;
	}
}


