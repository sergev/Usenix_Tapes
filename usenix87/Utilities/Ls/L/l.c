/*
**  This program lists all the file names in <directory>.  They
**  are listed in at most five columns.  Directories are listed
**  with a terminating /, executables with a terminating *.
**  The current directory is the default directory.
**
**  usage: l [-a] [directory]
*/

#include <stdio.h>	/* standard i/o package */
#include <sys/types.h>	/* needed for <sys/stat.h> */
#include <sys/stat.h>	/* stat structure declared */
#include <sys/dir.h>	/* direct structure declared */

#define TABSIZ=8
#define MAXCOL=5	/* maximum number of columns = 80/16 */
			/* 80 = number of columns on terminal */
			/* 16 = DIRSIZ + 2 blanks for spacing */

char *malloc();		/* memory allocator - section 3 */

int copyn();		/* copies s2 to s1, returns number copied */
int error();		/* prints an error message */
int namecmp();		/* comparison function for file names */
int qsort();		/* quicker sort - section 3 */

char *progname;

main(argc, argv)
	int	argc;
	char	*argv[];
{
	int	fd, i, aflg;
	int	ncols, nrows, nfiles, nstructs, ntabs;
	int	col, row, filenum, len, pad;
	char 	filename[DIRSIZ+1], *fmt1, *fmt2;
	char	iobuf[BUFSIZ];
	char	*usage = "usage: %s [-a] [directory]\n";
	char	*cannot = "%s: cannot %s %s\n";
	char	*dir, *dot;
	unsigned dirsize;
	struct	direct *head, **headp;
	struct	stat stbuf;

	aflg = 0;
	dir = (char *)0;
	dot = ".";
	progname = argv[0];

	if (argc > 3)
		error(usage, (char *)0, (char *)0);

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-a") == 0 && aflg == 0)
			aflg++;
		else
		if (dir == (char *)0)
			dir = argv[i];
		else
			error(usage, (char *)0, (char *)0);
	}

	if (dir == (char *)0)
		dir = dot;
	else
	if (chdir(dir) == -1)
		error(cannot, "chdir", dir);

	if ((fd = open(dot , 0)) == -1)
		error(cannot, "open", dir);

	if (stat(dot, &stbuf) == -1)
		error(cannot, "stat", dir);

	setbuf(stdout, iobuf);

	dirsize = (unsigned)stbuf.st_size;
	nstructs = dirsize/sizeof(struct direct);
	head = (struct direct *)malloc(dirsize);
	if (read(fd, (char *)head, dirsize) == -1)
		error(cannot, "read", dir);
	headp = (struct direct **)malloc(sizeof(struct direct **) * nstructs);

	/* set up pointers only to the files to be listed */

	if (aflg)
	{
		for (nfiles = i = 0; i < nstructs; i++)
			if (head[i].d_ino != (ino_t)0)
				headp[nfiles++] = &head[i];
	}
	else
	{
		for (nfiles = i = 0 ; i < nstructs; i++)
			if (head[i].d_ino != (ino_t)0 &&
					head[i].d_name[0] != '.')
				headp[nfiles++] = &head[i];
	}

	if (nfiles == 0)
		exit(0);

	/* sort only the pointers */

	qsort((char *)headp, nfiles, (int)sizeof(struct direct **), namecmp);

	nrows  = nfiles / MAXCOL + (nfiles % MAXCOL ? 1 : 0);
	ncols  = nfiles / nrows  + (nfiles % nrows  ? 1 : 0);

	for (row = 0; row < nrows; row++)
	{
		for (col = 0; col < ncols; col++)
		{
			if ((filenum = col*nrows + row) >= nfiles)
				continue;

			len = copyn(filename, headp[filenum]->d_name, DIRSIZ);

			stat(filename, &stbuf);

			if (stbuf.st_mode & S_IFDIR)
			{
				pad = 1;
				fmt1 = "%s/\n";
				fmt2 = "%.*s/%.*s";
			}
			else
			if (stbuf.st_mode & S_IEXEC)
			{
				pad = 1;
				fmt1 = "%s*\n";
				fmt2 = "%.*s*%.*s";
			}
			else
			{
				pad = 2;
				fmt1 = "%s\n";
				fmt2 = "%.*s%.*s";
			}

			if ((col == ncols - 1) || (filenum + nrows >= nfiles))
				fprintf(stdout, fmt1, filename);
			else
			{
				ntabs = ((DIRSIZ+pad-len) > TABSIZ ? 2 : 1);
				fprintf(stdout, fmt2, len,
					filename, ntabs, "\t\t");
			}
		}
		fflush(stdout);	/* write the row out */
	}

	exit(0);
}

/*
**  Sorts the file names in alphabetical order.
*/

int namecmp(pp1, pp2)
	struct direct **pp1, **pp2;
{
	register struct direct *p1 = *pp1;
	register struct direct *p2 = *pp2;

	return(strncmp(&p1->d_name[0], &p2->d_name[0], DIRSIZ));
}

/*
**  Copies s2 to s1 until n characters have been copied
**  or the null of s2 is reached, whichever occurs sooner.
**  Returns the number of non-null characters copied,
**  and s1 is null terminated.
*/

int copyn(s1, s2, n)
	register char *s1, *s2;
	register int n;
{
	register int i;

	for (i = 0 ; i < n; i++)
		if ((*s1++ = *s2++) == '\0')
			return(i);
	*s1 = '\0';
	return(i);
}

error(format, verb, noun)
	char *format, *verb, *noun;
{
	fprintf(stderr, format, progname, verb, noun);
	exit(-1);
}
