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

char opn [MAXOPS] =
{
	2, 0, 1, 2, 1, 2, 2, 2, 1, 1,
	1, 2, 2, 1, 0, 0, 0, 0, 2, 2,
	0, 1, 1, 1, 1, 2, 2, 2, 1, 1,
	2, 2, 2, 1, 0, 1, 1, 1, 1, 2,
	2, 2, 2, 2, 1, 0, 0, 0, 1, 0,
	2, 1, 2, 3, 0, 2, 2, 2	
} ;

char ltab[MAXOPS] =
{
	0,0,0,0,0,0,0,1,0,1,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,1,1,1,1,
	1,1,1,1,1,1,0,0,0,0,0,0,0,0,
	0,0,0,1,0,0,1,0,0,0,0,0,0,0,
	0,0
} ;

struct symstr *symtab [TABSIZ] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
} ;

char clss [] =
{
	INITIAL,
	LABEL,
	VERB,
	VERB,
	PLACE,
	PLACE,
	PLACE,
	TEXT,
	OBJECT,
	OBJECT,
	OBJECT,
	VARIABLE,
	NULL,
	0
} ;

char token [MAXLINE] ;

int linlen = 0 ;
int linewd [LINELEN] ;

char lex[LEXLEN][LEXSIZ] ;

short int objval[OBJECTS] ;
short int objbit[OBJECTS] ;
short int objloc[OBJECTS] ;

short int plcbit[PLACES] ;

short int varval[VARS] ;
short int varbit[VARS] ;

short int codebuf[BUFSIZE] ;
char textbuf[TBUFSIZE] ;

int nrep, ninit, nvars, nobj, nplace ;
int here, there, status, argwd[LINELEN] ;

int dbunit = -1 ;
