# include       "sps.h"
# include       "flags.h"
# include       <h/text.h>
# include       <stdio.h>

/*
** NEEDED - Determine which processes are needed for the printout
** and add these to a list of needed processes.
*/
struct process  *needed ( process, text )

register struct process         *process ;
struct text                     *text ;

{
	register struct process *p ;
	register struct process *plist ;
	struct process          *lastp ;
	int                     uid ;
	extern struct flags     Flg ;
	extern union userstate  User ;
	extern struct info      Info ;
	extern struct ttyline   Notty ;
	struct ttyline          *findtty() ;
	char                    *getcmd() ;

	plist = (struct process*)0 ;
	lastp = &process[ Info.i_nproc ] ;
	/* Normalise internal pointers from kernel addresses. For each kmem
	   address in the `proc' and `text' structures, we convert that
	   address for our own internal use. */
	for ( p = process ; p < lastp ; p++ )
	{                               
		if ( !p->pr_p.p_stat )  
			continue ;
		/* Normalise internal text pointers */
		if ( p->pr_p.p_textp )
			p->pr_p.p_textp = &text[p->pr_p.p_textp - Info.i_text0];
		/* Normalise internal linked list of processes */
		p->pr_plink = p->pr_p.p_link ?
			&process[ p->pr_p.p_link  - Info.i_proc0 ] :
			(struct process*)0 ;
		/* Normalise internal parent pointers */
		p->pr_pptr = p->pr_p.p_pptr ?
			&process[ p->pr_p.p_pptr - Info.i_proc0 ] :
			(struct process*)0 ;
		/* Check for valid parent pointers */
		if ( !p->pr_pptr )
		{
			p->pr_pptr = process ;
			continue ;
		}
		if ( p->pr_pptr < process || p->pr_pptr >= lastp )
		{
			fprintf( stderr, "sps - process %d has bad pptr\n",
				p->pr_p.p_pid ) ;
			p->pr_pptr = process ;
		}
	}
	/* For each process, see if it is a candidate for selection.
	   If so, retrieve its command arguments and upage information. */
	uid = getuid() ;
	for ( p = process ; p < lastp ; p++ )
	{                               
		if ( !p->pr_p.p_stat )
			continue ;
		/* Count processes and sizes */
		summarise( p ) ;
		/* Select the given processes. Bear in mind that selection
		   of processes based on the `F' and `T' flags must be
		   postponed until the upage is accessed. */
		if ( !Flg.flg_F && !Flg.flg_T && !selectproc( p, process, uid ))
			continue ;
		/* Try to find the process' command arguments. Accessing the
		   arguments also involves retrieving the upage. */
		p->pr_cmd = getcmd( p ) ;
		/* If the upage was found successfully, use this information */
		if ( p->pr_upag )       
		{
# ifdef BSD42
			p->pr_rself = User.u_us.u_ru ;
			p->pr_rchild = User.u_us.u_cru ;
# else
			p->pr_vself = User.u_us.u_vm ;
			p->pr_vchild = User.u_us.u_cvm ;
# endif
			p->pr_tty = findtty( p ) ;
			p->pr_files = filecount() ;
		}
		else
			p->pr_tty = &Notty ;
		/* Select on the basis of the `F' and `T' flags */
		if ( Flg.flg_F          
		&& !(p->pr_p.p_pgrp && p->pr_p.p_pgrp == p->pr_tty->l_pgrp) )
			continue ;
		if ( Flg.flg_T && !selecttty( p ) )
			continue ;
		/* Arrive here with a selected process. Add this to the
		   linked list of needed processes. */
		p->pr_plink = plist ;   
		plist = p ;
		p->pr_child = (struct process*)0 ;
		p->pr_sibling = (struct process*)0 ;
	}
	return ( plist ) ;
}

/* SUMMARISE - Summarises the given process into the `Summary' structure */
/*
** SHOULD ACCOUNT HERE FOR THE SIZE OF LOADED PAGE TABLES, BUT WE DON'T REALLY
** KNOW THEIR RESIDENT SIZES.
*/
summarise ( p )

register struct process         *p ;

{
	register struct text    *tp ;
	int                     busy ;
	extern struct summary   Summary ;

	Summary.sm_ntotal++ ;
	if ( p->pr_p.p_stat == SZOMB )
		return ;
	/* Firstly, account for processes */
	Summary.sm_ktotal += p->pr_p.p_dsize + p->pr_p.p_ssize ;
	Summary.sm_kloaded += p->pr_p.p_rssize ;
	Summary.sm_kswapped += p->pr_p.p_swrss ;
	if ( p->pr_p.p_flag & SLOAD )
		Summary.sm_nloaded++ ;
	else
		Summary.sm_nswapped++ ;
	busy = (p->pr_p.p_stat == SRUN) || (p->pr_p.p_stat==SSLEEP
	     && (p->pr_p.p_pri<PZERO && p->pr_p.p_pid > MSPID) ) ;
	if ( busy )
	{
		Summary.sm_nbusy++ ;
		Summary.sm_kbusy += p->pr_p.p_dsize + p->pr_p.p_ssize ;
	}
	/* Now account for their texts */
	if ( !(tp = p->pr_p.p_textp) || !tp->x_count )
		return ;                
	Summary.sm_ktotal += tp->x_size ;
	Summary.sm_kloaded += tp->x_rssize ;
	Summary.sm_kswapped += tp->x_swrss ;
	if ( busy )
		Summary.sm_kbusy += tp->x_size ;
	tp->x_count = 0 ;
}
