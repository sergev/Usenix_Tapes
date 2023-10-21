# include       "sps.h"
# include       <h/tty.h>
# include       <h/text.h>
# ifdef SUN
# include       <h/vnode.h>
# include       <ufs/inode.h>
# else
# include       <h/inode.h>
# endif
# include       <h/buf.h>
# ifdef BSD42
# include       <h/quota.h>
# include       <h/mbuf.h>
# include       <h/socket.h>
# include       <h/socketvar.h>
# endif

/* 1 if `w' is in the address range defined by `a1' and `a2' ... */
# define        INRANGE( w, a1, a2 ) \
			( (caddr_t)(a1) <= (w) && (w) < (caddr_t)(a2) )

/* WAITINGFOR - Determine what a process is waiting for and describe it. */
char    *waitingfor ( p )

struct process                  *p ;

{
	register caddr_t        w ;
	register struct ttyline *lp ;
	register struct symbol  *s ;
	register char           *cp ;
# ifdef BSD42
	struct socket           sc ;
# endif
	static char             wbuf[ 8 ] ;
	extern struct info      Info ;
	extern struct symbol    Symbollist[] ;
	char                    *sprintf() ;

	w = p->pr_p.p_wchan ;
	if ( !w )
		return ( "null" ) ;
	/* Waiting for a child process, alternatively in a vfork() ? */
	if ( INRANGE( w, Info.i_proc0, &Info.i_proc0[ Info.i_nproc ] ) )
		return ( p->pr_p.p_flag & SNOVM ? "vfork" : "child" ) ;
	/* Waiting for a page to be brought in ? */
	if ( INRANGE( w, Info.i_swbuf0, &Info.i_swbuf0[ Info.i_nswbuf ] ) )
		return ( "swap" ) ;
	/* Waiting for discio through a block device to complete ? */
	if ( INRANGE( w, Info.i_buf0, &Info.i_buf0[ Info.i_nbuf ] ) )
		/* SHOULD ACTUALLY READ AS "blkio" BUT "discio" IS WHAT
		   IS GENERALLY MEANT HERE. */
		return ( "discio" ) ;
	/* Waiting for a text page to be brought in ? */
	if ( INRANGE( w, Info.i_text0, &Info.i_text0[ Info.i_ntext ] ) )
		return ( "swtext" ) ;
# ifdef BSD42
	/* Waiting for an event associated with the quota system ? */
	if ( INRANGE( w, Info.i_quota0, &Info.i_quota0[ Info.i_nquota ] ) )
		return ( "quota" ) ;
# endif
	/* Waiting for tty I/O ? If so, find which tty it is */
	for ( lp = Info.i_ttyline ; lp->l_name[0] ; lp++ )
		if ( INRANGE( w, &lp->l_addr[0], &lp->l_addr[1] ) )
		{
			switch ( w - (int)lp->l_addr )
			{
				case (int)&((struct tty*)0)->t_rawq :
					/* Read from a tty or slave pty */
					cp = "rtty??" ;
					break ;
				case (int)&((struct tty*)0)->t_outq :
					/* Write to a tty or slave pty */
					cp = "wtty??" ;
					break ;
				case (int)&((struct tty*)0)->t_state :
					/* Tty not open */
					cp = "otty??" ;
					break ;
				case (int)&((struct tty*)0)->t_outq.c_cf :
					/* Read from a controller pty */
					cp = "rpty??" ;
					break ;
				case (int)&((struct tty*)0)->t_rawq.c_cf :
					/* Write to a controller pty */
					cp = "wpty??" ;
					break ;
				default :
					cp = "?tty??" ;
					break ;
			}
			cp[4] = lp->l_name[0] ;
			cp[5] = lp->l_name[1] ;
			return ( cp ) ;
		}
	/* Waiting for an inode ? */
	if ( INRANGE( w, Info.i_inode0, &Info.i_inode0[ Info.i_ninode ] ) )
		switch ( ((int)w - (int)Info.i_inode0) % sizeof( struct inode ))
		{
# ifdef BSD42
# ifndef SUN
			case (int)&((struct inode*)0)->i_exlockc :
				/* Exclusive lock on this inode */
				return ( "exlock" ) ;
			case (int)&((struct inode*)0)->i_shlockc :
				/* Shared lock on this inode */
				return ( "shlock" ) ;
# endif
# else
			case 1 :
				return ( "wpipe" ) ;
			case 2 :
				return ( "rpipe" ) ;
			case (int)&((struct inode*)0)->i_un.i_group.g_datq :
				return ( "rmux" ) ;
# endif
			default :
				/* Inode probably locked */
				return ( "inode" ) ;
		}
# ifdef BSD42
	/* Waiting for a structure inside an mbuf ? If so, try to find why */
	if ( INRANGE( w, Info.i_mbutl,
	&Info.i_mbutl[ NMBCLUSTERS * CLBYTES / sizeof( struct mbuf ) ] ) )
		switch ( ((int)w - (int)Info.i_mbutl) % sizeof( struct mbuf )
			- (int)&((struct mbuf*)0)->m_dat[0] )
		{
			case (int)&((struct socket*)0)->so_timeo :
				/* Socket timeout event */
				return ( "socket" ) ;
			case (int)&((struct socket*)0)->so_rcv.sb_cc :
				/* Read from an empty socket. Here we actually
				   attempt to determine whether the socket
				   structure in question really does refer to
				   a socket, or whether it is in fact a pipe
				   in disguise. */
				return ( getsocket( (struct socket*)(w
				    - (int)&((struct socket*)0)->so_rcv.sb_cc),
						&sc )
					&& sc.so_type == SOCK_STREAM
					&& !sc.so_rcv.sb_hiwat
					&& !sc.so_rcv.sb_mbmax
					&& (sc.so_state
					    & (SS_ISCONNECTED|SS_CANTRCVMORE))
					? "rpipe" : "rsockt" ) ;
			case (int)&((struct socket*)0)->so_snd.sb_cc :
				/* Write to a full socket. Again, we try
				   to determine whether or not this is a
				   real socket or a pipe. */
				return ( getsocket( (struct socket*)(w
				    - (int)&((struct socket*)0)->so_snd.sb_cc),
						&sc )
					&& sc.so_type == SOCK_STREAM
					&& sc.so_rcv.sb_hiwat == 2048
					&& sc.so_rcv.sb_mbmax == 4096
					&& (sc.so_state
					    & (SS_ISCONNECTED|SS_CANTSENDMORE))
					? "wpipe" : "wsockt" ) ;
			default :
				/* Other mbuf event */
				return ( "mbuf" ) ;
		}
# endif
	/* Look in the symbol table for known wait addresses. */
	for ( s = Symbollist ; s->s_kname ; s++ )
		if ( s->s_wait && w == *s->s_info )
			return ( s->s_wait ) ;  
	/* No reason for the wait state has been found.
	   Return the wait channel as a hexadecimal address. */
# ifdef SUN
	(void)sprintf( wbuf, "x%05x", w ) ;
# else
	(void)sprintf( wbuf, "x%05x", w - 0x80000000 ) ;
# endif
	return ( wbuf ) ;
}

# ifdef BSD42
/*
** GETSOCKET - Reads a `struct socket' from the kernel virtual memory address
** identified by `ks' into the buffer `s'.
*/
getsocket ( ks, s )

struct socket                   *ks ;
struct socket                   *s ;

{
	extern int              Flkmem ;

	memseek( Flkmem, (long)ks ) ;
	return ( read( Flkmem, (char*)s, sizeof( struct socket ) )
		== sizeof( struct socket ) ) ;
}
# endif
