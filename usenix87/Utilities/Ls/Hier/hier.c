/*
 * Show file system hierarchy.
 *
 * Usage: see below.
 *
 * Unlike ls(1), it sorts files across lines rather than down columns.
 * Fixing this would be non-trivial, involving saving filenames until it
 * was time to dump them.
 *
 * Also, due to the behavior of sftw() (like ftw()), it never lists "." and
 * ".." files.
 *
 * Warning:  If you use ftw() instead of sftw(), -a option will stop working.
 *
 * Warning:  If you use ftw() instead of sftw(), a bug will appear.  This is
 * because two calls in a row from ftw() to OneFile() may be made for ordinary
 * files, where the second is in a directory a level above the first one.
 * OneFile() would have to check to see if each ordinary file's path is
 * different than the previous one's, indicating a change of directory level.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <ftw.h>
#ifdef	BSD
#include <sys/dir.h>			/* for DIRSIZ */
#include <strings.h>
#else
#include <sys/ndir.h>			/* for DIRSIZ */
#include <string.h>
#endif	/* BSD */


/*********************************************************************
 * MISCELLANEOUS GLOBAL VALUES:
 */

#define	PROC				/* null; easy to find procs */
#define	FALSE	0
#define	TRUE	1
#define	CHNULL	('\0')
#define	CPNULL	((char *) NULL)
#define	REG	register

char *usage[] = {
    "usage: %s [-adp] [-c columns] [-i indent] [directories...]",
    "-a include directories and files whose names start with \".\"",
    "-d show directories only",
    "-p print filenames packed onto lines, not aligned",
    "-c set width of display (or use COLUMNS env variable; default = 80)",
    "-i set indentation per level; default = 4",
    "Does current directory by default.",
    CPNULL,
};

char	*myname;			/* how program was invoked	*/
int	aflag	= FALSE;		/* -a (all files) option	*/
int	dflag	= FALSE;		/* -d (directories) option	*/
int	pflag	= FALSE;		/* -p (packed filenames) option	*/
int	columns	= 0;			/* from -c option or env	*/
int	indent	= 0;			/* from -i option or default	*/

#define	COLUMNS	80			/* width of display		*/
#define	INDENT	 4			/* per directory level		*/

int	startlen;			/* of current arg (filename)	*/
int	nextcol = 0;			/* column in output line	*/


/************************************************************************
 * M A I N
 *
 * Initialize, then call sftw() for each given filename after clearing
 * global startlen to indicate a new starting file.  When done, if global
 * nextcol != 0 (in the middle of an output line), finish the last line.
 */

PROC main (argc, argv)
	int	argc;
	char	**argv;
{
extern	int	optind;			/* from getopt()	*/
extern	char	*optarg;		/* from getopt()	*/
REG	int	option;			/* option "letter"	*/
	char	*argdef = ".";		/* default argument	*/
	char	*colstr;		/* from environment	*/

	char	*getenv();
	int	OneFile();

/* #define DEPTH (_NFILE - 5)		/* for ftw(), but not sftw() */

/*
 * PARSE ARGUMENTS:
 */

	myname = *argv;

	while ((option = getopt (argc, argv, "adpc:i:")) != EOF)
	{
	    switch (option)
	    {
		case 'a':	aflag	= TRUE;			break;
		case 'd':	dflag	= TRUE;			break;
		case 'p':	pflag	= TRUE;			break;
		case 'c':	columns	= atoi (optarg);	break;
		case 'i':	indent	= atoi (optarg);	break;
		default:	Usage();
	    }
	}

	if (dflag && pflag)
	    Error ("-d and -p don't make sense together");

/*
 * FINISH INITIALIZING:
 */

	if ((columns == 0)				/* no value given */
	 && (((colstr = getenv ("COLUMNS")) == CPNULL)	/* undefined	  */
	  || ((columns = atoi (colstr)) == 0)))	  /* defined null or zero */
	{
	    columns = COLUMNS;		/* use default */
	}

	if (indent == 0)		/* no value given */
	    indent = INDENT;		/* use default	  */

	argc -= optind;			/* skip options	*/
	argv += optind;

	if (argc == 0)			/* no filenames given */
	{
	    argc = 1;
	    argv = & argdef;		/* use default */
	}

/*
 * WALK EACH FILE TREE:
 */

	while (argc-- > 0)
	{
	    startlen = 0;

	    if (sftw (*argv, OneFile, aflag))
		Error ("file tree walk failed for file \"%s\"", *argv);

	    argv++;
	}

	if (nextcol)			/* last line not finished */
	    putchar ('\n');

	exit (0);

} /* main */


/************************************************************************
 * O N E   F I L E
 *
 * Called from sftw() to handle (print) one file, given a filename (starting
 * file plus sub-file part) and ftw() file type.  Always returns zero (all
 * is well, keep going).
 *
 * It's messy because of the need to print multiple non-directory basenames
 * on one line.  Uses global startlen to save time figuring depth beyond
 * starting file.  If currently zero, this is the starting file; print the
 * fullname, on a line alone, with no indent.
 *
 * Use globals startlen, indent, columns, and nextcol.
 */

PROC int OneFile (filename, statp, type)
	char	*filename;	/* name		*/
	struct	stat *statp;	/* info, unused	*/
	int	type;		/* ftw() type	*/
{
REG	char	*basename;	/* part of filename */

/*
 * PRINTING FORMATS (matching ftw() types):
 */

static	char	*FMT_D   = "%s/\n";
static	char	*FMT_DNR = "%s/ (not readable)\n";
static	char	*FMT_NS  = "%s (not stat'able)\n";
static	char	*FMT_F1	 = "%s\n";		/* for starting file */
static	char	*FMT_F2	 = "%-*s";		/* for sub-file	     */

#ifdef	BSD
#define	FILEWIDTH MAXNAMLEN			/* for FMT_F2 */
#else
#define	FILEWIDTH (DIRSIZ + 1)			/* for FMT_F2 */
#endif	/* BSD */
REG	int	filewidth = FILEWIDTH;		/* if ! pflag */

#define	NEWLINE	 { putchar ('\n'); nextcol = 0; }  /* for speed and clarity */

/*
 * OPTIONALLY IGNORE NON-DIRECTORY (even if named as an input argument):
 */

	if (dflag && (type == FTW_F))
	    return (0);

/*
 * HANDLE STARTING FILE:
 */

	if (startlen == 0)
	{
	    startlen = strlen (filename);	/* set for later */

	    if (nextcol)		/* end previous line */
		NEWLINE;		/* sets nextcol == 0 */

	    printf ((type == FTW_D)   ? FMT_D	:
		    (type == FTW_DNR) ? FMT_DNR	:
		    (type == FTW_NS)  ? FMT_NS	: FMT_F1, filename);

	    return (0);
	}

/*
 * SET BASENAME FOR ALL OTHER TYPES:
 */

	basename = ((basename = strrchr (filename, '/')) == CPNULL) ?
		   filename : (basename + 1);	/* past "/" if any */

/*
 * HANDLE NON-DIRECTORY SUB-FILE (print multiple per line):
 */

	if (type == FTW_F)
	{
	    if (pflag)				/* else use preset value */
		filewidth = strlen (basename) + 1;

	    if (nextcol	&& (nextcol + filewidth >= columns))	/* overflow */
		NEWLINE;			/* sets nextcol == 0 */

	    if (nextcol == 0)		/* start new line with indent */
		nextcol = PrintIndent (filename);

	    printf (FMT_F2, filewidth, basename);
	    nextcol += filewidth;
	    return (0);
	}

/*
 * HANDLE DIRECTORY OR OTHER SUB-FILE (print on line by itself):
 */

	if (nextcol)			/* end previous line */
	    NEWLINE;			/* sets nextcol == 0 */

	PrintIndent (filename);

	printf ((type == FTW_D)   ? FMT_D   :
		(type == FTW_DNR) ? FMT_DNR : FMT_NS, basename);

	return (0);

} /* OneFile */


/************************************************************************
 * P R I N T   I N D E N T
 *
 * Given a filename and globals startlen and indent, print the total
 * indentation needed before the name, which is indent times the number of
 * slashes past startlen (which should be >= 1).  Return the indent value.
 */

PROC int PrintIndent (filename)
REG	char	*filename;
{
REG	int	depth = 0;
REG	int	totind;
	int	retval;

	filename += startlen;		/* start of sub-part */

	while (*filename != CHNULL)
	    if (*filename++ == '/')
		depth++;

	retval = totind = indent * depth;

	while (totind-- > 0)
	    putchar (' ');

	return (retval);

} /* PrintIndent */


/************************************************************************
 * U S A G E
 *
 * Print usage messages (char *usage[]) to stderr and exit nonzero.
 * Each message is followed by a newline.
 */

PROC Usage()
{
REG	int	which = 0;		/* current line */

	while (usage [which] != CPNULL)
	{
	    fprintf (stderr, usage [which++], myname);
	    putc ('\n', stderr);
	}

	exit (1);

} /* Usage */


/************************************************************************
 * E R R O R
 *
 * Print an error message to stderr and exit nonzero.  Message is preceded
 * by "<myname>: " using global char *myname, and followed by a newline.
 */

/* VARARGS */
PROC Error (message, arg1, arg2, arg3, arg4)
	char	*message;
	long	arg1, arg2, arg3, arg4;
{
	fprintf (stderr, "%s: ", myname);
	fprintf (stderr, message, arg1, arg2, arg3, arg4);
	putc ('\n', stderr);

	exit (1);

} /* Error */
