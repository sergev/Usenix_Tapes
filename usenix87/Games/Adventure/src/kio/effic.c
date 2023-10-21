/*
**		    Copyright (c) 1985	Ken Wellsch
**
**     Permission is hereby granted to all users to possess, use, copy,
**     distribute, and modify the programs and files in this package
**     provided it is not for direct commercial benefit and secondly,
**     that this notice and all modification information be kept and
**     maintained in the package.
**
*/

#include "kio.h"

main (argc,argv)
int argc ;
char *argv[] ;
{
	int fd ;
	register int i, j, free ;
	int used ;

	if ( argc != 2 )
	{
		printf ("Usage: %s file\n",argv[0]) ;
		exit (1) ;
	}

	if ( ( fd = openk (argv[1]) ) == ERROR )
	{
		printf ("Unable to open %s\n",argv[1]) ;
		exit (1) ;
	}
	
	for ( free = i = 0 ; i < MAXIBLK ; i++ )
		if ( Sblk[i] == EMPTY )
			free ++ ;
	
	printf ("Key file %s statistics:\n\n",argv[1]) ;

	printf ("\tFree slots in super block: %d (%f%%)\n",free,
		((float)free*100.0/MAXIBLK)) ;

	for ( used = free = i = 0 ; i < MAXIBLK ; i++ )
	{
		if ( Sblk[i] != EMPTY )
		{
			if ( GetBlk (i) != i )
			{
				printf ("can't read block %d!\n",Sblk[i]) ;
				exit (1) ;
			}
			for ( j = 0 ; j < MAXENTRIES ; j++ )
			{
				if ( Iblk.i_siz[j] == 0 )
					free ++ ;
				used++ ;
			}
		}
	}

	printf ("\tTotal record entries available: %d\n",used) ;

	printf ("\tTotal unused record entries: %d (%f%%)\n",free,
		((float)free*100.0/used)) ;
	
	closek (fd) ;
	exit (0) ;
}

error (rnam,a1,a2,a3,a4,a5)
char *rnam, *a1, *a2, *a3, *a4, *a5 ;
{
	printf ("%s: ",rnam) ;
	printf (a1,a2,a3,a4,a5) ;
	exit (1) ;
}
