#include <stdio.h>

typedef struct
{
	char attribute;
	unsigned file_time;
	unsigned file_date;
	long file_size;
	char file_name[13];
} file_desc;

typedef struct
{
	char dos_reserved[21];
	file_desc file;
} fcb;

#define maxfiles 128 

char printbuf[256];
file_desc *getfirst(),*getnext();
char *index(), *rindex();
int mode = 0x10;
int verbose=0,column=4,recurse=0;
int quiet = 0;
int drivenum = 0;
long time();
short year;

do_return(result)
{
	verbose = quiet = recurse = 0;
	column = 4;
	return result;
}
#ifndef MAIN
ls
#else
main
#endif
(argc,argv)
char *argv[];
{
	int noargs;
	char *current;
	char namebuf[128];
	/* 
	 * initialize statics
	 */
	mode = 0x10;
	verbose=0;column=4;recurse=0;
	quiet = 0;
	drivenum = 0;
	/*
	 * get current time
	 */
	year = (int)((time(NULL) >> 25) & 0x7F) + 80;
	/*
	 * set up a default search name
	 */
	if (noargs = (argc == 1))
		argc++;
	while(--argc)
	{
		if (noargs)
			current = "*.*";
		else
			current = *(++argv);	/* get current file name */
		if (*current == '-')
		{
			++current;	/* point past - */
			while (*current)
			{
				switch (*current++)
				{
				case 'l':
				case 'L':
					verbose = 1;
					if (column != 1)
						column = 2;
					if (quiet)
					{
						fprintf(stderr,"ls : verbose and quiet conflict\n");
						do_return(-1);
					}
					break;
				case 'q':
				case 'Q':
					quiet = 1;
					if (verbose)
					{
						fprintf(stderr,"ls : quiet and verbose conflict\n");
						do_return(-1);
					}
					break;
				case 'c':
				case 'C':
					column = 1;
					break;
				case 'a':
				case 'A':
					mode = 0x2 + 0x4 + 0x10;
					break;
				case 'r':
				case 'R':
					recurse = 1;
					mode = 0x2 + 0x4 + 0x10;
					break;
				default:
					break;
				}
			}
			/* if we're down to one argument after looking at all the
			   switches, we need to set noargs to true */
			if (noargs = (argc == 1))
				argc++;
			continue;
		}
		/* if a drive is specified, figure out what drive it is */
		if (current[1] == ':')
		{
			drivenum = toupper(current[0]) - 'A' + 1;
		}
		else
			drivenum = 0;
		/* if no wild cards, look for directory and drive names */
		if ( NULL == index(current,'?') && NULL == index(current,'*'))
		{
			if (getfirst(current)->attribute & 0x10)
			{
				strcpy(namebuf,current);
				strcat(namebuf,"\\*.*");
				current = namebuf;
			} 
			/* look for drive names */
			else if (current[strlen(current)-1] == ':' && 
						!current[strlen(current)])
			{
				strcpy(namebuf,current);
				strcat(namebuf,"\\*.*");
				current = namebuf;
			}
		}
		do_dir(current);
	}
	do_return( 0);
}

do_dir(current)
	char *current;
{
	typedef file_desc fblock[maxfiles];	/* as many as we'll likely need */
	file_desc *files;
	file_desc *curr_file,*getnext();
	void *malloc();
	unsigned int ftime,date;
	int i,j,col;
	int files_cmp();
	long total = 0;
	char atts[4]; /* drw */
	/* allocate file block */
	if (NULL == (files = malloc(sizeof(fblock))))
	{
		fprintf(stderr,"Not enough memory to do directory\n");
		return -1;
	}
	/* look for match */
	i = 0;
	if (!(curr_file = getfirst(current)))
	{
		printf(stderr,"ls : no files matching %s\n",current);
		free(files);
		return;	
	}
	files[i++] = *curr_file;
	/* get all matching */
	while ((curr_file = getnext()) && i < maxfiles)
		files[i++] = *curr_file;	
	if (i > 1)
		qsort(files,i,sizeof(file_desc),files_cmp);
	if (!quiet)
	{
		write(1,"\r\n",2);
		write(1,current,strlen(current));
		write(1,"\r\n",2);
	}
	col = 1;
	for (j = 0; j < i; j++)
	{
		register char *c = files[j].file_name;
		if (*c == '.')
			continue;	/* filter out . and .. */
		while (*c)
		{
			*c = tolower(*c);
			c++;
		}
		if (verbose)
		{
			register char att = files[j].attribute;
			register int fyear;
			fyear = ((files[j].file_date >> 9) & 0x7F)+80; 
			atts[3] = 0;	/* terminate string */
			atts[0] = att & 0x10 ? 'd' : '-';
			atts[1] = att & 2 ? '-' : 'r';
			atts[2] = att & 1 ? '-' : 'w';
			if (atts[0] == 'd')
			{
				register int k;
				sprintf(printbuf,"%s %s\\",atts,files[j].file_name);
				write(1,printbuf,strlen(printbuf));
				k = 12 - strlen(files[j].file_name)+7;
				while(k--)
					write(1," ",1);

			}
			else
			{
				total += files[j].file_size;
				sprintf(printbuf,"%s %-12s %-6ld ",atts,files[j].file_name,
						files[j].file_size);
				write(1,printbuf,strlen(printbuf));
			}
			ftime = files[j].file_time;
			date = files[j].file_date;
			if (year == fyear)
			{
				sprintf(printbuf,"%02d/%02d %02d:%02d ",
					((date >> 5) & 0x0F),	/* month */
					date & 0x1F,		/* day	*/
					(ftime >> 11) & 0x1F,		/* hours */
					(ftime >> 5) & 0x3F);		/* minutes */
				write(1,printbuf,strlen(printbuf));
			}
			else
			{
				sprintf(printbuf,"%02d/%02d       ",
					((date >> 5) & 0x0F),	/* month */
					fyear,					/* file year */
					(ftime >> 11) & 0x1F,		/* hours */
					(ftime >> 5) & 0x3F);		/* minutes */
				write(1,printbuf,strlen(printbuf));
			}
		}
		else
		{
			if (files[j].attribute & 0x10)
			{
				register int k;
				sprintf(printbuf,"%s\\",files[j].file_name);
				write(1,printbuf,strlen(printbuf));
				k = 16 - strlen(files[j].file_name);
				while(--k)
					write(1," ",1);

			}
			else
			{
				sprintf(printbuf,"%-13s   ",files[j].file_name);
				write(1,printbuf,strlen(printbuf));
			}
		}
		if (col == column)
		{
			col = 1;
			write(1,"\r\n",2);
		}
		else
			col++;
	}
	write(1,"\r\n",2);
	if (verbose)
	{
		sprintf(printbuf,"%ld bytes in %d files ",total,i);
		write(1,printbuf,strlen(printbuf));
		pr_freespace();
	}
	if (recurse)
		for (j = 0; j < i; j++)
		{
			/* we've got a subdirectory */
			if (files[j].attribute & 0x10 && files[j].file_name[0] != '.')
			{
				char *path;
				char dirname[48];
				if (!strcmp(current,"*.*"))
					dirname[0] = '\0';
				else
					strcpy(dirname,current);
				if (path = rindex(dirname,'\\'))
					*(++path) = '\0';
				strcat(dirname,files[j].file_name);	/* get name */
				strcat(dirname,"\\*.*");
				do_dir(dirname);
			}
		}
	free(files);
}

files_cmp(a,b)
	file_desc *a,*b;
{
	return strcmp(a->file_name,b->file_name);
}

fcb tmp;

file_desc *getfirst(fname)
	char *fname;
{
	register int result;
	/* set the disk transfer address */
	bdos(0x1A,&tmp);
	result = bdos(0x4E,fname,mode);
	/* make the find first call */
	if(2 == result || 18 == result)
		return NULL;
	return &(tmp.file);
}

file_desc *getnext()
{
	register int result;
	/* set the disk transfer address */
	bdos(0x1A,&tmp);
	result = bdos(0x4f,0,0);
	/* make the find next call */
	if (18 == result)
		return NULL;
	return &(tmp.file);
}

/*	determine available space on default drive
*/

pr_freespace()
{
	/* register arguments for INT */
    struct   {int ax,bx,cx,dx,si,di,ds,es;}sysr;
    unsigned int    retstat;		    /* flags returned from INT */

#define 	    cls_avail  sysr.bx	    /* number of available clusters */
#define 	    cls_total  sysr.dx	    /* total number of clusters on volume */
#define 	    byt_sectr  sysr.cx	    /* bytes per sector */
#define 	    sec_clstr  sysr.ax	    /* sectors per cluster */

    unsigned long   byt_clstr,		    /* bytes per cluster */
		    vol_total,		    /* total size of volume, bytes */
		    fre_total;		    /* size of free space, bytes */

    sysr.ax = 0x3600;		    /* get disk free space function code */
    sysr.dx = drivenum;			    /* (DL) = drive id, 0=current, 1=A, */
    retstat = sysint(0x21,&sysr,&sysr);		    /* invoke DOS */
    if( sec_clstr!=0xffff ) 
	{
		byt_clstr = byt_sectr * sec_clstr;
		vol_total = cls_total * byt_clstr;
		fre_total = cls_avail * byt_clstr;
		fprintf(stdout,
		"%lu out of %lu bytes available on %c:\n", 
		fre_total,vol_total, 
		drivenum ? drivenum + 'A' - 1 : bdos(0x19,0,0)+'A');
	}
}
