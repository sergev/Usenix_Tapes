# include       "sps.h"
# include       "flags.h"

/* PRCMD - Prints the command arguments according to the switches */
prcmd ( p, lpad, width )

register struct process         *p ;
int                             lpad ;
int                             width ;

{
	extern struct flags     Flg ;
	extern unsigned         Termwidth ;

	printf( "%*d ", lpad, p->pr_p.p_pid ) ;
	if ( Flg.flg_f )
	{
		printf( "%5d ", p->pr_p.p_ppid ) ;
		width -= 6 ;
	}
	if ( Flg.flg_g )
	{
		printf( "%5d ", p->pr_p.p_pgrp ) ;
		width -= 6 ;
	}
	width += Termwidth ;
	if ( Flg.flg_w )
		printf( "%s\n", p->pr_cmd ) ;
	else if ( width > 0 )
		printf( "%-.*s\n", width, p->pr_cmd ) ;
	if ( p->pr_csaved )
		free( p->pr_cmd ) ;
}
