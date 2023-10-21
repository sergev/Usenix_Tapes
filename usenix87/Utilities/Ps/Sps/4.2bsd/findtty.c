# include       "sps.h"
# include       <h/tty.h>

/* FINDTTY - Attempts to determine to which tty a process is connected */
struct ttyline  *findtty ( p )

register struct process         *p ;

{
	register struct ttyline *lp ;
	extern struct info      Info ;
	extern struct ttyline   Notty ;
	extern union userstate  User ;

	if ( !p->pr_p.p_pgrp )
		return ( &Notty ) ;
	for ( lp = Info.i_ttyline ; lp->l_name[0] ; lp++ )
		if ( lp->l_dev == User.u_us.u_ttyd )
			return ( lp ) ;
	return ( &Notty ) ;
}
