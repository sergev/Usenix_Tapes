# include       "sps.h"
# include       "flags.h"
# include       <h/text.h>

/* PRINTPROC - Pretty print a process according to the switches. */
printproc ( p, md )

register struct process         *p ;            
int                             md ;            

{
	register char           *chp ;
	register struct text    *tp ;
	register struct hashtab *hp ;
	char                    chbuf[10] ;
	time_t                  time ;
	time_t                  chtime ;
# ifdef BSD42
	time_t                  utime ;
	time_t                  uchtime ;
# endif
	extern short            Lastuid, Lastpgrp ;
	extern struct flags     Flg ;
	char                    *waitingfor() ;
	struct hashtab          *hashuid() ;
	double                  percentmem() ;

	/* List tty name and foreground/background/detached information */
	printf( "%2.2s%c", p->pr_tty->l_name,
		!p->pr_p.p_pgrp ? ' ' :
# ifdef SDETACH
		p->pr_p.p_flag & SDETACH ? '_' :
# endif
		p->pr_p.p_pgrp == p->pr_tty->l_pgrp ? '.' : ' ' ) ;
	hp = hashuid( p->pr_p.p_uid ) ;
	if ( !md  )                             
	{       /* If a top-level process, list the user name */
		if ( hp )
			printf( "%-8.8s ", hp->h_uname ) ;
		else
			printf( "user%-4.4d ", p->pr_p.p_uid ) ;
	}
	else                                    
	{       /* Usually list an asterisk for a child process */
		md = md > 8 ? 8 : md ;
		printf( "%*s%c", md, "",
			p->pr_p.p_pgrp == Lastpgrp ? '|' : '*' ) ;      
		/* But beware of setuid processes */
		md = 8 - md ;
		if ( p->pr_p.p_uid == Lastuid )
			printf( "%-*.*s", md, md, "" ) ;
		else if ( hp )
			printf( "%-*.*s", md, md, hp->h_uname ) ;
		else
		{
			md -= 4 ;
			printf( "user%-*.*d", md, md, p->pr_p.p_uid ) ;
		}
	}
	Lastuid = p->pr_p.p_uid ;
	Lastpgrp = p->pr_p.p_pgrp ;
	if ( Flg.flg_d )                        
	{       /* List disc I/O and paging information */
		if ( !p->pr_upag || p->pr_p.p_stat == SZOMB )
		{
			prcmd( p, 49, -63 ) ;
			return ;
		}
		printf( "%2d %8d+%8d %4d %8d %8D ",
			p->pr_files,
# ifdef BSD42
			p->pr_rself.ru_majflt,
			p->pr_rself.ru_minflt,
			p->pr_rself.ru_nswap,
			p->pr_rself.ru_inblock + p->pr_rself.ru_oublock,
			KBYTES( p->pr_rself.ru_idrss + p->pr_rself.ru_isrss
				+ p->pr_rself.ru_ixrss ) ) ;
# else
			p->pr_vself.vm_majflt,
			p->pr_vself.vm_minflt,
			p->pr_vself.vm_nswap,
			p->pr_vself.vm_inblk + p->pr_vself.vm_oublk,
			KBYTES( (p->pr_vself.vm_idsrss
				+ p->pr_vself.vm_ixrss) / Info.i_hz ) ) ;
# endif
		prcmd( p, 5, -63 ) ;
		return ;
	}
	if ( !Flg.flg_v )                       
	{       /* Not verbose so just list command arguments */
		prcmd( p, 5, -19 ) ;
		return ;
	}
	/* Arrive here if being verbose ; list cpu information */
	switch ( p->pr_p.p_stat )               
	{                                       
		case SSLEEP :
		case SWAIT :
		case SIDL :
			/* Determine why a process should be in a wait state */
			chp = waitingfor( p ) ;
			break ;
		case SRUN :
			chp = "run" ;
			break ;
		case SZOMB :
			chp = "exit" ;
			break ;
		case SSTOP :
			chp = "stop" ;
			break ;
	}
	/* If the process is loaded, list the status information in capitals */
	printf( "%-6.6s ", p->pr_p.p_flag & SLOAD ?
		(capitals( chp, chbuf ), chbuf) : chp ) ;
	/* List process flags */
	printf( "%c%c%c", p->pr_p.p_flag & SSYS ? 'U' :
		p->pr_p.p_flag & STRC ? 'T' : ' ',
		p->pr_p.p_flag & SVFORK ? 'V' :
		p->pr_p.p_flag & SPHYSIO ? 'I' : ' ',
		p->pr_p.p_flag & SUANOM ? 'A' : ' ' ) ;
	/* List process niceness */
	if ( p->pr_p.p_nice != NZERO )          
		printf( "%3d ", p->pr_p.p_nice - NZERO ) ;
	else
		printf( "    " ) ;
	if ( p->pr_p.p_stat == SZOMB )
	{
		prcmd( p, 41, -69 ) ;
		return ;
	}                                       
	/* List process and text virtual sizes */
	printf( "%4d", KBYTES( p->pr_p.p_dsize + p->pr_p.p_ssize ) ) ;
	if ( tp = p->pr_p.p_textp )
		printf( "+%3d ", KBYTES( tp->x_size ) ) ;
	else
		printf( "     " ) ;
	/* List process and text real sizes */
	printf( "%4d", KBYTES( p->pr_p.p_rssize ) ) ;
	if ( tp )
		printf( "+%3d %2.0f ", KBYTES( tp->x_rssize ),
			percentmem( p ) );
	else
		printf( "        " ) ;
	/* List information obtained from the upage. This includes the process
	   times and command arguments. */
	if ( !p->pr_upag )
	{
		prcmd( p, 20, -69 ) ;
		return ;
	}                                       
	/* List process time information */
# ifdef BSD42
	time   = Flg.flg_q ? p->pr_rself.ru_utime.tv_sec :
		 p->pr_rself.ru_utime.tv_sec + p->pr_rself.ru_stime.tv_sec ;
	utime  = Flg.flg_q ? p->pr_rself.ru_utime.tv_usec :
		 p->pr_rself.ru_utime.tv_usec + p->pr_rself.ru_stime.tv_usec ;
	chtime = Flg.flg_q ? p->pr_rchild.ru_utime.tv_sec :
		 p->pr_rchild.ru_utime.tv_sec + p->pr_rchild.ru_stime.tv_sec ;
	uchtime = Flg.flg_q ? p->pr_rchild.ru_utime.tv_usec :
		 p->pr_rchild.ru_utime.tv_usec + p->pr_rchild.ru_stime.tv_usec ;
	prcpu( time, utime ) ;
	if ( chtime != 0L )
	{
		printf( "+" ) ;
		prcpu( chtime, uchtime ) ;
	}
# else
	time   = Flg.flg_q ? p->pr_vself.vm_utime :
		 p->pr_vself.vm_utime + p->pr_vself.vm_stime ;
	chtime = Flg.flg_q ? p->pr_vchild.vm_utime :
		 p->pr_vchild.vm_utime + p->pr_vchild.vm_stime ;
	prcpu( time ) ;
	if ( chtime != 0L )
	{
		printf( "+" ) ;
		prcpu( chtime ) ;
	}
# endif
	else
		printf( "      " ) ;
# ifdef BSD42
	if ( time || utime )
# else
	if ( time )
# endif
# ifdef SUN
		printf( " %2.0f ", (double)p->pr_p.p_pctcpu ) ;
# else
		printf( " %2.0f ", p->pr_p.p_pctcpu * 100.0 ) ;
# endif
	else
		printf( "    " ) ;
	/* Finally, list the process command arguments. */
	prcmd( p, 5, -69 ) ;                    
}

/* CAPITALS - Converts letters in `chp' to upper-case in buffer `buf'. */
capitals ( chp, buf )

register char                   *chp ;
register char                   *buf ;

{
	while ( *buf = *chp++ )
	{
		if ( 'a' <= *buf && *buf <= 'z' )
			*buf -= 'a' - 'A' ;
		buf++ ;
	}
}
