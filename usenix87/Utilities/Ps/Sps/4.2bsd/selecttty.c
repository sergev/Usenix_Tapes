# include       "sps.h"
# include       "flags.h"

/* SELECTTTY - Decides whether this process is interesting for its tty */
selecttty ( p )

register struct process         *p ;

{
	register union flaglist *fp ;
	extern struct flags     Flg ;

	for ( fp = Flg.flg_Tlist ; fp->f_ttyline ; fp++ )
		if ( fp->f_ttyline == p->pr_tty )
			return ( 1 ) ;
	return ( 0 ) ;
}
