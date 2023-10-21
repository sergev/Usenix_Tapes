/*
**	ftimes [-acmv] file
**	newer [-acmntvw] file1 file2
**	older [-acmntvw] file1 file2
**
**	Output the access, change and modify times of the file.
**	If no flag is selected, the default is the modify time.
**	The v flag chooses a human readable (ctime) format.
**
**	If invoked as "newer (older) file1 file2", then the program will
**	return 0 status (true) iff the relation file1 R file2 holds, else 1.
**	The comparison is done on the first specified time, or the
**	modify time, if defaulted.
**	e.g.
**		newer -cam f1 f2 compares on the creation time while
**		newer -acm f1 f2 compares on the access time
**
**	In addition, if the t flag is set, the newer (older) of the two
**	sets of times is output if the relation holds. The n flag does a
**	similar thing for the filename.
**
**	The w flag causes the winner's details to be output, regardless of
**	whether the relation was true.
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
*/

#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<time.h>

static int	aflag	= 0,		/* display access time */
		cflag	= 0,		/* display change time */
		mflag	= 0,		/* display modify time */
		prname	= 0,		/* print name after true comparison */
		prtime	= 0,		/* print time after true comparison */
		verbose = 0,		/* display in human readable format */
		winner	= 0;		/* print stats of winner */
static char	criterion	= ' ';	/* which time to compare */

main(argc, argv)
	int		argc;
	char		*argv[];
{
	register int	n, errflag = 0;
	register char	*progname;
	extern int	optind;		/* defined in getopt */
	extern char	*optarg;	/* defined in getopt */
	int		getopt(), stat();
	char		*rindex();

	if ((progname = rindex(argv[0], '/')) == NULL)
		progname = argv[0];
	else
		progname++;
	while ((n = getopt (argc, argv, "acmntvw")) != EOF)
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
			verbose++;
			break;
		case 'w':
			winner++;
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
	if (strcmp(progname, "newer") == 0)
	{
		if(errflag || argc != 2)
		{
			fprintf(stderr, "usage: %s [-acmntvw] file1 file2\n", progname);
			exit(2);
		}
		if ((n = newer(argv[0], argv[1])) || winner)
		{
			if (prtime)
				ftimes(n ? argv[0] : argv[1]);
			if (prname)
				printf("%s\n", n ? argv[0] : argv[1]);
		}
		exit (n ? 0 : 1);		/* 0 is true to shell */
	}
	else if (strcmp(progname, "older") == 0)
	{
		if(errflag || argc != 2)
		{
			fprintf(stderr, "usage: %s [-acmntvw] file1 file2\n", progname);
			exit(2);
		}
		if ((n = newer(argv[1], argv[0])) || winner)
		{
			if (prtime)
				ftimes(n ? argv[0] : argv[1]);
			if (prname)
				printf("%s\n", n ? argv[0] : argv[1]);
		}
		exit (n ? 0 : 1);		/* 0 is true to shell */
	}
	else					/* plain ftimes */
	{
		if (errflag || argc != 1)
		{
			fprintf(stderr, "usage: %s [-acmv] file\n", progname);
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
		mflag = 1;			/* at least mod time */
	if (aflag) {
		if(verbose)
			printf("ACCESS TIME = %s", ctime(&sb.st_atime));
		else
			printf("%ld", sb.st_atime);
	}
	if (cflag) {
		if(verbose)
			printf("CHANGE TIME = %s", ctime(&sb.st_ctime));
		else
			printf("%ld", sb.st_ctime);
	}
	if (mflag) {
		if(verbose)
			printf("MODIFY TIME = %s", ctime(&sb.st_mtime));
		else
			printf("%ld", sb.st_mtime);
	}
	if (!verbose)
		printf("\n");
	return (0);
}
