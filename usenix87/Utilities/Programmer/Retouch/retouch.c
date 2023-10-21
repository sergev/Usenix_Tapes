/* @(#) retouch.c 1.0 10/25/85
*/
/* retouch.c: set update time to match SCCS id

   Copyright (c) 1985, Joel West (ihnp4!gould9!joel)
   License for non-commercial use freely granted as long
   as this notice is retained.

   The id should be of the form
		@(#) xxxxxxx 10/23/85
	or	@(#) xxxxxxx 85/10/23
   it assumes date is the last non-blank thing on the line
   and this program will only work between 1970 and 2000, inclusive

   works best if only one id (ignores others) and if id is at the
   top of the file.

   Does not retouch if not found in first 10 lines; should be
   an optionable parameter.  A line is a newline or a null,
   so this just might work on a binary file.

*/

#include <sys/types.h>
typedef struct {
    time_t actime;
    time_t modtime;
} utimbuf;

#include <sys/stat.h>
#include <errno.h>
#ifdef BSD
extern int errno;
#endif

#include <stdio.h>
#define ungetchar(c) ungetc(c,stdin)

#ifdef BSD
# include <strings.h>
# include <sys/time.h>
#else
# include <string.h>
# include <time.h>
#endif
#ifdef USG
# define index(s,c) strchr(s,c)
#endif

#ifndef TRUE
# define TRUE (1)
# define FALSE (0)
#endif

/* below are hour and minute at local time that should be
   time-stamped; default 00:00:00.  No user control over seconds  */
#define STDHR 00
#define STDMIN 00

#define SCCSKEY "@(#)"
#define SCCSCHR '@'

#define EOL '\n'
#define EOS '\0'

int optg = 0, optd = 0, optv = 0;
long stddate;

main(argc,argv)
int argc;
char **argv;
{	int depthlim,line,i,sccslen,touched;
	char *p,*filenam,buff[1024];
	int c,errind;

	depthlim = 10;
	sccslen = strlen(SCCSKEY);
	errind = 0;

	for (i = 1; i<argc && errind == 0; i++)
	{   if (*argv[i] != '-')
		break;
	    p = argv[i];
	    while (*(++p))
	    switch(*p)
	    {	case 'g':
		    optg++;
		    break;
		case 'v':
		    optv++;
		    break;
		case 'd':
		    if (optd)
			errind++;
		    else
		    {   optd++;
			stddate = dateatol(argv[++i]);
			if (stddate < 0)
			{   fprintf(stderr,"%s: invalid date format\n",argv[i]);
			    exit (0);
			}
		    }
		    break;
		default:
		    errind++;
	    }
	}

	if (errind || i >= argc)
	{   fprintf(stderr, 
		    "usage: %s [-v ] [-g] [-d date] file1 [file2 ... ]\n", 
		    argv[0]);
	    exit (0);
	}

	for (; i<argc; i++)
	{   filenam = argv[i];
	    if (freopen(filenam, "r", stdin) == NULL)
	    {	perror(filenam);
		exit(errno);
	    }
	    touched = 0;
	    if (optd)		/* one date for all */
	    {	setupdtime(filenam, stddate);
		touched++;
		goto settouch;
	    }
	    
	    for (line = 1; line <= depthlim; line++)
	    {	while ( 1 )
		{   c = getchar();
		    if (c == EOF)
			goto settouch;
		    if (c == EOS || c == EOL)
			break;
		    if (c == SCCSCHR)
		    {
			ungetchar(c);
			gets(buff);		/* won't work on binary */
			if (strncmp(buff, SCCSKEY, sccslen) == 0)
			{	p = buff+strlen(buff);	/* use last nonblank */
			    while (*(--p) == ' ')	/* field in string */
				;
			    while (*p != ' ' && *p != '\t')
				--p;
			    if ( setupdtime(filenam, dateatol(++p)) )
				touched++;
			    goto settouch;
			}
		    }
		} /* within line */
	    } /* end of line */
settouch:
	    if (touched==0)
		fprintf(stderr, "%s: not retouched\n", filenam);
	    else
		if (optv)
		    fprintf(stderr, "%s: retouched\n", filenam);
	} /* end of file */
}

/* Reset the file's update time to the specified date.  If the date is dubious,
   don't reset it at all
*/
int setupdtime(fname, mtime)
char *fname;
long mtime;
{	utimbuf timep;
	struct stat statbuf;

	stat(fname, &statbuf);
	timep.actime = statbuf.st_atime;	/* copy current accessed time */

	if (mtime > 0)			/* valid date */
	{   timep.modtime = mtime;
	    utime(fname, &timep);
	    return (TRUE);
	}
	return (FALSE);
}

#define NSECYR 31536000
/* 365*24*60*60 */
#define NSECDA 86400
#define APPROXT(yr,mo,da) (((yr-70)*NSECYR) + ((mo*30)+da)*NSECDA)
#define INVDATE (-1)
long dateatol(string)
char *string;
{	char *p,*q;
	int mdy[3],mo,da,yr,i,num;
	struct tm *tmptr;
	long timsec,approxsec,deltasec,lasttimsec,maxdelta,mindelta;
	int deltasign;
	static char *numstr = "0123456789";

	p = string;
	while (*p == ' ')			/* skip leading blanks */
	    p++;
	for (i = 0 ; i<3; i++)
	{   num = 0;
	    q = NULL;				/* watch for null field */
	    while (*p != EOS && *p != '/' && *p != ' ')
	    {	q = index(numstr,*p++);
		if (q == NULL)			/* invalid digit */
		    break;
		num = num*10 +(q-numstr);
	    }
	    if (q)
		mdy[i] = num;
	    else
		return (INVDATE);		/* invalid or null field */
	    p++;	/* skip delimiter */
	}
	--p;
	while (*p++ == ' ')
	    ;
	if (*--p != EOS)			/* not end of string */
	    return (INVDATE);

	if (mdy[0] > 11)
	{   yr = mdy[0];		/* assume YY/MM/DD */
	    mo = mdy[1] - 1;		/* months 0..11 not 1..12 */
	    da = mdy[2];
	}
	else
	{   mo = mdy[0] - 1;		/* assume MM/DD/YY */
	    da = mdy[1];
	    yr = mdy[2];
	}
	
	if (yr > 1900)
	    yr = yr - 1900;

	approxsec = APPROXT(yr,mo,da);
	timsec = approxsec;
	lasttimsec = 0;
	mindelta = NSECDA;
	maxdelta = NSECYR;
	while (1)
	{    tmptr = (optg ? gmtime(&timsec) : localtime(&timsec)) ;
/* decode absolute time into local or gmt */
	     if (yr == tmptr->tm_year && 
		 mo == tmptr->tm_mon &&
		 da == tmptr->tm_mday)
	     {
/* the date is right, set the time to midnight local time; this 
may behave strangely on daylight savings time changeover days */
		if (STDHR == tmptr->tm_hour &&
		    STDMIN == tmptr->tm_min &&
		    0 == tmptr->tm_sec)
		    break;			/* exactly right */
		else
		{   timsec -= (tmptr->tm_hour - STDHR) * 3600 + 
			      (tmptr->tm_min - STDMIN) * 60 +
			       tmptr->tm_sec;
		    maxdelta = mindelta;
		    mindelta = 1;
		}
	    }
	    else
	    {	deltasec = APPROXT(tmptr->tm_year,tmptr->tm_mon,tmptr->tm_mday)
			   - approxsec;
		deltasign = (deltasec < 0) ? -1 : 1;
/* approximately the difference in the number of seconds between 
   the guessed date and the actual date */
		if (abs(deltasec) > abs(timsec-lasttimsec))
		    deltasec = timsec-lasttimsec;
		deltasec = abs(deltasec);
		deltasec = (deltasec > maxdelta) ? deltasec / 2 :
			((deltasec >= mindelta) ? (deltasec-1) : mindelta); 
		lasttimsec = timsec;
		timsec -= deltasec * deltasign;	/* binary convergence */
	     }
	}

	return (timsec);
}
