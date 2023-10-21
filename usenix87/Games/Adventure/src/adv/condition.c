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

int condition (bp,cbuf,len)
  int bp, len ;
  short int cbuf[] ;
{
	register int i, cond, instr ;
	int op[3], lmode ;
	int n, negate, passon ;

	negate = 0 ;
	passon = 0 ;
	lmode  = 0 ;

	forever
	{
		instr = cbuf [bp++] ;
		n = opnum (instr) ;
		for ( i = 0 ; i < n ; i++, bp++ )
			op[i] = cbuf [bp] ;

		if ( bp > len )
			error ("Condition","bad opcode %d!",instr) ;

#ifdef SHOWOP
		showop (YES,instr,op) ;
#endif SHOWOP

		if ( instr == NOT )
		{
			negate =  !negate ;
			continue ;
		}

		switch (instr)
		{
			case BIT:
				cond = ( (bitval(op[0]) & bitval(op[1])) != 0 );
				break ;

			case CHANCE:
				cond = ( eval (op[0]) > rnd (100) ) ;
				break ;

			case IFAT:
				cond = ( ref (op[0]) == eval (here) ) ;
				break ;

			case IFEQ:
				cond = ( eval (op[0]) == eval (op[1]) ) ;
				break ;

			case IFGE:
				cond = ( eval (op[0]) >= eval (op[1]) ) ;
				break ;

			case IFGT:
				cond = ( eval (op[0]) > eval (op[1]) ) ;
				break ;

			case IFHAVE:
				cond = ( where ( ref (op[0]) )  == INHAND ) ;
				break ;

			case IFKEY:
				cond = 0 ;
				for ( i = 0 ; i < linlen ; i++ )
					if ( (cond = (op[0] == linewd[i])) )
						break ;
				break ;

			case IFLE:
				cond = ( eval (op[0]) <= eval (op[1]) ) ;
				break ;

			case IFLOC:
				cond = ( where ( ref(op[0]) ) == ref (op[1]) ) ;
				break ;

			case IFLT:
				cond = ( eval (op[0]) < eval (op[1]) ) ;
				break ;

			case IFNEAR:
				cond = ( where ( ref (op[0]) ) == INHAND ||
			 		near ( ref (op[0]) ) == YES ) ;
				break ;

			case QUERY:
				cond = (query ( ref (op[0]) ) == YES) ;
				break ;

			default:
				error ("Condition","bad opcode %d!",instr) ;
				break ;
		}

		cond = ((cond||negate)&&(!(cond&&negate))) ;

		switch ( lmode )
		{
			case OR :
				cond = ( cond || passon ) ;
				break ;

			case AND :
				cond = ( cond && passon ) ;
				break ;

			case EOR :
				cond = ((cond||passon)&&(!(cond&&passon))) ;
				break ;
		}

		switch ( cbuf[bp] )
		{
			case OR :
				lmode = OR ;
				break ;

			case AND :
				lmode = AND ;
				break ;

			case EOR :
				lmode = EOR ;
				break ;

			default :
				if ( cond )
					return (bp) ;
				return (flushc (bp,cbuf,len)) ;
		}

		bp++ ;
		passon = cond ;
		negate = 0 ;
	}
}

int flushc (bp,cbuf,len)
  register int bp ;
  int len ;
  short int cbuf[] ;
{
	register int instr, level ;
	int new, n ;

	for ( level = 1, new = 1 ; level > 0 ; )
	{
		if ( bp >= len )
			error ("Flushc","nonterminating condition!") ;

		instr = cbuf[bp++] ;

#ifdef SHOWOP
		showop (NO,instr,&(cbuf[bp])) ;
#endif SHOWOP

		if ( instr == NOT )
			continue ;

		bp += opnum (instr) ;

		if ( logical (instr) )
		{
			if ( new )
				level++ ;
			n = cbuf[bp] ;
			if ( n == OR || n == AND || n == EOR )
			{
				new = 0 ;
				bp++ ;
			}
			else
				new = 1 ;
			continue ;
		}

		if ( instr == ELSE && level <= 1 )
			break ;
		if ( instr == FIN )
			level-- ;
		else
			if ( instr == EOF )
				break ;
	}
	return (bp) ;
}

int query (op)
  short int op ;
{
	register int c, d ;

	forever
	{
		say (op) ;
		printf ("? ") ;
		c = getchar () ;
		while ( (d=getchar()) != '\n' )
			if ( d == EOF )
				break ;
		if ( c == 'y' || c == 'Y' )
			return (YES) ;
		if ( c == 'n' || c == 'N' )
			return (NO) ;
	}
}
