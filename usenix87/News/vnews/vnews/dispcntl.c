#include "vnews.h"

#ifdef STATTOP
#define PRLINE	0	/* prompter line */
#define SPLINE	1	/* secondary prompt line */
#define ARTWIN	2	/* first line of article window */
#else
#define PRLINE	(ROWS-1)/* prompter line */
#define SPLINE	(ROWS-2)/* secondary prompt line */
#define ARTWIN	0	/* first line of article window */
#endif

int dumpart(), dumpheaders(), dumphelp(), dumphdr();
int nullsub();

struct window artwin = {		/* window containing article */
	0, 0, dumpart, 2};
struct window indexwin = {		/* window containing article index */
	0, 0, dumpheaders, 2};
struct window helpwin = {		/* window containing help message */
	0, 0, dumphelp, 2};
struct window emptywin = {		/* empty window */
	0, 0, nullsub, 2};
struct window hdrwin = {		/* window containing header */
	0, 0, dumphdr, 2};
extern struct svartlist *alptr;		/* article list stack */



/*
 * Open the current article.  Returns nonzero on failure.
 */
openart() {
	struct stat statb;
	FILE *gethead();

	if (fp != NULL)
		return 0;
#ifdef CURSES
	move(0, 0);	/* let user know we are thinking */
	refresh();
#else
	_amove(0, 0);	/* let user know we are thinking */
	vflush();
#endif
	artwin.w_force = 2;
	lastlin = 0;
	fp = fopen(filename, "r");
	if (fp == NULL) {
		msg("Can't open %s", filename);
		artlines = 0;
		artread = 1;
		return TRUE;
	}
	fstat(fileno(fp), &statb);
	artlength = statb.st_size;
	if (gethead(&h, fp) == NULL) {
		msg("Garbled header");
		fclose(fp);
		fp = NULL;
		artlines = 0;
		artread = 1;
		return TRUE;
	}
	{	/* strip off any notesfile header */
		register c;
		register char *p = h.h_title + strlen(h.h_title) - 5;
		if (p > h.h_title
		 && (strcmp(p, " (nf)") == 0 || strcmp(p, "(nf)\"") == 0)) {
			if ((c = getc(fp)) != '#') {
				ungetc(c, fp);
			} else {
				while ((c = getc(fp)) != '\n' && c != EOF);
				while ((c = getc(fp)) != '\n' && c != EOF);
				while ((c = getc(fp)) != '\n' && c != EOF);
			}
		}
	}
	artbody = ftell(fp);
	fmthdr();
	artlines = lastlin;
	artread = 0;
	if (! cflag && hdrend < ARTWLEN)
		hdronly = 1;
	artwin.w_artlin = 0;
	return 0;
}



/*
 * Print out whatever the appropriate header is
 */
fmthdr() {
	char *vbriefdate();
	char date[64];

	if (ngrp) {
		ngrp = 0;
		if (!hflag) {
			sprintf(bfr, "Newsgroup %s", curng->ng_name);  tfappend(bfr);
			tfappend("");
		}
	}
	hdrstart = lastlin;
	if (!hflag) {
		sprintf(bfr, "Article %s %s",
			h.h_ident, vbriefdate(h.h_subdate, date));
		tfappend(bfr);
	}
	vhprint(&h, pflag ? 1 : 0);
	sprintf(bfr, "(%d lines)", curart->i_nlines); tfappend(bfr);
	tfappend("");
	hdrend = lastlin;
}



/* Arpa format: Sat, 14-May-83 03:45:14 EDT */
/* Bugs: doesn't work on article with non-arpa dates */
char *
vbriefdate(q, date)
	register char *q;
	char *date;
	{
	register char *p;
	register i;
	char day[2];

	p = date;
	if (q[3] == ',') {
		for (i = 3 ; --i >= 0 ; )
			*p++ = *q++;
		*p++ = ' ';
		q += 2;
	}
	day[0] = *q++;
	if (*q != ' ' && *q != '-') day[1] = *q++;
	else day[1] = '\0';
	q++;
	for (i = 3 ; --i >= 0 ; )
		*p++ = *q++;
	*p++ = ' ';
	*p++ = day[0];
	if (day[1]) *p++ = day[1];
	q += 5;
	*p++ = ' ';
	if (q[-1] != '0') *p++ = q[-1];
	for (i = 4 ; --i >= 0 ; )
		*p++ = *q++;
	*p++ = '\0';
	return date;
}		

/*
 * Print the file header to the temp file.
 */
vhprint(hp, verbose)
register struct arthead *hp;
int	verbose;
{
	register char	*p1, *p2;
	int	i;
	char	fname[BUFLEN];

	fname[0] = '\0';		/* init name holder */

	p1 = index(hp->h_from, '(');	/* Find the sender's full name. */
	if (p1 == NULL && hset(hp->h_path))
		p1 = index(hp->h_path, '(');
	if (p1 != NULL) {
		strcpy(fname, p1+1);
		p2 = index(fname, ')');
		if (p2 != NULL)
			*p2 = '\0';
	}

	sprintf(bfr, "Subject: %s", hp->h_title);
	if ((i = strlen(bfr) - 7) > 9
	 && strcmp(bfr + i, " - (nf)") == 0
	 && (strncmp(bfr+9, "Re: ", 4) != 0 || i < 9+39))
		bfr[i] = '\0';		/* clobber "- (nf)" */
	tfappend(bfr);
	if (!hflag && hset(hp->h_keywords))
		sprintf(bfr, "Keywords: %s", hp->h_keywords), tfappend(bfr);
	if (verbose) {
		sprintf(bfr, "From: %s", hp->h_from); tfappend(bfr);
		sprintf(bfr, "Path: %s", hp->h_path); tfappend(bfr);
		if (hset(hp->h_organization))
			sprintf(bfr, "Organization: %s", hp->h_organization), tfappend(bfr);
	}
	else {
		if (p1 != NULL)
			*--p1 = '\0';		/* bump over the '(' */
#ifdef INTERNET
		/*
		 * Prefer Path line if it's in internet format, or if we don't
		 * understand internet format here, or if there is no reply-to.
		 */
		sprintf(bfr, "From: %s", hp->h_from);
#else
		sprintf(bfr, "Path: %s", tailpath(hp));
#endif
		if (fname[0] != '\0') {
			strcat(bfr, " (");
			strcat(bfr, fname);
			if (hset(hp->h_organization) && !hflag) {
				strcat(bfr, " @ ");
				strcat(bfr, hp->h_organization);
			}
			strcat(bfr, ")");
		}
		tfappend(bfr);
		if (p1 != NULL)
			*p1 = ' ';
		if (hset(hp->h_ctlmsg)) {
			sprintf(bfr, "Control: %s", hp->h_ctlmsg);
			tfappend(bfr);
		}
	}

	if (verbose) {
		sprintf(bfr, "Newsgroups: %s", hp->h_nbuf); tfappend(bfr);
		sprintf(bfr, "Date: %s", hp->h_subdate); tfappend(bfr);
		if (hset(hp->h_sender))
			sprintf(bfr, "Sender: %s", hp->h_sender), tfappend(bfr);
		if (hset(hp->h_replyto))
			sprintf(bfr, "Reply-To: %s", hp->h_replyto), tfappend(bfr);
		if (hset(hp->h_followto))
			sprintf(bfr, "Followup-To: %s", hp->h_followto), tfappend(bfr);
	}
	else if (strcmp(hp->h_nbuf, curng->ng_name) != 0)
		sprintf(bfr, "Newsgroups: %s", hp->h_nbuf), tfappend(bfr);
}

/*
 * Append file to temp file, handling control characters, folding lines, etc.
 * We don't grow the temp file to more than nlines so that a user won't have
 * to wait for 20 seconds to read in a monster file from net.sources.
 * What we really want is coroutines--any year now.
 */

#define ULINE 0200
static char *maxcol;


appfile(iop, nlines)
	register FILE *iop;
	{
	register int c;
	register char *icol;	/* &bfr[0] <= icol <= maxcol */

	if (artread || artlines >= nlines)
		return;
	maxcol = bfr;
	icol = bfr;
	while ((c = getc(iop)) != EOF) {
		switch (c) {
		case ' ':
			if (icol == maxcol && icol < bfr + LBUFLEN - 1) {
				*icol++ = ' ';
				maxcol = icol;
			} else {
				icol++;
			}
			break;
		case '\t':
			icol = (icol - bfr &~ 07) + 8 + bfr;
			growline(icol);
			break;
		case '\b':
			if (icol > bfr) --icol;
			break;
		case '\n':
			outline();
			if (artlines >= nlines)
				return;
			icol = bfr;
			break;
		case '\r':
			icol = bfr;
			break;
		case '\f':
			outline(); outline(); outline();
			if (artlines >= nlines)
				return;
			icol = bfr;
			break;
		default:
			if (c < ' ' || c > '~')
				break;
			else if (icol >= bfr + LBUFLEN - 1)
				icol++;
			else if (icol == maxcol) {
				*icol++ = c;
				maxcol = icol; }
			else if (*icol == ' ')
				*icol++ = c;
			else if (c == '_')
				*icol++ |= ULINE;
			else
				*icol++ = (c | ULINE);
			break;
		}
	}
	if (maxcol != bfr)		/* file not terminated with newline */
		outline();
	artread++;
}



growline(col)
	char *col;
	{
	while (maxcol < col && maxcol < bfr + LBUFLEN - 1)
		*maxcol++ = ' ';
}


outline() {
	*maxcol = '\0';
	if (strncmp(bfr, ">From ", 6) == 0) {
		register char *p;
		for (p = bfr ; (*p = p[1]) != '\0' ; p++);
	}
	tfappend(bfr);
	if (maxcol > bfr)
		artlines = lastlin;
	maxcol = bfr;
}



prget(prompter, buf)
	char *prompter, *buf;
	{
	char *p, *q, *r;
	int c, lastc;

	curflag = CURP2;
	r = buf;
	lastc = '\0';
	for (;;) {
		*r = '\0';
		p = secpr;
		for (q = prompter ; *q ; q++)
			*p++ = *q;
		for (q = buf ; *q ; q++) {
			if (p < &secpr[SECPRLEN-1] && *q >= ' ' && *p <= '~')
				*p++ = *q;
		}
		*p = '\0';
		c = vgetc();
		if (c == '\n' || c == INTR) {
			break;
		}
		if (c == cerase) {
			if (lastc == '\\')
				r[-1] = c;
			else if (r > buf)
				r--;
		} else if (c == ckill) {
			if (lastc == '\\')
				r[-1] = c;
			else
				r = buf;
		} else if (r < buf + SECPRLEN - 1) {
			*r++ = c;
		} else {
			beep();
		}
		lastc = c;
	}
	curflag = CURHOME;
	secpr[0] = '\0';
	if (c == INTR) {
		nextwin = curwin;
		return 1;
	} else
		return 0;
}



/*** Routines for handling temporary file ***/

/*
 * Append to temp file.
 * Long lines are folded.
 */

tfappend(line)
	char *line;
	{
	while (strlen(line) > COLS) {
		tfput(line, lastlin++);
		line += COLS;
	}
	tfput(line, lastlin++);
}


tfput(line, linno)
	char *line;
	{
	register char *p;
	register FILE *rtfp;		/* try to make it a litte faster */
	register int i;

	p = line;
#if BSDREL > 7
	i = (COLS + 1) &~ 1;
#else
	i = COLS;
#endif
	tfseek(linno, 1);
	rtfp = tfp;
	while (--i >= 0) {
		if (*p)
			putc(*p++, rtfp);
		else
			putc('\0', rtfp);
	}
	tflinno++;
}


tfget(line, linno)
	char *line;
	{
	tfseek(linno, 0);
#if BSDREL > 7
	fread(line, (COLS + 1) &~ 1, 1, tfp);
#else
	fread(line, COLS, 1, tfp);
#endif
	line[COLS] = '\0';
	tflinno++;
}


tfseek(linno, wrflag) {
	static int last = 1;

	if (linno != tflinno || wrflag != last) {
#if BSDREL > 7
		fseek(tfp, (long)linno * (COLS + 1 &~ 1) + tfoffset, 0);
#else
		fseek(tfp, (long)linno * COLS + tfoffset, 0);
#endif
		tflinno = linno;
		last = wrflag;
	}
}



/*** display management ***/


msg(s, a1, a2, a3, a4, a5, a6) char *s; {
	sprintf(secpr, s, a1, a2, a3, a4, a5, a6);
}


scroll(amount) {
	register struct window *w;

	w = &artwin;
	if (indexpg)
		w = &indexwin;
	if ((w->w_artlin += amount) < 0)
		w->w_artlin = 0;
}



/*
 * Update the display.
 * The display is entirely controlled by this routine,
 * which means that this routine may get pretty snarled.
 */

static struct window *savewin;		/* window on last call to updscr */

updscr() {
	int i;
	register struct window *w = curwin;
	char buf[40];

	if (checkin())
		return;
	if (w != savewin)
		w->w_force = 2;
	if ((w->w_force || w->w_artlin != w->w_svartlin)
	 && (quitflg == 0 || w == &emptywin)) {
#ifndef CURSES
		if (w->w_force < 2)
			ushift(ARTWIN, ARTWIN+ARTWLEN-1, w->w_artlin - w->w_svartlin);
#endif
		for (i = ARTWIN ; i < ARTWIN + ARTWLEN ; i++)
			clrline(i);
		(*w->w_dump)(w->w_artlin, ARTWIN, ARTWLEN);
		w->w_force = 0;
		w->w_svartlin = w->w_artlin;
		savewin = w;
	}
	clrline(SPLINE), clrline(PRLINE);
#ifdef STATTOP
	mvaddstr(PRLINE, 0, prompt);
#else
	if (strlen(secpr) <= COLS)
		mvaddstr(PRLINE, 0, prompt);
#endif
	mvaddstr(PRLINE, 48, timestr);
	if (alptr->al_type == SVARTICLE)
		sprintf(buf, "%.18s", curng->ng_name);
	else
		sprintf(buf, "%.18s %d/%d ", curng->ng_name, curind, numthisng);
	mvaddstr(PRLINE, 20, buf);
	if (ismail)
		mvaddstr(PRLINE, 62, ismail > 1? "MAIL" : "mail");
	mvaddstr(SPLINE, 0, secpr);
	if (curflag == CURP1)
		move(PRLINE, strlen(prompt));
	else if (curflag == CURHOME)
		move(0, 0);
	refresh();
}


addnum(n) {
	if (n >= 10)
		addnum(n / 10);
	addch(n % 10 + '0');
}



/*
 * Dump the article to the screen.  Updscr has already cleared the region.
 */

dumpart(artlin, scrlin, count) {
	register int i;

	if (hdronly && count > hdrend - artlin)
		count = hdrend - artlin;
#ifdef DIGPAGE
	if (endsuba > 0 && count > endsuba - artlin)
		count = endsuba - artlin;
#endif
	if (count > lastlin - artlin)
		count = lastlin - artlin;
	for (i = 0 ; i < count ; i++) {
		tfget(bfr, artlin + i);
		mvaddstr(scrlin + i, 0, bfr);
	}
}


/*
 * All headers command: show headers for this group.
 */
dumpheaders(artlin, scrlin, count)
{
	register int i;

	for (i = count ; --i >= 0 ; artlin++, scrlin++) {
		if (artlin == 0) {
			sprintf(bfr, " IND FILE LINES Newsgroup %s, %d articles", curng->ng_name, numthisng);
			mvaddstr(scrlin, 0, bfr);
		} else if (artlin >= 2 && artlin < numthisng + 2) {
			move(scrlin, 0);
			addch(artlin-1 == curind ? '>' :
			    (xflag || isunread(thisng[artlin-2].i_artnum) ? ' ': 'D'));
			sprintf(bfr, "%3d%5d%5d %-36s  %s",
				artlin-1, thisng[artlin-2].i_artnum, thisng[artlin-2].i_nlines,
				thisng[artlin-2].i_title, thisng[artlin-2].i_from);
			bfr[COLS] = '\0';
			addstr(bfr);
		}
	}
}


/*
 * Dump the help message to the screen.  The help msg can't be scrolled.
 */
dumphelp(artlin, scrlin, count) {
	FILE *helpf;
#ifndef VHELP
	char VHELP[FPATHLEN];
#endif

#ifndef VHELP
	sprintf(VHELP, "%s/vnews.help", LIB);
#endif
	if ((helpf = fopen(VHELP, "r")) == NULL) {
		addstr("Can't open help file");
		return;
	}
	while (fgets(bfr, LBUFLEN, helpf) != NULL) {
		nstrip(bfr);
		mvaddstr(scrlin, 0, bfr);
		scrlin++;
	}
	fclose(helpf);
}


/*
 * Dump the full header on the screen.  The header cannot be scrolled.
 * We use the routine appfile to append to the file, which forces us
 * to do some juggling to return to the article.
 */
dumphdr(artlin, scrlin, count) {
	long saveoff;
	int svartread, svartlines, svlastlin;
	int stoppos = scrlin + count - 1 ;
	register int i;

	saveoff = ftell(fp);
	fseek(fp, 0L, 0);
	svartread = artread, svartlines = artlines, svlastlin = lastlin;
	artread = 0;
	appfile(fp, artlines + count);
	if (count > lastlin - svlastlin)
		count = lastlin - svlastlin;
	for (i = 0 ; i < count ; i++) {
		tfget(bfr, svlastlin + i);
		if (bfr[0] == '\0')
			break;
		mvaddstr(scrlin + i, 0, bfr);
	}
	nochange(i + 1, stoppos) ;
	artread = svartread, artlines = svartlines, lastlin = svlastlin;
	fseek(fp, saveoff, 0);
}


nullsub() {;}
