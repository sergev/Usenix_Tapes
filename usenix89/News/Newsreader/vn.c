/*
	vn news reader for visual page oriented display of news
	aimed at scanning large numbers of articles.

	Known bugs:

		non-erasure of stuff on prompt line when the new
		string includes an escape sequence (like PS1 maybe)
		because it doesn't realize that the escape sequence
		won't overprint the existing stuff

		control-w may not update pages which have been scanned
		in funny orders by jumping into the middle of groups

	R. L. McQueer, Author
*/
#include <stdio.h>
#include <setjmp.h>
#include "tty.h"
#include "vn.h"

/* UNIX error number */
extern int errno;

extern NODE *Newsorder [];
extern char Erasekey, Killkey;
extern int Rot;
extern char *Ps1,*Printer;
extern char *Orgdir,*Savefile,*Savedir;
extern int Ncount, Cur_page, Lrec, L_allow, C_allow;
extern int Headflag;
extern PAGE Page;
extern int Digest;
extern char *No_msg;
extern char *Help_msg;
extern char *Hdon_msg;
extern char *Hdoff_msg;
extern char *Roton_msg;
extern char *Rotoff_msg;

extern char *Aformat;

extern char *Contstr;

static int C_info;
static int Dskip, Drec;

static char *Unsub_msg = "Unsubscribed";
static char *Egroup_msg = "Entire newsgroup";

/*
	Help message table.  Character for command, plus its help
	message.  Table order is order of presentation to user.
*/
static struct HELPTAB
{
	char cmd, *msg;
	int dig;
	char *amsg;
} 
Helptab [] =
{
	{ QUIT, "quit", 1, NULL},
	{ UP, "move up [number of lines]", 1, NULL},
	{ DOWN, "move down [number of lines]", 1, NULL},
	{ BACK, "previous page [number of pages]", 1, NULL},
	{ FORWARD, "next page [number of pages]", 1, NULL},
	{ DIGEST, "unpack digest", 1, "exit digest"},
	{ READ, "read article [number of articles]", 1, NULL},
	{ ALTREAD, "read article (alternate 'r')", 1, NULL},
	{ READALL, "read all articles on page", 1, NULL},
	{ READSTRING, "specify articles to read", 1, NULL},
	{ SAVE, "save or pipe article [number of articles]", 1, NULL},
	{ SAVEALL, "save or pipe all articles on page", 1, NULL},
	{ SAVESTRING, "specify articles to save", 1, NULL},
	{ ALTSAVE, "specify articles to save (alternate ctl-s)", 1, NULL},
	{ PRINT, "print article [number of articles]", 1, NULL},
	{ PRINTALL, "print all article on page", 1, NULL},
	{ PRINTSTRING, "specify articles to print", 1, NULL},
	{ UPDATE, "update .newsrc status to cursor", 0, NULL},
	{ UPALL, "update .newsrc status for whole newsgroup", 0, NULL},
	{ UPSEEN, "update .newsrc status for all pages displayed", 0, NULL},
	{ ORGGRP, "recover original .newsrc status for newsgroup", 0, NULL},
	{ ORGSTAT, "recover all original .newsrc status", 0, NULL},
	{ SSTAT, "display count of groups and pages - shown and total", 0, NULL},
	{ GRPLIST, "list newsgroups with new article, updated counts", 0, NULL},
	{ NEWGROUP, "specify newsgroup to display and/or resubscribe to", 1, NULL},
	{ UNSUBSCRIBE, "unsubscribe from group", 0, NULL},
	{ MARK, "mark/unmark article [number of articles]", 1, NULL},
	{ ART_MARK, "mark/unmark article [number of articles]", 1, NULL},
	{ UNMARK, "erase marks on articles", 1, NULL},
	{ HEADTOG, "toggle flag for display of headers when reading", 1, NULL},
	{ SETROT, "toggle rotation for reading", 1, NULL},
	{ REDRAW, "redraw screen", 1, NULL},
	{ UNESC, "escape to UNIX to execute a command", 1, NULL},
	{ HELP, "show this help menu", 1, NULL}
};

main(argc,argv)
int argc;
char **argv;
{
	/*
		initialize environment variables,
		scan .newsrc file, using any command line options present.
	 */
	term_set (START);
	envir_set ();
	sig_set (BRK_IN);

	scan_newsrc (argc-1,argv+1);
	tty_set (BACKSTOP);

	if (Lrec >= 0)
		session ();
	else
	{
		new_groups ();
		fprintf (stderr,"\nNo News\n");
	}

	tty_set (COOKED);
	wr_newsrc ();
	term_set (STOP);
}

/*
	main session handler processing input commands
	locals:
		count - count attached to command
		highrec - highest line on current page
		crec - current line

	NOTE: this is where a setjmp call is made to set the break reentry
		location.  Keep the possible user states in mind.
*/
session ()
{
	char alist [RECLEN], c;
	int newg, i, j, count, highrec, crec;
	jmp_buf brkbuf;

	tty_set (RAWMODE);
	newg = new_groups();
	find_page (0);
	Digest = 0;

	/* reentry point for break from within session interaction */
	setjmp (brkbuf);
	sig_set (BRK_SESS,brkbuf);
	Headflag = FALSE;
	Rot = 0;

	/* done this way so that user gets "really quit?" break treatment */
	if (newg > 0)
	{
		printf ("\n%s",Contstr);
		getnoctl();
		newg = 0;
	}
	/* if breaking from a digest, recover original page */
	if (Digest)
	{
		find_page(Cur_page);
		Digest = 0;
	}
	show ();
	crec = RECBIAS;
	highrec = Page.h.artnum + RECBIAS;
	term_set (MOVE,0,crec);

	/*
		handle commands until QUIT, update global/local status
		and display for each.
	 */
	for (count = getkey(&c); c != QUIT; count = getkey(&c))
	{
		for (i=0; i < (j = sizeof(Helptab)/sizeof(struct HELPTAB)); ++i)
			if (Helptab[i].cmd == c)
				break;

		if (i >= j || (Digest && !Helptab[i].dig))
		{
			preinfo (UDKFORM,(int) c);
			term_set (MOVE, 0, crec);
			continue;
		}

		switch (c)
		{
		case HEADTOG:
			if (Headflag)
			{
				Headflag = FALSE;
				prinfo (Hdoff_msg);
			}
			else
			{
				Headflag = TRUE;
				prinfo (Hdon_msg);
			}
			term_set (MOVE,0,crec);
			break;
		case SETROT:
			if (Rot == 0)
			{
				Rot = 13;
				prinfo (Roton_msg);
			}
			else
			{
				Rot = 0;
				prinfo (Rotoff_msg);
			}
			term_set (MOVE,0,crec);
			break;
		case SSTAT:
			count_msg ();
			term_set (MOVE,0,crec);
			break;
		case GRPLIST:
			tot_list ();
			show();
			term_set (MOVE,0,crec);
			break;
		case REDRAW:
			show();
			term_set (MOVE,0,crec);
			break;
		case UNSUBSCRIBE:
			(Page.h.group)->state &= ~FLG_SUB;
			wr_newsrc ();
			prinfo (Unsub_msg);
			term_set (MOVE,0,crec);
			break;

		case UPDATE:
			(Page.h.group)->rdnum = Page.b[crec-RECBIAS].art_id;
			wr_show ();
			wr_newsrc();
			term_set (MOVE,0,crec);
			break;
		case UPALL:
			(Page.h.group)->rdnum = (Page.h.group)->art;
			wr_newsrc();
			wr_show();
			prinfo (Egroup_msg);
			term_set (MOVE,0,crec);
			break;
		case ORGGRP:
			(Page.h.group)->rdnum = (Page.h.group)->orgrd;
			wr_newsrc();
			wr_show();
			prinfo (Egroup_msg);
			term_set (MOVE,0,crec);
			break;
		case UPSEEN:
			for (i = 0; i < Ncount; ++i)
				if ((Newsorder[i])->rdnum < (Newsorder[i])->pgrd)
					(Newsorder[i])->rdnum = (Newsorder[i])->pgrd;
			prinfo ("All pages displayed to this point updated");
			wr_show();
			wr_newsrc();
			term_set (MOVE,0,crec);
			break;
		case ORGSTAT:
			for (i = 0; i < Ncount; ++i)
				(Newsorder[i])->rdnum = (Newsorder[i])->orgrd;
			prinfo ("Original data recovered");
			wr_show();
			wr_newsrc();
			term_set (MOVE,0,crec);
			break;
		case UP:
			if (crec != RECBIAS)
			{
				crec -= count;
				if (crec < RECBIAS)
					crec = RECBIAS;
				term_set (MOVE, 0, crec);
			}
			else
				putchar ('\07');
			break;
		case DOWN:
			if (crec < (highrec - 1))
			{
				crec += count;
				if (crec >= highrec)
					crec = highrec - 1;
				term_set (MOVE, 0, crec);
			}
			else
				putchar ('\07');
			break;
		case MARK:
		case ART_MARK:
			count += crec - 1;
			if (count >= highrec)
				count = highrec - 1;
			for (i=crec; i <= count; ++i)
			{
				if (Page.b[i-RECBIAS].art_mark != ART_MARK)
					Page.b[i-RECBIAS].art_mark = ART_MARK;
				else
					Page.b[i-RECBIAS].art_mark = ' ';
				if (i != crec)
					term_set (MOVE, 0, i);
				printf ("%c\010",Page.b[i-RECBIAS].art_mark);
			}
			if (count != crec)
				term_set (MOVE, 0, crec);
			write_page ();
			break;
		case UNMARK:
			for (i=0; i < Page.h.artnum; ++i)
			{
				if (Page.b[i].art_mark == ART_MARK)
				{
					Page.b[i].art_mark = ' ';
					term_set (MOVE, 0, i+RECBIAS);
					putchar (' ');
				}
			}
			term_set (MOVE, 0, crec);
			write_page ();
			break;
		case BACK:
			count *= -1;	/* fall through */
		case FORWARD:
			if (forward (count, &crec, &highrec) >= 0)
				show();
			else
				preinfo ("No more pages");
			term_set (MOVE,0,crec);
			break;
		case DIGEST:
			if (Digest)
			{
				Digest = 0;
				find_page (Cur_page);
				show();
				crec = Drec + RECBIAS + 1;
				highrec = Page.h.artnum + RECBIAS;
				if (crec >= highrec)
					crec = highrec - 1;
				term_set (MOVE,0,crec);
				break;
			}
			Dskip = count - 1;
			Drec = crec - RECBIAS;
			if (digest_page(Drec,Dskip) >= 0)
			{
				show();
				crec = RECBIAS;
				highrec = Page.h.artnum + RECBIAS;
				term_set (MOVE,0,crec);
				break;
			}
			Digest = 0;
			preinfo ("Can't unpack the article");
			term_set (MOVE,0,crec);
			break;
		case NEWGROUP:
			if ((i = spec_group()) < 0)
			{
				term_set (MOVE,0,crec);
				break;
			}
			Digest = 0;
			show();
			crec = RECBIAS;
			highrec = Page.h.artnum + RECBIAS;
			term_set (MOVE,0,crec);
			break;

		case SAVE:
			genlist (alist,crec-RECBIAS,count);
			savestr (alist);
			term_set (MOVE,0,crec);
			break;
		case SAVEALL:
			genlist (alist,0,L_allow);
			savestr (alist);
			term_set (MOVE,0,crec);
			break;
		case SAVESTRING:
		case ALTSAVE:
			userlist (alist);
			savestr (alist);
			term_set (MOVE,0,crec);
			break;
		case READ:
		case ALTREAD:
			genlist (alist,crec-RECBIAS,count);
			readstr (alist,&crec,&highrec,count);
			break;
		case READALL:
			genlist (alist,0,L_allow);
			readstr (alist,&crec,&highrec,0);
			break;
		case READSTRING:
			userlist (alist);
			readstr (alist,&crec,&highrec,0);
			break;
		case PRINT:
			genlist (alist,crec-RECBIAS,count);
			printstr (alist);
			term_set (MOVE,0,crec);
			break;
		case PRINTALL:
			genlist (alist,0,L_allow);
			printstr (alist);
			term_set (MOVE, 0, crec);
			break;
		case PRINTSTRING:
			userlist (alist);
			printstr (alist);
			term_set (MOVE, 0, crec);
			break;

		case HELP:
			help ();
			show ();
			term_set (MOVE, 0, crec);
			break;
		case UNESC:
			user_str (alist,Ps1);
			term_set (ERASE);
			fflush (stdout);
			tty_set (SAVEMODE);
			if (chdir(Orgdir) < 0)
				printf ("change to original directory, %s, failed",Orgdir);
			else
				system (alist);
			tty_set (RESTORE);
			printf (Contstr);
			getnoctl ();
			cd_group ();
			show ();
			term_set (MOVE, 0, crec);
			break;
		default:
			printex ("Unhandled key: %c", c);
			break;
		}
	}

	Digest = 0;
	for (i=0; i < Ncount; ++i)
	{
		if ((Newsorder[i])->rdnum < (Newsorder[i])->pgrd)
			break;
	}
	if (i < Ncount)
	{
		user_str (alist,"Some displayed pages not updated - update ? ");
		if (alist[0] == 'y')
		{
			for (i=0; i<Ncount; ++i)
			{
				if ((Newsorder[i])->rdnum < (Newsorder[i])->pgrd)
					(Newsorder[i])->rdnum = (Newsorder[i])->pgrd;
			}
		}
	}
	sig_set (BRK_OUT);
}

/*
	count_msg displays count information
*/
count_msg ()
{
	int i, gpnum, gscan, gpage;
	unsigned long mask;
	gpnum = 1;
	for (gscan = gpage = i = 0; i<Ncount; ++i)
	{
		if (((Newsorder[i])->state & FLG_PAGE) != 0)
		{
			if (((Newsorder[i])->pnum + (Newsorder[i])->pages - 1) < Cur_page)
				++gpnum;
			++gpage;
			for (mask=1; mask != 0L; mask <<= 1)
				if (((Newsorder[i])->pgshwn & mask) != 0L)
					++gscan;
		}
	}
	prinfo (CFORMAT,Cur_page+1,Lrec+1,gscan,gpnum,gpage);
}

/*
	forward utility handles paging to allow it to happen globally.
	(from readstr, for instance)
*/
forward (count, crec, highrec)
int count, *crec, *highrec;
{
	if (!Digest)
	{
		if ((count < 0 && Cur_page <= 0) || (count > 0 && Cur_page >= Lrec))
			return (-1);
		Cur_page += count;
		if (Cur_page < 0)
			Cur_page = 0;
		if (Cur_page > Lrec)
			Cur_page = Lrec;
		find_page (Cur_page);
		*crec = RECBIAS;
		*highrec = Page.h.artnum + RECBIAS;
		return (0);
	}
	/*
	** in digests, paging past the end of the digest returns to
	** page extracted from.
	*/
	if (Dskip > 0 && (Dskip + count*L_allow) < 0)
		Dskip = 0;
	else
		Dskip += count * L_allow;
	find_page (Cur_page);
	if (Dskip >= 0)
	{
		if (digest_page(Drec,Dskip) >= 0)
		{
			*crec = RECBIAS;
			*highrec = Page.h.artnum + RECBIAS;
			return (0);
		}
	}
	Digest = 0;
	*crec = Drec + RECBIAS + 1;
	*highrec = Page.h.artnum + RECBIAS;
	if (*crec >= *highrec)
		*crec = *highrec - 1;
	return (0);
}

/*
	error/abnormal condition cleanup and abort routine
	pass stack to printf
*/
printex (s,a,b,c,d,e,f)
char *s;
long a,b,c,d,e,f;
{
	static int topflag=0;
	if (topflag == 0)
	{
		++topflag;
		term_set (STOP);
		tty_set (COOKED);
		fflush (stdout);
		fprintf (stderr,s,a,b,c,d,e,f);
		fprintf (stderr," (error code %d)\n",errno);
		exit (1);
	}
	else
		fprintf (stderr,s,a,b,c,d,e,f);
}

/*
	getkey obtains user keystroke with count from leading
	numerics, if any.
*/
getkey (c)
char *c;
{
	int i;
	for (i = 0; (*c = getchar() & 0x7f) >= '0' && *c <= '9'; i = i * 10 + *c - '0')
		;
	/* @#$!!! flakey front ends that won't map newlines in raw mode */
	if (*c == '\012' || *c == '\015')
		*c = '\n';
	if (i <= 0)
		i = 1;
	return (i);
}


/*
	get user key ignoring most controls
*/
getnoctl ()
{
	char c;
	while ((c = getchar() & 0x7f) < ' ' || c == '\177')
	{
		if (c == '\015' || c == '\012' || c == '\b' || c == '\t')
			return (c);
	}
	return ((int) c);
}

/*
	generate list of articles on current page,
	count articles, starting with first.
*/
genlist (list,first,count)
char *list;
int first,count;
{
	int i;
	for (i=first; i < Page.h.artnum && count > 0; ++i)
	{
		sprintf (list,"%d ",Page.b[i].art_id);
		list += strlen(list);
		--count;
	}
}

/*
	send list of articles to printer
*/
printstr (s)
char *s;
{
	char *ptr, cmd [RECLEN], *strpbrk();
	prinfo ("preparing print command ....");
	for (ptr = s; (ptr = strpbrk(ptr, LIST_SEP)) != NULL; ++ptr)
		*ptr = ' ';
	while (*s == ' ')
		++s;
	if (Digest)
		dig_list (s);
	if (*s != '\0')
	{
		sprintf (cmd,"%s %s 2>/dev/null",Printer,s);
		if (system (cmd) == 0)
			prinfo ("Sent to printer");
		else
			preinfo ("Print failed");
	}
	else
		preinfo (No_msg);
	if (Digest)
		dig_ulist (s);
}

/*
	concatenate articles to save file with appropriate infoline messages.
	prompt for save file, giving default.  If save file begins with "|"
	handle as a filter to pipe to.  NOTE - every user specification of
	a new Savefile "loses" some storage, but it shouldn't be a very great
	amount.
*/
savestr (s)
char *s;
{
	char *ptr, cmd [RECLEN], newfile [MAX_C+1], prompt[MAX_C];
	char *strtok(), *strpbrk(), *str_store();

	for (ptr = s; (ptr = strpbrk(ptr, LIST_SEP)) != NULL; ++ptr)
		*ptr = ' ';
	while (*s == ' ')
		++s;
	if (Digest)
		dig_list (s);
	if (*s != '\0')
	{
		sprintf (prompt,SAVFORM,Savefile);
		user_str (newfile,prompt);
		ptr = newfile;
		if (*ptr == '|')
		{
			sprintf(cmd,"cat %s %s",s,ptr);
			term_set (ERASE);
			fflush (stdout);
			tty_set (SAVEMODE);
			system (cmd);
			tty_set (RESTORE);
			printf (Contstr);
			getnoctl ();
			show ();
		}
		else
		{
			prinfo ("saving .... ");
			if (*ptr == '\0')
				ptr = Savefile;
			else
				Savefile = str_store(ptr);
			if (*ptr != '/' && *ptr != '$')
				sprintf(cmd,"cat %s >>%s/%s 2>/dev/null",s,Savedir,ptr);
			else
				sprintf(cmd,"cat %s >>%s 2>/dev/null",s,ptr);
			if (system (cmd) == 0)
				prinfo ("Saved");
			else
				preinfo ("Could not append save file");
		}
	}
	else
		preinfo (No_msg);
	if (Digest)
		dig_ulist (s);
}

/*
	basic page display routine.  erase screen and format current page
*/
show ()
{
	int i;
	unsigned long mask;

	term_set (ERASE);
	C_info = 0;
	i = Cur_page - (Page.h.group)->pnum + 1;
	if (Digest)
		printf (DHFORMAT,Page.h.name);
	else
		printf (HFORMAT,Page.h.name,i,(Page.h.group)->pages);

	mask = 1L << (i-1);
	(Page.h.group)->pgshwn |= mask;
	mask = 1;
	for (--i; i > 0 && (mask & (Page.h.group)->pgshwn) != 0 ; --i)
		mask <<= 1;
	if (i <= 0)
		(Page.h.group)->pgrd = Page.b[(Page.h.artnum)-1].art_id;

	for (i=0; i < Page.h.artnum; ++i)
	{
		if (Digest)
		{
			printf(Aformat,Page.b[i].art_mark,ART_UNWRITTEN,Page.b[i].art_id);
			printf("%s",Page.b[i].art_t);
			continue;
		}

		if ((Page.h.group)->rdnum >= Page.b[i].art_id)
			printf(Aformat,Page.b[i].art_mark,ART_WRITTEN,Page.b[i].art_id);
		else
			printf(Aformat,Page.b[i].art_mark,ART_UNWRITTEN,Page.b[i].art_id);
		printf("%s",Page.b[i].art_t);
	}

	if (!Digest && ((Page.h.group)->state & FLG_SUB) == 0)
		prinfo ("%s, %s",Unsub_msg,Help_msg);
	else
		prinfo (Help_msg);
}

/*
	update written status marks on screen
*/
wr_show ()
{
	int i,row;
	char c;

	row = RECBIAS;
	for (i=0; i < Page.h.artnum; ++i)
	{
		term_set (MOVE,WRCOL,row);
		if ((Page.h.group)->rdnum >= Page.b[i].art_id)
			c = ART_WRITTEN;
		else
			c = ART_UNWRITTEN;
		printf("%c",c);
		++row;
	}
}

/*
	obtain user input of group name, becomes current page if valid.
	returns -1 or page number.  calling routine does the show, if needed
*/
spec_group ()
{
	char nbuf [MAX_C + 1];
	NODE *p, *hashfind();
	user_str (nbuf,"Newsgroup ? ");
	if (*nbuf == '\0' || (p = hashfind(nbuf)) == NULL)
	{
		preinfo ("Not a newsgroup");
		return (-1);
	}
	if ((p->state & FLG_PAGE) == 0)
	{
		if ((p->state & FLG_SUB) == 0)
		{
			p->state |= FLG_SUB;
			wr_newsrc ();
			prinfo ("Not subscribed: resubscribed for next reading session");
		}
		else
			prinfo ("No news for that group");
		return (-1);
	}
	if ((p->state & FLG_SUB) == 0)
	{
		p->state |= FLG_SUB;
		wr_newsrc ();
	}
	find_page (p->pnum);
	return (p->pnum);
}

/*
	obtain user input with prompt p on the info line.
	handle erase and kill characters, suppresses leading
	white space.
*/
user_str (s,p)
char *s;
char *p;
{
	int i;
	prinfo ("%s",p);
	for (i=0; C_info < C_allow && (s[i] = getchar() & 0x7f) != '\012' && s[i] != '\015'; ++i)
	{
		if (s[i] == Erasekey)
		{
			if (i > 0)
			{
				term_set (RUBSEQ);
				i -= 2;
				--C_info;
			}
			continue;
		}
		if (s[i] == Killkey)
		{
			prinfo ("%s",p);
			i = -1;
			continue;
		}
		if ((s[i] == ' ' || s[i] == '\t') && i == 0)
		{
			i = -1;
			continue;
		}
		++C_info;
		putchar (s[i]);
	}
	s[i] = '\0';
}


/*
	print something on the information line,
	clearing any characters not overprinted.
*/
preinfo (s,a,b,c,d,e,f)
{
	int l;
	char buf[RECLEN];

	term_set (MOVE,0,INFOLINE);
	putchar ('\07');
	term_set (ONREVERSE);
	sprintf (buf,s,a,b,c,d,e,f);
	printf (" %s ",buf);
	term_set (OFFREVERSE);
	l = strlen(buf) + 2;
	if (l < C_info)
		term_set (ZAP,l,C_info);
	C_info = l;
}

prinfo (s,a,b,c,d,e,f)
char *s;
long a,b,c,d,e,f;
{
	int l;
	char buf[RECLEN];
	term_set (MOVE,0,INFOLINE);
	sprintf (buf,s,a,b,c,d,e,f);
	printf ("%s",buf);
	l = strlen(buf);
	if (l < C_info)
		term_set (ZAP,l,C_info);
	C_info = l;
}

/*
	help menu
*/
help ()
{
	int i,lcount,lim; 
	term_set (ERASE);
	lim = L_allow + RECBIAS - 2;
	printf("%s\n",HELP_HEAD);
	lcount = HHLINES;
	for (i=0; i < (sizeof(Helptab))/(sizeof(struct HELPTAB)); ++i)
	{
		if (Digest && !(Helptab[i].dig))
			continue;
		++lcount;
		if (Digest && Helptab[i].amsg != NULL)
			h_print (Helptab[i].cmd,Helptab[i].amsg);
		else
			h_print (Helptab[i].cmd,Helptab[i].msg);
		if (lcount >= lim)
		{
			printf ("\n%s",Contstr);
			getnoctl ();
			term_set (MOVE,0,lim+1);
			term_set (ZAP,0,strlen(Contstr));
			term_set (MOVE,0,lim-1);
			putchar ('\n');
			lcount = 0;
		}
	}
	if (lcount > 0)
	{
		printf ("\n%s",Contstr);
		getnoctl ();
	}
}

/*
	h_print prints a character and a legend for a help menu.
*/
h_print(c,s)
char c,*s;
{
	if (strlen(s) > (C_allow - 14))
		s [C_allow - 14] = '\0';
	if (c > ' ' && c != '\177')
		printf ("	 %c - %s\n",c,s);
	else
	{
		switch (c)
		{
		case '\177':
			printf ("  <delete> - %s\n",s);  
			break;
		case '\040':
			printf ("   <space> - %s\n",s);  
			break;
		case '\033':
			printf ("  <escape> - %s\n",s);  
			break;
		case '\n':
			printf ("  <return> - %s\n",s);  
			break;
		case '\t':
			printf ("     <tab> - %s\n",s);  
			break;
		case '\b':
			printf (" <back sp> - %s\n",s);  
			break;
		case '\f':
			printf ("<formfeed> - %s\n",s);  
			break;
		case '\07':
			printf ("    <bell> - %s\n",s);  
			break;
		case '\0':
			printf ("    <null> - %s\n",s);  
			break;
		default:
			if (c < '\033')
			{
				c += 'a' - 1;
				printf(" control-%c - %s\n",c,s);
			}
			else
				printf("       %c0%o - %s\n",'\\',(int) c,s);
			break;
		}
	}
}

tot_list ()
{
	int i,max,len;
	char ff[80];

	term_set (ERASE);

	for (max=i=0; i < Ncount; ++i)
	{
		if ((Newsorder[i])->pages == 0)
			continue;
		if ((len = strlen((Newsorder[i])->nd_name)) > max)
			max = len;
	}

	sprintf (ff,"%%%ds: %%3d new %%3d updated\n",max);

	for (len=i=0; i < Ncount; ++i)
	{
		if ((Newsorder[i])->pages == 0)
			continue;
		printf (ff, (Newsorder[i])->nd_name,
				(Newsorder[i])->art - (Newsorder[i])->orgrd,
				(Newsorder[i])->rdnum - (Newsorder[i])->orgrd);
		++len;
		if (len == L_allow && i < (Ncount-1))
		{
			printf("\nq to quit, anything else to continue ... ");
			if (getnoctl() == 'q')
				break;
			printf ("\n\n");
			len = 0;
		}
	}
	if (i >= Ncount)
	{
		printf(Contstr);
		getnoctl();
	}
}
