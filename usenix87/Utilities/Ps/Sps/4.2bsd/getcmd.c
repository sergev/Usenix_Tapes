# include       "sps.h"
# include       "flags.h"
# include       <h/vm.h>
# ifdef BSD42
# include	<machine/pte.h>
# else
# include       <h/pte.h>
# endif

/*
** GETCMD - Returns a character string read from a process' upage.
** This character string should represent the arguments to the current process.
*/
char    *getcmd ( p )

register struct process         *p ;

{
	register int            *ip ;
	register char           *cp ;
	register char           *cp0 ;
	unsigned                nbad ;
	struct dblock           db ;
	struct pte              ptetbl[ UPAGES + CLSIZE ] ;
	union
	{
		char            a_argc[ CLSIZE * NBPG ] ;
		int             a_argi[ CLSIZE * NBPG / sizeof( int ) ] ;
	} argbuf ;
	extern struct flags     Flg ;
	extern union userstate  User ;
	extern int              Flmem, Flswap ;
	char                    *strcat(), *strncpy(), *strsave() ;

	p->pr_csaved = 0 ;
	p->pr_upag = 0 ;
	if ( p->pr_p.p_stat == SZOMB )
		return ( "** Exit **" ) ;
	if ( !(p->pr_p.p_flag & SLOAD) && Flg.flg_o )
		return ( "** Swapped out **" ) ;
	/* Find the process' upage */
	if ( !getupage( p, ptetbl ) )           
		return ( "** No upage **" ) ;
	/* Is this a system process ? */
	if ( p->pr_p.p_flag & SSYS )            
		switch ( p->pr_p.p_pid )
		{
			case 0 :
				p->pr_upag = 1 ;
				return ( "Unix Swapper" ) ;
			case 2 :
				p->pr_upag = 1 ;
				return ( "Unix Pager" ) ;
			default :
				break ;
		}
	/* Look at the top of the upage to locate the command arguments.
	   The page is loaded if the process itself is loaded and the pte
	   contains is marked as valid. */
	if ( (p->pr_p.p_flag & SLOAD)
	&& !ptetbl[0].pg_fod && ptetbl[0].pg_pfnum )
	{       /* If the page is loaded, read the arguments from
		   physical memory. */
		memseek( Flmem, (long)ctob( ptetbl[0].pg_pfnum ) ) ;
		if ( read( Flmem, argbuf.a_argc, CLSIZE*NBPG ) != CLSIZE*NBPG )
			return ( "** Memory read error **" ) ;
	}
	else                            
	{       /* Otherwise the page is on the swap device */
		vstodb( 0, ctod( CLSIZE ), &User.u_us.u_smap, &db, 1 ) ;
# ifdef BSD42
		swseek( (long)dtob( db.db_base ) ) ;
# else
		swseek( (long)ctob( db.db_base ) ) ;
# endif
		if ( Flg.flg_o )
			return ( "** Swapped page **" ) ;
		if ( read( Flswap, argbuf.a_argc, CLSIZE*NBPG ) != CLSIZE*NBPG )
			return ( "** Swap device read error **" ) ;
	}
	/* Look down until the end of command arguments is found. */
	p->pr_upag = 1 ;
	p->pr_csaved = 1 ;
	ip = &argbuf.a_argi[ CLSIZE*NBPG / sizeof( int ) ] ;
	ip -= 2 ;
	while ( *--ip )
		if ( ip == &argbuf.a_argi[0] )
			goto getsysargs ;
	/* Process the command arguments, looking for nulls and unprintable
	   characters. */
	cp0 = (char*)(ip + 1) ;
	if ( !*cp0 )                    
		cp0++ ;                 
	if ( *cp0 )
	{
		nbad = 0 ;                      
		for ( cp = cp0 ; cp < &argbuf.a_argc[ CLSIZE*NBPG ] ; cp++ )
		{
			*cp &= 0177 ;
			if ( !*cp )             
			{       /* Replace nulls with spaces */
				*cp = ' ' ;
				continue ;
			}
			if ( *cp < ' ' || *cp == 0177 )
			{       /* Replace control characters with ?'s */
				if ( ++nbad > 5 )
				{
					*cp++ = ' ' ;
					break ;
				}
				*cp = '?' ;
				continue ;
			}
			if ( !Flg.flg_e && *cp == '=' )
			{       /* Break on an `=' if we are not interested
				   in the environment strings. */
				*cp = '\0' ;
				while ( cp > cp0 && *--cp != ' ' )
					*cp = '\0' ;
				break ;
			}
		}
		while ( *--cp == ' ' )
			*cp = '\0' ;
		return ( strsave( cp0 ) ) ;
	}
getsysargs :
	/* If the command arguments cannot be accessed from the user's memory
	   space, get the command name from the system's idea of what the
	   name should be. */
	argbuf.a_argc[0] = '(' ;
	(void)strncpy( &argbuf.a_argc[1], User.u_us.u_comm,
		sizeof( User.u_us.u_comm ) ) ;
	(void)strcat( &argbuf.a_argc[0], ")" ) ;
	return ( strsave( argbuf.a_argc ) ) ;
}

/*
** VSTODB - Given a base/size pair in virtual swap area,
** return a physical base/size pair which is the
** (largest) initial, physically contiguous block.
/* This code is stolen from the kernel file /sys/sys/vm_drum.c.
*/
vstodb ( vsbase, vssize, dmp, dbp, rev )

register int                    vsbase ;
register int                    vssize;
struct dmap                     *dmp ;
register struct dblock          *dbp ;
int                             rev ;

{
	register int            blk ;
	register swblk_t        *ip ;
# ifdef BSD42
	extern struct info      Info ;
# endif

# ifdef BSD42
	blk = Info.i_dmmin ;
# else
	blk = DMMIN ;
# endif
	ip = dmp->dm_map ;
	while ( vsbase >= blk )
	{
		vsbase -= blk ;
# ifdef BSD42
		if ( blk < Info.i_dmmax )
# else
		if ( blk < DMMAX )
# endif
			blk *= 2 ;
		ip++ ;
	}
	dbp->db_size = vssize < blk - vsbase ? vssize : blk - vsbase ;
	dbp->db_base = *ip + (rev ? blk - (vsbase + dbp->db_size) : vsbase);
}
