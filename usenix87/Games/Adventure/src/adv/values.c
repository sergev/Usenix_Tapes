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

static int powof2[16] =
{
	000001, 000002, 000004, 000010,
	000020, 000040, 000100, 000200,
	000400, 001000, 002000, 004000,
	010000, 020000, 040000, 0100000
} ;

int bitval (key)
  register int key ;
{
	register int val ;

	switch (class(key))
	{
		case INIT:
			if ( key < 0 || key > 15 )
				error ("Bitval","bad key %d!",key) ;
			val = powof2[key] ;
			break ;

		case OBJECT:
			val = objbit[indx(key)] ;
			break ;

		case VARIABLE:
			val = varval[indx(key)] ; /* single indirection */
			switch (class(val))
			{
				case OBJECT:
					val = objbit[indx(val)] ;
					break ;

				case PLACE:
					val = plcbit[indx(val)] ;
					break ;
				
				default:
					val = varbit[indx(key)] ;
					break ;
			}
			break ;
		
		case PLACE:
			val = plcbit[indx(key)] ;
			break ;

		default:
			error ("Bitval","bad key %d!",key) ;
			break ;
	}
	return (val) ;
}

int setbit (key,bits)
  int key, bits ;
{
	register int val ;

	switch (class(key))
	{
		case OBJECT:
			objbit[indx(key)] = bits ;
			break ;

		case VARIABLE:
			varbit[indx(key)] = bits ;
			val = varval[indx(key)] ;
			switch (class(val))
			{
				case OBJECT:
					objbit[indx(val)] = bits ;
					break ;

				case PLACE:
					plcbit[indx(val)] = bits ;
					break ;
			}
			break ;
		
		case PLACE:
			plcbit[indx(key)] = bits ;
			break ;

		default:
			error ("SetBit","bad key %d!",key) ;
			break ;
	}
	return ;
}

int setval (key,val)
  int key, val ;
{
	switch (class(key))
	{
		case OBJECT:
			objval[indx(key)] = val ;
			break ;

		case VARIABLE:
			varval[indx(key)] = val ;
			break ;

		default:
			error ("Setval","bad key %d!",key) ;
			break ;
	}
	return ;
}
