# include       "sps.h"
# include       "flags.h"
# include       <stdio.h>
# include       <h/tty.h>
# ifdef CHAOS
# include       <chunix/chsys.h>
# include       <chaos/chaos.h>
# endif

/*
** TTYSTATUS - Reads the kernel memory for tty structures of active processes.
** The addresses of the associated struct ttys of /dev/kmem are kept in the
** info structure. Here we use those addresses to access the structures.
** Actually, we are mostly interested just in the process group of each tty.
*/
ttystatus ()
{
	register struct ttyline *lp ;
	struct tty              tty ;
	extern struct flags     Flg ;
	extern struct info      Info ;
	extern int              Flkmem ;

	if ( Flg.flg_y )
		printf( "Ty   Dev       Addr Rawq Canq Outq  Pgrp\n" ) ;
	lp = Info.i_ttyline ;
# ifdef CHAOS
	while ( lp->l_name[0] && lp->l_name[0] != 'C' )
# else
	while ( lp->l_name[0] )
# endif
	{
		memseek( Flkmem, (long)lp->l_addr ) ;
		if ( read( Flkmem, (char*)&tty, sizeof( struct tty ) )
		!= sizeof( struct tty ) )
		{
			fprintf( stderr,
				"sps - Can't read struct tty for tty%.2s\n",
				lp->l_name ) ;
			lp->l_pgrp = 0 ;
			lp++ ;
			continue ;
		}
		lp->l_pgrp = tty.t_pgrp ;
		prtty( lp, &tty ) ;
		lp++ ;
	}
# ifdef CHAOS
	chaosttys( lp ) ;               
# endif
}

/* PRTTY - Print out the tty structure */
prtty ( lp, tty )

register struct ttyline         *lp ;
register struct tty             *tty ;

{
	extern struct flags     Flg ;

	if ( !Flg.flg_y )
		return ;
	printf( "%-2.2s %2d,%2d 0x%08x %4d %4d %4d %5d\n",
		lp->l_name,
		major( lp->l_dev ),
		minor( lp->l_dev ),
		lp->l_addr,
		tty->t_rawq.c_cc,
		tty->t_canq.c_cc,
		tty->t_outq.c_cc,
		tty->t_pgrp ) ;
}

# ifdef CHAOS

/* CHAOSTTYS - Finds ttys attached to the Chaos net */
chaosttys ( lp )

register struct ttyline         *lp ;

{
	register struct connection      **cnp ;
	register int                    i ;
	struct tty                      tty ;
	struct connection               *conntab[CHNCONNS] ;
	struct connection               conn ;
	extern struct info              Info ;
	extern int                      Flkmem ;

	memseek( Flkmem, (long)Info.i_Chconntab ) ;
	(void)read( Flkmem, (char*)conntab, sizeof( conntab ) ) ;
	for ( i = 0, cnp = conntab ; cnp < &conntab[CHNCONNS] ; i++, cnp++ )
	{
		if ( !*cnp )
			continue ;
		memseek( Flkmem, (long)*cnp ) ;
		(void)read( Flkmem, (char*)&conn, sizeof( struct connection ) );
		if ( !(conn.cn_flags & CHTTY) )
			continue ;
		memseek( Flkmem, (long)conn.cn_ttyp ) ;
		(void)read( Flkmem, (char*)&tty, sizeof( struct tty ) ) ;
		if ( lp >= &Info.i_ttyline[MAXTTYS] )
			prexit( "sps - Too many chaos ttys\n" ) ;
		lp->l_addr = conn.cn_ttyp ;
		lp->l_pgrp = tty.t_pgrp ;
		lp->l_dev = tty.t_dev ;
		lp->l_name[0] = 'C' ;
		lp->l_name[1] = i < 10 ? '0'+i : i-10 <= 'z'-'a' ? i-10+'a' :
				i-10-('z'-'a')+'A' ;
		prtty( lp, &tty ) ;
		lp++ ;
	}
}

# endif
