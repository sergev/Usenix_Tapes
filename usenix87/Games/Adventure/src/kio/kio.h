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

#include <sys/file.h>

#define MAXENTRIES	16
#define MAXIBLK		1024
#define EMPTY		-1
#define ERROR		-1

#define EXISTING	0
#define NEW		1

struct mkey
{
	int b ;
	int e ;
} ;

struct iblk
{
	long int i_loc[MAXENTRIES] ;
	short int i_siz[MAXENTRIES] ;
} ;

extern struct iblk Iblk ;
extern short int Sblk[MAXIBLK] ;
extern char knam[20], rnam[20] ;
extern int Modified ;
extern int CurBlk ;
extern long int RecLoc ;
extern short int IndLoc ;
extern int kfd ;
extern int rfd ;
