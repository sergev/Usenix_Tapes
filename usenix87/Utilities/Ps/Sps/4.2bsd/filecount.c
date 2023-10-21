# include       "sps.h"

/* FILECOUNT - Counts the # open files for the current process */
filecount ()
{
	register int            i ;
	register struct file    **f ;
	register int            count ;
	extern union userstate  User ;

	count = 0 ;
	for ( i = 0, f = User.u_us.u_ofile ; i < NOFILE ; i++ )
		if ( *f++ )
			count++ ;
	return ( count ) ;
}
