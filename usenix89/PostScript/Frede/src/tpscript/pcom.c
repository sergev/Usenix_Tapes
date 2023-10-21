/* LINTLIBRARY */
/*
 *	pcom.c
 *
 *	Some common code for programs which generate postscript.
 *
 *	We assume that the lineprinter software brackets jobs with EOF,
 *	and possibly sets jobname etc., so we don't do it ourselves.
 *	The program that calls these routines should do any necessary
 *	scaling first.
 *	We will set the following variables:
 *	/pgtop		top of page 
 *	/stm		job start time in ms
 *	/spg		pages at start of job
 *	/jobname	string - name of current job
 *	We define the following postscript routines:
 *	/mf		enable manual feed
 *	/af		enable auto feed
 *	/ps		print string on output stream
 *	/a4		set a4 page size	(Apple LaserWriter Only)
 *
 */

#include	"pscript.h"
#include	<stdio.h>
#if	VERBOSE
#include	<time.h>
#if	AUSAM
#include	<passwd.h>
#else	AUSAM
#include	<pwd.h>
#endif	AUSAM
#if	SYSV
#include	<sys/utsname.h>
#endif	SYSV
#if	UNSW
#include	<table.h>
#endif	UNSW
#endif	VERBOSE

	/* output stream on which postscript appears */
FILE		*postr = NULL;

long	time();
char	*ctime(),
	*getuser(),
	*systemid();

char	*ptstr[] = {
	"",		/* default - usually determined by paper tray */
	"letter",
	"legal",
	"note",
#ifdef	ALW
	"a4",		/* on Apple LaserWriter ONLY */
#endif	ALW
};

char	*pcom0tab[] = {
	"initmatrix",
#ifdef	ALW
	"/a4 [ [300 72 div 0 0 -300 72 div -52 3436 ] 292 3365",
	"{statusdict /jobstate (printing) put 0 setblink",
	"margins exch 157 add exch 245 add 8 div round cvi frametoroket",
	"statusdict /jobstate (busy) put 1 setblink} /framedevice load",
	"60 45 {dup mul exch dup mul add 1.0 exch sub} /setscreen load",
	"{} /settransfer load /initgraphics load /erasepage load ] cvx",
	"statusdict begin bind end readonly def",
#endif	ALW
	(char *)0
};

char	*pcom1tab[] = {
#if	VERBOSE
	/* job start time in ms */
	"/stm usertime def",
	/* remember total pages used */
	"/pgc statusdict begin pagecount end def",
#endif	VERBOSE
	/* move origin up page so bottom is bottom of imageable region */
	"clippath pathbbox pop pop exch pop 0 exch translate",
	/* save top of page */
	"clippath pathbbox /pgtop exch def pop pop pop",
#if	VERBOSE
	/* routine to print strings on output stream */
	"/ps { print flush } def",
#endif
	/* routine to print a page and begin a new one */
	/* the restore/save pair is important - it ensures that VM garbage
	 * collection is done at least once every page. The user program
	 * (ie lpscript, tpscript, etc.) MUST call endinit() as the
	 * last part of its initialisation - this prints an initial
	 * "save".
	 */
	"/page { copypage erasepage restore save home } def",
	/* routine to initialise a path */
	"/home { newpath 0 pgtop moveto } def",
	/* routines to select manual or auto feed */
	"/mf { statusdict /manualfeed true put",
#ifdef	ALW
	/* fix for manual feed bug (see p. 29 Appendix D Inside LaserWriter) */
	" usertime 5000 add { dup usertime lt { pop exit } if } loop",
#endif	ALW
	" } def",
	"/af { statusdict /manualfeed false put } def",
	(char *)0
};

pcomminit(scale, rotation, papertype, manualfeed, font, title, creator)
float	rotation,
	scale;
int	papertype;
bool	manualfeed;
char	*font,
	*title,
	*creator;
{
	register char	**ptab;
	long		clock;
#if	VERBOSE
	char		*user;
	char		jobname[100];
#endif	VERBOSE

	fprintf(postr, "%%!PS-Adobe-1.0\n");
	time(&clock);
#if	VERBOSE
	user = getuser();
	if(title)
		strcpy(jobname, title);
	else
	{
		strcpy(jobname, user);
		strcat(jobname, "/");
		strcat(jobname, creator);
	}
	fprintf(postr, "%%%%Title: %s\n", jobname);
#endif	VERBOSE
	fprintf(postr, "%%%%DocumentFonts: %s\n",
		font ? font : "");
	fprintf(postr, "%%%%Creator: %s\n", creator);
	fprintf(postr, "%%%%CreationDate: %s", ctime(&clock));
	fprintf(postr, "%%%%Pages: (atend)\n");
#if	VERBOSE
	fprintf(postr, "%%%%For: %s\n", user);
#endif	VERBOSE
	fprintf(postr, "%%%%EndComments\n");
	ptab = pcom0tab;
	while(*ptab)
		fprintf(postr, "%s\n", *ptab++);
	fprintf(postr, "%s\n", ptstr[papertype]);
	if(rotation != PD_ROTATION)
		fprintf(postr, "%.1f rotate\n", rotation);
	if(scale != 1.0 && scale != 0.0)
		fprintf(postr, "%.4f dup scale\n", scale);
	ptab = pcom1tab;
	while(*ptab)
		fprintf(postr, "%s\n", *ptab++);
#if	VERBOSE
	fprintf(postr, "/jobname (%s) def\n", jobname);
	fprintf(postr, "userdict /jobname jobname put\n");
	fprintf(postr, "( :: '%s' :: job starts\\n) ps\n", jobname);
#endif	VERBOSE
	fprintf(postr, "%s\n", manualfeed ? "mf" : "af");
}

endinit()
{
	/* this save is for the restore/save pairs in page */
	fprintf(postr, "\nsave\n");
	/* all variable assignments are now local to a page */

	/* initialise current path - move to top of page */
	fprintf(postr, "home\n");
	fprintf(postr, "%%%%EndProlog\n");
}

pch(ch)
int	ch;
{
	if(ch < ' ' || ch > '~')
		fprintf(postr, "\\%03o", ch);
	else
	{
		if(ch == '(' || ch == ')' || ch == '\\')
			putc('\\', postr);
		putc(ch, postr);
	}
}

pcommfinish(pages, fonts)
int pages;
char *fonts;
{
	fprintf(postr, "\n%%%%Trailer\n");
#if	VERBOSE
	fprintf(postr, "jobname ps (: Job finished:\\n) ps\n");
	fprintf(postr, "(\\ttime (s) = ) ps usertime stm sub 1000 div ==\n");
	fprintf(postr, "(\\tpages = ) ps statusdict begin pagecount end pgc sub == flush\n");
	if(pages >= 0)
		fprintf(postr, "%%%%Pages: %d\n", pages);
#endif	VERBOSE
	if(fonts)
		fprintf(postr, "%%%%DocumentFonts: %s\n");
}

#if	VERBOSE
char *
getuser()
{
	char		*lname;
	static char	username[100];
#if	AUSAM
	char		sbuf[SSIZ];
	struct pwent	pe;
	extern int	getpwlog();

	pe.pw_limits.l_uid = getuid();
	if(getpwlog(&pe, sbuf, sizeof(sbuf)) == PWERROR)
		lname = (char *)0;
	else
		lname = pe.pw_strings[LNAME];
	pwclose();
	if(lname == (char *)0)
		lname = "?";
	strcpy(username, lname);

#else	/* ! AUSAM */
	extern	char		*getlogin();
	extern	struct passwd	*getpwuid();
	struct	passwd		*pwdp;

	if ( ( lname = getlogin() ) != NULL )
		strcpy( username, lname );
	else
	{
		if ( ( pwdp = getpwuid( getuid() ) ) == (struct passwd *)0 )
			sprintf( username, "User%d", getuid() );
		else
			strcpy( username, pwdp->pw_name );
	}
#endif	/* ! AUSAM */
	strcat(username, "@");
	strcat(username, systemid());
	return(username);
}

char *
systemid()
{
	static char	sysname[100] = "";

#if	UNSW
	getaddr(G_SYSNAME, sysname);
#endif	UNSW

#if	BSD
	gethostname( sysname, sizeof(sysname) );
#endif	BSD

#if	SYSV
	struct utsname	un;

	uname(&un);
	strcpy(sysname, un.nodename);
#endif	SYSV

	return(sysname);
}
#endif	VERBOSE
