/*
 * Readr - visual news interface.
 */

#define EXTERN
#include "vnews.h"

#define PIPECHAR '|'	/* indicate save command should pipe to program */
#define META	0200	/* meta chatacter bit (as in emacs) */
#define UNSUB	0400	/* unsubscribe bit (indicates 'u' command) */
/* flags for vsave routine */
#define SVHEAD	01	/* write out article header */
#define OVWRITE	02	/* overwrite the file if it already exists */

char *getmailname();
int onint(), onquit(), onhup();
int onstop();

static char tfname[] = "/tmp/vnXXXXXX";	/* name of temp file */
static char prbuf[SECPRLEN+1];		/* buf for getting command args */
static int errclean;			/* clean up on error		*/
static char *bptr;			/* temp pointer.		*/
extern struct svartlist *alptr;		/* article list stack		*/


readr()
{
#ifdef	SIGCONT
	int (*ocont)();
#endif
	FILE *popen();

#ifdef MYDEBUG
	debugf = fopen("DEBUG", "w");
	setbuf(debugf, NULL);
#endif

/*
	fp = popen("pwd", "r");
	if (fp == NULL)
		xerror("Cannot fork/exec/popen pwd!\n");
	fgets(curdir, sizeof curdir, fp);
	pclose(fp);
	nstrip(curdir);
*/

	mktemp(tfname);
	if ((tfp = fopen(tfname, "w+")) == NULL)
		xerror("Can't create temp file");
	unlink(tfname);
	mailf = getmailname();

	/* loop reading articles. */
	fp = NULL;
	curwin = Aflag? &indexwin : &artwin;
	if (getnxtng(FORWARD)) {
		fprintf(stderr, "No news.\n");
		return;
	}

	ttysave();
	signal(SIGINT, onint);
	signal(SIGQUIT, onquit);
	signal(SIGHUP, onhup);
	ttyraw();
	errclean = 1;
	timer();

	quitflg = 0;
	while (quitflg == 0) {		/* command loop */
		if (hupflag) {
			errclean = 0;
			return;
		}
		if (!indexpg)
			openart();
		vcmd();
	}
	errclean = 0;
	botscreen();
	ttycooked();
}

/*
 * Read and execute a command.
 */

vcmd() {
	register c;
	register char *p, *q;
	char *promptp;
	char *cmdp;
	char cmd[20];
	int count;
	int countset;

	if (indexpg) {
		atend = numthisng + 2 <= indexwin.w_artlin + ARTWLEN;
	} else {
		appfile(fp, artwin.w_artlin + ARTWLEN + 1);
		atend = artlines <= artwin.w_artlin + ARTWLEN
		     && (! hdronly || artlines <= hdrend);
	}
	setprompt();
	if (atend && ! indexpg && ! erased)
		setnew(0);		/* article read */
	curflag = CURP1;
	promptp = prompt + strlen(prompt);
	cmdp = cmd;
	do {
		c = vgetc();
		if (c == cerase) {
			if (cmdp > cmd)
				cmdp--;
		} else if (c == ckill) {
			cmdp = cmd;
		} else {
			if (c == '\\') {
				strcat(prompt, "\\");
				c = vgetc();
			}
			if (c == INTR) {
				secpr[0] = '\0';
				cmdp = cmd;
			} else {
				*cmdp++ = c;
			}
		}
		*cmdp = '\0';

		p = cmd;
		q = promptp;
		countset = 0;
		count = 0;
		while ((c = *p) >= '0' && c <= '9') {
			if (p > cmd + 5)
				*--cmdp = '\0';		/* limit to 5 digits */
			else
				*q++ = c;
			count = (count * 10) + (c - '0');
			countset = 1;
			p++;
		}
		c = 0;
		if (*p == 'u') {
			c = UNSUB;
			*q++ = 'u';
			p++;
		}
		if (*p == '\033') {			/* escape */
			c |= META;
			*q++ = 'M';
			*q++ = '-';
			p++;
		}
		*q = '\0';
		c |= *p;
	} while (p >= cmdp);

	secpr[0] = '\0';
	if (countset == 0)
		count = 1;
	nextwin = NULL;
	docmd(c, count, countset);
	if (nextwin != NULL)
		curwin = nextwin;
	else if (indexpg)
		curwin = &indexwin;
	else
		curwin = &artwin;
	if (artwin.w_artlin > hdrstart && hdronly)
		hdronly = 0, artwin.w_force |= 1;
}


/*
 * Generate the prompt (percentage of article read).
 */
setprompt() {
	int percent;

	if (atend)
		strcpy(prompt, "100% ");
	else if (hdronly && ! indexpg)
		strcpy(prompt, "0% ");
	else {
		if (indexpg) {
			percent = (800L * (indexwin.w_artlin + ARTWLEN))
				/ (numthisng + 2);
		} else {
			percent = ((800 * (ftell(fp) - artbody)) / (artlength - artbody)
				* (artwin.w_artlin + ARTWLEN)) / lastlin;
		}
		percent = (percent + 4) >> 3;
		if (percent > 99)
			percent = 99;
		strcpy(prompt, "00% ");
		prompt[0] = percent / 10 + '0';
		prompt[1] = percent % 10 + '0';
	}
}		


/*
 * Process one command, which has already been typed in.
 */
docmd(c, count, countset)
{
	int i;
	char *ptr1;
	char *findhist();

	switch (c) {

	/* Turn on/off debugging. */
	case 'z':
		debugging = count;
		msg("debugging level %d", debugging);
		break;

	/* No/Next.  Go on to next article. */
	case 'n':
		setnew(0);
		nextart(count, 1);
		break;

	/* go to specific article, or treat like space */
	case '\n':
		if (countset) {
			setartno(count);
			break;
		}
		/* else fall through */

	/* Show more of current article, or advance to next article */
	case ' ':
		if (indexpg) {
			if (atend)
				indexwin.w_artlin = 0, indexpg = 0;
			else if (hasscroll && numthisng - indexwin.w_artlin <= ARTWLEN)
				scroll(numthisng + 2 - indexwin.w_artlin - ARTWLEN);
			else
				scroll(ARTWLEN);
		} else {
			if (atend)
				nextart(1, 1);
			else if (hdronly) {
				hdronly = 0;
				artwin.w_force |= 1;
				if (hasscroll)
					scroll(hdrstart - artwin.w_artlin);}
			else if ((appfile(fp, artwin.w_artlin + 2 * ARTWLEN), artread)
			 && hasscroll && artlines - artwin.w_artlin <= ARTWLEN + 2)
				scroll(artlines - artwin.w_artlin - ARTWLEN);
			else
				scroll(ARTWLEN);
		}
		break;


	/* All headers: show headers for this newsgroup */
	case 'a':
		indexpg = ! indexpg;
		break;

	/* Back up count pages */
	case META|'v':
	case '\2':	/* Control-B */
		scroll(-ARTWLEN * count);
		break;

	/* forward half a page */
	case '\4':	/* Control-D, as in vi */
		scroll(ARTWLEN/2 * count);
		break;

	/* backward half a page */
	case '\25':	/* Control-U */
		scroll(-ARTWLEN/2 * count);
		break;

	/* forward count lines */
	case '\16':	/* Control-N */
	case '\32':	/* Control-Z */
		scroll(count);
		break;

	/* bakcwards count lines */
	case '\20':	/* Control-P */
	case '\31':	/* Control-Y */
		scroll(-count);
		break;

	/* Turn displaying of article back on */
	case 'l':
	case 'd':
		/* this now happens automaticly */
		break;

	/* display header */
	case 'h':
		scroll(hdrstart - curwin->w_artlin);
		hdronly = 1;
		artwin.w_force |= 1;
		break;

	/*
	 * Unsubscribe to the newsgroup and go on to next group
	 */
	case UNSUB|'g':
	case UNSUB|'n':
		if (count) {
			curng->ng_unsub = 1;
			if (getnxtng(FORWARD))
				quitflg++;
		} else /* resubscribe */
			curng->ng_unsub = 0;
		break;

		/* unsubscribe to the current discussion */
	case UNSUB|'d':
		ud_command();
		break;

	case UNSUB|'a': ptr1 = "author";     goto badusub;
	case UNSUB|'s': ptr1 = "site";       goto badusub;
badusub:	msg("Unsubscribing to this %s: not implemented", ptr1);
		break;

		/* Print the current version of news */
	case 'v':
		msg("News version: %s", news_version);
		nextwin = curwin;
		break;


		/* decrypt joke */
	case 'D': 
		if (indexpg)
			goto illegal;
		appfile(fp, 32767);
		for (i = hdrend ; i < artlines ; i++) {
			register char c, *p;
			tfget(bfr, i);
			for (p = bfr ; (c = *p) != '\0' ; p++) {
				if (c >= 'a' && c <= 'z')
					*p = (c - 'a' + 13) % 26 + 'a';
				else if (c >= 'A' && c <= 'Z')
					*p = (c - 'A' + 13) % 26 + 'A';
			}
			tfput(bfr, i);
		}
		artwin.w_force |= 1;
		hdronly = 0;
		break;

		/* write out the article someplace */
	case 's':
	case 'w':
		{
		int wflags;

		nextwin = curwin;
		if (openart())
			break;
		wflags = 0;
		if (c == 's')
			wflags |= SVHEAD;
		if (count != 1)
			wflags |= OVWRITE;
		msg("file: ");
		curflag = CURP2;
		while ((c = vgetc()) == ' ');
		if (c == INTR) {
			secpr[0] = '\0';
			break;
		}
		if (c == '|') {
			prbuf[0] = PIPECHAR;
			if (prget("| ", prbuf+1))
				break;
		} else {
			pushback(c);
			if (prget("file: ", prbuf))
				break;
		}
		bptr = prbuf;
		if (*bptr != PIPECHAR && *bptr != '/') {
			char	hetyped[BUFLEN];
			char	*boxptr;
			strcpy(hetyped, bptr);
			if (boxptr = getenv("NEWSBOX"))
				if (index(boxptr, '%'))
					sprintf(bptr, boxptr, curng->ng_name);
				else
					strcpy(bptr, boxptr);
			else if (hetyped[0] == '~' && hetyped[1] == '/') {
				strcpy(hetyped, bptr+2);
				strcpy(bptr, userhome);
			} else
				bptr[0] = '\0';
			if (bptr[0])
				strcat(bptr, "/");
			if (hetyped[0] != '\0')
				strcat(bptr, hetyped);
			else
				strcat(bptr, "Articles");
		}
		vsave(bptr, wflags);
		}
		break;

		/* back up  */
	case '-':
	case 'b':
		if (curind <= count) {
			msg("Can't back across newsgroup boundary.");
			break;
		}
		setartno(curind - count);
		break;

		/* skip forwards */
	case '+':
		/* this may not be what we want */
		setartno(curind + count);
		break;

	/* exit - time updated to that of most recently read article */
	case 'q':
		quitflg = 1;
		break;

	/* cancel the article. */
	case 'c':
		strcpy(prompt, "cancel? ");	/* be sure not a typo */
		if ((c = vgetc()) != '\n' && c != 'y')
			break;
		cancel_command();
		break;

	/* escape to shell */
	case '!':
		{
		register char *p;
		int flags;

		nextwin = curwin;
		if (prget("!", prbuf))
			break;
		p = prbuf;
		flags = CWAIT;
		if (*p == '\0') {
			strcpy(prbuf, SHELL);
			flags = 0;
		}
		while (*p) p++;
		while (p > prbuf && p[-1] == ' ')
			p--;
		if (*--p == '&') {
			*p = '\0';
			flags = BKGRND;
			strcpy(bfr, prbuf);
		} else if (*p == '|') {
			*p = '\0';
			sprintf(bfr, "(%s)2>&1|mail '%s'", prbuf, username);
			flags |= BKGRND;
		} else {
			nextwin = &emptywin;
			strcpy(bfr, prbuf);
		}
		shcmd(bfr, flags);
		}
		break;

	/* reply/followup */
	case 'r':
	case 'f':
	{
		static char *arg[3] = {bfr, bfr + FPATHLEN, NULL};

#ifdef REPLYPROG
		strcpy(bfr, REPLYPROG);
#else
		sprintf(bfr, "%s/postreply", LIB);
#endif
		sprintf(bfr + FPATHLEN, "-v%c%s", c, filename);
		prun(arg, 0);
		if (pstatus != 0) {
			msg("reply/followup %s", pstatus == 22<<8? "suppressed" : "command failed");
		}
		nextwin = &emptywin;
		break;
	}

	/* next newsgroup */
	case 'N':
		if (next_ng_command())
			quitflg = 1;
		break;

	/* display parent article */
	case 'p':
		parent_command() ;
		break;

	/* specific message ID. */
	case '<':
		if (prget("<", prbuf))
			break;
		goto_id(prbuf);
		break;

	/* erase - pretend we haven't seen this article. */
	case 'e':
		erased = 1;
		setnew(1);
		nextart(count, 0);
		break;

	case 'H':
		if (openart())
			break;
		nextwin = &hdrwin;
		break;

	case '#':
		msg("Article %d of %d (#%d of max %ld)",
			curind, numthisng, curart->i_artnum,
			pngsize);
		nextwin = curwin;
		break;

		/* error */
	case '?':
		nextwin = &helpwin;
		break;

	illegal:
	default:
		beep();
		nextwin = curwin;
		break;
	}

	return FALSE;
}

cancel_command()
{
	struct arthead *hptr = &h;
	char *ptr1;
	int i;

	if (openart())
		return;
	strcpy(bfr, hptr->h_path);
	ptr1 = index(bfr, ' ');
	if (ptr1)
		*ptr1 = 0;
	i = strcmp(username, bfr);
	if (i != 0) {
		if (isadmin())
			msg("Cancelling locally only");
		else {
			msg("Can't cancel what you didn't write.");
			return;
		}
	}
	if (!cancel(stderr, hptr, i) && hptr == &h) {
		nextart(1, 1);
	}
}



next_ng_command()
{
	struct ngentry *ngp;

	if (prget("group? ", bptr = prbuf))
		return FALSE;
	if (!*bptr || *bptr == '-') {
		return getnxtng(*bptr? BACKWARD : FORWARD);
	}
	while (isspace(*bptr))
		bptr++;
	if ((ngp = findgroup(bptr)) == NULL) {
		msg("No such group: %s.", bptr);
		return FALSE;
	}
	switchng(ngp);
	return FALSE;
}


/*
 * Find parent of article.
 */
parent_command() {
	struct artrec a;
	DPTR dp;

	readrec(curart->i_dptr, &a);
	if (a.a_parent == DNULL) {
		msg("no parent");
		nextwin = curwin;
		return;
	}
	readrec(dp = a.a_parent, &a);
	if (a.a_flags & A_NOFILE) {
		msg("parent not on system");
		nextwin = curwin;
		return;
	}
	spclgrp(dp, &a);
}


/*
 * display an article with a particular article or message ID.
 */
goto_id(ident)
	char *ident;
	{
	struct artrec a;
	DPTR dp;
	char *ptr1, *ptr2;
	char id[NAMELEN];

	bfr[0] = '<';
	strcpy(bfr+1, ident);
	if (index(bfr, '@') == NULL && index(bfr, '>') == NULL) {
		ptr1 = bfr;
		if (*ptr1 == '<')
			ptr1++;
		ptr2 = index(ptr1, '.');
		if (ptr2 != NULL) {
			*ptr2++ = '\0';
			sprintf(prbuf, "<%s@%s.UUCP>", ptr2, ptr1);
			strcpy(bfr, prbuf);
		}
	}
	if (index(bfr, '>') == NULL)
		strcat(bfr, ">");
	scopyn(bfr, id, NAMELEN);
	if ((dp = lookart(id, &a)) == DNULL) {
		msg("%s not found", ident);
		nextwin = curwin;
		return;
	}
	spclgrp(dp, &a);
}



/*
 * Unsubscribe to a discussion.
 */
ud_command() {
	struct artrec a;
	DPTR dp;
	register int i ;

	if (ndunsub >= MAXUNSUB) {
		msg("Unsubscribed to too many discussions");
		return;
	}
	dp = curart->i_dptr;
	readrec(dp, &a);
	while (a.a_parent != DNULL) {
		readrec(dp = a.a_parent, &a);
	}
	dunsub[ndunsub++] = dp;
	msg("unsubscribed to discussion %s", a.a_ident);

	/* Mark any articles in the discussion as read. */
	for (i = 0 ; i < numthisng ; i++) {
		if (thisng[i].i_basetime == a.a_subtime)
			if (alptr->al_type == SVNEWSGROUP)
				clrunread(thisng[i].i_artnum) ;
	}
	nextart(1, 1) ;
}

/*
 * Execute a shell command.
 */

shcmd(cmd, flags)
	char *cmd;
	{
	char *arg[4];

	arg[0] = SHELL, arg[1] = "-c", arg[2] = cmd, arg[3] = NULL;
	prun(arg, flags);
}


prun(args, flags)
	char **args;
	{
	int pid;
	int i;
	int (*savequit)();
	char *env[100], **envp;
	char a[BUFLEN + 2];
	extern char **environ;

	if (!(flags & BKGRND)) {
		botscreen();
		ttycooked();
	}
	while ((pid = fork()) == -1)
		sleep(1);		/* must not clear alarm */
	if (pid == 0) {
		for (i = 3 ; i < 20 ; i++)
			close(i);
		if (flags & BKGRND) {
			signal(SIGINT, SIG_IGN);
			signal(SIGQUIT, SIG_IGN);
			close(0), close(1), close(2);
			open("/dev/null", 2);
			dup(0), dup(0);
		}
		/* set $A */
		sprintf(a, "A=%s", filename);
		env[0] = a;
		for (envp = env + 1 ; *environ != NULL && envp < env + 98 ; environ++)
			if ((*environ)[0] != 'A' || (*environ)[1] != '=')
				*envp++ = *environ;
		*envp = NULL;

		execve(args[0], args, env);
		fprintf(stderr, "%s: not found\n", args[0]);
		exit(20);
	}
	if (!(flags & BKGRND)) {
		savequit = signal(SIGQUIT, SIG_IGN);
		while ((i = wait(&pstatus)) != pid && (i != -1 || errno == EINTR));
		if (flags & CWAIT) {
			fprintf(stderr, "continue? ");
			while ((errno = 0, i = getchar()) != '\n'
				&& (i != EOF || errno == EINTR))
#ifdef old_version /* never worked under BSD */
					if (intflag && i == EOF) {
						fprintf(stderr, "\ncontinue? ");
						intflag = 0;
					}
#else
					;
#endif
		}
		signal(SIGQUIT, savequit);
		ttyraw();
		clearok(curscr, 1);
	}
}

#ifdef DIGPAGE


/*
 * Find end of current subarticle in digets.
 */

findend(l) {
	register i;
	register char *p;

	for (i = l ; i < l + ARTWLEN && i < lastlin ; i++) {
		tfget(bfr, i);
		for (p = bfr ; *p == '-' ; p++);
		if (p > bfr + 24)
			return i + 1;
	}
	return 0;
}

#endif


char *
getmailname() {
	char mailname[FPATHLEN];
	register char *p;

	if( (p = getenv("MAIL")) != NULL)
		return p;
#if BSDREL > 7
	sprintf(mailname, "/usr/spool/mail/%s", username);
#else
	sprintf(mailname, "/usr/mail/%s", username);
#endif
	return savestr(mailname);
}



/*** stolen from rfuncs2.c and modified ***/

vsave(to, flags)
register char *to;
{
	register FILE *ufp;
	int	isprogram = 0;
	int	isnew = 1;
	long	saveoff;
	char	temp[20];
	char	*fname;
	char	prog[BUFLEN + 24];

#define hh h
#define hfp fp
	saveoff = ftell(fp);
	fname = to;
	if (*to == PIPECHAR) {
		isprogram++;
		if (strlen(to) > BUFLEN) {
			msg("Command name too long");
			goto out;
		}
		flags |= OVWRITE;
		strcpy(temp, "/tmp/vnXXXXXX");
		mktemp(temp);
		fname = temp;
		_amove(ROWS - 1, 0);
		vflush();
	}
	if ((flags & OVWRITE) == 0) {
		ufp = fopen(fname, "r");
		if (ufp != NULL) {
			fclose(ufp);
			isnew = 0;
		}
	}
	if ((ufp = fopen(fname, (flags & OVWRITE) == 0? "a" : "w")) == NULL) {
		msg("Cannot open %s", fname);
		goto out;
	}
	/*
	 * V7MAIL code is here to conform to V7 mail format.
	 * If you need a different format to be able to
	 * use your local mail command (such as four ^A's
	 * on the end of articles) substitute it here.
	 */
	if (flags & SVHEAD) {
#ifdef V7MAIL
		fprintf(ufp, "From %s %s",
#ifdef INTERNET
				hh.h_from,
#else
				hh.h_path,
#endif
					ctime(&curart->i_subtime));
#endif
#ifdef V7MAIL
		fseek(fp, 0L, 0);
		while (fgets(bfr, LBUFLEN, fp) != NULL) {
			if (prefix(bfr, "From "))
				putc('>', ufp);
			fputs(bfr, ufp);
		}
		putc('\n', ufp);	/* force blank line at end (ugh) */
#else
		fseek(fp, artbody, 0);
		while (fgets(bfr, LBUFLEN, fp) != NULL) {
			fputs(bfr, ufp);
		}
#endif
	} else {
		fseek(fp, artbody, 0);
		while (fgets(bfr, LBUFLEN, fp) != NULL) {
			fputs(bfr, ufp);
		}
	}

	fclose(ufp);
	if (isprogram) {
		sprintf(prog, "(%s)<%s", to + 1, fname);
		shcmd(prog, CWAIT);
		nextwin = &emptywin;
	} else {
		if ((flags & OVWRITE) == 0)
			msg("file: %s %s", to, isnew ? "created" : "appended");
		else
			msg("file: %s written", to);
	}

out:
	if (isprogram) {
		unlink(fname);
	}
	fseek(fp, saveoff, 0);
}



/*** routines originally in rfuncs.c ***/


xerror(fmt, a1, a2, a3, a4)
char	*fmt;
{
	int clean = errclean;

	errclean = 0;
	fflush(stdout);
	if (clean) {
		botscreen();
		ttycooked();
	}
	fprintf(stderr, "vnews: ");
	fprintf(stderr, fmt, a1, a2, a3, a4);
	fprintf(stderr, ".\n");
	if (clean)
		writeoutrc();
	xxit(1);
}



xxit(status)
int	status;
{
	exit(status);
}
