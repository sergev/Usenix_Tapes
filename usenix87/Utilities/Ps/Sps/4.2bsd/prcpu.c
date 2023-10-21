# include       "sps.h"

# ifdef BSD42

/* PRCPU - Print cpu time */
prcpu ( time, utime )

register time_t                 time ;
time_t                          utime ;

{
	time += utime / 1000000 ;
	utime %= 1000000 ;
	if ( time < 0L )
	{       /* Ignore negative times */
		printf( "     " ) ;     
		return ;
	}
	if ( time < 60L*10L )
	{       /* Print as seconds if less than 1000 seconds */
		printf( "%3d.%1d", (int)time, (int)utime/100000 ) ;
		return ;
	}
	/* Print as minutes if less than 10 hours ; print as hours if less than
	   10 days, else print as days. */
	if ( time < 60L*60L*10L )               
		printf( "%3D M", time/60L ) ;
	else if ( time < 24L*60L*60L*10L )
		printf( "%3D H", time/60L/60L ) ;
	else
		printf( "%3D D", time/60L/60L/24L ) ;
}

# else

/* PRCPU - Print cpu time */
prcpu ( time )

register time_t                 time ;

{
	extern struct info      Info ;

	if ( time < 0L )
	{       /* Ignore negative times */
		printf( "     " ) ;     
		return ;
	}
	if ( time < Info.i_hz*60L*10L )
	{       /* Less than 10 minutes */
		printf( "%3D.%1D", time/Info.i_hz,
			(time % Info.i_hz / (Info.i_hz/10L)) ) ;
		return ;
	}
	/* If less than 10 hours, print as minutes */
	time /= Info.i_hz ;
	/* Print as minutes if less than 10 hours ; print as hours if less than
	   10 days, else print as days. */
	if ( time < 60L*60L*10L )               
		printf( "%3D M", time/60L ) ;
	else if ( time < 24L*60L*60L*10L )
		printf( "%3D H", time/60L/60L ) ;
	else
		printf( "%3D D", time/60L/60L/24L ) ;
}

# endif
