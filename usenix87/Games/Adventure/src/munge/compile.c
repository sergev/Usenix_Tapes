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

int compile (key)
  int key ;
{
	register int where, i ;
	int opcode, opnum ;

	clrcode () ;
	while ( chkmaj () == OK )
	{
		if ( gettok (token,MAXLINE) != OK )
			break ;
		if ( findop (token,&opcode,&opnum) == ERROR )
		{
			synerr ("Compile","%s -- unrecognized opcode.",token) ;
			return ;
		}
		if ( opnum < 0 )	/* infinite operand opcode */
		{
			while ( gettok (token,MAXLINE) == OK )
			{
				where = eval(token) ;
				appcode (opcode) ;
				appcode (where) ;
			}
		}
		else	/* zero, one, two, or three operand opcode */
		{
			appcode (opcode) ;
			for ( i = 1 ; i <= opnum ; i++ )
			{
				if ( gettok (token,MAXLINE) != OK )
				{
					synerr("Compile","missing operand(s)") ;
					return ;
				}
				where = eval(token) ;
				appcode (where) ;
			}
			if ( gettok (token,MAXLINE) != NEWLINE )
			{
				synerr ("Compile","%s ignored.",token) ;
				return ;
			}
		}
	}
	outcode (key) ;
	return ;
}

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

int findop (tok,opc,cls)
  char *tok ;
  int *opc, *cls ;
{
	register int i, high, low ;
	int m ;

	*opc = -1 ;
	*cls = 0 ;
	low = 0 ;
	high = MAXOPS-1 ;

	for ( i = (high+low)/2 ; low <= high ; i = (high+low)/2 )
	{
		if ( ( m = strncmp (tok,ops[i],MATCHOPS) ) == 0 )
		{
			*opc = i ;
			*cls = class [i] ;
			return (OK) ;
		}

		if ( m > 0 )
			low = i+1 ;
		else
			high = i-1 ;
	}
	return (ERROR) ;
}

#define isalnum(c)	(((c)>='a'&&(c)<='z')|| \
			 ((c)>='A'&&(c)<='Z')|| \
			 ((c)>='0'&&(c)<='9'))

int eval (s)
  register char *s ;
{
	register int val ;
	register char *t ;
	int aflag, sign ;
	char *os, nc ;
	int value ;

	if ( isalnum(*s) && seval (s,&value,0) == OK )
		return (value) ;

	aflag = 0 ;
	sign = 0 ;
	os = s ;

	for ( val = 0 ; *s != EOS ; val += value )
	{
		switch (*s)
		{
			case ATSIGN:
				aflag ++ ;
				s++ ;
				break ;
			case PLUS:
				sign = 1 ;
				s++ ;
				break ;
			case MINUS:
				sign = -1 ;
				s++ ;
				break ;
		}

		for ( t = s ; *s != EOS && *s != PLUS && *s != MINUS ; s++ )
			;

		nc = *s ;
		*s = EOS ;

		if ( seval (t,&value,aflag) != OK )
		{
			Ungetc (NEWLINE) ;
			synerr ("Eval","Unrecognized expression `%s'.",os) ;
			return (ERROR) ;
		}

		*s = nc ;
		if ( sign < 0 )
			value = -value ;
	}
	return (val) ;
}

int seval (s,val,aflag)
  int *val, aflag ;
  char *s ;
{
	register struct symstr *p ;
	int value ;
	extern struct symstr *lookup () ;

	if ( ( p = lookup (s) ) != NIL )
	{
		if ( aflag )
			*val = p->s_aux ;
		else
			*val = p->s_val ;
		return (OK) ;
	}

	if ( ctoi (s,&value) == OK )
	{
		*val = value ;
		return (OK) ;
	}

	return (ERROR) ;
}

#define isdigit(c)	((c)>='0'&&(c)<='9')

int ctoi (s,v)
  register char *s ;
  register int *v ;
{
	register int val ;
	int sign ;

	sign = 1 ;
	if ( *s == PLUS )
		s++ ;
	else
		if ( *s == MINUS )
		{
			s++ ;
			sign = -1 ;
		}
	for ( *v = val = 0 ; *s != EOS ; s++ )
	{
		if ( isdigit(*s) )
			val = val * 10 + ( *s - '0' ) ;
		else
			return (ERROR) ;
	}
	*v = sign * val ;
	return (OK) ;
}
