/*
** vn news reader.
**
** reader.c - article reading interface - "more" like.
**
** see copyright disclaimer / history in vn.c source file
*/

#include <stdio.h>
#include <sys/types.h>
#include "tty.h"
#include "config.h"
#include "vn.h"
#include "head.h"
#include "reader.h"

#define PERTAB 8	/* tab expansion factor */
#define BACKTRACK 24

extern char *Printer,*Editor,*Mailer,*Poster,*Orgdir,*Savefile,*Savedir,*Ccfile;
extern int L_allow;
extern int C_allow;
extern int Rot;
extern int Headflag;
extern int Digest;
extern char *No_msg;
extern char *Roton_msg;
extern char *Rotoff_msg;
extern char *Hdon_msg;
extern char *Hdoff_msg;

extern char *T_head, *FT_head, *N_head, *L_head, *RT_head, *DIS_head;
extern char *TO_head, *F_head, *P_head, *M_head, *R_head;

extern char Cxrtoi[], Cxitor[];

static FILE *Fpread;
static char *Fname;
static char *Lookahead;
static int Rlines;
static int Hlines;

#ifdef ADDRMUNGE
static int Newaddr;
#endif

/*
	readstr routine is the "funnel" to the reading state,
	and controls signal setting.  Some "session" context is passed
	along to allow jumping back to a different display

	WARNING:

	NOTHING below readstr should call strtok()
*/

readstr (s,crec,highrec,count)
char *s;
int *crec, *highrec;
int count;
{
	char *fnext, *strtok();
	int pc;
	Fname = strtok(s,LIST_SEP);
	if (Fname != NULL)
	{
		term_set (ERASE);
		sig_set (BRK_READ,&Fpread);
		fnext = strtok(NULL,LIST_SEP);
		while (Fname != NULL && readfile(fnext,&pc) >= 0)
		{
			if (Digest)
				unlink (Fname);
			Fname = fnext;
			fnext = strtok (NULL,LIST_SEP);
		}
		if (Digest && Fname != NULL)
			unlink (Fname);
		if (pc != 0)
			forward (pc, crec, highrec);
		else
		{
			*crec += count;
			if (*crec >= *highrec)
				*crec = *highrec - 1;
		}
		sig_set (BRK_RFIN);
		show ();
		term_set (MOVE, 0, *crec);
	}
	else
	{
		preinfo ("%s",No_msg);
		term_set (MOVE, 0, *crec);
	}
}

/*
	readfile presents article:
		sn - name of next article, NULL if last.
		pages - pages to advance on return, if applicable
	returns 0 for "continue", <0 for "quit"
*/
static readfile (sn,pages)
char *sn;
int *pages;
{
	FILE *fopen();
	int lines,percent,artlin;
	long rew_pos, ftell();
	char c,  buf[RECLEN], mid[RECLEN], ngrp[RECLEN], dist[RECLEN];
 	char from[RECLEN], title[RECLEN], flto[RECLEN], reply[RECLEN];
 	char pstr[24], dgname[48], getpgch(), *index(), *digest_extract();
	char *tgetstr();

	*pages = 0;

	term_set(ERASE);

	if (Digest)
	{
		lines = atoi(Fname);
		if ((Fname = digest_extract(dgname,lines)) == NULL)
		{
			printf ("couldn't extract article %d",lines);
			return (0);
		}
	}

	if ((Fpread = fopen(Fname,"r")) == NULL)
	{
		printf ("couldn't open article %s",Fname);
		return (0);
	}

	Hlines = gethead (mid, from, title, ngrp, flto, reply, dist, &artlin);
	printf (ANFORM,Fname,Cxrtoi[PG_HELP]);
	lines = 1;
	rew_pos = ftell(Fpread);
	if (Headflag)
	{
		rewind(Fpread);
		Rlines = 0;
	}
	else
	{
		/* use do_out to guard against control sequences */
		Rlines = Hlines;
		sprintf (buf,"%s%s\n",T_head,title);
		lines += do_out(buf,1);
		sprintf (buf,"%s%s\n",F_head,from);
		lines += do_out(buf,1);
		if (index(ngrp,',') != NULL)
		{
			sprintf (buf,"%s%s\n",N_head,ngrp);
			lines += do_out(buf,1);
		}
		if (*flto != '\0')
		{
			sprintf (buf,"%s%s\n",FT_head,flto);
			lines += do_out(buf,1);
		}
		printf ("%s%d\n",L_head,artlin);	/* no controls */
		++lines;
	}

	/* will return out of outer while loop */
	Lookahead = NULL;
	while (1)
	{
		/*
		** lines counts folded lines from do_out.
		** globals Hlines and Rlines refer to records.
		** If Lookahead is null after this loop, we've
		** hit EOF.
		*/
		lines += do_out(Lookahead,L_allow-lines);
		while (1)
		{
			if (Lookahead == NULL)
			{
				if (fgets(buf,RECLEN-1,Fpread) == NULL)
					break;
				Lookahead = buf;
				if (Rot != 0 && Rlines >= Hlines)
					rot_line(buf);
				++Rlines;
			}
			if (lines >= L_allow)
				break;
			lines += do_out(buf,L_allow-lines);
		}

		if (Lookahead != NULL)
		{
			/*
			** calculation is truncated rather than rounded,
			** so we shouldn't get "100%".  Subtract 2 for
			** 1 line lookahead and empty line at beginning
			** of article.
			*/
			if (Headflag)
				percent = ((Rlines-2)*100)/(artlin+Hlines);
			else
				percent = ((Rlines-Hlines-2)*100)/artlin;
			sprintf (pstr,PAGE_MID,percent);
		}
		else
		{
			if (sn == NULL)
				strcpy (pstr,PAGE_END);
			else
			strcpy (pstr,PAGE_NEXT);
		}
		c = getpgch(pstr,mid,from,reply,title,ngrp,flto,dist);

		/*
			handle user input:
			CAUTION!!  return cases must close Fpread.
		*/
		switch (c)
		{
		case PG_NEXT:
			fclose (Fpread);
			return (0);
		case PG_FLIP:
			*pages = 1;	/* fall through */
		case PG_QUIT:
			fclose (Fpread);
			return (-1);
		case PG_REWIND:
			if (Headflag)
			{
				Rlines = 0;
				rewind (Fpread);
			}
			else
			{
				fseek (Fpread,rew_pos,0);
				Rlines = Hlines;
			}
			Lookahead = NULL;
			lines = 2 - RECBIAS;
			break;
		case PG_SEARCH:
			searcher(buf);
			lines = 2 - RECBIAS;
			lines += do_out(buf,L_allow-lines);
			break;
		case PG_WIND:
			fseek (Fpread,0L,2);
			lines = 2 - RECBIAS;
			Lookahead = NULL;
			break;
		case PG_STEP:
			if (Lookahead == NULL)
			{
				fclose (Fpread);
				return (0);
			}
			lines = L_allow - 1;
			break;
		default:
			if (Lookahead == NULL)
			{
				fclose (Fpread);
				return (0);
			}
			lines = 2 - RECBIAS;
			break;
		}
	}
}

/*
	gethead obtains subject, reply, message id, from, lines, newsgroup and
	followup-to lines of article for later use in mailing replies and
	posting followups, does not rewind, but leaves file at end of header
	lines.  Returns number of header lines.
*/
static gethead (mid, from, title, ngrp, flto, reply, dist, lin)
char *mid, *from, *title, *ngrp, *flto, *reply, *dist;
int *lin;
{
	int count;
	char buf [RECLEN], *index();
	long pos,ftell();

#ifdef ADDRMUNGE
	Newaddr = 1;
#endif

	*lin = 0;
	*dist = *mid = *from = *title = *ngrp = *flto = *reply = '\0';

	/* for conditional is abnormal - expected exit is break */
	for (count = 0; count < HDR_LINES && fgets(buf,RECLEN-1,Fpread) != NULL; ++count)
	{

		/* reset position and bail out at first non-header line */
		if (index(buf,':') == NULL)
		{
			pos = ftell(Fpread);
			pos -= strlen(buf);
			fseek (Fpread,pos,0);
			break;
		}

#ifdef MAILSMART
		if (strncmp(buf,RT_head,RTHDLEN) == 0)
		{
			buf [strlen(buf)-1] = '\0';
			strcpy (reply,buf+RTHDLEN);
			continue;
		}
#else
		if (strncmp(buf,P_head,PHDLEN) == 0)
		{
			buf [strlen(buf)-1] = '\0';
			strcpy (reply,buf+PHDLEN);
			continue;
		}
#endif
		if (strncmp(buf,DIS_head,DISHDLEN) == 0)
		{
			buf [strlen(buf)-1] = '\0';
			strcpy (dist,buf+DISHDLEN);
			continue;
		}
		if (strncmp(buf,M_head,MHDLEN) == 0)
		{
			buf [strlen(buf)-1] = '\0';
			strcpy (mid,buf+MHDLEN);
			continue;
		}
		if (strncmp(buf,F_head,FHDLEN) == 0)
		{
			buf [strlen(buf)-1] = '\0';
			strcpy (from,buf+FHDLEN);
			continue;
		}
		if (strncmp(buf,T_head,THDLEN) == 0)
		{
			buf [strlen(buf)-1] = '\0';
			strcpy (title,buf+THDLEN);
			continue;
		}
		if (strncmp(buf,N_head,NHDLEN) == 0)
		{
			buf [strlen(buf)-1] = '\0';
			strcpy (ngrp,buf+NHDLEN);
			continue;
		}
		if (strncmp(buf,FT_head,FTHDLEN) == 0)
		{
			buf [strlen(buf)-1] = '\0';
			strcpy (flto,buf+FTHDLEN);
			continue;
		}
		if (strncmp(buf,L_head,LHDLEN) == 0)
		{
			buf [strlen(buf)-1] = '\0';
			*lin = atoi(buf+LHDLEN);
			continue;
		}
	}
#ifdef MAILSMART
	if (*reply == '\0')
		strcpy(reply,from);
#endif
	return (count);
}

/*
	getpgch prints prompt and gets command from user
	handles "mail", "save" and "followup" internally
	as well as flag resets.
*/
static char getpgch(prompt,mid,from,reply,title,ngrp,flto,dist)
char *prompt, *mid, *from, *reply, *title, *ngrp, *flto, *dist;
{
	char c;
	int ic;
	term_set (ONREVERSE);
	printf("%s\015",prompt);
	term_set (OFFREVERSE);
	while ((ic=getnoctl()) != EOF)
	{
		switch (c = Cxitor[ic])
		{
		case SETROT:
			term_set (ZAP,0,PPR_MAX);
			if (Rot == 0)
			{
				Rot = 13;
				printf ("%s\n",Roton_msg);
			}
			else
			{
				Rot = 0;
				printf ("%s\n",Rotoff_msg);
			}
			if (Lookahead != NULL && Rlines > Hlines)
				rot_line(Lookahead);
			break;
		case HEADTOG:
			term_set (ZAP,0,PPR_MAX);
			if (Headflag)
			{
				Headflag = FALSE;
				printf ("%s\n",Hdoff_msg);
			}
			else
			{
				Headflag = TRUE;
				printf ("%s\n",Hdon_msg);
			}
			break;
		case PG_HELP:
			term_set (ZAP,0,PPR_MAX);
			help_pg ();
			break;
		case PG_REPLY:
			mail (reply,title,from);
			break;
		case PG_FOLLOW:
			followup (mid,title,ngrp,flto,from,dist);
			break;
		case SAVE:
			saver ();
			break;
		case PRINT:
			printr ();
			break;
		default:
			term_set (ZAP,0,PPR_MAX);
			return (c);
		}

		term_set (ONREVERSE);
		printf("%s\015",prompt);
		term_set (OFFREVERSE);
	}
	term_set (ZAP,0,PPR_MAX);
	return (c);
}

/*
	save article
	Like the savestr routine, it "loses" some storage every time
	the user specifies a new file, but this should not be significant
*/
static saver ()
{
	char *fn,cmd[RECLEN],*str_store(),*rprompt();

	tty_set (SAVEMODE);
	sprintf (cmd,SAVFORM,Savefile);
	fn = rprompt(cmd,cmd);
	if (fn != NULL)
		Savefile = str_store(fn);
	if (*Savefile != '/' && *Savefile != '$')
		sprintf (cmd,"cat %s >>%s/%s",Fname,Savedir,Savefile);
	else
		sprintf (cmd,"cat %s >>%s",Fname,Savefile);
	system (cmd);
	tty_set (RESTORE);
}

/*
	invoke editor on new temp file, mail using reply line,
	possibly first allowing user to overide the reply (not INLETTER)
*/
static mail (p, t, f)
char *p, *t, *f;
{
	char *new, fn[L_tmpnam], cmd [RECLEN+60], *rprompt ();
	FILE *fp, *fopen();

	tmpnam (fn);
	if ((fp = fopen(fn,"w")) == NULL)
		printex ("can't open %s\n",fn);

	if ((new = index(p, '(')) != NULL)
		*new = '\0';	/* a poor way of deleting comments */

#ifdef ADDRMUNGE
	if (Newaddr)
	{
		Newaddr = 0;
		ADDRMUNGE(p);
	}
#endif

	if (strncmp(t, FPFIX, FPFLEN) == 0)
		t += FPFLEN;	/* don't add multiple Re:s */
#ifdef INLETTER
 	fprintf (fp,"%s%s\n%s%s%s\n\n%s:\n", TO_head, p, T_head, FPFIX, t, f);
#else
	fprintf (fp,"%s%s%s\n\n%s:\n", T_head, FPFIX, t, f);
#endif

	edcopy (fp);
	fclose (fp);
	tty_set (SAVEMODE);

#ifndef INLETTER
	sprintf (cmd,"ADDRESS: %s\nreturn to accept, or input new address: ",p);
	if ((new = rprompt(cmd,cmd)) != NULL)
		strcpy (p,new);
#endif

	sprintf (cmd,"%s %s", Editor, fn);
	chdir (Orgdir);
	system (cmd);
	cd_group ();
	new = rprompt ("still want to mail it ? ",cmd);
	if (new != NULL && (*new == 'y' || *new == 'Y'))
	{
#ifndef INLETTER
		sprintf (cmd,"%s '%s' <%s", Mailer, p, fn);
#else
		sprintf (cmd,"%s <%s", Mailer, fn);
#endif
		system (cmd);
		printf ("given to mailer\n");
	}
	else
		printf ("not mailed\n");
	unlink (fn);
	tty_set (RESTORE);
	term_set (RESTART);
}

/*
	post a followup article, invoking editor for user after creating
	new temp file.  remove after posting.  Hack in ".followup" if posting
	newsgroup ends in ".general" - similar hack for preventing mod &
	announce groups - should really be more thorough and parse the
	whole string.  User can change, anyway.
*/
static followup (m,t,n,ft,oa,dist)
char *m, *t, *n, *ft, *oa, *dist;
{
	char fn[L_tmpnam], *new, cmd [RECLEN], *rprompt();
	FILE *f, *fopen();
	char *rindex();

	if (*ft != '\0')
		strcpy (cmd,ft);
	else
		strcpy (cmd,n);
	new = rindex(cmd,'.');
	if (new != NULL && strcmp(new,".general") == 0)
		strcpy (new,".followup");
	if ( strncmp(cmd, "mod.", 4) == 0 || strcmp(new, ".announce") == 0)
	{
		term_set (ONREVERSE);
		printf("Cannot post a follow-up to \"%s\", reply with mail to moderator\007\n",
			cmd);
		term_set (OFFREVERSE);
		return;
	}

	tmpnam (fn);
	if ((f = fopen(fn,"w")) == NULL)
		printex ("can't open %s\n",fn);

	if (strncmp(t, FPFIX, FPFLEN) == 0)
		t += FPFLEN;	/* don't add multiple Re:s */
	fprintf (f,"%s%s%s\n%s%s\n%s%s\n",T_head,FPFIX,t,N_head,cmd,R_head,m);
	if (*dist != '\0')
		fprintf(f,"%s%s\n",DIS_head,dist);
	fprintf (f,"\nin article %s, %s says:\n",m,oa);
	edcopy (f);
	fclose (f);
	tty_set (SAVEMODE);
	sprintf (cmd,"%s %s", Editor, fn);
	chdir (Orgdir);
	system (cmd);
	cd_group ();
	new = rprompt("still want to post it ? ",cmd);
	if (new != NULL && (*new == 'y' || *new == 'Y'))
	{
		sprintf (cmd,"%s <%s", Poster, fn);
		system (cmd);
		printf ("given to posting program\n");
		save_article (fn);
	}
	else
		printf ("not posted\n");
	unlink (fn);
	tty_set (RESTORE);
	term_set (RESTART);
}

/*
	get user buffer, return whitespace delimited token
	without using strtok().  buffer is allowed to overwrite
	prompt string.
*/
static char *
rprompt(s,buf)
char *s,*buf;
{
	printf("%s",s);
	fgets (buf,RECLEN-1,stdin);
	while (*buf == ' ' || *buf == '\t')
		++buf;
	if (*buf == '\n' || *buf == '\0')
		return (NULL);
	for (s = buf; *s != ' ' && *s != '\t' && *s != '\n' && *s != '\0'; ++s)
		;
	*s = '\0';
	return (buf);
}

/*
	edcopy copies article to file which user is editting for
	a reply or followup, so it may be referenced.  It places
	ED_MARK in the left hand margin.
*/
edcopy(fp)
FILE *fp;
{
	long current;
	char buf[RECLEN];
	int i;

	/* save position, rewind and skip over header lines */
	current = ftell(Fpread);
	rewind (Fpread);
	for (i=0; i < HDR_LINES; ++i)
	{
		if (fgets(buf,RECLEN-1,Fpread) == NULL)
			break;
		if (strncmp(buf,L_head,LHDLEN) == 0)
			break;
	}

	/* if line already begins with ED_MARK, forget about the space */
	while (fgets(buf,RECLEN-1,Fpread) != NULL)
	{
		if (buf[0] == ED_MARK)
			fprintf(fp,"%c%s",ED_MARK,buf);
		else
			fprintf(fp,"%c %s",ED_MARK,buf);
	}

	/* restore position */
	fseek(Fpread,current,0);
}

/*
	help menus
*/
static help_pg()
{
	h_print (Cxrtoi[PG_NEXT],HPG_NEXT);
	h_print (Cxrtoi[PG_QUIT],HPG_QUIT);
	h_print (Cxrtoi[PG_FLIP],HPG_FLIP);
	h_print (Cxrtoi[PG_REWIND],HPG_REWIND);
	h_print (Cxrtoi[PG_WIND],HPG_WIND);
	h_print (Cxrtoi[PG_SEARCH],HPG_SEARCH);
	h_print (Cxrtoi[PG_STEP],HPG_STEP);
	h_print (Cxrtoi[PG_REPLY],HPG_REPLY);
	h_print (Cxrtoi[PG_FOLLOW],HPG_FOLLOW);
	h_print (Cxrtoi[SAVE],HPG_SAVE);
	h_print (Cxrtoi[PRINT],HPG_PRINT);
	h_print (Cxrtoi[SETROT],HPG_ROT);
	h_print (Cxrtoi[HEADTOG],HPG_HEAD);
	h_print (Cxrtoi[PG_HELP],HPG_HELP);
	printf ("%s\n",HPG_DEF);
}

rot_line (s)
unsigned char *s;
{
	for ( ; *s != '\0'; ++s)
	{
		if (*s >= 'A' && *s <= 'Z')
		{
			*s += Rot;
			if (*s > 'Z')
				*s -= 26;
			continue;
		}
		if (*s >= 'a' && *s <= 'z')
		{
			*s += Rot;
			if (*s > 'z')
				*s -= 26;
		}
	}
}

/*
** output record.  folds record to terminal width on word boundaries,
** returning number of lines output.  If limit is reached, remainder
** of buffer waiting to be output is returned.  Processes several
** special characters:
**	form-feed - return "lim" lines so we stop on that line
**	tabs - counts "expanded" width
**	backspace - assumes they work, -1 width unless in first col.
**	bell - pass through with zero width
**	newline - end of record.
**	del - turns into '_'
**	other control - 'A' - 1 added ('01' = ctl-A).  Makes escape = "[".
**		(prevents "letter bombs" containing inappropriate control
**			sequences for the terminal).
**
** Sets Lookahead pointer to remainder of line or NULL.
*/
static do_out(s,lim)
char *s;
int lim;
{
	int len,i;
	char cs,*word,*start;

	Lookahead = NULL;
	if (s == NULL)
		return(0);
	len = 0;
	start = word = s;

	/*
	** NOTE: "normal" return is buried inside switch, at newline
	** ending record
	*/
	for (i=0; i < lim; ++i)
	{
		for ( ; len < C_allow; ++s)
		{
			switch (*s)
			{
			case '\n':
				*s = '\0';	/* fall through */
			case '\0':
				printf("%s\n",start);
				return(i+1);
			case '\t':
				len = ((len/PERTAB)+1)*PERTAB;
				word = s;
				break;
			case '\b':
				if (len > 0)
					--len;
				break;
			case '\014':
				*s = ' ';
				i = lim-1;	/* fall through */
			case ' ':
				word = s+1;
				++len;
				break;
			case '\177':
				*s = '_';
				++len;
				break;
			default:
				if (*s < ' ')
					*s += 'A' - 1;
				++len;		/* fall through */
			case '\07':
				break;
			}
		}
		cs = *s;
		*s = '\0';
		if ((len = strlen(word)) < BACKTRACK)
		{
			*s = cs;
			s = word;
			cs = *s;
			*s = '\0';
		}
		else
			len = 0;
		printf("%s\n",start);
		start = s;
		*s = cs;
	}
	Lookahead = start;
	return(lim);
}

save_article(tempfname)
char *tempfname;
{
	FILE *in, *out;
	int c;
	time_t timenow, time();
	char *today, *ctime();


	if ((in = fopen(tempfname, "r")) == NULL)
		return;
	if ((out = fopen(Ccfile, "a")) == NULL)
	{
	    fclose(in);
	    return;
	}
	timenow = time((time_t)0);
	today = ctime(&timenow);
	fprintf(out,"From vn %s",today);
	while ((c=getc(in)) != EOF)
		putc(c, out);
	putc('\n', out);
	fclose(in);
	fclose(out);
	printf ("a copy has been saved in %s\n", Ccfile);
}

/*
	send article to printer
*/
static printr ()
{
	char cmd[RECLEN];

	tty_set (SAVEMODE);
	printf("Sent to printer\n");
	sprintf (cmd,"%s %s 2>/dev/null",Printer,Fname);
	system (cmd);
	tty_set (RESTORE);
}

/*
	search article for specified search pattern, returning the line on which
		it is found in buf, a null buffer otherwise. The input file will
		be positioned either after the line on which the pattern is
		found, or unaaltered if match fails.
*/
searcher (buf)
char	*buf;
{
	static char	searchstr[RECLEN] = "";
	char	lasave[RECLEN];
	char	*s, *reg, *rprompt(), *regcmp(), *regex();
	long	current;
	int	orlines;

	/* save position, then request search pattern */
	current = ftell(Fpread);
	orlines = Rlines;

 	tty_set (SAVEMODE);
 	sprintf (lasave,SEARCHFORM,searchstr);
	s = rprompt(lasave,lasave);
	tty_set (RESTORE);
	if (s != NULL)
		strcpy(searchstr, lasave);
 
	/* Now compile the search string */
	if(( reg = regcmp(searchstr, (char *)0)) == NULL) {
		printf("Invalid search string \"%s\"\n", searchstr);
		*buf = '\0';
		return;
	}

	/* try lookahead buffer first */
	if (Lookahead != NULL && regex(reg,Lookahead) != NULL)
	{
		strcpy(buf,Lookahead);
		regfree(reg);
		return;
	}

	/* Lookahead can point into buf */
	if (Lookahead != NULL)
		strcpy(lasave,Lookahead);

	/* now start reading lines, rotating if necessary and do search */
	while (fgets(buf,RECLEN-1,Fpread) != NULL)
	{
		if (Rot != 0 && Rlines >= Hlines)
			rot_line(buf);
		++Rlines;
		if( regex(reg, buf) != NULL ){	/* Got it */
			term_set (ONREVERSE);
			printf("\n\tSkipping ....\n\n");
			term_set (OFFREVERSE);
			regfree(reg);
			return;
		}
	}

	/* no dice, so restore position */
	regfree(reg);
	term_set (ONREVERSE);
	printf("Cannot find string \"%s\" in remainder of article\007\n",
		searchstr);
	term_set (OFFREVERSE);
	fseek(Fpread,current,0);
	Rlines = orlines;
	if (Lookahead != NULL)
		strcpy(buf,lasave);
	else
		*buf = '\0';
	return(0.0);
}
