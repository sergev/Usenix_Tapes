# include       "sps.h"
# include       "flags.h"
# include       <h/tty.h>

/*
** FLAGSETUP - Replaces any users or processes specified by flagdecode()
** with numerical equivalents. The lists are terminated by negative values.
** or null pointers. Ttystatus() must have been previously called to
** initialise the Info structure with chaos tty values.
*/
flagsetup ()
{
	register union flaglist *fp ;
	register char           *chp ;
	register int            i ;
	register struct ttyline *lp ;
	int                     found ;
	extern struct flags     Flg ;
	extern struct info      Info ;

	/* Look for specified users */
	if ( Flg.flg_U )                
	{
		if ( !Flg.flg_Ulist->f_chp )
			prexit( "sps - User name was expected after -u flag\n");
		for ( fp = Flg.flg_Ulist ; chp = fp->f_chp ; fp++ )
		{
			found = 0 ;
			for ( i = 0 ; i < MAXUSERID ; i++ )
				if ( !strncmp( chp, Info.i_hnames[i].h_uname,
					UNAMELEN ) )
				{
					fp->f_uid = Info.i_hnames[i].h_uid ;
					found = 1 ;
					break ;
				}
			if ( !found )
				prexit( "sps - Unknown user: %s\n", chp ) ;
		}
		fp->f_uid = -1 ;
	}
	/* Look for specified process ids */
	if ( Flg.flg_P )                
	{
		if ( !Flg.flg_Plist->f_chp )
			prexit(
			     "sps - Process id was expected after -p flag\n" ) ;
		for ( fp = Flg.flg_Plist ; chp = fp->f_chp ; fp++ )
		{
			if ( chp[0] < '0' || chp[0] > '9' )
				prexit( "sps - Bad process id: %s\n", chp ) ;
			fp->f_pid = atoi( chp ) ;
		}
		fp->f_pid = -1 ;
	}
	/* Look for specified ttys */
	if ( !Flg.flg_T )               
		return ;
	if ( !Flg.flg_Tlist->f_chp )
		prexit( "sps - Tty name was expected after -t flag\n" ) ;
	for ( fp = Flg.flg_Tlist ; chp = fp->f_chp ; fp++ )
	{       /* Under VMUNIX, all ttys have two character names.
		   Thus, a flag of the form `t 8' should be expanded to
		   become `t 08'. */
		if ( !chp[1] )
			chp[1] = chp[0], chp[0] = '0' ;
		found = 0 ;
		for ( lp = Info.i_ttyline ; lp->l_name[0] ; lp++ )
			if ( !strncmp( chp, lp->l_name, 2 ) )
			{
				fp->f_ttyline = lp ;
				found = 1 ;
				break ;
			}
		if ( !found )
			prexit( "sps - Unknown tty name: %.2s\n", chp ) ;
	}
	fp->f_ttyline = (struct ttyline*)0 ;
}
