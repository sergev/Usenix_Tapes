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

#include <stdio.h>

/*VARARGS1*/
int error (rnam,a1,a2,a3,a4,a5,a6,a7,a8,a9)
char *rnam, *a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8, *a9 ;
{
	fprintf (stderr,"%s: ",rnam) ;
	fprintf (stderr,a1,a2,a3,a4,a5,a6,a7,a8,a9) ;
	fprintf (stderr,"\n") ;
	exit (1) ;
}
