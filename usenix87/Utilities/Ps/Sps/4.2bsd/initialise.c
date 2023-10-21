# include       "sps.h"
# include       "flags.h"
# include       <pwd.h>
# include       <stdio.h>

/*
** INITIALISE - Called to reset the `Info' structure with new kernel
** addresses and user and tty information.
*/
initialise ()
{
	register FILE           *fd ;
	char                    *fileinfo ;
	extern struct flags     Flg ;
	extern struct info      Info ;
	FILE                    *fopen() ;

	fileinfo = Flg.flg_j ? Flg.flg_j : FILE_INFO ;
	/* Read kernel addresses */
	initsymbols() ;                 
	/* Read user names */
	initusers() ;                   
	(void)umask( ~0644 ) ;          
	if ( !(fd = fopen( fileinfo, "w" )) )
	{
		fprintf( stderr, "sps - Can't create info file %s", fileinfo ) ;
		sysperror() ;
	}
	/* Find tty addresses */
	inittty() ;                     
	if ( fwrite( (char*)&Info, sizeof( struct info ), 1, fd ) != 1 )
	{
		fprintf( stderr, "sps - Can't write info file %s", fileinfo ) ;
		sysperror() ;
		exit( 1 ) ;
	}
	(void)fclose( fd ) ;
	printf( "sps is initialised\n" ) ;
}

/* INITUSERS - Read the passwd file and fill in the user name arrays */
initusers ()
{
	register struct passwd  *pw ;
	register struct hashtab *hp ;
	struct passwd           *getpwent() ;
	char                    *strncpy() ;
	struct hashtab          *hashuid(), *hashnext() ;

	while ( pw = getpwent() )
	{       /* For each user in the passwd file, first see if that uid
		   has been already allocated in the hash table. */
		if ( hp = hashuid( pw->pw_uid ) )
		{
			fprintf( stderr,
		   "sps - Names %s and %s conflict in passwd file for uid %d\n",
				hp->h_uname, pw->pw_name, pw->pw_uid ) ;
			continue ;
		}
		/* Try to find a free slot in the hash table and fill it. */
		if ( !(hp = hashnext( pw->pw_uid )) )
			prexit( "sps - Too many users in passwd file\n" ) ;
		hp->h_uid = pw->pw_uid ;
		(void)strncpy( hp->h_uname, pw->pw_name, UNAMELEN ) ;
	}
	(void)endpwent() ;
}
