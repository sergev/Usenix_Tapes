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

#define SMEMSIZ		450

static struct symstr smem[SMEMSIZ] ;
static int psmem = SMEMSIZ ;

int define (nam,val)
  char *nam ;
  int val ;
{
	register struct symstr *p ;
	register int h ;
	extern char *strsav () ;

	/* assume the symbol does not exist */

	if ( psmem <= 0 )
		error ("Symbol Define","symbol table full on `%s'",nam) ;

	p = &(smem[--psmem]) ;

	p->s_nam = strsav (nam) ;
	p->s_val = val ;

	h = hash (nam) ;
	p->s_nxt = symtab[h] ;
	symtab[h] = p ;

	return ;
}

int find (name)
  char *name ;
{
	register struct symstr *p ;

	for ( p = symtab[hash(name)] ; p != NIL ; p = p->s_nxt )
	{
		if ( strncmp (name,p->s_nam,MATCH) == 0 )
		{
			return (p->s_val) ;
		}
	}
	return (ERROR) ;
}

int hash (s)
  register char *s ;
{
	register int h, i ;

	for ( i = h = 0 ; *s != EOS && i < MATCH ; i++ )
		h += *s++ ;
	
	return ( ( h % TABSIZ ) ) ;
}

#define CMEMSIZ		2600

static char cmem[CMEMSIZ] ;
static int  pcmem = 0 ;

char *strsav (s)
  char *s ;
{
	register char *p ;
	register int n ;

	n = strlen(s) + 1 ;
	if ( (pcmem+n) >= CMEMSIZ )
		error ("Strsave","Out of symbol space on `%-10.10s'\n",s) ;

	p = &(cmem[pcmem]) ;
	pcmem += n ;
	(void) strcpy (p,s) ;
	return (p) ;
}

#ifdef PSTAB

int pstab ()
{
	register int i, j ;
	register struct symstr *p ;

	for ( i = j = 0 ; i < TABSIZ ; i++ )
	{
		for ( p = symtab[i] ; p != NIL ; p = p->s_nxt )
		{
			printf ("   %-6.6s  %5d",p->s_nam,p->s_val) ;
			if ( (++j)%4 == 0 )
				printf ("\n") ;
		}
	}
	printf ("\n\n") ;

	printf ("Total number of symbols in symbol table is %d\n",j) ; 
	printf ("Total symbol structures still available is %d\n",psmem) ;
	printf ("Total character storage space used is %d\n",pcmem) ;

	return ;
}

#endif PSTAB
