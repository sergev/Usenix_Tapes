/*
** The initial implementation of keepnews was written in Icon by
** Bill Mitchell at the University of Arizona.  This version
** is motivated by the need for greater speed and various func-
** tional improvements.
** 
** -- Mark Swenson {ihnp4,noao,utah-cs,mcnc}!arizona!mms
**    Dept. of Computer Science / University of Arizona
**
**
** keepnews extracts the header information from a redirected news article
** and checks to see if the article has already been saved.  If so, it
** appends a corresponding message in KROOT/log, otherwise it creates a
** file in the location specified by the first newsgroup listed, and
** links the remaining newsgroups to this file.  The body of the article
** is not read unless it is actually saved, and if that is the case,
** stdin is copied directly into this new file.
**
** The -n and -t options have been added to allow the user to supply
** additional newsgroups (eg. -n net.another.one net.another.two) or
** additional subject matter (eg. -t more subject material) respectively.
** These flags can be intermixed, and everything following each is taken
** as its argument up to the next -n, -t, or newline.
** 
*/

#include		<stdio.h>
#define	KROOT		"/usr/spool/keepnews"
#define	MAXGROUPS	20
#define	LINE		128

extern	FILE	*fopen();
extern	char	*index(), *malloc();
char		*match(); 
static	char	*head[5] =	{
				"From: ",
			 	"Newsgroups: ",
			 	"Subject: ",
			 	"Posted: ",
			 	"Article-I.D.: "
				} ;
main(argc,argv) char **argv;
{   FILE  *rlogfp, *flogfp, *filefp;
					/* use buffers extensively for speed */
    char  line[2*LINE];
    char  content[5][2*LINE];		/* data corresponding to head[] */
    char  *dirlist[MAXGROUPS];		/* individual newsgroups */
    char  pathlist[MAXGROUPS][LINE];	/* path to archive each group */
    char  flog[LINE];			/* path to leaf log */
    char  tmpbuf[LINE];			/* temp buffer */
    char  logline[4*LINE];		/* data for log files */
    char  rlog[LINE];			/* path to KROOT/log */
    char  tfile[LINE];			/* temp buffer */
    char  installfile[LINE];		/* actual file name for archive */
    char  lnfilename[LINE];		/* temp name for linking */
    char  wbuf[8*LINE];			/* write output buffer */
    char  *f = wbuf;			/* index to header lines in wbuf */
    char  t_list[LINE];			/* user supplied subject */
    char  n_list[LINE];			/* user supplied newsgroups */
    char  ch, *m, *tmpline;
    int   t_flag = 0;			/* user supplied subject added */
    int   n_flag = 0;			/* user supplied groups added */
    int   c, i, j, ofd, rcnt;
    int   numgroups = 1 ;
    int   line_length = LINE;		/* for internal reference */

    while (--argc > 0)			/* get any user supplied info */
      { if ((++argv)[0][0] == '-')
	  { ch = argv[0][1] ;
	    argv++ ;
	    argc-- ;
	  }
	switch (ch)
	  { case 'n':
		n_flag = 1 ;
		sprintf(n_list,"%s,%s",n_list,*argv) ;
		break ;
	    case 't':
		t_flag = 1 ;
		sprintf(t_list,"%s %s",t_list,*argv) ;
		break ;
	    default:
		fprintf(stderr,"Unknown flag: -%s\n",c) ;
		exit(1) ;
	  }
      }

    umask(0022) ;
    setuid(geteuid());
    if (access(KROOT,7))
      { fprintf(stderr,"Can't find or access keepnews root directory %s\n",
				KROOT);
	exit(1);
      }

    rcnt = read(0, wbuf, 1024);
    while ((j=getline(line,2*line_length,f)) > 0) 	/* get header info */
      { if (j < 0)
	  { fprintf(stderr,"Attempt to install incompatible article failed.\n");
	    exit(1);
	  }
	f += j + 1;
        for (i = 0 ; i <= 4 ; i++)
	    if (tmpline = match(line,head[i]))
		{ strcpy(content[i],tmpline) ; break ; }
      }

    /* add '-t' and '-n' information to article information */
    if (n_flag)
	sprintf(content[1],"%s%s",content[1],n_list);
    if (t_flag)
	sprintf(content[2],"%s (+%s +)",content[2],t_list);
    if (*content[1] == NULL)
      { fprintf(stderr,"Attempt to install incompatible article failed.\n");
        exit(1);
      }

    /* break newsgroups into separate groups */
    strcpy(*dirlist=malloc(strlen(content[1])),content[1]) ;
    while (dirlist[numgroups] = index(dirlist[numgroups-1],','))
	*dirlist[numgroups++]++ = NULL ;
    numgroups-- ;

    /* convert each newsgroup name into a directory path */
    for (j=0 ; j <= numgroups ; j++)
      { sprintf(pathlist[j],"%s/%s",KROOT,dirlist[j]) ;
	while (m = index(pathlist[j],'.')) *m = '/' ;

	m = pathlist[j] ;	/* create any missing ancestor directories */
	while (m = index(m,'/'))
	  { *m = NULL ; mkdir(pathlist[j],0755) ; *m++ = '/' ; }
        mkdir(pathlist[j],0755) ;
      }

    sprintf(rlog,"%s/log",KROOT) ;
    sprintf(logline,"%s; %s, %s, %s, %s",content[1],content[4],
		content[0],content[3],content[2]) ;
    sprintf(installfile,"%s/%s",pathlist[0],content[4]) ;
    sprintf(flog,"%s/log",pathlist[0]) ;

    rlogfp = fopen(rlog,"a") ;
    if (access(installfile,0))
      { fprintf(rlogfp,"Saved: %s\n",logline) ;	/* update "root" log */
	fclose(rlogfp) ;

	flogfp = fopen(flog,"a") ;	/* update "leaf" log */
	fprintf(flogfp,"%s\n",logline) ;
	fclose(flogfp) ;

	/*  write entire article header and body to archive file */
	ofd = creat(installfile, 0644) ;
	write(ofd, wbuf, rcnt) ;
	if (rcnt == 1024)
	  { while ((rcnt = read(0, wbuf, 1024)) == 1024)
	        write(ofd, wbuf, 1024) ;
	    write(ofd, wbuf, rcnt) ;
	  }
	close(ofd) ;

	/* install hard links to remaining newsgroups */
	for (i = 1 ; i <= numgroups ; i++)
	  { sprintf(tfile,"%s/%s",pathlist[i],content[4]) ;
	    sprintf(tmpbuf,"%s/log",pathlist[i]) ;
	    link(installfile,tfile) ;
	    flogfp = fopen(tmpbuf,"a") ;
	    fprintf(flogfp,"%s\n",logline) ;
	    fclose(flogfp) ;
	  }
      }
    else if (n_flag)		/* t_flag is implied */
      { fprintf(rlogfp,"Saved: %s\n",logline) ;	/* update "root" log */
	fclose(rlogfp) ;

	/* install hard links to remaining newsgroups */
	for (i = 0 ; i <= numgroups ; i++)
	  { sprintf(tfile,"%s/%s",pathlist[i],content[4]) ;
	    sprintf(tmpbuf,"%s/log",pathlist[i]) ;
	    link(installfile,tfile) ;
	    flogfp = fopen(tmpbuf,"a") ;
	    fprintf(flogfp,"%s\n",logline) ;
	    fclose(flogfp) ;
	  }
      }
    else if (t_flag)
      { fprintf(rlogfp,"Saved: %s\n",logline) ;	/* update "root" log */
	fclose(rlogfp) ;

	/* update "leaf" log for each leaf */
	for (i = 0 ; i <= numgroups ; i++)
	  { sprintf(tmpbuf,"%s/log",pathlist[i]) ;
	    flogfp = fopen(tmpbuf,"a") ;
	    fprintf(flogfp,"%s\n",logline) ;
	    fclose(flogfp) ;
	  }
      }
    else	/* update only KROOT log */
      { fprintf(rlogfp,"Duplicate request: %s\n",content[4]) ;
	fclose(rlogfp) ;
      }
    exit(0) ;
}

char *
match(s,t) char *s, *t;   	/* return end of t in s, NULL if none */
{   while (*t != NULL && *s++ == *t++) ;
    if (*t == NULL) return(s) ;
    else return(NULL) ;
}

getline(s,lim,f) char *s, *f;	/* get line into s from f & return length */
{   int c, i;			/* don't include newline; -1 if len > lim */
    char *ftmp;

    ftmp = index(f,'\n');
    *ftmp = NULL;
    if (strlen(f) <= lim)
	strcpy(s,f);
    else
	return(-1);
    *ftmp++ = '\n';
    f = ftmp;
    return(strlen(s));
}
