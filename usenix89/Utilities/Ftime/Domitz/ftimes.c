/*
**	ftimes [-acmvf"format"][+format] file
**	newer [-acmntvwf"format"][+format] file1 file2
**	older [-acmntvwf"format"][+format] file1 file2
**
**	Output the access, change and modify times of the file.
**	If no flag is selected, the default is the modify time.
**	The v flag chooses a human readable format.
**
**	If invoked as "newer (older) file1 file2", then the program 
**	will return 0 status (true) iff the relation file1 R file2 
**	holds, else 1.  The comparison is done on the first speci-
**	fied time, or the modify time, if defaulted.
**	e.g.
**		newer -cam f1 f2 compares on the creation time while
**		newer -acm f1 f2 compares on the access time
**
**	In addition, if the t flag is set, the newer (older) of the 
**	two sets of times is output if the relation holds. The n 
**	flag does a similar thing for the filename.
**
**	The w flag causes the winner's details to be output, regard-
**	less of whether the relation was true.
**
**	A return status of 2 indicates problems, like bad options or
**	file not found.
**
**	This is public domain software. It may be freely distributed,
**	modified and used. Please use it for peaceful purposes
**	and also not for profit.
**
**	Bug reports and comments to:
**
**	Ken Yap (ken@rochester.{arpa,uucp})
**
**	I do not promise any sort of support or warrant the
**	suitability of this software for any purpose whatsoever.
**
**	Last modified:
**
**	Ken Yap (ken@rochester.{arpa,uucp})	26th Sep 1985
**
**	Added verbose -v option to output in human readable format
**	John Korsmeyer (sol1!john)		9th Nov 1985
**
**	Added code to do comparisons.
**	Ken Yap (ken@rochester.{arpa,uucp})	10th Feb 1986
**
**	Added code to format the date in the same way the date(1)
**	formats date information.  I 'borrowed' it from the mtime
**	routine written by "jchvr@ihlpg" and revised by Rich Salz
**	(rs@mirror).
**	Robert O. Domitz (...!petsd!pecnos!rod)	19th Feb 1986
*/

#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<time.h>

static int	aflag	= 0,	/* display access time */
		cflag	= 0,	/* display change time */
		mflag	= 0,	/* display modify time */
		prname	= 0,	/* print name after true comparison */
		prtime	= 0,	/* print time after true comparison */
		fmtflag	= 0,	/* display format provided */
		verbose = 0,	/* display in human readable format */
		winner	= 0;	/* print stats of winner */
static char	criterion= ' ';	/* which time to compare */
static	char	*format = "%a %h %d %H:%M:%S 19%y";

main(argc, argv)
	int		argc;
	char		*argv[];
{
	register int	n, errflag = 0;
	register char	*progname;
	extern int	optind;		/* defined in getopt */
	extern char	*optarg;	/* defined in getopt */
	int		getopt(), stat();
	char		*strrchr();

	if ((progname = strrchr(argv[0], '/')) == NULL)
		progname = argv[0];
	else
		progname++;
	while ((n = getopt (argc, argv, "acmntvwf:")) != EOF)
	{
		switch (n)
		{
		case 'a':
			if (criterion == ' ')
				criterion = 'a';
			aflag++;
			break;
		case 'c':
			if (criterion == ' ')
				criterion = 'c';
			cflag++;
			break;
		case 'm':
			if (criterion == ' ')
				criterion = 'm';
			mflag++;
			break;
		case 'n':
			prname++;
			break;
		case 't':
			prtime++;
			break;
		case 'v':
			verbose++;	/* special message format */
			prname++;	/* print the name */
			prtime++;	/* print the time */
			break;
		case 'w':
			winner++;
			break;
		case 'f':
			format = optarg;
			fmtflag++;
			break;
		default:
			errflag++;
			break;
		}
	}
	if (criterion == ' ')
		criterion = 'm';

	argc -= optind;
	argv += optind;
	if (**argv == '+') { /* Process format from + string */
		format = *argv;
		format++;	/* Skip the leading '+' */
		fmtflag++;

		argc--;
		argv++;
	}

	if (strcmp(progname, "newer") == 0)
	{
		if(errflag || argc != 2)
		{
			fprintf(stderr,
				"usage: %s [-acmntwvf'format'] file1 file2\n",
				progname);
			fprintf(stderr,
				"or:	%s [-acmntwv][+format] file1 file2\n",
				progname);
			exit(2);
		}
		if ((n = newer(argv[0], argv[1])) || winner)
		{
			if (prtime)
				ftimes(n ? argv[0] : argv[1]);
			if (prname)
				printf("%s\n", n ? argv[0] : argv[1]);
		}
		exit (n ? 0 : 1);	/* 0 is true to shell */
	}
	else if (strcmp(progname, "older") == 0)
	{
		if(errflag || argc != 2)
		{
			fprintf(stderr,
				"usage: %s [-acmntwvf'format'] file1 file2\n",
				progname);
			fprintf(stderr,
				"or:	%s [-acmntwv][+format] file1 file2\n",
				progname);
			exit(2);
		}
		if ((n = newer(argv[1], argv[0])) || winner)
		{
			if (prtime)
				ftimes(n ? argv[0] : argv[1]);
			if (prname)
				printf("%s\n", n ? argv[0] : argv[1]);
		}
		exit (n ? 0 : 1);	/* 0 is true to shell */
	}
	else					/* plain ftimes */
	{
		if (errflag || argc != 1)
		{
			fprintf(stderr,
				"usage: %s [-acmvf'format'] file\n",
				progname);
			fprintf(stderr,
				"or:	%s [-acmv][+format] file\n",
				progname);
			exit(2);
		}
		exit(ftimes(argv[0]));
	}
}

int newer(file1, file2)
	char		*file1, *file2;
{
	struct stat	sb1, sb2;

	if (stat(file1, &sb1) < 0)
	{
		perror(file1);
		exit (2);
	}
	if (stat(file2, &sb2) < 0)
	{
		perror(file2);
		exit (2);
	}
	switch (criterion)
	{
	case 'a':
		return (sb1.st_atime > sb2.st_atime);
		break;
	case 'c':
		return (sb1.st_ctime > sb2.st_ctime);
		break;
	case 'm':
	default:
		return (sb1.st_mtime > sb2.st_mtime);
		break;
	}
	/*NOTREACHED*/
}

int ftimes(file)
	char		*file;
{
	struct stat	sb;

	if (stat(file, &sb) < 0)
	{
		perror(file);
		return (2);
	}
	if (!aflag && !cflag)
		mflag = 1;		/* at least mod time */
	if (aflag) {
		if(verbose | fmtflag) {
			if (verbose) printf("Access time = ");
			prformat(format, localtime(&sb.st_atime));
		} else
			printf("%ld", sb.st_atime);
	}
	if (cflag) {
		if(verbose | fmtflag) {
			if (verbose) printf("Change time = ");
			prformat(format, localtime(&sb.st_ctime));
		} else
			printf("%ld", sb.st_ctime);
	}
	if (mflag) {
		if(verbose | fmtflag) {
			if (verbose) printf("Modify time = ");
			prformat(format, localtime(&sb.st_mtime));
		} else
			printf("%ld", sb.st_mtime);
	}
	if (!verbose && !fmtflag)
		printf("\n");
	return (0);
}
/*H prformat */
/*
**	Originally written by jchvr@ihlpg,
**	"HFVR VERSION=Thu Mar 21 13:29:02 1985".
**	Rewritten by Rich $alz (rs@mirror), 10-Jan-86.
**	"Do with me what you will."
*/


prformat(p, T)
    register char	*p;
    register struct tm	*T;
{
    static char	DAY[] = "SunMonTueWedThuFriSat";
    static char	MON[] = "JanFebMarAprMayJunJulAugSepOctNovDec";

    for (p; *p; p++)
	if (*p != '%')
	    printf("%c", *p);
	else
	    switch (*++p) {
		default:
		    printf("%c", *p);
		case '\0':
		    break;
		case 'n':
		    printf("\n");
		    break;
		case 't':
		    printf("\t");
		    break;
		case 'm':
		    printf("%.2d", T->tm_mon + 1);
		    break;
		case 'd':
		    printf("%.2d", T->tm_mday);
		    break;
		case 'y':
		    printf("%.2d", T->tm_year);
		    break;
		case 'D':
		    printf("%.2d/%.2d/%.2d",
			   T->tm_mon + 1, T->tm_mday, T->tm_year);
		    break;
		case 'H':
		    printf("%.2d", T->tm_hour);
		    break;
		case 'M':
		    printf("%.2d", T->tm_min);
		    break;
		case 'S':
		    printf("%.2d", T->tm_sec);
		    break;
		case 'T':
		    printf("%.2d:%.2d:%.2d",
			T->tm_hour, T->tm_min, T->tm_sec);
		    break;
		case 'j':
		    printf("%.3d", T->tm_yday + 1);
		    break;
		case 'w':
		    printf("%.2d", T->tm_wday);
		    break;
		case 'a':
		    printf("%3.3s", &DAY[T->tm_wday * 3]);
		    break;
		case 'h':
		    printf("%3.3s", &MON[T->tm_mon * 3]);
	    }
    printf("\n");
}


