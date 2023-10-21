# include       "sps.h"
# include       <h/conf.h>
# include       <h/tty.h>
# include       <sys/stat.h>
# include       <stdio.h>

/* INITTTY - Initialise the tty part of the info structure */
inittty ()
{
	register struct ttyline *lp ;
# ifdef BSD42
	register struct direct  *dp ;
	DIR                     *dfd ;
# else
	struct direct           dir ;
	FILE                    *dfd ;
# endif
	struct stat             statbuf ;
	static char             filedev[] = FILE_DEV ;
	extern struct info      Info ;
	extern int              Flkmem ;
# ifdef BSD42
	DIR                     *opendir() ;
	struct direct           *readdir() ;
# else
	FILE                    *fopen() ;
# endif

	lp = Info.i_ttyline ;
# ifdef BSD42
	if ( !(dfd = opendir( filedev )) )
# else
	if ( !(dfd = fopen( filedev, "r" )) )
# endif
		prexit( "Can't open %s\n", filedev ) ;
	if ( chdir( filedev ) < 0 )
		prexit( "sps - Can't chdir to %s\n", filedev ) ;
# ifdef BSD42
	/* Read all entries in the device directory, looking for ttys */
	while ( dp = readdir( dfd ) )
	{       /* Skip entries that do not match "tty" or "console" */
		if ( strncmp( "tty", dp->d_name, 3 )
		&&   strcmp( "console", dp->d_name ) )
			continue ;
		/* Skip "tty" itself */
		if ( dp->d_namlen == 3 )
			continue ;
# ifdef CHAOS
		/* Skip chaos ttys ; they are accessed during ttystatus() */
		if ( dp->d_namelen > 3 &&
		dp->d_name[ sizeof( "tty" ) - 1 ] == 'C' )
			continue ;
# endif
		if ( lp >= &Info.i_ttyline[ MAXTTYS ] )
			prexit( "sps - Too many ttys in %s\n", filedev ) ;
		/* Copy the tty name into the information entry */
		if ( !strcmp( dp->d_name, "console" ) )
		{
			lp->l_name[0] = 'c' ;
			lp->l_name[1] = 'o' ;
		}
		else
		{
			lp->l_name[0] = dp->d_name[3] ;
			lp->l_name[1] = dp->d_name[4] ;
		}
		/* Ensure that this tty is actually a valid character device */
		if ( stat( dp->d_name, &statbuf ) < 0 )
			continue ;
# else
	/* Read all entries in the device directory, looking for ttys */
	while ( fread( (char*)&dir, sizeof( struct direct ), 1, dfd ) == 1 )
	{       /* Skip entries that do not match "tty" or "console" */
		if ( strncmp( "tty", dir.d_name, 3 )
		&&   strcmp( "console", dir.d_name ) )
			continue ;
		/* Skip "tty" itself */
		if ( dir.d_name[3] == '\0' )
			continue ;
# ifdef CHAOS
		/* Skip chaos ttys ; they are accessed during ttystatus() */
		if ( dir.d_name[ sizeof( "tty" ) - 1 ] == 'C' )
			continue ;
# endif
		if ( lp >= &Info.i_ttyline[ MAXTTYS ] )
			prexit( "sps - Too many ttys in %s\n", filedev ) ;
		/* Copy the tty name into the information entry */
		if ( !strcmp( dir.d_name, "console" ) )
		{
			lp->l_name[0] = 'c' ;
			lp->l_name[1] = 'o' ;
		}
		else
		{
			lp->l_name[0] = dir.d_name[3] ;
			lp->l_name[1] = dir.d_name[4] ;
		}
		/* Ensure that this tty is actually a valid character device */
		if ( stat( dir.d_name, &statbuf ) < 0 )
			continue ;
# endif
		if ( (statbuf.st_mode & S_IFMT) != S_IFCHR )
			continue ;
		/* Find the device # of the tty and the address of its
		   associated struct tty in /dev/kmem. */
		lp->l_dev = statbuf.st_rdev ;
		memseek( Flkmem,
		     (long)&Info.i_cdevsw[ major( statbuf.st_rdev ) ].d_ttys ) ;
		if ( read( Flkmem, (char*)&lp->l_addr, sizeof( lp->l_addr ) )
		!= sizeof( lp->l_addr ) )
		{
			fprintf( stderr, "sps - Can't read struct tty for %s\n",
# ifdef BSD42
				dp->d_name ) ;
# else
				dir.d_name ) ;
# endif
			continue ;
		}
		lp->l_addr += (int)minor( statbuf.st_rdev ) ;
		lp++ ;
	}
# ifdef BSD42
	(void)closedir( dfd ) ;
# else
	(void)fclose( dfd ) ;
# endif
}
