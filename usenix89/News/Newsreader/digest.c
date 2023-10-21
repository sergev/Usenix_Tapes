#include <stdio.h>
#include "vn.h"
#include "head.h"

extern int Digest;
extern int L_allow;
extern int C_allow;
extern PAGE Page;

extern char *F_head, *T_head, *L_head, *D_head;

digest_page (idx,skip)
int idx;
{
	char *ptr,name[24],*title,*index();
	FILE *fp;
	int i,len;
	char subj[RECLEN],date[RECLEN],from[RECLEN],junk[RECLEN],*str_store();
	long pos;

	Digest = Page.b[idx].art_id;
	sprintf (name,"%d",Digest);

	if ((fp = fopen(name,"r")) == NULL)
		return (-1);

	skip_header (fp);

	/* skip over some articles if requested to */
	for (i=skip; i > 0; --i)
	{
		if (dig_advance(fp,from,subj,date,junk,&pos) < 0)
			return (-1);
	}

	/* every new call to a digest Page "loses" a small amount of storage */
	title = str_store(Page.b[idx].art_t);
	if ((ptr = index(title,'~')) != 0)
		*ptr = '\0';
	title [C_allow - 20] = '\0';

	for (i=0; i < L_allow &&
			(len = dig_advance(fp,from,subj,date,junk,&pos)) >= 0; ++i)
	{
		Page.b[i].art_id = i+1+skip;
		Page.b[i].art_mark = ' ';
		subj [C_allow] = '\0';
		from [C_allow] = '\0';
		sprintf (name,"%d",len);
		form_title (date,subj,name,from,100);
		strcpy (Page.b[i].art_t,date);
	}

	fclose (fp);

	if (i == 0)
		return (-1);

	Page.h.name = title;
	Page.h.artnum = i;
	return (i);
}

/*
	returns name of file containing "article", NULL for failure
*/
char * digest_extract (s,art)
char *s;
int art;
{
	char name[24];
	FILE *fout,*fin;
	char subj[RECLEN],date[RECLEN],from[RECLEN],bufr[RECLEN];
	char extra[RECLEN];
	char *index();
	long pos;
	int lines;

	sprintf(name,"%d",Digest);
	if ((fin = fopen(name,"r")) == NULL)
		return (NULL);

	for (skip_header (fin); art > 0; --art)
		if ((lines = dig_advance(fin,from,subj,date,extra,&pos)) < 0)
		{
			fclose (fin);
			return (NULL);
		}

	tmpnam(s);

	if ((fout = fopen(s,"w")) == NULL)
	{
		fclose (fin);
		unlink (s);
		return (NULL);
	}

	fseek(fin,0L,0);

	while (fgets(bufr,RECLEN-1,fin) != NULL && index(bufr,':') != NULL)
	{
		if (strncmp(bufr,F_head,FHDLEN) == 0)
		{
			fprintf (fout,"%s%s\n",F_head,from);
			continue;
		}
		if (strncmp(bufr,T_head,THDLEN) == 0)
		{
			fprintf (fout,"%s%s\n",T_head,subj);
			continue;
		}
		if (strncmp(bufr,D_head,DHDLEN) == 0)
		{
			fprintf (fout,"%s%s\n",D_head,date);
			continue;
		}
		/* defer line count header - it comes last */
		if (strncmp(bufr,L_head,LHDLEN) == 0)
			continue;
		fprintf (fout,"%s",bufr);
	}

	/* toss in extra header lines, line count header, extra newline */
	fprintf (fout,"%s%s%d\n\n",extra,L_head,lines);

	fseek (fin,pos,0);

	while (fgets(bufr,RECLEN-1,fin) != NULL && strncmp(bufr,"--------",8) != 0)
		fprintf(fout,"%s",bufr);

	fclose (fin);
	fclose (fout);
	return (s);
}

dig_list (s)
char *s;
{
	char *ptr,*out,*new,ns[L_tmpnam],tmp[RECLEN],*strtok();
	int i;

	prinfo ("Extracting articles .....");
	strcpy (tmp,s);
	out = s;

	for (ptr = strtok(tmp," "); ptr != NULL; ptr = strtok(NULL," "))
	{
		i = atoi(ptr);
		if ((new = digest_extract(ns,i)) != NULL)
		{
			sprintf (out,"%s ",new);
			out += strlen(new) + 1;
		}
	}

	*out = '\0';

	if (*s == '\0')
		strcpy (s,"NULLDIGEST");
}

dig_ulist (s)
char *s;
{
	char *strtok();
	for (s = strtok(s," "); s != NULL; s = strtok(NULL," "))
		unlink (s);
}

/*
	returns # lines in article, -1 for failure
	scans past article, returns position of start.
	also returns "extra" header lines encountered, WITH newlines.
*/
static dig_advance (fp,from,subj,date,extra,pos)
FILE *fp;
char *from,*subj,*date,*extra;
long *pos;
{
	char buf[RECLEN];
	char *ptr, *index();
	int state,lcount;

	lcount = state = 0;
	*extra = '\0';

	while (fgets(buf,RECLEN-1,fp) != NULL)
	{
		buf[strlen(buf) - 1] = '\0';

		switch(state)
		{
		case 0:
			/* skip blank lines before header */
			if (strlen(buf) == 0)
				break;
			state = 1;	/* fall through */
		case 1:
			if (strncmp(buf,F_head,FHDLEN) == 0)
			{
				strcpy (from,buf+FHDLEN);
				break;
			}
			if (strncmp(buf,T_head,THDLEN) == 0)
			{
				strcpy (subj,buf+THDLEN);
				break;
			}
			if (strncmp(buf,D_head,DHDLEN) == 0)
			{
				strcpy (date,buf+DHDLEN);
				break;
			}
			/* put wierd header lines in extra */
			if ((ptr = index(buf,':')) != NULL)
			{
				*ptr = '\0';
				if (index(buf, ' ') == NULL)
				{
					*ptr = ':';
					sprintf(extra,"%s\n",buf);
					extra += strlen(extra);
					break;
				}
				*ptr = ':';
			}
			state = 2;

			/* remember the newline we lopped off */
			*pos = ftell(fp)-strlen(buf)-1;	/* fall through */
		case 2:
			++lcount;
			if (strncmp("--------",buf,8) == 0)
			{
				--lcount;
				return (lcount);
			}
			break;
		}
	}

	return (-1);
}

static skip_header (fp)
FILE *fp;
{
	char buf[RECLEN];

	while (fgets(buf,RECLEN-1,fp) != NULL)
		if (strncmp("--------",buf,8) == 0)
			break;
}
