#

/*	Line-printer accounting program
 *
 *
 *	This program reads the line-printer accounting file and
 *	outputs the number of pages used by each uid.  No output is
 *	produced when this value is 0
 *
 */

#define	LPFILE	"/usr/adm/pdufile.ep"

extern int fout;
int lpbuf[512];
char name[80];

main(argc, argv)
int	argc;
char    **argv; {

	register int i, nwords, *lpbufr;
	int j, fd;
	char *fname;

	if (argc > 1)
		fname = argv[1];
	else fname = LPFILE;

	if ((fd = open (fname,0)) < 0) {
		printf ("Cannot find file %s\n", fname);
		exit();
		}

	fout = dup (1);
	seek (fd, 0, 0);

	for (j=0; (nwords = (read (fd,lpbuf,1024)+1)/2) > 0; j += 512)
		for (i=0, lpbufr=lpbuf; i < nwords; i++, lpbufr++)
			if (*lpbufr)
				print(i+j,*lpbufr);
	flush();

} /* end MAIN */

print(uid, pages)
register int uid, pages; {

	register char *namer;

	namer = name;

	if (getpw (uid, name))
		*namer = '\0';

	while (*namer++ != ':');
	*--namer = '\0';

	printf("UID %5d: %-10s  %5d\n", uid, name, pages);

} /* end PRINT */
