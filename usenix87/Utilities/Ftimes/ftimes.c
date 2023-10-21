/*
**	ftimes [-acm] file
**
**	Output the access, change and modify times of the file.
**	If no flag is selected, the default is the modify time.
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
**  Added verbose -v option to output in human readable format
**  John Korsmeyer (sol1!john) 9th Nov 1985
*/

#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<time.h>

static int	aflag	= 0,		/* display access time */
		cflag	= 0,		/* display change time */
		mflag	= 0;		/* display modify time */
		verbose = 0;		/* display in human readable format */

main(argc, argv)
	int		argc;
	char		*argv[];
{
	register int	n, errflag = 0;
	struct stat	sb;
	extern int	optind;		/* defined in getopt */
	extern char	*optarg;	/* defined in getopt */
	int		getopt(), stat();

	while ((n = getopt (argc, argv, "acmv")) != EOF)
	{
		switch (n)
		{
		case 'a':
			aflag++; break;
		case 'c':
			cflag++; break;
		case 'm':
			mflag++; break;
		case 'v':
			verbose++; break;
		default:
			errflag++; break;
		}
	}
	argc -= optind;
	if (errflag || argc != 1)
	{
		fprintf(stderr, "usage: %s [-acm] file\n", argv[0]);
		exit(2);
	}
	argv += optind;
	if (stat(argv[0], &sb) < 0)
	{
		perror(argv[0]);
		exit(1);
	}
	if (!aflag && !cflag)
		mflag = 1;			/* at least mod time */
	if (aflag) {
		if(verbose)
			printf("%s%s", "ACCESS TIME = ", ctime(&sb.st_atime));
		else
			printf("%ld", sb.st_atime);
	}
	if (cflag) {
		if(verbose)
			printf("%s%s", "CHANGE TIME = ", ctime(&sb.st_ctime));
		else
			printf("%ld", sb.st_ctime);
	}
	if (mflag) {
		if(verbose)
			printf("%s%s", "MODIFY TIME = ", ctime(&sb.st_mtime));
		else
			printf("%ld", sb.st_mtime);
	}
	printf("\n");
	exit(0);
}
