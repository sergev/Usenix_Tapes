#include <stdio.h>
#include <sys/file.h>
#include "vn.h"
#include "head.h"

extern int Nfltr,Ncount,Lrec,L_allow,Cur_page,C_allow;
extern FILTER Filter[];
extern NODE *Newsorder[];
extern PAGE Page;
extern int Digest;

extern char *Aformat;

extern char *T_head, *F_head, *L_head;

static int Tdes;	/* temp file descriptor */
static int Pgsize;	/* block size for seeking file */

/*
	routines which deal with the temp file containing
	display pages.  Note the "invisible" file feature -
	tempfile is unlinked from /usr/tmp immediately.  when
	Tdes is closed by UNIX the disk space will be given back.
*/

temp_open ()
{
	char tmpart [L_tmpnam];
	Lrec = -1;
	tmpnam (tmpart);
	Pgsize = sizeof (HEAD) + L_allow * sizeof(BODY);
	if ((Tdes = open(tmpart,O_RDWR|O_CREAT)) < 0)
		printex ("can't open %s",tmpart);
	unlink (tmpart);
}

/*
	create page records for newsgroup s
	all articles between low and hi are to be included.
*/
outgroup (s,low,hi)
char *s;
int low,hi;
{
	int i,aid;
	char title[RECLEN],gd[RECLEN];
	g_dir(s,gd);
	if (chdir(gd) < 0)
	{
		grp_indic(s,0);
		return;
	}
	grp_indic(s,1);
	aid = 0;
	for (i=low+1; i <= hi; ++i)
	{
		if (digname (i,title) >= 0)
		{
			Page.b[aid].art_id = i;
			Page.b[aid].art_mark = ' ';
			strcpy (Page.b[aid].art_t, title);
			if ((++aid) >= L_allow)
			{

				/* start next page */
				Page.h.artnum = L_allow;
				do_write ();
				++Lrec;
				aid = 0;
			}
		}
	}

	/* last page (partial) */
	if (aid != 0)
	{
		Page.h.artnum = aid;
		do_write ();
		++Lrec;
	}
}

/*
	set current page to n.  use Pgsize and lseek to find it in
	temp file (descriptor Tdes).
*/
find_page (n)
int n;
{
	long off,lseek();
	int i,last;
	Cur_page = n;
	off = Pgsize;
	off *= (long) n;
	lseek (Tdes, off, 0);
	if (read(Tdes, (char *) &(Page.h), sizeof(HEAD)) < sizeof(HEAD))
		printex("bad temp file read");
	i = Pgsize - sizeof(HEAD);
	if (read(Tdes, (char *) Page.b, i) < i)
		printex("bad temp file read");
	last = -1;
	for (i=0; i < Ncount; ++i)
	{
		if ((Newsorder[i])->pages > 0)
		{
			if ((Newsorder[i])->pnum > n)
				break;
			last = i;
		}
	}
	if (last < 0)
		printex ("can't find page %d",n);
	Page.h.group = Newsorder[last];
	Page.h.name = (Page.h.group)->nd_name;
	cd_group ();
}

write_page ()
{
	long off,lseek();
	if (!Digest)
	{
		off = Pgsize;
		off *= (long) Cur_page;
		lseek (Tdes, off, 0);
		do_write();
	}
}

static do_write()
{
	int num;

	if (write(Tdes, (char *) &(Page.h), sizeof(HEAD)) < sizeof(HEAD))
		printex ("Bad temp file write");
	num = L_allow * sizeof(BODY);
	if (write(Tdes, (char *) Page.b, num) < num)
		printex ("Bad temp file write");
}

/*
	find article title:
	n - articles id
	t - returned title - must have storage for 3 times max title.
*/
static digname (n, t)
int n;
char *t;
{
	int i,tfilter,afilter;
	FILE *fp,*fopen();
	char ff [MAX_C+1],fn [MAX_C+1],fl [MAX_C+1],*index();

	/* open article */
	sprintf (t,"%d",n);
	if ((fp = fopen(t,"r")) == NULL)
		return (-1);

	/* get subject, from and lines by reading article */
	ff[0] = fn[0] = fl[0] = '?';
	ff[1] = fn[1] = fl[1] = '\0';
	ff[C_allow] = fn[C_allow] = fl[C_allow] = '\0';
	for (i = 0; i < HDR_LINES && fgets(t,RECLEN-1,fp) != NULL; ++i)
	{
		if (index(CHFIRST,t[0]) == NULL)
			continue;
		t[strlen(t) - 1] = '\0';
		if (strncmp(T_head,t,THDLEN) == 0)
		{
			strncpy(fn,t+THDLEN,C_allow);
			continue;
		}
		if (strncmp(F_head,t,FHDLEN) == 0)
		{
			strncpy(ff,t+FHDLEN,C_allow);
			continue;
		}
		if (strncmp(L_head,t,LHDLEN) == 0)
		{
			strncpy(fl,t+LHDLEN,C_allow);
			break;
		}
	}
	fclose (fp);

	/* filter the article [at]filter = 1 if no options, 0 if OK -1 if rejected */
	afilter = tfilter = 1;
	for (i=0; i < Nfltr; ++i)
	{
		switch (Filter[i].hcomp)
		{
		case FIL_AUTHOR:
			if (afilter == 0)
				break;
			if (afilter > 0)
				afilter = -1;
			if (do_filt(Filter[i].neg,Filter[i].rex,ff) >= 0)
				afilter = 0;
			break;
		case FIL_TITLE:
			if (tfilter == 0)
				break;
			if (tfilter > 0)
				tfilter = -1;
			if (do_filt(Filter[i].neg,Filter[i].rex,fn) >= 0)
				tfilter = 0;
			break;
		default:
			printex ("unexpected filter comparison: x%x",(int) Filter[i].hcomp);
		}
		if (afilter == 0 && tfilter == 0)
			break;
	}
	if (afilter < 0 || tfilter < 0)
		return (-1);

	form_title (t,fn,fl,ff,n);
	return (0);
}

form_title (t,fn,fl,ff,n)
char *t,*fn,*fl,*ff;
int n;
{
	char *ptr,*index();
	int i;

	if ((ptr = index(ff,'(')) != NULL && strlen(ptr) > 3)
		ff = ptr;
	sprintf (t,TFORMAT,fn,fl,ff);
	sprintf(ff,Aformat,' ',' ',n);
	i = C_allow - strlen(ff) + 1;	/* remember newline in Aformat */
	t[i] = '\0';
	ctl_xlt(t);
	return (0);
}

/* replace control characters in titles */
static ctl_xlt(s)
char *s;
{
	while (*s != '\0')
	{
		if (*s < ' ')
			*s += 'A' - 1;
		++s;
	}
}


static do_filt (neg,reg,str)
char neg,*reg,*str;
{
	char *regex();
	if (regex(reg,str) != NULL)
	{
		if (neg != '!')
			return (0);
		return (-1);
	}
	if (neg != '!')
		return (-1);
	return (0);
}
