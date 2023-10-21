/* pathalias -- by steve bellovin, as told to peter honeyman */
#ifndef lint
static char	*sccsid = "@(#)extern.c	8.1 (down!honey) 86/01/19";
#endif lint

#include "def.h"

int	Home;
char	*Cfile;
char	**Ifiles;
char	*ProgName;
int	fdnode, fdlink;		/* File descriptors for temporary files */
int	nodecount, linkcount;
int	Vflag;
int	Cflag;
int	Iflag;
int	Tflag;
int	Lineno = 1;
char	*Netchars = "!:@%";	/* sparse, but sufficient */
int	Ncount;
int	Lcount;
char	*Graphout;
char	*Linkout;
int	*Table;			/* hash table + priority queue */
long	Tabsize;		/* size of Table */	
int	Private;		/* list of private nodes in this file */
long	Hashpart;		/* used while mapping -- above is heap */
int	Scanstate = NEWLINE;	/* scanner (yylex) state */
