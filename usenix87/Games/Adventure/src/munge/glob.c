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

int inunit = STDIN ;

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

char list = NO ;

short int ninit	 = 0 ;
short int nrep	 = 0 ;
short int nvars	 = 0 ;
short int nobj	 = 0 ;
short int nplace = 0 ;

char state [STATES] =
{
	RELEASE,
	RELEASE,
	KEEP,
	KEEP,
	RELEASE,
	RELEASE,
	RELEASE,
	RELEASE,
	KEEP,
	KEEP,
	KEEP,
	RELEASE,
	KEEP,
	RELEASE,
	RELEASE,
	RELEASE,
	RELEASE,
	RELEASE,
	RELEASE,
	RELEASE,
	RELEASE,
	RELEASE,
	RELEASE,
	RELEASE,
	RELEASE
} ;

short int class [MAXOPS] =
{
	2, 0, -1,  2, -1, 2, 2, 2,  1, 1,
	1, 2,  2,  1,  0, 0, 0, 0,  2, 2,
	0, 1,  1, -1,  1, 2, 2, 2,  1, 1,
	2, 2,  2,  1,  0, 1, 1, 1, -1, 2,
	2, 2,  2,  2, -1, 0, 0, 0,  1, 0,
	2, 1,  2,  3,  0, 2, 2, 2
} ;

char token [MAXLINE] ;
int dbunit = -1 ;
