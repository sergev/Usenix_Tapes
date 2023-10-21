#include <stdio.h>
#include <ctype.h>
#include "config.h"
#include "tty.h"
#include "vn.h"

extern NODE *Newsorder [];
extern FILTER Filter [];
extern char *Onews, *Newsrc;
extern int Nfltr, Ncount, Lrec, C_allow;

/*
	global flags signifying options set
*/
#define GF_ALL 1	/* -x option - scan everything */
#define GF_SPEC 2	/* -n option(s) - user specified groups */
#define GF_OVER 4	/* command line specification - overide marks */

static char *Options[OPTLINES];
static int New_idx, Max_name, Optlines;
static unsigned Gflags = 0;

/*
	routines for dealing with the .newsrc file and options
*/

/*
	command name argument is already omitted from argv argc in this
	routine.  Only the option arguments are present.  We process
	options before we scan the rest of .newsrc, which redoes Newsorder,
	ie. we don't clobber Ncount until options are processed.
*/
scan_newsrc (argc,argv)
int argc;
char **argv;
{
	FILE *fp, *fopen();
	static char marks[] =
	{ 
		NEWS_ON, NEWS_OFF, '\0' 	};
	char *str_store ();
	int line, len, num;
	char buf [RECLEN], trail, optpflag, submark, *fret, *ptr, *strpbrk(), *strtok();


	/* initialize hash table, open temp file, fill table with active articles */
	hashinit ();
	fill_active ();
	temp_open();

	if (argc > 0)
	{
		Gflags |= GF_OVER;
		arg_opt(argc,argv);
		optpflag = 'y';
	}
	else
		optpflag = 'n';

	if ((fp = fopen (Newsrc,"r")) == NULL)
		printex ("can't open %s for reading",Newsrc);

	Optlines = 0;

	for (line = 1; (fret = fgets(buf,RECLEN-1,fp)) != NULL && emptyline(buf) == 1; ++line)
		;
	if (fret != NULL && strncmp (buf,"options",7) == 0)
	{
		Options[0] = str_store(buf);
		Optlines = 1;
		trail = buf [strlen(buf)-2];
		for ( ; (fret = fgets(buf,RECLEN-1,fp)) != NULL; ++line)
		{
			if (trail != '\\' && buf[0] != ' ' && buf[0] != '\t')
				break;
			if (Optlines >= OPTLINES)
				printex ("%s - too many option lines (%d allowed)",Newsrc,OPTLINES);
			Options[Optlines] = str_store(buf);
			++Optlines;
			if ((len = strlen(buf)) >= 2 && buf[len-2] != '\\')
				trail = buf[len-2];
			else
				trail = '\0';
		}
	}

	/* do the options from the newsrc file if there weren't command line args */
	if (Optlines > 0 && optpflag == 'n')
		newsrc_opt ();

	Ncount = 0;
	for ( ; fret != NULL; ++line, fret = fgets(buf,RECLEN-1,fp))
	{
		if (emptyline(buf) == 1)
			continue;
		if ((ptr = strpbrk(buf,marks)) == NULL)
		{
			fprintf (stderr,"\nwarning: line %d of %s (%s) - bad syntax\n",
			line,Newsrc,buf);
			continue;
		}
		submark = *ptr;
		*ptr = '\0';
		++ptr;
		num = 0;
		for (ptr = strtok(ptr," ,-\n"); ptr != NULL; ptr = strtok(NULL," ,-\n"))
		{
			len = atoi (ptr);
			for ( ; *ptr >= '0' && *ptr <= '9'; ++ptr)
				;
			if (*ptr != '\0' || len < num)
			{
				num = -1;
				fprintf (stderr,"\nwarning: line %d of %s (%s) - bad syntax\n",
				line,Newsrc,buf);
				break;
			}
			num = len;
		}
		if (num < 0)
			continue;
		chkgroup (buf,submark,num);
	}
	fclose (fp);

	/* now take care of groups not specified in .newsrc */
	art_active();

	/* free up the filter string storage */
	for (num=0; num < Nfltr; ++num)
		regfree (Filter[num].rex);
	Nfltr = 0;
}

static emptyline(s)
char *s;
{
	while (isspace(*s))
		++s;
	if (*s == '\0')
		return (1);
	return (0);
}

/*
	fill hash table from active news group list
	temporarily makes "Newsorder" active list order.
	This is needed to be able to process options
	before scanning user order.
*/
static fill_active ()
{
	FILE *f,*fopen ();
	char *nread, act_rec[RECLEN], *strtok();
	int num,lownum;
	NODE *hashenter();
	Ncount = Max_name = 0;
	if ((f = fopen (ACTFILE,"r")) == NULL)
		printex ("couldn't open %s\n",ACTFILE);
	while (fgets(act_rec, RECLEN-1, f) != NULL)
	{
		if (strtok (act_rec," \n") == NULL)
			continue;
		nread = strtok (NULL, " \n");
		if (nread != NULL)
			num = atoi(nread);
		else
			num = 0;
		nread = strtok (NULL, " \n");
		if (nread != NULL)
			lownum = atoi(nread);
		else
			lownum = 0;
		if (strlen(act_rec) > Max_name)
			Max_name = strlen(act_rec);
		Newsorder[Ncount] = hashenter (act_rec, num, lownum);
		if (Newsorder[Ncount] == NULL)
			break;
		++Ncount;
	}
	fclose (f);
}

/*
	check active newsgroups not mentioned in NEWSRC file
	(FLG_SCAN not set)
*/
static art_active ()
{
	char act_rec[RECLEN], *strtok();
	NODE *ptr,*hashfind();
	FILE *f,*fopen();
	if ((f = fopen (ACTFILE,"r")) == NULL)
		printex ("couldn't open %s\n",ACTFILE);
	New_idx = Ncount;
	while (fgets(act_rec, RECLEN-1, f) != NULL)
	{
		if (strtok (act_rec," \n") == NULL)
			continue;
		if ((ptr = hashfind (act_rec)) == NULL)
		{
			fgprintf ("%s - unexpected hash table failure\n",act_rec);
			continue;
		}
		if ((ptr->state & FLG_SCAN) == 0)
			chkgroup (ptr->nd_name, NEWS_ON, 0);
	}
}

/*
	check group for new articles:
	s - group
	c - subscription indicator from NEWSRC
	n - number read
*/
static chkgroup (s,c,n)
char *s,c;
int n;
{
	NODE *ptr, *hashfind();
	int lold,lowart;
	lold = Lrec;
	if ((ptr = hashfind(s)) != NULL && (ptr->state & FLG_SCAN) == 0)
	{
		Newsorder [Ncount] = ptr;
		++Ncount;
		ptr->pages = 0;
		ptr->state |= FLG_SCAN;
		if (c == NEWS_ON)
			ptr->state |= FLG_SUB;
		/* if "read" more than exist reset to zero */
		if (n > ptr->art)
			n = 0;
		lowart = ptr->rdnum;
		if (n < ptr->rdnum)
			n = ptr->rdnum;
		ptr->orgrd = ptr->pgrd = ptr->rdnum = n;
		ptr->pgshwn = 0L;

		/*
			scan directory decision is rather complex, since GF_ALL setting
			overides "n" value, and GF_SPEC indicates FLG_SPEC flag to be used.
			if GF_OVER set, FLG_SPEC overides subscription mark, otherwise
			FLG_SPEC AND subscribed is neccesary.
		    */
		if ((Gflags & GF_SPEC) != 0)
		{
			if ((ptr->state & FLG_SPEC) == 0)
				c = NEWS_OFF;
			else
			{
				if ((Gflags & GF_OVER) != 0)
					c = NEWS_ON;
			}
		}
		if ((Gflags & GF_ALL) != 0)
			n = lowart;
		if (c == NEWS_ON && ptr->art > n)
		{
			outgroup (s,n,ptr->art);
			if (lold != Lrec)
			{
				ptr->pnum = lold+1;
				ptr->pages = Lrec - lold;
				ptr->state |= FLG_PAGE;
			}
		}
	}
}

/*
	wr_newsrc writes the .newsrc file
*/
wr_newsrc ()
{
	FILE *fp,*fopen();
	NODE *p;
	char c;
	int i,rc;

	if (link(Newsrc,Onews) < 0)
		printex ("can't backup %s to %s before writing",Newsrc,Onews);

	if (unlink(Newsrc) < 0 || (fp = fopen(Newsrc,"w")) == NULL)
		printex ("can't open %s for writing (backed up in %s)",Newsrc,Onews);
	else
	{
		clearerr(fp);
		for (i=0; (rc = ferror(fp)) == 0 && i < Optlines; ++i)
			fprintf (fp,"%s",Options[i]);
		for (i=0; rc == 0 && i < Ncount; ++i)
		{
			p = Newsorder[i];
			if ((p->state & FLG_SUB) == 0)
				c = NEWS_OFF;
			else
				c = NEWS_ON;
			if (p->rdnum > 0)
				fprintf (fp,"%s%c 1-%d\n",p->nd_name,c,p->rdnum);
			else
				fprintf (fp,"%s%c 0\n",p->nd_name,c);
			rc = ferror(fp);
		}
		fclose (fp);
		if (rc != 0)
			printex ("write of %s failed, old copy stored in %s",Newsrc,Onews);
		else
			unlink (Onews);
	}
}

new_groups ()
{
	int i,wrem,w;
	char fs[24],c_end;
	if (New_idx >= Ncount || C_allow < (w = Max_name+1))
		return (0);
	term_set (ERASE);
	printf (NEWGFORM,Newsrc);
	sprintf (fs,"%%-%ds%%c",Max_name);
	wrem = C_allow;
	for (i=New_idx; i < Ncount; ++i)
	{
		if ((wrem -= w) < w)
		{
			wrem = C_allow;
			c_end = '\n';
		}
		else
			c_end = ' ';
		printf (fs,(Newsorder[i])->nd_name,c_end);
	}
	if ((++wrem) < C_allow)
		putchar ('\n');
	return (i-New_idx);
}

/*
	arg_opt must be called prior to option scanning, since
	it uses the options array.  This is a bit of a kludge,
	but it saves a bunch of work.  NOTE - no command name argument
*/
static arg_opt (argc,argv)
int argc;
char **argv;
{
	if (argc > OPTLINES)
		printex ("too many command line options (%d allowed)\n",OPTLINES);
	for (Optlines=0; Optlines < argc; ++Optlines)
	{
		Options[Optlines] = *argv;
		++argv;
	}
	newsrc_opt();
}

/*
	option setting routine:
	sets global flags: GF_ALL for -x option GF_SPEC for -n.
	sets up filter array for article scanning
*/
static newsrc_opt()
{
	int i;
	char curopt,tmp[RECLEN],*tok,*strtok(),*index();

	Nfltr = 0;
	curopt = '\0';
	for (i=0; i < Optlines; ++i)
	{
		strcpy(tmp,Options[i]);
		for (tok = strtok(tmp,",\\ \t\n"); tok != NULL; tok = strtok(NULL,",\\ \t\n"))
		{
			if (*tok != '-')
				do_opt (curopt,tok);
			else
			{
				for (++tok; *tok != '\0' && index(NARGOPT,*tok) != NULL; ++tok)
				{
					if (*tok == 'x')
						Gflags |= GF_ALL;
				}
				curopt = *tok;
				if (*(++tok) != '\0')
					do_opt (curopt,tok);
			}
		}
	}
}

static do_opt (opt,str)
char opt, *str;
{
	switch (opt)
	{
	case 'S':
		Gflags &= ~GF_OVER;
		break;
	case 'n':
		Gflags |= GF_SPEC;
		specmark(str);
		break;
	case 'w':
		specfilter (FIL_AUTHOR,str);
		break;
	case 't':
		specfilter (FIL_TITLE,str);
		break;
	default:
		break;
	}
}

static specfilter (comp,str)
char comp,*str;
{
	char *reg,neg,*regcmp();
	if (Nfltr >= NUMFILTER)
		printex ("%s: too many a,w option strings (%d allowed)",str,NUMFILTER);
	if ((neg = *str) == '!')
		++str;
	if ((reg = regcmp(str,(char *) 0)) == NULL)
		printex ("a,w option regular expression syntax: %s",str);
	Filter[Nfltr].rex = reg;
	Filter[Nfltr].neg = neg;
	Filter[Nfltr].hcomp = comp;
	++Nfltr;
}

/*
	handle the newsgroup specification string.
	("all" convention - braack!!!)
*/
static specmark (s)
char *s;
{
	unsigned ormask,andmask;
	int i,len;
	char *ptr,*re,pattern[RECLEN],*regex(),*regcmp();

	if (*s == '!')
	{
		++s;
		ormask = 0;
		andmask = ~FLG_SPEC;
		if (*s == '\0')
			return;
	}
	else
	{
		ormask = FLG_SPEC;
		andmask = 0xffff;
	}

	/* convert "all" not bounded by alphanumerics to ".*". ".all" becomes ".*" */
	for (ptr = s; (len = findall(ptr)) >= 0; ptr += len+1)
	{
		if (len > 0 && isalnum (s[len-1]))
			continue;
		if (isalnum (s[len+3]))
			continue;
		if (len > 0 && s[len-1] == '.')
		{
			--len;
			strcpy (s+len,s+len+1);
		}
		s[len] = '.';
		s[len+1] = '*';
		strcpy (s+len+2,s+len+3);
	}

	/* now use regular expressions */
	sprintf (pattern,"^%s$",s);
	if ((re = regcmp(pattern,(char *) 0)) == NULL)
		printex ("n option regular expression syntax: %s",s);
	for (i=0; i < Ncount; ++i)
	{
		if (regex(re,(Newsorder[i])->nd_name) != NULL)
		{
			(Newsorder[i])->state |= ormask;
			(Newsorder[i])->state &= andmask;
		}
	}
	regfree (re);
}

static findall (s)
char *s;
{
	int len;
	for (len=0; *s != '\0'; ++s,++len)
	{
		if (*s == 'a' && strncmp(s,"all",3) == 0)
			return (len);
	}
	return (-1);
}
