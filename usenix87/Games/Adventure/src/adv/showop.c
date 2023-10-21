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

static char *ops [MAXOPS] =
{
	"ADD",		"AND",		"ANYOF",	"APPORT",	
	"AT",		"BIC",		"BIS",		"BIT",	
	"CALL",		"CHANCE",	"DEFAULT",	"DEPOSIT",	
	"DIVIDE",	"DROP",		"ELSE",		"EOF",	
	"EOI",		"EOR",		"EVAL",		"EXEC",	
	"FIN",		"GET",		"GOTO",		"HAVE",	
	"IFAT",		"IFEQ",		"IFGE",		"IFGT",	
	"IFHAVE",	"IFKEY",	"IFLE",		"IFLOC",	
	"IFLT",		"IFNEAR",	"INPUT",	"ITLIST",	
	"ITOBJ",	"ITPLACE",	"KEYWORD",	"LDA",	
	"LOCATE",	"MOVE",		"MULT",		"NAME",	
	"NEAR",		"NOT",		"OR",		"PROCEED",	
	"QUERY",	"QUIT",		"RANDOM",	"SAY",	
	"SET",		"SMOVE",	"STOP",		"SUB",	
	"SVAR",		"VALUE"
} ;

int CurKey = -1 ;

int showop (active,op,args)
  int active, op ;
  short int args[3] ;
{
	register int n, i ;

	if ( active )
		printf ("%5d: ",CurKey) ;
	else
		printf ("<skip> ") ;

	if ( op < 0 || op >= 58 )
	{
		printf ("bad opcode (%d)\n",op) ;
		return ;
	}

	printf ("%-7.7s",ops[op]) ;

	n = opnum (op) ;

	for ( i = 0 ; i < n ; i++ )
		printf (" %5d",args[i]) ;
	
	printf ("\n") ;
	return ;
}
