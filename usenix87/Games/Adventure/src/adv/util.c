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

#ifndef class

	int class (key)
	  int key ;
	{
		if ( key < 0 || key > MAXKEY )
			return (clss[0]) ;
		return (clss[key/1000]) ;
	}

#endif class

int eval (key)
  int key ;
{
	register int val ;

	switch (class(key))
	{
		case INIT:
			val = key ;
			break ;

		case OBJECT:
			val = objval[indx(key)] ;
			break ;

		case VARIABLE:
			val = varval[indx(key)] ;
			break ;

		default:
			error ("Eval","bad key %d!",key) ;
			break ;
	}
	return (val) ;
}

#ifndef indx

	int indx (key)
	  int key ;
	{
		register int i, m ;

		i = key % 1000 ;

		switch (class(key))
		{
			case OBJECT:
				m = OBJECTS ;
				break ;

			case PLACE:
				m = PLACES ;
				break ;

			case VARIABLE:
				m = VARS ;
				break ;

			default:
				error ("Indx","bad key %d!",key) ;
				break ;
		}
		if ( i < 0 || i >= m )
			error ("Indx","bad index %d for key %d!",i,key) ;
		return (i) ;
	}

#endif indx

#ifndef logical

	int logical (instr)
	  short int instr ;
	{
		if ( instr < 0 || instr >= MAXOPS )
			error ("Logical","bad opcode %d!",instr) ;
		return (ltab[instr]) ;
	}

#endif logical

int movobj (key,loc)
  int key, loc ;
{
	if ( class(key) != OBJECT )
		error ("Movobj","not an object %d!",key) ;
	objloc[indx(key)] = loc ;
	return ;
}

int near (key)
  int key ;
{
	register int w, b, h ;

	w = where (key) ;
	b = bitval (key) ;
	h = eval(here) ;

	if ( w == h || ( w == (h-1) && (b&DUAL) ) )
		return (YES) ;
	return (NO) ;
}

#ifndef opnum

	int opnum (opcode)
	  int opcode ;
	{
		if ( opcode < 0 || opcode >= MAXOPS )
			error ("Opnum","bad opcode %d",opcode) ;

		return (opn[opcode]) ;
	}

#endif opnum

int ref (key)
  int key ;
{
	if ( class(key) == VARIABLE )
		return (varval[indx(key)]) ;
	return (key) ;
}

#ifndef where

	int where (key)
	  int key ;
	{
		if ( class(key) != OBJECT )
			error ("Where","not object key %d!",key) ;
		return (objloc[indx(key)]) ;
	}

#endif where
