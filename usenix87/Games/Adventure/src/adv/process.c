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

#ifdef SHOWOP
	extern int CurKey ;
#endif SHOWOP

int process (key)
  int key ;
{
	int rec, calls, dos, endb ;
	int srec[MAXCALLS], sbp[MAXCALLS], sdos[MAXCALLS] ;
	int dovar[MAXDOS], dopnt[MAXDOS], domod[MAXDOS] ;
	short int instr, op[3] ;
	register int bp, i, j ;
	int n ;

	rec = key ;
	calls = 0 ;
	dos = 0 ;

	forever
	{
		bp = 0 ;
		while ( rec == ERROR || ( endb = rdcode (rec) ) == ERROR )
		{
			if ( calls <= 0 )
				return ;
			rec = srec [calls] ;
			bp = sbp [calls] ;
			dos = sdos [calls] ;
			calls-- ;
		}

#ifdef SHOWOP
		CurKey = rec ;
#endif SHOWOP
		while ( bp < endb )
		{
			instr = codebuf [bp] ;
			if ( logical (instr) )
			{
				bp = condition (bp,codebuf,endb) ;
				continue ;
			}
			bp++ ;
			n = opnum (instr) ;
			for ( i = 0 ; i < n ; i++, bp++ )
				op [i] = codebuf [bp] ;

#ifdef SHOWOP
			showop (YES,instr,op) ;
#endif SHOWOP
			switch (instr)
			{
			  case ADD:

				setval (op[0],(eval(op[0])+eval(op[1]))) ;
				continue ;

			  case ANYOF:

				bp -= 2 ;
				for ( j = 0 ; codebuf [bp] == ANYOF ; bp+=2 )
					for ( i = 0 ; i < LINELEN ; i++ )
					    if ( codebuf [bp+1] == linewd [i] )
							j++ ;
				if ( j > 0 )
					continue ;
				break ;

			  case APPORT:

				movobj ( ref (op[0]), ref (op[1]) ) ;
				continue ;

			  case AT:

				bp -= 2 ;
				i = eval (here) ;
				for ( j = 0 ; codebuf [bp] == AT ; bp += 2 )
					if ( codebuf [bp+1] == i )
						j++ ;
				if ( j > 0 )
					continue ;
				break ;

			  case BIC:

				setbit (op[0],bitval(op[0])&(~bitval(op[1]))) ;
				continue ;

			  case BIS:

				setbit (op[0],bitval(op[0])|bitval(op[1])) ;
				continue ;

			  case CALL:

				if ( (++calls) >= MAXCALLS )
				   error ("Process","call stack overflow (%d)!",
					   calls) ;
				srec [calls] = rec ;
				sbp  [calls] = bp ;
				sdos [calls] = dos ;
				rec = ref ( op[0] ) ;
				j = class ( rec ) ;
				if ( j == OBJECT )
					rec += (MAXOTEXT * MAXOBJECTS) ;
				else
					if ( j == PLACE )
						rec += ( 2 * MAXPLACES ) ;
				break ;

			  case DEFAULT:

				if ( linlen != 1 )
					continue ;
				j = 0 ;
				for ( i = OBJKEY ; indx(i) < nobj ; i++ )
				{
					if ( near (i) == NO ||
				     	(bitval(i)&bitval(op[0])) == 0 )
						continue ;
					if ( j != 0 )
					{
						j = 0 ;
						break ;
					}
					j = i ;
				}
				if ( j == 0 )
					continue ;
				setval (argwd[1],j) ;
				setbit (argwd[1],bitval(j)) ;
				linewd[1] = j ;
				setval (status,2) ;
				linlen = 2 ;
				continue ;

			  case DEPOSIT:

				setval (ref(op[0]),eval(op[1])) ;
				continue ;

			  case DIVIDE:

				setval (op[0],(eval(op[0])/eval(op[1]))) ;
				continue ;

			  case DROP:

				movobj ( ref (op[0]), eval (here) ) ;
				continue ;

			  case ELSE:

				bp = flushc (bp,codebuf,endb) ;
				continue ;

			  case EOF:

				continue ;

			  case EOI:

				i = eval ( dovar[dos] ) ;
				switch ( domod[dos] )
				{
					/* only iterate with nearby objects */
					case ITLIST:

						do
						{
							if ( indx(++i) >= nobj )
							{
								dos-- ;
								break ;
							}
						}
						while ( near (i) != YES ) ;
						if ( indx(i) >= nobj )
							continue ;
						break ;

					case ITOBJ :

						if ( indx(++i) >= nobj )
						{
							dos-- ;
							continue ;
						}
						break ;

					case ITPLACE :

						if ( indx(++i) >= nplace )
						{
							dos-- ;
							continue ;
						}
						break ;

					default :
						error ("Process",
							"unknown IT* type!") ;
						break ;

				}
				setval (dovar[dos],i) ;
				bp = dopnt[dos] ;
				continue ;

			  case EVAL:

				setval (op[0],eval(ref(op[1]))) ;
				continue ;

			  case EXEC:

				executive (op[0],op[1]) ;
				continue ;

			  case FIN:

				continue ;

			  case GET:

				j = ref ( op[0] ) ;
				movobj (j,INHAND) ;
				i = eval (j) ;
				if ( i < 0 )
					setval (j,-1-i) ;
				continue ;

			  case GOTO:

				/* must do this so "back" will work */
				i = eval (here) ;
				j = bitval (here) ;

				setval (here,ref(op[0])) ;
				setbit (here,bitval(ref(op[0]))) ;
				setval (there,i) ;
				setbit (there,j) ;
				setbit (status,(bitval(status)|MOVED)) ;
				continue ;

			  case HAVE:

				if ( where ( ref (op[0]) ) == INHAND )
					continue ;
				break ;

			  case INPUT:

				command () ;
				continue ;

			  case ITLIST:

				dos++ ;
				dovar[dos] = op[0] ;
				/* do first object and all others nearby */
				setval (op[0],OBJKEY) ;
				dopnt[dos] = bp ;
				domod[dos] = ITLIST ;
				continue ;

			  case ITOBJ:

				dos++ ;
				dovar[dos] = op[0] ;
				setval (op[0],OBJKEY) ;
				dopnt[dos] = bp ;
				domod[dos] = ITOBJ ;
				continue ;

			  case ITPLACE:

				dos++ ;
				dovar[dos] = op[0] ;
				setval (op[0],PLACEKEY) ;
				dopnt[dos] = bp ;
				domod[dos] = ITPLACE ;
				continue ;

			  case KEYWORD:

				for ( i = 0 ; i < LINELEN ; i++ )
					if ( op[0] == linewd[i] )
						break ;
				if ( i < LINELEN )
					continue ;
				break ;

			  case LDA:

				setval (op[0],op[1]) ;
				continue ;

			  case LOCATE:

				setval (op[0],where(ref(op[1]))) ;
				continue ;

			  case MOVE:

				if ( op[0] != linewd[0] )
				{
					if ( linlen < 2 )
						continue ;
					if ( op[0] != linewd[1] )
						continue ;
				}
				i = eval (here) ;
				j = bitval (here) ;
				setval (here,ref(op[1])) ;
				setbit (here,bitval(ref(op[1]))) ;
				setval (there,i) ;
				setbit (there,j) ;
				setbit (status,(bitval(status)|MOVED)) ;
				return ;

			  case MULT:

				setval (op[0],(eval(op[0])*eval(op[1]))) ;
				continue ;

			  case NAME:

				for ( i = 0 ; i < LINELEN ; i++ )
					if ( op[1] == argwd[i] )
					{
						saynam (ref(op[0]),op[1]) ;
						break ;
					}
				if ( i >= LINELEN )
					saynam (ref(op[0]),ref(op[1])) ;
				continue ;

			  case NEAR:

				i = ref ( op[0] ) ;
				if ( where (i) == INHAND || near (i) == YES )
					continue ;
				break ;

			  case PROCEED:

				break ;

			  case QUIT:

				return ;

			  case RANDOM:

				setval (op[0],rnd(eval(op[1]))) ;
				continue ;

			  case SAY:

				say ( ref (op[0]) ) ;
				continue ;

			  case SET:

				setval (op[0],eval(op[1])) ;
				continue ;

			  case SMOVE:

				if ( op[0] != linewd[0] )
				{
					if ( linlen < 2 )
						continue ;
					if ( op[0] != linewd[1] )
						continue ;
				}
				i = eval (here) ;
				j = bitval (here) ;
				setval (here,ref(op[1])) ;
				setbit (here,bitval(ref(op[1]))) ;
				setval (there,i) ;
				setbit (there,j) ;
				setbit (status,(bitval(status)|MOVED)) ;
				say (op[2]) ;
				return ;

			  case STOP:

				fini () ;

			  case SUB:

				setval (op[0],(eval(op[0])-eval(op[1]))) ;
				continue ;

			  case SVAR:

				i = eval (op[1]) ;
				i = svar (eval(op[0]),i) ;
				setval (op[1],i) ;
				continue ;

			  case VALUE:

				sayval (ref(op[0]),ref(op[1])) ;
				continue ;

			  default:

				error ("Process","unrecognized opcode %d!",
					instr) ;
				break ;

			}
			break ;
		}

		if ( instr != CALL )
			rec = nextrec (rec) ;
	}
}

int nextrec (key)
  register int key ;
{
	register int n ;

	switch (class(key))
	{
		case INITIAL:
			if ( key < REPEAT )
			{
				if ( ++key >= ninit )
					key = ERROR ;
			}
			else
			{
				if ( ++key >= nrep+REPEAT )
					key = ERROR ;
			}
			break ;

		case LABEL:
			key = ERROR ;
			break ;

		case VERB:
			key += MAXVERBS ;
			n = key - VERBKEY ;
			n /= MAXVERBS ;
			if ( n >= MAXVCODE )
				key = ERROR ;
			break ;

		case PLACE:
			key += MAXPLACES ;
			n = key - PLACEKEY ;
			n /= MAXPLACES ;
			n -= 2 ;
			if ( n >= MAXPCODE )
				key = ERROR ;
			break ;
		
		case OBJECT:
			key += MAXOBJECTS ;
			n = key - OBJKEY ;
			n /= MAXOBJECTS ;
			n -= MAXOTEXT ;
			if ( n >= MAXOCODE )
				key = ERROR ;
			break ;
		
		default:
			error ("NextRey","bad key %d!",key) ;
			break ;
	}
	return (key) ;
}

int fini ()
{
	closek (dbunit) ;
	exit (0) ;
}
