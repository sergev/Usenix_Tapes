	/* the utmp/wtmp structure.  used by login and init tomaintain	*/
	/* /etc/utmp (and /usr/adm/wtmp) so we can tell who's logged in	*/

	struct utmpbuf
	{
		char	utmp_name[8];	/* user's login name		*/
		char	utmp_tty	/* where user is logged in	*/
			,utmp_null	/* unused			*/
			;
		long	utmp_logtime	/* when user logged in		*/
			;
		int	utmp_spare ;	/* unused			*/
	}	;
