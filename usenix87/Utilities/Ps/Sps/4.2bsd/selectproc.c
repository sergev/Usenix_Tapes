# include       "sps.h"
# include       "flags.h"

/*
** SELECTPROC - Given a process structure, this procedure decides whether
** the process is a candidate for printing.
*/
selectproc ( p, process, thisuid )

register struct process         *p ;            
register struct process         *process ;      
int                             thisuid ;       

{
	register union flaglist *fp ;
	register struct process *pp ;
	extern struct flags     Flg ;

	/* Flg.flg_AZ is an internal flag set if one of flags `A' to `Z'
	   was specified. If this is not set, a process is listed only
	   if it or one of its ancestors belongs to the invoking user. */
	if ( !Flg.flg_AZ )
		for ( pp = p ; pp > &process[1] ; pp = pp->pr_pptr )
			if ( thisuid == pp->pr_p.p_uid )
				return ( 1 ) ;
	if ( Flg.flg_A )
		return ( 1 ) ;
	if ( Flg.flg_P )
		for ( fp = Flg.flg_Plist ; fp->f_pid >= 0 ; fp++ )
			if ( fp->f_pid == p->pr_p.p_pid )
				return ( 1 ) ;
	if ( Flg.flg_U )
		for ( pp = p ; pp > &process[1] ; pp = pp->pr_pptr )
			for ( fp = Flg.flg_Ulist ; fp->f_uid >= 0 ; fp++ )
				if ( fp->f_uid == pp->pr_p.p_uid )
					return ( 1 ) ;
	switch ( p->pr_p.p_stat )
	{
		case SRUN :
			if ( Flg.flg_B )
				return ( 1 ) ;
			break ;
		case SSLEEP :
			if ( Flg.flg_B
			&&   p->pr_p.p_pri < PZERO && p->pr_p.p_pid > MSPID )
				return ( 1 ) ;
		case SWAIT :
		case SIDL :
			if ( Flg.flg_W )
				return ( 1 ) ;
			break ;
		case SSTOP :
			if ( Flg.flg_S )
				return ( 1 ) ;
			break ;
		case SZOMB :
			if ( Flg.flg_Z )
				return ( 1 ) ;
			break ;
		default :
			break ;
	}
	return ( 0 ) ;
}
