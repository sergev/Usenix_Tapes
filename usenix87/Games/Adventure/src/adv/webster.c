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

#include "adefs.h"

int webster ()
{
	register int key, i, j ;
	int totsym, size, val ;

	key = SYMTABREC ;
	if ( ( size = readk (dbunit,key,codebuf,BUFSIZE*2) ) == ERROR )
		error ("Webster","unable to access symbol table (%d)!",key) ;
	if ( size != sizeof(short int) )
		error ("Webster","bad symbol table size record (%d)!",size) ;
	
	for ( totsym = codebuf[0] ; totsym > 0 ; )
	{
		key++ ;
		if ( ( size = readk (dbunit,key,codebuf,BUFSIZE*2) ) < 1 )
			error ("Webster","bad sym table count (%d)!",totsym) ;
		size /= sizeof(short int) ;

		for ( i = 0 ; i < size ; )
		{
			for ( j = 0 ; codebuf[i] != EOS ; j++, i++ )
				token[j] = codebuf[i] ;
			token[j] = token[6] = EOS ;
			val = codebuf[i+1] ;
			i += 2 ;
			define (token,val) ;
			totsym-- ;
		}
	}

	nrep = find ("<NREP>") ;
	ninit = find ("<NINIT>") ;
	nvars = find ("<NVARS>") ;
	nobj = find ("<NOBJ>") ;
	nplace = find ("<NPLACE>") ;
	here = find ("HERE") ;
	there = find ("THERE") ;
	status = find ("STATUS") ;
	argwd[0] = find ("ARG1") ;
	argwd[1] = find ("ARG2") ;

	if ( nrep < 1 )
		error ("Webster","missing repeat code (%d)!",nrep) ;
	if ( nobj > OBJECTS )
		error ("Webster","too many objects (%d>%d)!",nobj,OBJECTS) ;
	if ( nplace > PLACES )
		error ("Webster","too many places (%d>%d)!",nplace,PLACES) ;
	if ( nvars > VARS )
		error ("Webster","too many variables (%d>%d)!",nvars,VARS) ;
	if ( class(here) != VARIABLE || class(there) != VARIABLE ||
	     class(argwd[0]) != VARIABLE || class(argwd[1]) != VARIABLE ||
	     class(status) != VARIABLE )
		error ("Webster","missing special variable!") ;

	return ;
}
