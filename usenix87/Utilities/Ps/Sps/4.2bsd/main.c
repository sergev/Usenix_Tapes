# include       "sps.h"
# include       "flags.h"
# include       <h/text.h>
# include       <stdio.h>

/* SPS - Show Process Status */
/* J. R. Ward - Hasler AG Bern - 24 May 1985 */
main ( argc,argv )

int                             argc ;
char                            **argv ;

{
	register struct process *plist ;
	register struct process *process ;
	register struct text    *text ;
	int                     flinfo ;
	extern struct flags     Flg ;
	extern struct info      Info ;
	extern int              Flmem ;
	extern int              Flkmem ;
	extern int              Flswap ;
	char                    *getcore() ;
	struct process          *needed(), *mktree() ;

	/* Renice as fast as possible for root (Suggested by Gregorio!mogul) */
	if ( !getuid() )
		(void)nice( -40 ) ;
	/* Decode the flag arguments */
	flagdecode( argc, argv ) ;      
	/* Determine the terminal width */
	if ( !Flg.flg_w && !Flg.flg_N && !Flg.flg_i )
		termwidth() ;
	/* Open the cpu physical memory, kernel virtual memory and swap device*/
	if ( Flg.flg_k )
	{
		Flmem = openfile( Flg.flg_k ) ;
		Flkmem = Flmem ;
	}
	else
	{
		Flmem = openfile( FILE_MEM ) ;
		Flkmem = openfile( FILE_KMEM ) ;
		if ( !Flg.flg_o )
			Flswap = openfile( FILE_SWAP ) ;
	}
	if ( Flg.flg_i )
	{       /* -i flag for info file initialisation */
		initialise() ;          
		exit( 0 ) ;
	}
	/* Read the information file */
	flinfo = openfile( Flg.flg_j ? Flg.flg_j : FILE_INFO ) ;
	if ( read( flinfo, (char*)&Info, sizeof( struct info ) )
	!= sizeof( struct info ) )
	{
		fprintf( stderr, "sps - Can't read info file %s",
			Flg.flg_j ? Flg.flg_j : FILE_INFO ) ;
		sysperror() ;
	}
	(void)close( flinfo ) ;
	/* Find current tty status */
	ttystatus() ;                   
	/* Now that we know the available ttys, decode the flags */
	flagsetup() ;                   
	process = (struct process*)getcore(Info.i_nproc*sizeof(struct process));
	text = (struct text*)getcore( Info.i_ntext * sizeof( struct text ) ) ;
	do
	{       /* Read current process and text status */
		readstatus( process, text ) ;
		/* Select those processes to be listed */
		plist = needed( process, text ) ;
		/* Form a tree of listed processes */
		plist = mktree( process, plist ) ;
		if ( !Flg.flg_N )
		{       /* Print the processes */
			prheader() ;
			printall( plist, 0 ) ;
		}
		prsummary() ;
		(void)fflush( stdout ) ;
		if ( Flg.flg_r )        
		{       /* If repeating, again get tty status */
			ttystatus() ;
			if ( Flg.flg_rdelay )
# ifdef BSD42
				sleep( Flg.flg_rdelay ) ;
# else
				sleep( (int)Flg.flg_rdelay ) ;
# endif
		}
	} while ( Flg.flg_r ) ;
	exit( 0 ) ;
}
