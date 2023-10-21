/*	SCAME scame1.c				*/

/*	Revision 1.0.0  1985-02-09		*/

static char *cpyrid = "@(#)Copyright (C) 1985 by Leif Samuelsson";

# include "scame.h"

extern int k_ubk();



/* clearecho makes the echo line disappear after a while */

clearecho()
{
	sleep(1);
	echo(NIL);
}

/* select buffer */

selectbuffer()
{
char c;
char bufname[80];
char tprompt[80];
	strcpy(tprompt,"Select Buffer");
	if (oldbuf >=0)
		sprintf(&tprompt[strlen(tprompt)]," (%s)",buftab[oldbuf].name);
	strcat(tprompt,": ");
	c = instring(tprompt,bufname,0," \t\033","\007\r");
	if (c == '\r' && (*bufname != '\0' || oldbuf >= 0)) {
		bufname[BUFNAMESIZE-1] = '\0';
		fixbuftab(PUT);
		if (*bufname == '\0') getbuffer(buftab[oldbuf].name);
		else getbuffer(bufname);
	}
	killing = FALSE;
}

Bool getbuffer(name)
char *name;
{
int i=0;
	while (i<noofbufs && !strneq(name,buftab[i].name,strlen(name)+1,TRUE))
 		i++;
	if (i != bufno && i < MAXBUFS) {
		if (wtmp(bufno)) {
			oldbuf = bufno;
			bufno = i;
			if (i < noofbufs && rtmp(bufno)) /* old buffer */
				fixbuftab(GET);
			else {				/* new buffer */
				if (i == noofbufs) noofbufs++;
				strcpy(cb.name,name);
				gotobegin();
				z = buf;
				cb.modified = FALSE;
				*cb.filename = '\0';
				cb.mtime = (time_t) 0;
				setmode(FUNDAMENTAL);
				cb.minormodes.autofill = 0L;
				cb.minormodes.overwrite = 0L;
				updateflag = TRUE;
			}
		}
	}
	else if (i >= MAXBUFS) { errmes(BTF); return(FALSE); }
	return (TRUE);
}


/* kill buffer */

kbuffer()
{
char c;
char bufname[80];
char tprompt[80];
	if (noofbufs == 1) {
		echo("Must have at least one buffer!");
		pchar(BELL);
		return;
	}
	sprintf(tprompt,"Kill Buffer (%s): ",cb.name);
	c = instring(tprompt,bufname,0," \t\033","\007\r");
	if (c == '\r') {
		bufname[BUFNAMESIZE-1] = '\0';
		fixbuftab(PUT);
		if (*bufname == '\0') removebuffer(cb.name);
		else removebuffer(bufname);
	}
}

removebuffer(name)
char *name;
{
int tmplen, i=0;
char tf1[FILENAMESIZE],tf2[FILENAMESIZE];
Bool select;
	select = FALSE;
	fixbuftab(PUT);
	wtmp(bufno);
	while (i<noofbufs && !strneq(name,buftab[i].name,strlen(name)+1,TRUE))
 		i++;
	if (i < noofbufs) {
		if (bufno == i) select = TRUE;
		else if (bufno > i) bufno--;
		if (windows == 2) {
			if (otherbuf == i) onewindow();
			else if (otherbuf > i) otherbuf--;
		}
		if (oldbuf == i) oldbuf = -1;
		else if (oldbuf > i) oldbuf--;

		/* and kill it */
		sprintf(tf1,tempfile,ppid,uid);
		sprintf(tf2,tempfile,ppid,uid);
		tmplen = strlen(tf1);
		while(i < noofbufs-1) {
			buftab[i] = buftab[i+1];
			sprintf(&tf1[tmplen],"%d",i);
			sprintf(&tf2[tmplen],"%d",i+1);
			if (rename(tf2,tf1) != 0)
				printf("Cannot rename %s to %s\r\n",tf2,tf1);
			i++;
		}
		noofbufs--;
		if (select) {
			if (oldbuf == -1) oldbuf = 0;
			bufno = oldbuf;
			rtmp(bufno);
			fixbuftab(GET);
			selectbuffer();
		}
	}
	else errmes(NSB);
}



/* List Buffers */
listbuffers()
{
int i,j;
char s[150];
	fixbuftab(PUT);
	typing = FALSE;
	typeout("    Buffer  (mode)                 Filename");
	typeout(NIL);
	for (i=0; i < noofbufs; i++) {
		s[0] = '\0';
		if (buftab[i].modified) strcat(s,"* ");
		else strcat(s,"  ");
		if (i == bufno) strcat(s,"- ");
		else strcat(s,"  ");
		strcat(s,buftab[i].name);
		strcat(s,":   (");
		strcat(s,modes[(int)(buftab[i].majormode)]);
		strcat(s,")");
		for (j = strlen(s); j < 35; j++) strcat(s," ");
		strcat(s,buftab[i].filename);
		s[79] = '\0';	/* trim if too long */
		if (!typeout(s)) break;
	}
}


/* wtmp(), rtmp(), fixbuftab() support for temporary files */

Bool wtmp(i)
int i;
{
int f;
char s[FILENAMESIZE];
Bool flg;
	flg=FALSE;
	sprintf(s, tempfile, ppid, uid);
	sprintf(&s[strlen(s)], "%d", i);
	if ((f=creat(s, 0600)) >= 0) {
		closegap();
		if (write(f,buf,z-buf) == z-buf) flg = TRUE;
		else echo("can't write tempfile");
		close(f);
	}
	else echo("can't create tempfile!");
	return(flg);
}

Bool rtmp(i)
int i;
{
int f;
char s[FILENAMESIZE];
	sprintf(s, tempfile, ppid, uid);
	sprintf(&s[strlen(s)], "%d", i);
	if ((f = open(s,0)) > -1) {
		z = buf + read(f, buf, (int) bufsize);
		close(f);
		dot = min(dot, z);
		updateflag = TRUE;
		return(TRUE);
 	}
	else {
		echo("can't read tempfile");
		return (FALSE);
	}
}

fixbuftab(flg)
int flg;
{
	switch(flg) {
		case (PUT):
			buftab[bufno] = cb;
			buftab[bufno].dot = dot - buf;
			buftab[bufno].home = home - buf;
			break;
		case (GET):
			cb = buftab[bufno];
			dot = buf + buftab[bufno].dot;
			home = buf + buftab[bufno].home;
			modeline();
	}
}


/* exec() - does a nice system() */

exec(s)
char *s;
{
int c,i;
	signal(SIGHUP, SIG_DFL);
	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
	setupterm(FALSE);
	cur(screenlen-1, 0);
	write(1,"\n",1);
	system(s);
	write(1, more, strlen(more));
	setupterm(TRUE);
	*commandprompt = '\0';
	if ((c= inchar(TRUE)) != ' ')
		unget(c);
	cls();
	for (i=0; i<=screenlen-2; i++) *oldscreen[i]= '\0';
	echo(NIL);
}


/* Tabify - turn spaces into tabs if possible */
tabify()
{
register char *tdot;
int q2,q3,q4;
	while (dot < z) {
		dot--;
		while (++dot < z
			&& (*dot != ' ' || *(dot+1) != ' ' || *(dot+2)!=' '));
		if (dot < z) {
			tdot=dot;
			while (++dot < z && *dot == ' ');
			findxy();
			q3 = (curx) % TABWID;
			backchar((long)q3);
			q2 = dot - tdot;
			if (q2 > 1) {
				q4 = (TABWID - 1 + q2) / TABWID;
				delchar(backchar((long)(q2 - q4)));
				backchar((long)q4);
				while (q4--) *dot++ = '\t'; }
			while(q3--) dot++; } }
}


/* Saving and restarting SCAME */

Bool restart()
{
int f;
char s[FILENAMESIZE];
long od;			/* same as otherdot */
long tz;			/* same as bufsize */
	sprintf(s, tempfile, ppid, uid);
	strcat(s, "d");
	if ((f = open(s,0)) > -1) {
		echo("(Continuing)");
		read(f, (char *) &noofbufs, sizeof(noofbufs));
		read(f, (char *) &bufno, sizeof(bufno));
		read(f, (char *) &oldbuf, sizeof(oldbuf));
		read(f, (char *) &tz, sizeof(tz));
		if (tz) incbuf(tz);
		read(f, (char *) mark, 16 * sizeof(char *));
		read(f, (char *) &od, sizeof(od));
		otherdot=buf+od;
		read(f, (char *) &windows, sizeof(windows));
		read(f, (char *) &otherbuf, sizeof(otherbuf));
		read(f, (char *) &wintop, sizeof(wintop));
		read(f, (char *) &winbot, sizeof(winbot));
		if (wintop != 0) winbot = screenlen-3;
		read(f, (char *) &quiet, sizeof(quiet));
		read(f, (char *) &execfd, sizeof(execfd));
		read(f, oldsstr, 80);
		read(f, fillprefix, 80);
		read(f, tmpmode_string, 80);
		read(f, (char *) &gvars, sizeof(struct gvarstruct));
		read(f, lastinput, LASTINPUTSIZE);
		read(f, (char *) &lstindex, sizeof(lstindex));
		read(f, oldshellcommand, 80);
		read(f, oldmailname, 80);
		read(f, (char *) disptab, sizeof(disptab));
		read(f, (char *) c_x_disptab, sizeof(c_x_disptab));
		read(f, (char *) m_o_disptab, sizeof(m_o_disptab));
		read(f, (char *) buftab, noofbufs * sizeof(struct bufstruct));
		if (rtmp(bufno)) fixbuftab(GET);
		modeline();
		close(f);
		clearecho();
		return(TRUE);
	}
	else return(FALSE);
}

savescame()
{
int f;
funcp fp;
long od;
char s[FILENAMESIZE];
Bool flg;
	*s='\0';
	sprintf(s, tempfile,ppid, uid);
	strcat(s, "d");
	if (wtmp(bufno)) {
		fixbuftab(PUT);
		flg = FALSE;
		if ((f=creat(s, 0600)) >= 0) {
			write(f, (char *) &noofbufs, sizeof(noofbufs));
			write(f, (char *) &bufno, sizeof(bufno));
			write(f, (char *) &oldbuf, sizeof(oldbuf));
			write(f, (char *) &bufsize, sizeof(bufsize));
			write(f, (char *) mark, 16 * sizeof(char *));
			od = otherdot - buf;
			write(f, (char *) &od, sizeof(od));
			write(f, (char *) &windows, sizeof(windows));
			write(f, (char *) &otherbuf, sizeof(otherbuf));
			write(f, (char *) &wintop, sizeof(wintop));
			write(f, (char *) &winbot, sizeof(winbot));
			write(f, (char *) &quiet, sizeof(quiet));
			write(f, (char *) &execfd, sizeof(execfd));
			write(f, oldsstr, 80);
			write(f, fillprefix, 80);
			write(f, tmpmode_string, 80);
			write(f, (char *) &gvars, sizeof(struct gvarstruct));

			write(f, lastinput, LASTINPUTSIZE);
			write(f, (char *) &lstindex, sizeof(lstindex));
			write(f, oldshellcommand, 80);
			write(f, oldmailname, 80);
			write(f, (char *) disptab, sizeof(disptab));
			write(f, (char *) c_x_disptab, sizeof(c_x_disptab));
			write(f, (char *) m_o_disptab, sizeof(m_o_disptab));
			write(f, (char *) buftab,
				noofbufs * sizeof(struct bufstruct));
			close(f);
			flg = TRUE;
		}
		else  echo("can't create tempfile!");
		return(flg);
	}
	return(FALSE);
}


vfile(dir,file, modflg,waitflg,grepstring)
char *dir, *file;
Bool modflg, waitflg;
char *grepstring;
{
int f;
char tline[81];
char mline[80];
char tfile[80];
char *cp;
int c;
int line;
Bool eof;
	line = wintop;
	*tfile = '\0';
	if (dir != NIL) sprintf(tfile,"%s/",dir);
	strcat(tfile,file);
	if ((f=open(tfile,0)) >= 0) {
		eof = FALSE;
		*commandprompt='\0';
		*tline = '\0';
		if (modflg) {
    			sprintf(mline,"SCAME  Viewing file: %s",tfile);
			tmodlin(mline);
		}
		while (!eof) {
			getline(f,tline,min(80,screenwidth),&eof);
			if (grepstring == NIL
			    || strsub(tline,grepstring,FALSE,TRUE) >= 0) {
				strcpy(screen[line],tline);
				if (grepstring != NIL) {
				    cp = screen[line];
				    while ((cp = index(cp,';')) != NIL) {
				        if (cp > screen[line] && *(cp-1) == '\\') {
						*(cp-1) = ';';
						*cp++ = ' ';
					}
					else *cp = '\0';
				    }
				}
				refline(line,FALSE);
				if (++line > winbot && !eof){
					echo(more);
					if ((c= inchar(FALSE)) != ' '){
						unget(c);
						write(1, " FLUSHED",8);
						clearecho();
						goto flushed;
					}
					echo(NIL);
					line= wintop;
				}
			}
	    	} /* while */
		if (grepstring != NIL) {
			strcpy(screen[line],"done");
			refline(line,FALSE);
			if (++line <= winbot) {
				screen[line][0] = '\0';
				refline(line,FALSE);
			}
		}
		if (waitflg && (c= inchar(FALSE)) != ' ') unget(c);
flushed:
		close(f);
		updateflag = TRUE;
    	} /* if open */
	else echo("Can't find file");
	if (modflg)
		tmodlin(NIL);
	modeline();
	cur(cury,curx);
	typing = FALSE;
	killing = FALSE;
} /* vfile */


/* Type Out (a bit like FT in TECO) */
Bool typeout(str)
char *str;
{
static int line;
char c;
	updateflag = TRUE;
	if (!typing) {
		if (str != NIL) strcpy(screen[wintop],str);
		else *screen[wintop] = '\0';
		refline(wintop,FALSE);
		typing = TRUE;
		line = wintop+1;
		*screen[line] = '\0';
		refline(line,FALSE);
	}
	else {
		if (line == winbot+1) {
			echo(more);
			*commandprompt='\0';
			if ((c = inchar(TRUE))!= ' ') {
 				unget(c);
				write(1," FLUSHED",8);
				clearecho();
				return (FALSE);
			}
			echo(NIL);
			line = wintop;
			*screen[line] = '\0';
			refline(line,FALSE);
		} 
		if (str != NIL) strcpy(screen[line],str);
		else *screen[line] = '\0';
		refline(line++,FALSE);
		if (line <= winbot) {
			*screen[line] = '\0';
			refline(line,FALSE);
		}
 	}
	return (TRUE);
}


/*  Find File.
 *  Search through buffers for a file. Create a new buffer
 *  for it if it isn't found.
 */

findfile()
{
int i,c;
Bool	flg;
char tstr[FILENAMESIZE], bufname[FILENAMESIZE];
char tprompt[80];
struct stat fst;

	basename(cb.filename,bufname,TRUE);
    ffloop:
	flg = FALSE;
	i = 0;
	while (i < noofbufs && strcmp(cb.filename,buftab[i].filename) != 0) {
		if (strneq(bufname,buftab[i].name,strlen(bufname)+1,TRUE))
			flg = TRUE;
		i++;
	}
	if (i < noofbufs) {
		getbuffer(buftab[i].name);
		if (stat(cb.filename, &fst) == 0
		    && fst.st_mtime > cb.mtime) {
			vfile(scamelib,"changedfile",FALSE,FALSE,NIL);
			if (yesorno("Restore file from disk") == 1)
				revertfile();
			typing = FALSE;
		}
	}
	else {
		if (flg) {
			sprintf(tprompt,
				"Name of buffer or RETURN to reuse %s: ",
				bufname);
			c = instring(tprompt, tstr,0," \t\033","\007\r");
			if (c == '\007') {
				fixbuftab(GET);
				return;
			}
			else if (*tstr != '\0') {
				strcpy(bufname, tstr);
				goto ffloop;
			}
		}
		bufname[BUFNAMESIZE-1] = '\0';
		strcpy(tstr,cb.filename);
		if (getbuffer(bufname)) {
			strcpy(cb.filename,tstr);
			readfile();
			findmode();
		}
		else fixbuftab(GET);
	}
	killing = FALSE;
}

hangup()
{
char tfile[FILENAMESIZE], hupfile[FILENAMESIZE];
char command[200];
char *cp;
int i;
	sprintf(tfile,tempfile,ppid, uid);
	strcat(tfile,"0");
  	savescame();
	for (i = 0; i < noofbufs; i++) {
		tfile[strlen(tfile)-1] = '0' + i;
		if (buftab[i].filename[0] != '\0') {
			strncpy(hupfile, 
				(cp= rindex(buftab[i].filename, '/')) == NIL?
 					buftab[i].filename: &cp[1],10);
			hupfile[10]='\0';
			strcat(hupfile,".hup");
			sprintf(command, "cp %s %s",tfile,hupfile);
			system(command);
		}
	}
	exitscame(1);
}
