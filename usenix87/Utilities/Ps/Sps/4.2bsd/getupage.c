# include       "sps.h"
# include       <h/vm.h>
# ifdef BSD42
# include	<machine/pte.h>
# else
# include       <h/pte.h>
# endif
# include       <stdio.h>

/*
** GETUPAGE - Reads the upage for the specified process as well as sufficient
** page tables entries for reading the command arguments. The pte's are read
** into the argument `ptetbl'. The upage is read into the external variable
** `User'. This procedure returns 1 if the upage was successfully read.
*/

# define        usrpt           (Info.i_usrpt)

getupage ( p, ptetbl )

register struct process         *p ;
register struct pte             *ptetbl ;

{
	register int            i ;
	register int            ncl ;
	struct pte              pte ;
	extern struct info      Info ;
	extern union userstate  User ;
	extern int              Flmem, Flkmem, Flswap ;

	/* If the process is not loaded, look for the upage on the swap device*/
	if ( !(p->pr_p.p_flag & SLOAD) )
	{                               
# ifdef BSD42
		swseek( (long)dtob( p->pr_p.p_swaddr ) ) ;
# else
		swseek( (long)ctob( p->pr_p.p_swaddr ) ) ;
# endif
# ifdef SUN
		if ( read( Flswap, (char*)&User.u_us, sizeof( union userstate ))
		!= sizeof( union userstate ) )
# else
		if ( read( Flswap, (char*)&User.u_us, sizeof( struct user ) )
		!= sizeof( struct user ) )
# endif
		{
			fprintf( stderr,
				"sps - Can't read upage of process %d\n",
			    p->pr_p.p_pid ) ;
			return ( 0 ) ;
		}
		return ( 1 ) ;          
	}                               
	/* The process is loaded. Locate the process pte's by reading
	   the pte of their base address from system virtual address space. */
	memseek( Flkmem, (long)&Info.i_usrptmap[ btokmx(p->pr_p.p_p0br)
		+ p->pr_p.p_szpt-1 ] ) ;
	if ( read( Flkmem, (char*)&pte, sizeof( struct pte ) )
	!= sizeof( struct pte ) )
	{
		fprintf( stderr,
		      "sps - Can't read indir pte for upage of process %d\n",
		    p->pr_p.p_pid ) ;
		return ( 0 ) ;
	}                               
	/* Now read the process' pte's from physical memory. We need to access
	   sufficient pte's for the upage and for the command arguments. */
	memseek( Flmem, (long)ctob( pte.pg_pfnum+1 )
		- (UPAGES+CLSIZE)*sizeof( struct pte ) ) ;
	if ( read( Flmem, (char*)ptetbl, (UPAGES+CLSIZE)*sizeof( struct pte ) )
	!= (UPAGES+CLSIZE)*sizeof( struct pte ) )
	{
		fprintf( stderr, "sps - Can't read page table of process %d\n",
			p->pr_p.p_pid ) ;
		return ( 0 ) ;
	}
	/* Now we can read the pages belonging to the upage.
	   Here we read in an entire click at one go. */
	ncl = (sizeof( struct user ) + NBPG*CLSIZE - 1) / (NBPG*CLSIZE) ;
	while ( --ncl >= 0 )            
	{                               
		i = ncl * CLSIZE ;
		memseek( Flmem, (long)ctob( ptetbl[ CLSIZE+i ].pg_pfnum ) ) ;
		if ( read( Flmem, User.u_pg[i], CLSIZE*NBPG ) != CLSIZE*NBPG )
		{
			fprintf( stderr,
				"sps - Can't read page 0x%x of process %d\n",
				ptetbl[ CLSIZE+i ].pg_pfnum, p->pr_p.p_pid ) ;
			return ( 0 ) ;
		}
	}
	return ( 1 ) ;
}
