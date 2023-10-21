/*	
 *	This is a program to make the debugging of C programs much
 *	easier and faster. It makes use of the capability of vi
 *	and terminfo. 
 *	Currently it can work with errors output from cc, lint, 
 *	ld, yacc, cyntax and make. 
 *	
 *	Written by M.C. KAM
 */
#define		SINGLE
#define 	MAXEXSIZE	62
#define		BELL		"\007\007\007\007\007\007\007\007\007"

#include	<stdio.h>

#ifdef	BSD
#define	strchr	index
#define	strrchr	rindex
#endif

extern		char	*strrchr() ;
extern		int	strcmp() ;
extern		char	*getcmdpath() ;
extern		char	*getenv() ;
extern		FILE	*tmpfile() ;

extern		int	makelintag() ;
extern		int	makeldtag() ;
extern		int	makecctag() ;
extern		int	makecyntag() ;
static		void    creattag() ;


typedef	struct
{
	int	(*translator)() ;
	int	checktype ;

}	ERRDRIVER ;

ERRDRIVER	driverlist[] = 
{
		makecyntag,  1,
		makecctag,  0,
		makeldtag,  0,
		makelintag, 0,
		NULL
} ;


static		FILE		*srcefp ;
static		FILE		*errfp;
static		FILE		*tmpfp ;
static		FILE		*terrfp ;
static		char		tempfile[64] ;
static		char		cursrcefile[128] ;
static		char		curlinebuf[512] ;
static		char		lbuf[512];
static		int		curlineno ;
static		int		patmatch ;
static		FILE		*fp;
static		int		tabsize = 0 ;
static		int		line  = 0 ;
static		char		tagfile[50] ;

main(argc, argv, envp)
int	argc;
char	**argv;
char	**envp;
{
	int			i ;
	char			**newenvp ;
	int			eline;
	int			initflag = 0 ;
	char			filename[100];
	char			errmsg[132] ;
	char			tagmsg[512] ;
	char			envstr[300] ;
	char			srhpat[50] ;
	char			*homep ;
	extern	char		*getlogin() ;
	extern	char		*ttyname() ;
	extern  char		*strrchr() ;
	long			addr ;
	

	

	if (isatty(0) == 0)
	{
		fputs(BELL, stderr) ;	
		sleep(1) ;
		fputs(BELL, stderr) ;	
		sleep(1) ;
		fputs(BELL, stderr) ;	
		exit(1) ;
	}
	else homep = strrchr(ttyname(0), '/') + 1 ;
	line = 0 ;
	strcpy(tagfile, "/tmp/tags.") ;
	strcat(tagfile, homep) ;
	strcat(tagfile, getlogin());
	switch (*argv[1])
	{
	case '+' : 
		(void) getheader(&line, &patmatch, &tabsize) ;
		 if ((++line )  > tabsize)  
		 {
			fputs(BELL, stdout) ;
			fseek(fp, 3 * sizeof(int) + tabsize * sizeof(long), 0) ;
			fread((char *) &addr, sizeof (long), 1, fp) ; 
			fseek(fp, addr + 55 * sizeof(char), 0) ;
			fprintf(fp, "/$/%62.s\n", "") ;
			fclose(fp) ;
			tprintf("ve : no more error line found");
			exit(0) ;
		 }
		 break;
	case '=' : 
		(void) getheader(&line, &patmatch, &tabsize) ;
		 break;
	case '-' : initflag = -1 ;
		(void) getheader(&line, &patmatch, &tabsize) ;
		if (line > 1)
			line-- ;
		else 
		{
			fputs(BELL, stdout) ;
			tprintf("ve : no more previous error line");
			exit(0) ;
		}
		break;
	default: 
		 if (argc < 2)
		 {
			fprintf(stderr, "usage : %s file\n", argv[0]) ;
			exit(1) ;
		 }
		 else 
		 {
			 if ((errfp = fopen(argv[1], "r")) == NULL)
			 {
				fprintf(stderr, "%s : can not open error file : %s\n", argv[0], argv[1]) ;
				exit(2) ;
			 }
		 }
		 if (getc(errfp) != EOF)
		   rewind(errfp) ;
		 else 
		 {
			fputs(BELL, stdout) ;
			tprintf("ve : no recognisable syntax error ");
			exit(1) ;
		 }
		 patmatch = (argc <= 2) ;
		 openfile() ;
		 CallTranslator() ;
		 initflag = 1 ;
		 line++ ;
		 fputs(BELL, stderr) ;
		 break;
	}
	fseek(fp, (line - 1) * sizeof( long), 1) ;
	fread((char *) &addr, sizeof (long), 1, fp) ; 
	fseek(fp, addr, 0) ;
	fgets(lbuf, 512, fp) ;
	rewind(fp) ;
	fwrite((char *) &line, sizeof (int), 1, fp) ; 
	fseek(fp, 3 * sizeof(int) + tabsize * sizeof(long), 0) ;
	fread((char *) &addr, sizeof (long), 1, fp) ; 
	fseek(fp, addr, 0) ;
	if (*lbuf == '"')
	{
		if (sscanf(lbuf, "\"%[^\"]\", line %d: %[^\^\n]%[^\n]", 
			   filename, &eline, errmsg, tagmsg) > 3 
		    && 
		    (
			patmatch == 1
			&&
			initflag != 1
		    )
		   )
		{
		     char	*ch ;

			tprintf("%s, %d: %s", filename, eline, errmsg) ;
			ch = (initflag == -1 ? "?" : "/") ;
			fprintf(fp, "err\t%50.50s\t%s%s%-*.1s\n", filename, ch,  tagmsg, MAXEXSIZE + 2  - strlen(tagmsg), ch);
		}
		else    
		{
			tprintf("%s, %d: %s", filename, eline, errmsg) ;
			fprintf(fp, "err\t%50.50s\t%5d%60.s\n", filename, eline, "");
		}
	}
	else  if (*lbuf == '\'') 
	{
	     char	*ch ;

		ch = (initflag == -1 ? "?" : "/") ;
		sscanf(lbuf, "'%[^']', %s : %[^\n]", filename, srhpat, tagmsg)  ;
		tprintf("%s, %s", filename, tagmsg ) ;
		fprintf(fp, "err\t%50.50s\t%s%.*s%-*.1s\n", 
			filename, ch,  MAXEXSIZE + 1, srhpat, 
			MAXEXSIZE + 2  - strlen(srhpat), ch);
	}
	fclose(fp) ;
	if (initflag == 1 ) 
	{
		for (i = 0 ; envp[i] != NULL && strncmp(envp[i], "EXINIT", 6) ; i++) ;
		if (envp[i] != NULL)
		{
			sprintf(
				envstr, 
				"EXINIT=%s | set tags=%s\\ tags | set nu | map ? :!%s +\r:ta err\r | map ? :!%s +\r | map ? :ta err\r | map ? :!%s =\r:ta err\r | map ? :!%s -\r:ta err\r | set nomagic", 
				envp[i]+7, 
				tagfile,
				argv[0],
				argv[0],
				argv[0],
				argv[0]
			       );
			envp[i] = envstr ;
			fflush(stdout) ;
			execle(getcmdpath("vi"), "vi", "-t", "err", 0, envp) ;
			perror("ve") ;
			exit(2) ;
		}
		else
		{
			int	j ;
			newenvp = (char **) malloc ((i + 2) * sizeof (char **)) ;

			for (j = 0 ; j < i ; j++)
				newenvp[j] = envp[j] ;
			sprintf(
				envstr, 
				"EXINIT=set tags=%s\\ tags | set nomagic | map ? :!%s +\r:ta err\r | map ? :!%s =\r:ta err\r |  map ? :!%s -\r:ta err\r | set nu",
				 tagfile,
				 argv[0],
				 argv[0],
				 argv[0]
				);
			newenvp[i+1] = NULL ;
			newenvp[i] = envstr ;

			fflush(stdout) ;
			execle(getcmdpath("vi"), "vi", "-t", "err", 0, newenvp) ;
			perror("ve") ;
			exit(2) ;
		}
	}
}


CallTranslator()
{
	register	ERRDRIVER	*errdp ;

	for (
	      errdp = driverlist ; 
	      errdp->translator != NULL ;
	      errdp++
	    ) 
	{
	      if ((* errdp->translator)( errfp, terrfp) == 1 && errdp->checktype == 0)
	          break ;  
	      else
	          rewind(errfp) ;
	}
	fclose(errfp) ;
	rewind(terrfp) ;
	preprocess() ;
	
}
	
preprocess()
{


	*cursrcefile = '\0' ;
	tabsize = 0 ;
	while (fgets(lbuf, 512, terrfp) != NULL)
	{
	         switch (*lbuf )
		 {
		    case '\'' :
			fputs(lbuf, tmpfp) ;
			 tabsize++ ;
			break ;
		    case '"' :  lbuf[strlen(lbuf) - 1]  = '\0' ;
				extract(lbuf) ;
				 tabsize++ ;
				break ;
		 }
	}
	if (tabsize != 0)
		creattag(tmpfp) ;
	else
	{
		fputs(BELL, stdout) ;
	    	tprintf("ve : no recognisable syntax error ");
	    	exit(1) ;
		
	}

}


extract(linebuf)
char	*linebuf ;
{
	char	filename[64] ;
	int	eline ;
	
	if (sscanf(linebuf, "\"%[^\"]\", line %d:", filename, &eline) <= 1)
		return ;
	if (! strcmp(cursrcefile, filename))
	{
		if (curlineno > eline)
		{

			rewind(srcefp) ;
			curlineno = 0 ;
		}
		processsrce(linebuf, eline) ;
	}
	else
	{
		if (srcefp != NULL)
		    fclose(srcefp) ;
		if ((srcefp = fopen(filename, "r")) == NULL)
			   fprintf(tmpfp, "%s\n", linebuf) ;
		else
		{
			curlineno = 0 ;
			strcpy(cursrcefile, filename) ;
			processsrce(linebuf, eline) ;
		}
	}
}
			
			






processsrce(linebuf, eline)
int	eline ;
char	linebuf[];
{
	if (curlineno < eline)
	{
		do 
		{
		    if (fgets(curlinebuf, 512, srcefp) == NULL)
			break ;
		    else curlineno ++ ;
		}
		while (curlineno < eline) ;
		if (curlineno < eline)
		{
			fprintf(tmpfp, "%s\n", linebuf) ;
			return ;
		}
		else
		{
		  int	LineLen ;
		  register char	*cp ;

		  cp = curlinebuf ;
		  /*
		   *  put extra '\' to quote the magic char
		   */
		   while (*cp != '\0')
		   {
			if (*cp == '\\' || *cp == '/')
			{
				mvstr(cp) ;
				*cp++ = '\\' ;
			}
			cp++ ;
		   }
		  if ((LineLen = strlen(curlinebuf)) <= MAXEXSIZE)
			curlinebuf[LineLen - 1] = '$' ;
		  else
			curlinebuf[MAXEXSIZE] = '\0' ;
		}
	}
	fprintf(tmpfp, "%s^%s\n", linebuf, curlinebuf) ;
}

mvstr(tcp)
char	tcp[] ;
{
	register	int	slen ;
	
	for (slen = strlen(tcp) + 1 ; slen > 0 ; slen--) 
	{
		tcp[slen ] = tcp[slen - 1] ;
	}
}
	


#ifdef  TERMINFO
#include	<curses.h>
#include 	<term.h>
#endif

tprintf(format, p1, p2, p3, p4)
char	*format ;
{
	char    buf[512] ;

#ifdef TERMINFO
	setupterm(0, 1, 0) ;
	if (has_status_line == 0)
	{
	/*
         *	put it out to the status line	
	 */

		
		putp(to_status_line) ;
		fprintf(stdout, format, p1, p2, p3, p4) ;
		putp(from_status_line) ;
	}
	else
#endif
	{
		fprintf(stdout, format, p1, p2, p3, p4) ;
		fputc('\n', stdout) ;
	}

#ifdef TERMINFO
	resetterm() ;
#endif

}


openfile()
{
	
		if ((tmpfp = tmpfile()) == NULL || (terrfp = tmpfile()) == NULL)
		{
			fprintf(stderr, "ve : can not open temporary file for writing\n") ;
			exit(2) ;
		}
}




static	void
creattag(tempfp)
FILE	*tempfp ;
{
	long	address  ;
	long	startadd  ;
	register 	int	c ;
	extern  long  ftell() ;
	
	

	rewind(tempfp) ;
	if ((fp = fopen(tagfile, "w+")) == NULL)
	{
		perror("ve") ;
		exit(1) ;
	}
/*
	setbuf(fp, NULL) ;
/**/
	startadd = 3 * sizeof (int) + (tabsize + 1) * sizeof( long) ;
	fwrite((char *) &line, sizeof(int), 1, fp) ;
	fwrite((char *) &patmatch, sizeof(int), 1, fp) ;
	fwrite((char *) &tabsize, sizeof(int), 1, fp) ;
	fwrite((char *) &startadd, sizeof(long), 1, fp) ;
	while ((c = getc(tempfp)) != EOF)
	{
		if (c == '\n')
		{
			address = ftell(tempfp) + startadd ;
			fwrite((char *) &address, sizeof(long), 1, fp) ;
		}
	}
	rewind(tempfp) ;
	while (( c = getc(tempfp)) != EOF)
		putc(c, fp) ;
	fseek(fp, 3 * sizeof(int), 0) ;
}



			
			
	
	
	
	
	
getheader(errno, type, size)
int	*errno ;
int	*type ;
int	*size ;
{
	if ((fp = fopen(tagfile, "r+")) == NULL)
	{
		perror("ve") ;
		exit(1) ;
	}
	else
	{
/*
		setbuf(fp, NULL) ;
/**/
		fread((char *)errno, sizeof(int), 1, fp) ;
		fread((char *)type, sizeof(int), 1, fp) ;
		fread((char *)size, sizeof(int), 1, fp) ;
	}
}

#ifdef	GETLOGIN

#include	<pwd.h>

char *
getlogin()
{
	extern	struct	passwd	*getpwuid();
	struct	passwd		*pwd;

	if ((pwd = getpwuid(getuid())) == (struct passwd *) NULL)
		return NULL;
	else
		return(pwd->pw_name);
}
#endif	GETLOGIN

#ifdef	TMPFILE
FILE    *
tmpfile()
{
      char     filename[15] ;   
      FILE     *fp;

      strcpy(filename, "/tmp/tagXXXXXX") ;
      if ((fp = fopen(mktemp(filename), "w+")) != NULL)
      {
		unlink(filename);
		return(fp);
      }
      return(NULL);
}
#endif	TMPFILE

/*
 *
 *	getcmdpath.c returns the full path name of the argument cmd
 *	by looking through the environment variable PATH. 
 *	
 *
 */
char  *
getcmdpath(cmd)
char *cmd;
{
    extern char *getenv();
    extern char	*strchr();
    char *path, *cp;
    static  char buf[200];
    char patbuf[512];

    if (cmd == NULL || *cmd == '/')
	return(cmd) ;
	strcpy(patbuf, getenv("PATH"));
	path = patbuf;
	cp = path;

	while(1) {
	    if (path == NULL || *path == '\0') 
		return(NULL) ;
	    if (*path == ':')
		strcpy(buf, cmd);
	    else
	    {
	        cp = strchr(path, ':');
	        if (cp != NULL)
	    	    *cp = '\0';
	        sprintf(buf, "%s/%s", path, cmd);
	    }
	    path = ++cp;

	    if (access(buf, 1) == 0) {
		return(buf) ;
	    }
	}
}
