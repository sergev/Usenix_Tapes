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

#include "mdefs.h"

main (argc,argv)

  int argc ;
  char *argv[] ;

{
	if ( argc > 2 )
	{
		printf ("Usage: %s [commandfile]\n",argv[0]) ;
		exit (1) ;
	}
	if ( argc == 2 )
	{
		if ( ( inunit = openf (argv[1]) ) < 0 )
		{
			perror (argv[1]) ;
			exit (1) ;
		}
	}
	dbunit = makedb ("adv") ;
	parse () ;
	wstab () ;
	iodone () ;
	exit (0) ;
}

int makedb (dbname)
  char *dbname ;
{
	register int unit ;

	unit = creatk (dbname) ;
	if ( unit == ERROR )
		error ("Makedb","unable to create database %s",dbname) ;
	return (unit) ;
}

int iodone ()
{
	closek (dbunit) ;
	return ;
}

