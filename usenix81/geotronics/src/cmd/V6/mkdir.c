#
/*
 * mkdir.c - make a directory
 *
 *	modified 20-Sep-1980 by D A Gwyn:
 *	1) created (replaces mkdir.s); sets GID as well as UID;
 *	2) return exit code.
 */

#define	DEFMODE	0750			/* default protection mode */

main( argc , argv )
	int		argc;
	char		*argv[];
	{
	register char	*argp;
	char		*bp1, *bp2, buf1[100], buf2[100], c, *dir, *np;
	int		errflag, gid, uid;
	struct statbuf
		{
		char	minor;
		char	major;
		int	inumber;
		int	flags;
		char	nlinks;
		char	uid;
		char	gid;
		char	size0;
		int	size1;
		int	addr[8];
		int	actime[2];
		int	modtime[2];
		}	stbuf;
#define	OWNWRITE	0200
#define	GRPWRITE	0020
#define	OTHWRITE	0002

	errflag = 0;			/* records failures */
	uid = getuid() & 0377;
	gid = getgid() & 0377;

	while ( --argc )
		{
		bp1 = buf1;
		bp2 = buf2;
		np = 0;			/* no '/' seen yet */
		argp = (++argv)[0];
		while ( c = *argp++ )
			{
			*bp1++ = c;
			*bp2++ = c;
			if ( c == '/' )
				np = bp2;	/* last '/' so far */
			}
		*bp1++ = '/';
		*bp1++ = '.';
		*bp1 = 0;		/* name/. */
		if ( np )		/* name had '/' in it */
			{
			dir = buf2;
			if ( --np != buf2 )
				*np = 0;	/* replaces '/' */
			else
				*++np = 0;	/* dir is "/" */
			}
		else
			dir = ".";
		if ( uid )		/* not super-user */
			{
			if ( stat( dir , &stbuf ) == -1 )
				{
				printf( "mkdir: %s not found\n" , dir );
				errflag =| 1;
				continue;
				}
			if ( ! (uid == stbuf.uid && (stbuf.flags & OWNWRITE)
			     || gid == stbuf.gid && (stbuf.flags & GRPWRITE)
			     || (stbuf.flags & OTHWRITE)) )
				{
				printf( "mkdir: %s not writable\n" , dir );
				errflag =| 2;
				continue;
				}
			}
		argp = argv[0];
		if ( mknod( argp , DEFMODE | 0140000 , 0 ) == -1 )
			{
			printf( "mkdir: mknod failed on %s\n" , argp );
			errflag =| 4;
			continue;
			}
		chown( argp , uid , gid );
		if ( link( argp , buf1 ) == -1 )
			{
			printf( "mkdir: link %s failed\n" , buf1 );
			errflag =| 8;
			continue;
			}
		*bp1++ = '.';
		*bp1 = 0;		/* name/.. */
		if ( link( dir , buf1 ) == -1 )
			{
			printf( "mkdir: link %s failed\n" , buf1 );
			errflag =| 16;
			}
		}
	exit( errflag );
	}
