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

#include "kio.h"

struct iblk Iblk ;

short int Sblk[MAXIBLK] ;

char knam[20], rnam[20] ;

int Modified = 0 ;
int CurBlk = -1 ;
long int RecLoc = 0L ;
short int IndLoc = 0 ;
int kfd = -1 ;
int rfd = -1 ;
