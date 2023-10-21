/*
 *  U U M A I L
 *  This program when invoked in the form:
 *  uumail host!user will consult the local usemap for
 *  the proper routing.
 * 
 *  If it finds it, it will invoke the proper uux call
 *  to send the mail.
 *  Otherwise it aborts with an error 68 (host unknown)
***************************************************************************
This work in its current form is Copyright 1986 Stan Barber
with the exception of resolve, gethostname and the original getpath which
as far as I know are in the Public Domain. This software may be distributed
freely as long as no profit is made from such distribution and this notice
is reproducted in whole.
***************************************************************************
This software is provided on an "as is" basis with no guarantee of 
usefulness or correctness of operation for any purpose, intended or
otherwise. The author is in no way liable for this software's performance
or any damage it may cause to any data of any kind anywhere.
***************************************************************************
 * $Log:	uumail.c,v $
 * Revision 4.2  87/02/02  15:43:50  sob
 * Added fix for lost line at the beginning of the message problems
 * 
 * Revision 4.1  86/12/15  13:14:44  sob
 * Added attempted fix for null userid in from line on some system V machines.
 * 
 * Revision 4.0  86/11/17  16:02:36  sob
 * Release version 4.0 -- uumail
 * 
 * Revision 3.16  86/10/28  23:26:05  sob
 * installed header bug fix from steve@jplgodo
 * 
 * Revision 3.15  86/10/20  15:05:52  sob
 * Ready for beta test
 * 
 * Revision 3.14  86/10/20  13:24:02  sob
 * Additions to correctly deal with KNOWNHOST
 * 
 * Revision 3.13  86/10/10  18:25:03  sob
 * Now deals with all forms of addresses correctly... close to beta release now
 * 
 * Revision 3.12  86/10/07  13:22:23  sob
 * Made more changes to be compatilble with new Configure script
 * 
 * Revision 3.11  86/10/07  01:07:33  sob
 * Correction for LOG definition
 * 
 * Revision 3.10  86/10/07  01:02:19  sob
 * Altered to work correctly with new Configure script
 * 
 * Revision 3.9  86/10/06  15:03:04  sob
 * Fixed another problem with getting neighbors in getpath.
 * 
 * Revision 3.8  86/10/01  15:52:08  sob
 * Reveresed sense of nghborflag to make it work right.
 * 
 * Revision 3.7  86/09/22  17:52:36  sob
 * This version appears to work when using and not using resolve.
 * Now to alpha test
 * 
 * Revision 3.6  86/09/22  15:58:19  sob
 * This version works correctly with resolve in the compilation. Now
 * to check the other alternatives.
 * Stan
 * 
 * Revision 3.5  86/09/22  12:44:24  sob
 * Added KNOWNHOST definition to allow a place to punt unresolved mail.
 * Hopefully this will help sites with small databases
 * 
 * Revision 3.4  86/09/02  23:32:14  sob
 * Now works with resolve... need to clean up for release.
 * Stan
 * 
 * Revision 3.3  86/08/18  14:13:16  sob
 * checkpoint
 * 
 * Revision 3.2  86/07/11  17:59:19  sob
 * This version now adapted to work with the uumail package.
 * Thanks to Mark and the UUCP project for their work on smail!
 * Stan
 * 
 * Revision 3.1  86/05/27  15:01:27  sob
 * Added modification suggested by tp@ndm20.UUCP to allow user 
 * to specify if uuname will be used to determine uucp neighbors.
 * Stan Barber
 * 
 * Revision 3.0  86/03/14  12:05:00  sob
 * Release of 3/15/86 --- 3rd Release
 * 
 * Revision 2.20  86/03/14  11:57:46  sob
 * 
 * 
 * Revision 2.19  86/03/11  11:29:11  sob
 * Added Copyright Notice
 * 
 * Revision 2.18  86/03/04  18:20:40  sob
 * Fixed some problems with local vs. non-local mail.
 * 
 * Revision 2.17  86/02/26  03:06:47  sob
 * Added error checking for a null name.
 * 
 * Revision 2.16  86/02/23  23:49:40  sob
 * 
 * 
 * Revision 2.15  86/02/23  23:19:09  sob
 * This version will hopefully work with the new pipeoutput option from 
 * aliasing.
 * Stan
 * 
 * Revision 2.14  86/02/18  02:56:38  sob
 * Correct pointer problem with linked list.
 * Stan
 * 
 * Revision 2.13  86/02/17  18:43:37  sob
 * Updated with linked list for addresses. This will allow aliasing which
 * will be added next.
 * Stan
 * 
 * Revision 2.12  86/02/17  17:33:12  sob
 * This update incorporates changes to the command line flags to
 * conform more with the syntax of sendmail.
 * Stan
 * 
 * Revision 2.11  86/02/07  16:06:16  sob
 * Altered the code to always unlink the temporary letter file when
 * DEBUG is NOT defined.
 * Stan
 * 
 * Revision 2.10  85/12/26  16:50:23  sob
 * Added fixes to allow uupath myhostname to work correctly.
 * Stan
 * 
 * Revision 2.9  85/12/10  20:36:01  sob
 * Added new return flag from getpath EX_TEMPFAIL to signal that the
 * path database is currently being updated.
 * 
 * Revision 2.8  85/12/02  16:51:39  sob
 * Added a fix to cope with percents in addresses returned by opath.
 * Thank to steve@umd-cs.UUCP for the bug report.
 * 
 * Revision 2.7  85/11/18  12:36:48  sob
 * Added the -h option to cause uumail NOT to add a From_ line.
 * 
 * Revision 2.6  85/11/14  20:20:06  sob
 * Added #ifdef DEBUG to allow compiliation with out DEBUG installed
 * 
 * Revision 2.5  85/11/14  20:14:11  sob
 * Another little buggie in the log format...sheesh.
 * 
 * Revision 2.4  85/11/13  15:53:18  sob
 * Reformated the log file a little bit.
 * 
 * Revision 2.3  85/11/08  03:03:51  sob
 * This is the release version.
 * 
 * 
 * Revision 2.2  85/09/30  02:51:18  sob
 * This version uses opath when defined during compile time.
 * With a bit of cleaning up, this is a release version.
 * 
 * Revision 2.1  85/09/30  02:46:06  sob
 * *** empty log message ***
 * 
 * Revision 2.0  85/09/09  18:22:56  UUCP
 * *** empty log message ***
 * 
 * Revision 2.0  85/09/09  18:22:56  UUCP
 * Added flags to conform with sendmail. Also updated the flags it could send
 * to uux to conform with 4.3 uux command.
 * Will add name resolution and header checking.
 * Also will allow multiple addresses per line.
 * 
 * Revision 1.7  85/08/03  00:49:14  UUCP
 * Cleaned up with lint.
 * Stan Barber
 * 
 * Revision 1.6  85/07/11  19:30:00  sob
 * changed PATHSIZE to PATHSIZ to conform with uupath
 * 
 * Revision 1.5  85/07/11  18:08:13  sob
 * This one works both as uumail and uupath!
 * Stan
 * 
 * Revision 1.4  85/07/10  18:35:05  sob
 * moved DBM to getpath
 * Stan Barber
 * 
 * Revision 1.3  85/07/09  01:28:14  sob
 * First attempt to integrate uupath
 * Not successful. Changed PATHALIAS define
 * to DBM... will ultimately alter getpath as well
 * added gethostname call to fill in for local host.
 * 
 * Revision 1.2  85/07/08  05:29:16  sob
 * This one works with pathalias database...
 * need to modify to substitue for uupath.
 * Stan
 * 
 * Revision 1.1  85/07/08  03:11:10  sob
 * Initial revision
 * 
 */
#define _DEFINE

#include "uuconf.h"
#include "patchlevel.h"

EXTERN bool uupath;
extern int      errno;
extern struct passwd *getpwuid();
extern FILE	*popen();
extern char     *ctime();
extern char	*getlogin();
extern char	*index();
extern char	*rindex();
extern char	*malloc();
extern char     *getenv();
extern char     *Alias();
extern bool	nghborflag;
EXTERN char	progname[12];
EXTERN char  *paths;
char * bangpath[BUFSIZ];
char templet[64];
struct mailname addrlist;	/* list of addresses */
int local;


main(argc, argv)
	char **argv;
{
	FILE *out, *tmpf;	/* output to uux, temp file */

	char sender[512];	/* accumulated path of sender */
	char user[NAMESIZ];
	char cmd[2000];
	char **av;
	int i,
            error = 0,
            hopcount = 30;
		
        char c,
           grade = 'C',
	   *path,			/* uupath to the system */
           *p, *q, *r,			/* tmp pointer to argv's */
	   *FullName;
	   bool GrabTo,safecf,NoAlias,startuux,noheader,metoo;
	   extern intsig();
	   struct mailname *lp;
	   int form;

	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		(void) signal(SIGINT, intsig);
	if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
		(void) signal(SIGHUP, intsig);
	(void) signal(SIGTERM, intsig);
	(void) signal(SIGPIPE, SIG_IGN);
	/* initialize Boolean Values */
	startuux = safecf = TRUE;
	metoo=GrabTo=NoAlias=noheader = FALSE;

	for (i = 3; i < 20; i++)
		(void) close(i);
	errno = 0;
	nghborflag = TRUE;	/* use uuname if possible */
	gethostname(Myname,32);
	paths=DATABASE;
#ifdef LOG
	logfile=LOGFILE;
#endif
	FullName=getenv("NAME");
	argv[argc] = NULL;
	av = argv;
	p = rindex(*av, '/');
	if (p++ == NULL)
		p = *av;
	strcpy(progname ,p);
	if(strcmp(p,"uupath") == 0)
		uupath = TRUE;
	while ((p = *++av) != NULL && p[0] == '-')
	{
		switch (p[1])
		{

		  case 'g':   /* set grade */
			grade = p[2];
			break;
	
# ifdef DEBUG
		  case 'd':	/* debug */
			Debug= atoi(&p[2]);
			if (Debug == 0) Debug = 1;
			setbuf(stdout, (char *) NULL);
			(void) printf("Version %s\n", Version);
			break;
# endif DEBUG

		  case 'f':	/* from address */
		  case 'r':	/* obsolete -f flag */
			p += 2;
			if (*p == '\0' && ((p = *++av) == NULL || *p == '-'))
			{
				p = *++av;
				if (p == NULL || *p == '-')
				{
					syserr("No \"from\" person");
					av--;
					break;
				}
			}
			if (from != NULL)
			{
				syserr("More than one \"from\" person");
				break;
			}
			from = p;
			break;

		  case 'F':	/* set full name */
			p += 2;
			if (*p == '\0' && ((p = *++av) == NULL || *p == '-'))
			{
				syserr("Bad -F flag");
				av--;
				break;
			}
			FullName = p;
			break;

		  case 'w':	/* just print the path */
			uupath = TRUE;
			break;
		  case 'h':	/* don't add a From line */
			noheader = TRUE;
			break;
		  case 'N':	/* don't use uuname to get neighbors */
			nghborflag = FALSE;
			break;
		  case 'n':     /* don't alias any addresses */
			NoAlias = TRUE;
			break;
		  case 't':	/* read Header for addresses */
			GrabTo = TRUE;
		        break;
		  case 'o':	/* sendmail-like options flag */
			switch(p[2]){
				case 'm': /* send this message to the sender*/
					metoo=TRUE;
					break;
				case 'c': /* don't connect to expensive
						mailer, or don't start uux*/
					startuux = FALSE;
					break;
				}
			break;
		  case 'b':	/* sendmail-like operation mode flag */
			switch(p[2]){
				case MD_DELIVER: /* deliver the mail */
					uupath = FALSE;
					break;
				case MD_VERIFY: /* verify an address */
					uupath = TRUE;
					break;
				default:
					(void) fprintf(stderr,
					"%s: mode %c not supported\n",
							progname,p[2]);
					exit(EX_USAGE);
				}
			break;
			}
    
		}

    handle = JUSTUUCP;	/* default handling is just uucp */
    if(*av==NULL && GrabTo!= TRUE)
	{
		(void) fprintf(stderr,"Usage: %s [flags] address\n",progname);
		exit(EX_USAGE);
	}


#ifndef NOALIAS
	if (AliasFile == NULL) AliasFile = ALIASFILE;
#endif

/* get login name of the sender... use environment variables if possible */

	if(from==NULL || strlen(from) == 0){
		if (((from = getenv("LOGNAME")) == NULL) || (strlen(from) == 0))
			from = getenv("USER");
		if ((from == NULL) || (strlen(from) == 0))
			from = getlogin();
		if ((from == NULL) || (strlen(from) == 0))
			from = getpwuid(geteuid())->pw_name;
	}

/* if from is still null, set it equal to UUMAIL daemon -- kluge! */

if (from == NULL) from = MAILERDAEMON;

/* If this is not uupath, then there must be a letter */

if (!uupath)
{    
#ifdef DEBUG
	if (Debug) (void) printf("Mail from %s\n",from);
#endif
	/*
	 * Make temporary file for letter
	 * (I wish ACCESS(2) would take care of this better !!)
	 */
	if ((p=getenv("HOME"))== NULL)
		p="/tmp";
	sprintf(&templet[0],"%s/.uumXXXXXX",p);
	mktemp(templet);
	unlink(templet);
	if ((i=open(templet,2)) < 0)
		{
			p="/tmp";
	
			sprintf(&templet[0],"%s/.uumXXXXXX",p);
			mktemp(templet);
			unlink(templet);
		}
	else
		{
			close(i);
			unlink(templet);
		}
#ifdef DEBUG
	if (Debug>2) (void) fprintf(stderr,"Temp file is %s\n",templet);
#endif
	if((tmpf = fopen(templet, "w")) == NULL){
		(void) fprintf(stderr, "%s : can't open %s for writing\n", progname,templet);
		fclose(stdin);
		exit(EX_CANTCREAT);

		}
	while(fgets(lbuf,sizeof lbuf,stdin))
		fputs(lbuf,tmpf);
	(void) fclose(tmpf);
	(void) fclose(stdin);
/* file now saved */
	if((tmpf = fopen(templet, "r")) == NULL){
		(void) fprintf(stderr, "%s : can't open %s for reading\n", progname,templet);
		unlink(templet);
		exit(EX_OSERR);
	}
	
}	
    (void) strcpy(sender, "");

    path = malloc(PATHSIZ);

/* build address list */


    while (*av != NULL && *av != NULL)
		add(*av++,&addrlist);

    if(metoo)   add(from,&addrlist); /* add me to the list too */

    if(!NoAlias) alias();		/* do aliasing if not turned off */
#ifndef NOALIAS				/* process forwarding files */
    for (lp = addrlist.m_next;lp;lp = lp->m_next) forward(lp->m_name);
#endif

    for (lp = addrlist.m_next;lp;lp = lp->m_next) /* mail it out */
    {

    if(strlen(lp->m_name) == 0) continue;
 
    local = 0;
 
    q = p = lp->m_name;
    
    
/* this is uupath command */    
    if (uupath) 
	{
		if ((error = getpath (p, path,paths)) != EX_OK)
			patherror(error,p);
		if (strcmp(path,"%s") == 0) local = 1;
	}
#ifdef RESOLVE
    else 
    {
	/* check for pipe to another program output here */
		if (lp->m_pipe == TRUE){
				strcat(cmd,lp->m_name);
				goto pipeout;
	}
 r = malloc(PATHSIZ);
 strcpy(r,p);
 if (index(p,'@') != NULL) handle = ALL;
/* try one */
    form = resolve(p,path,user);
    if ( (form == LOCAL && path[0] == '\0') 
		|| form == ERROR || form == ROUTE || form == DOMAIN){
		if (user[0] != '\0') strcpy(p,user);
		path[0] = user[0] = '\0';
/* try two */
	        if (index(p,'@') != NULL) handle = ALL;
		form = resolve(p,path,user);
    }
/* we could punt at this point, but let's forward it to a known
   host that might be able to resolve it */

#ifdef KNOWNHOST /* ugly... alternate suggestions welcome */
    if ( (exitstat || form == ERROR)
	 && (index(r,'@') != NULL || index(r,'!') != NULL) ){
	    strcpy(p,KNOWNHOST);
	    strcat(p,"!");
	    strcat(p,r);
	    user[0] = '\0';
	    form = resolve(p,path,user);
	}
#endif

#ifdef DEBUG
    if ((Debug > 1) && (!exitstat)) 
		(void) fprintf(stderr,"resolve returns %s!%s\n",path,user);
#endif

    if (exitstat || form == ERROR )
			 /* no match in pathalias database */
	{
		deadletter(tmpf,local,exitstat,p);
		unlink(templet);
		exit(exitstat);
	}
   

   if (form == LOCAL)
	{
		strcpy(path,Myname);
		local = 1;
	}
    } /* end of else uupath */
#else
	else
	{
	       p = index(q,'!');
	       if (p == NULL) local++;
	       else *p = '\0';
	       if(!local){
			if ((error = getpath (q,path,paths)) != EX_OK)
				patherror(error,q);
			if (*path != '\0' && index(path,'!')) 
					*(rindex(path,'!')+1) = '\0';
	 	        p++;
		        strcat(path,p);
		        p = index(path,'!');
		        if (p != NULL) {
				*p = '\0';
			        p++;
				strcpy(user,p);
		 	}
			else
			{
				strcpy(user,path);
				local++;
			}
		}
	}
#endif

#ifdef DEBUG
       if(Debug>3)
		(void) fprintf(stderr,
			"p = %s path = %s user = %s\n",p, path,user);
#endif

	if (uupath)
		{
			if (*path != '\0') *(rindex(path,'!')+1) = '\0';
			(void) printf("Path to %s:  %susername\n",
							lp->m_name,path);
			continue;
		}
	else
		{
			if (local)
				(void) sprintf(cmd, LOCALMAIL, user);
			else {
				(void) sprintf(cmd,"%s - ",REALUUX);
			if (!startuux)
				strcat(cmd,"-r");
#ifndef NORETURN
			if (from)
				{
					strcat(cmd," -a");
					strcat(cmd,from);
				}
#endif
#ifndef NOGRADE
			if (grade)
				{
					char work[10];
					(void) sprintf(work," -g%c",grade);
					strcat(cmd,work);
				}
#endif		
			if (index(user, '!'))
				{
					char work[100];
					(void)sprintf(work,
					 " %s!rmail \\(%s\\)",path,user);
					strcat(cmd,work);
				}
			else
				{
					char work[100];
					(void) sprintf(work, " %s!rmail %s",
					path,user);
					strcat(cmd,work);
				}
			}
pipeout:
#ifdef DEBUG
	if (Debug) (void) fprintf(stderr,"Command is %s\n",cmd);
#endif
		rewind(tmpf);
#ifdef DEBUG
		if (Debug)
			out = fopen("UUMAIL.TEST","w");
		else
#endif
			out = popen(cmd, "w");
/*		fputs(lbuf, out); */
/* reprocess header ? */
		if (!noheader) Putfrom(tmpf,out);
		while (fgets(lbuf, sizeof lbuf, tmpf))
			fputs(lbuf, out);

/* may not be needed */
		if (local!=0)
			(void) fprintf(out,"\n.\n");

#ifdef DEBUG
		if (Debug)
			i = fclose(out);
		else
#endif
			i = pclose(out);
		if ((i & 0377) != 0)
			{
				(void) fprintf(stderr, "pclose: status 0%o\n", i);
				deadletter(tmpf,local,EX_OSERR);
#ifdef DEBUG
				if (Debug <3)
#endif
						 unlink(templet);
				exit(EX_OSERR);
			}
#ifdef LOG
		maillog(cmd);
#endif
	   }
    }
#ifdef DEBUG
    if (Debug <3)
#endif
                unlink(templet);
exit(EX_OK);
}

/* print an error message on stderr */

syserr(string)
char * string;
{
	(void) fprintf(stderr,"%s\n",string);
}

/* make a unix type From line and send it out the stream */
/* based on smail code (rline subroutine )  to be compatible with RFC 976 */

Putfrom(into,outto)
FILE *into, *outto;
{
	char	*asctime();
	struct	tm *bp, *localtime();
	char	*tp, *zp,*c;
	int	parts,fromflag=0;
	char 	*partv[16];
	char buf[BUFSIZ], addr[PATHSIZ], domain[PATHSIZ], user[NAMESIZ];
	int form;
	extern build();
	long iop, offset;
	(void) strcpy( bangpath,"");  /* zero bang path */
	/* code from smail follows  (Thanks Mark!) */
	for( ;; )
	{
		offset=ftell(into); /* get current position in file */
		if ( fgets( buf, sizeof(buf), into )==NULL )
			break;
		if ( strncmp( "From ", buf, 5 ) 
		    && strncmp( ">From ", buf, 6 ) )
			break;
/*
**  Crack the line apart using ssplit.
*/
		if( c = index( buf, '\n' ) );
			*c = '\0';

 		parts = ssplit( buf, ' ', partv );
/*
**  Tack host! onto the bangpath argument if "remote from host" is present.
*/

		if ( parts > 3 
		    && !strncmp( "remote from ", partv[parts-3], 12 ) )
		{
			(void) strcat( bangpath, partv[parts-1] );
			(void) strcat( bangpath, "!" );
		} 
/*
**  Stuff user name into addr, overwriting the user name from previous 
**  From_ lines, since only the last one counts.  Then rewrite user@host 
**  into host!user, since @'s don't belong in the From_ argument.
*/
		(void) strncpy( addr, partv[1], partv[2]-partv[1]-1 ); 
		addr[partv[2]-partv[1]-1] = '\0';	/* ugh */

		(void) parse( addr, domain, user );
		if(*domain == '\0') {
			form= LOCAL;
		} else {
			form = UUCP;
		}

		build( domain, user, form, addr );

	}
/*
**  Now tack the user name onto the from argument.
*/
	(void) strcat( bangpath, addr );
/*
**  If we still have no from argument, we have junk headers.
**  We use the from name determined at startup time.
*/
	if (bangpath[0] == '\0') strcpy(bangpath,from);

/*
 * Format time
 */
	time(&iop);
	bp = localtime(&iop);
	tp = asctime(bp);
/*	zp = tzname[bp->tm_isdst];*/

/*	sprintf(buf, "%s%s %.16s %.3s %.5s", from, tp, zp, tp+20);*/
 	(void) sprintf(buf, "From %s %.16s %.4s", bangpath, tp, tp+20); 

	if (local == 0){
			strcat(buf," remote from ");
			strcat(buf,Myname);
		   }

	strcat(buf,"\n");
	/* if there is no output file (no headers), exit */
	if (outto == NULL) return;
	(void) write(outto->_file,buf,strlen(buf));
	/* now reset the input file to the beginning of the header
	 * following the "From " lines
	 */
	(void) fseek(into,offset,0);
	(void) fflush(outto);

}

/* end of code derived from smail */


/* go here on a signal we want to catch */
intsig()
{
	unlink(templet);
	exit(EX_OK);
}

/* put command strings in the logfile */

#ifdef LOG

maillog(command)
char * command;
{
	FILE *f;
	char atime[24];
	long clock;
	time (&clock);
	strncpy(atime,ctime(&clock),24);

	if ((f=fopen(logfile,"a")) != NULL)
		{
			(void) fprintf(f,"%s: %s - %s\n",progname,atime,command);
			fclose(f);
		}
}

#endif

/* print a path error and exit */
patherror(error,name)
int error;
char * name;
{
	switch(error){
		case EX_NOHOST:
		(void) fprintf(stderr,"System %s not found in network map.\n",
						 name);
			break;
		case EX_NOINPUT:
			 (void) fprintf(stderr,
				"Database %s could not be opened.\n",paths);
			break;
		case EX_TEMPFAIL:
			 (void) fprintf(stderr,
		"Database %s is being updated.\nTry again later.\n",paths);
			break;
		}
	exit(EX_NOHOST);
}
