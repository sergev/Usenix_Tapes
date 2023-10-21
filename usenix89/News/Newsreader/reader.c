#include <stdio.h>
#include "tty.h"
#include "vn.h"
#include "head.h"
#include "reader.h"

#define PERTAB 8	/* tab expansion factor */
#define BACKTRACK 24

extern char *Editor,*Mailer,*Poster,*Orgdir,*Savefile,*Savedir;
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

extern char *T_head, *FT_head, *N_head, *L_head;
extern char *F_head, *P_head, *M_head, *R_head;

static FILE *Fpread;
static char *Fname;

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
	int lines,hlines,rlines,percent,artlin;
	long rew_pos, ftell();
	float total,lcount;
	double atof();
	char *optr;
	char c, *rf, buf[RECLEN], path[RECLEN], mid[RECLEN], ngrp[RECLEN];
	char from[RECLEN], title[RECLEN], flto[RECLEN], pstr[24];
	char dgname[48], getpgch(), *index(), *digest_extract();
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

	hlines = gethead (path, mid, from, title, ngrp, flto, &artlin);
	total = (float) artlin + 1.0;
	if (total < .99)
		total = 1.0;
	lcount = 0.0;
	printf (ANFORM,Fname);
	lines = 1;
	rlines = 0;
	rew_pos = ftell(Fpread);
	if (Headflag)
	{
		rewind(Fpread);
	}
	else
	{
		/* use do_out to guard against control sequences */
		sprintf (buf,"%s%s\n",T_head,title);
		lines += do_out(buf,&optr,1);
		sprintf (buf,"%s%s\n",F_head,from);
		lines += do_out(buf,&optr,1);
		if (index(ngrp,',') != NULL)
		{
			sprintf (buf,"%s%s\n",N_head,ngrp);
			lines += do_out(buf,&optr,1);
		}
		if (*flto != '\0')
		{
			sprintf (buf,"%s%s\n",FT_head,flto);
			lines += do_out(buf,&optr,1);
		}
		printf ("%s%d\n",L_head,artlin);	/* no controls */
		++lines;
	}

	/* will return out of outer while loop */
	optr = NULL;
	while (1)
	{
		/*
		** lines counts folded lines from do_out.
		** hlines, rlines and lcount refer to records.
		*/
		rf = buf;
		lines += do_out(optr,&optr,L_allow-lines);
		while (lines < L_allow && (rf = fgets(buf,RECLEN-1,Fpread)) != NULL)
		{
			lcount += 1.0;

			if (Rot != 0)
			{
				if (!Headflag || rlines >= hlines)
					rot_line(buf);
				++rlines;
			}
			lines += do_out(buf,&optr,L_allow-lines);
		}

		if (rf != NULL)
		{
			if (Headflag)
				percent = 100.0*(lcount/(total+hlines)) + .5;
			else
				percent = 100.0*(lcount/total) + .5;
			sprintf (pstr,PAGE_MID,percent);
		}
		else
		{
			if (sn == NULL)
				strcpy (pstr,PAGE_END);
			else
			strcpy (pstr,PAGE_NEXT);
		}
		c = getpgch(pstr,path,mid,from,title,ngrp,flto);

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
				rewind (Fpread);
			else
				fseek (Fpread,rew_pos,0);
			lcount = 0.0;
			rlines = 0;
			lines = 2 - RECBIAS;
			break;
		case PG_WIND:
			fseek (Fpread,0L,2);
			lines = 2 - RECBIAS;
			break;
		case PG_STEP:
			if (rf == NULL)
			{
				fclose (Fpread);
				return (0);
			}
			lines = L_allow - 1;
			break;
		default:
			if (rf == NULL)
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
	gethead obtains subject, path, message id, from, lines, newsgroup and
	followup-to lines of article for later use in mailing replies and
	posting followups, does not rewind, but leaves file at end of header
	lines.  Returns number of header lines.
*/
static gethead (path, mid, from, title, ngrp, flto, lin)
char *path, *mid, *from, *title, *ngrp, *flto;
int *lin;
{
	int count;
	char buf [RECLEN],*index();
	long pos,ftell();

	*lin = 0;
	*path = *mid = *from = *title = *ngrp = *flto = '\0';

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

		if (strncmp(buf,P_head,PHDLEN) == 0)
		{
			buf [strlen(buf)-1] = '\0';
			strcpy (path,buf+PHDLEN);
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
	return (count);
}

/*
	getpgch prints prompt and gets command from user
	handles "mail", "save" and "followup" internally
	as well as flag resets.
*/
static char getpgch(prompt,path,mid,from,title,ngrp,flto)
char *prompt, *path, *mid, *from, *title, *ngrp, *flto;
{
	char c;
	int ic;
	term_set (ONREVERSE);
	printf("%s\015",prompt);
	term_set (OFFREVERSE);
	while ((ic=getnoctl()) != EOF)
	{
		switch (c = ic)
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
			mail (path,title,from);
			break;
		case PG_FOLLOW:
			followup (mid,title,ngrp,flto);
			break;
		case SAVE:
			saver ();
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
	invoke editor on new temp file, mail using path line,
	first allowing user to overide the path if wished.
*/
static mail (p, t, f)
char *p, *t, *f;
{
	char *new, fn[L_tmpnam], cmd [RECLEN+60], *rprompt ();
	FILE *fp, *fopen();

	tmpnam (fn);
	if ((fp = fopen(fn,"w")) == NULL)
		printex ("can't open %s\n",fn);
	fprintf (fp,"%s%s%s\n\n%s:\n", T_head, FPFIX, t, f);
	edcopy (fp);
	fclose (fp);
	tty_set (SAVEMODE);
	sprintf (cmd,"ADDRESS: %s\nreturn to accept, or input new address: ",p);
	if ((new = rprompt(cmd,cmd)) != NULL)
		strcpy (p,new);
	sprintf (cmd,"%s %s", Editor, fn);
	chdir (Orgdir);
	system (cmd);
	cd_group ();
	new = rprompt ("still want to mail it ? ",cmd);
	if (new != NULL && (*new == 'y' || *new == 'Y'))
	{
		sprintf (cmd,"%s %s <%s", Mailer, p, fn);
		system (cmd);
		printf ("given to mailer\n");
	}
	else
		printf ("not mailed\n");
	unlink (fn);
	tty_set (RESTORE);
}

/*
	post a followup article, invoking editor for user after
	creating new temp file.  remove after posting.  Hack in
	".followup" if posting newsgroup ends in ".general" -
	should really be more thorough and parse the whole string.
	User can change, anyway.
*/
static followup (m,t,n,ft)
char *m, *t, *n, *ft;
{
	char fn[L_tmpnam], *new, cmd [RECLEN], *rprompt();
	FILE *f, *fopen();
	char *rindex();
	tmpnam (fn);
	if ((f = fopen(fn,"w")) == NULL)
		printex ("can't open %s\n",fn);
	if (*ft != '\0')
		strcpy (cmd,ft);
	else
		strcpy (cmd,n);
	if ((new = rindex(cmd,'.')) != NULL && strcmp(new,".general") == 0)
		strcpy (new,".followup");
	fprintf (f,"%s%s%s\n%s%s\n%s%s\n",T_head,FPFIX,t,N_head,cmd,R_head,m);
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
	}
	else
		printf ("not posted\n");
	unlink (fn);
	tty_set (RESTORE);
}

/*
	get user buffer, return whitespace delimited token
	without using strtok().  buffer is allowed to overwrite
	prompt string.
*/
char *rprompt(s,buf)
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
	h_print (PG_NEXT,HPG_NEXT);
	h_print (PG_QUIT,HPG_QUIT);
	h_print (PG_FLIP,HPG_FLIP);
	h_print (PG_REWIND,HPG_REWIND);
	h_print (PG_WIND,HPG_WIND);
	h_print (PG_STEP,HPG_STEP);
	h_print (PG_REPLY,HPG_REPLY);
	h_print (PG_FOLLOW,HPG_FOLLOW);
	h_print (SAVE,HPG_SAVE);
	h_print (SETROT,HPG_ROT);
	h_print (HEADTOG,HPG_HEAD);
	h_print (PG_HELP,HPG_HELP);
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
*/
static do_out(s,rest,lim)
char *s;
char **rest;
int lim;
{
	int len,i,j;
	char cs,*word,*start;

	*rest = NULL;
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
	*rest = start;
	return(lim);
}
